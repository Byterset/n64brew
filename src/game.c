#include <assert.h>
#include <math.h>

#ifndef __N64__
#include <stdio.h>
#endif

#ifdef __N64__
#include "mathdef.h"
#else
#include "float.h"
#endif

#include <stdlib.h>

#include "util/memory.h"
// game
#include "actors/gardener/character.h"
#include "constants.h"
#include "game.h"
#include "gameutils.h"
#include "item.h"
#include "player.h"
#include "util/trace.h"
#include "math/vector3.h"
#include "controls/controller.h"
#include "constants.h"

#define GENERATE_DEBUG_BODIES 0

static Game game;

void Game_initGameObjectPhysBody(PhysBody *body, GameObject *obj)
{
	struct Vector3 objCenter;
	Game_getObjCenter(obj, &objCenter);
	PhysBody_init(body, modelTypesProperties[obj->modelType].mass,
				  modelTypesProperties[obj->modelType].radius, &objCenter,
				  obj->id);
	obj->physBody = body;
}

void Game_init(LevelData *worldObjects,
			   int worldObjectsCount,
			   PhysWorldData *physWorldData)
{
	int i, initIndex, itemsCount, physicsBodiesCount, charactersCount;
	GameObject *player_object;
	GameObject *obj;
	// GameObject *catherine;
	Character *characters;
	Item *items;
	PhysBody *physicsBodies;
	GameObject *temp;
	game.worldObjects = (GameObject *)malloc(worldObjectsCount * sizeof(GameObject));

	for (i = 0; i < worldObjectsCount; i++)
	{
		GameObject_init(game.worldObjects + i, worldObjects[i].id, &worldObjects[i].position, &worldObjects[i].rotation);
		game.worldObjects[i].modelType = worldObjects[i].modelType;
		game.worldObjects[i].subtype = worldObjects[i].subtype;
	}

	game.paused = FALSE;
	// game.worldObjects = temp;
	game.worldObjectsCount = worldObjectsCount;

	// init world objects loaded from map data
	for (i = 0, obj = game.worldObjects; i < game.worldObjectsCount; i++, obj++)
	{
		obj->animState = NULL;
		obj->visible = TRUE;
		obj->solid = TRUE;
	}

	// camera
	vector3Init(&game.viewPos, 0.0F, 0.0F, -400.0F);
	vector3Init(&game.viewRot, 0.0F, 0.0F, 0.0F);
	game.viewZoom = 2.0f;
	game.freeView = 0;

	player_object = Game_findObjectByType(GooseModel);
	// catherine = Game_findObjectByType(CatherineModel);
	invariant(player_object != NULL);
	// invariant(catherine != NULL);

	Player_init(&game.player, player_object);
	// Player_init(&game.player, catherine);
	PhysState_init(&game.physicsState, physWorldData);

	// setup camera
	vector3Copy(&game.viewTarget, &game.player.player_object->position);

	// TODO: move these to be statically allocated per map?
	itemsCount = Game_countObjectsInCategory(ItemModelType);
	items = (Item *)malloc(itemsCount * sizeof(Item));
	invariant(items);
	initIndex = 0;
	for (i = 0; i < game.worldObjectsCount; ++i)
	{
		obj = game.worldObjects + i;
		if (modelTypesProperties[obj->modelType].category == ItemModelType)
		{
			invariant(initIndex < itemsCount);
			Item_init(items + initIndex, obj, &game);
			initIndex++;
		}
	}

	charactersCount = Game_countObjectsInCategory(CharacterModelType);
	characters = (Character *)malloc(charactersCount * sizeof(Character));
	invariant(characters);
	initIndex = 0;
	for (i = 0; i < game.worldObjectsCount; ++i)
	{
		obj = game.worldObjects + i;
		if (modelTypesProperties[obj->modelType].category == CharacterModelType)
		{
			invariant(initIndex < charactersCount);
			Character_init(characters + initIndex,
						   Game_findObjectByType(GardenerCharacterModel),
						   /*book*/ &items[0], // TODO: make items owned by character
						   &game);
			initIndex++;
		}
	}

	physicsBodiesCount = 0;
#if GENERATE_DEBUG_BODIES
	for (i = 0; i < NUM_PHYS_BODIES; ++i)
	{
		vector3Init(&pos, RAND(200), RAND(10) + 10, RAND(200));
		PhysBody_init(&physicsBodies[i],
					  /* mass */ 10.0,
					  /* radius */ 20.0, &pos, i);
		physicsBodiesCount++;
	}
#else
	physicsBodiesCount = itemsCount + charactersCount +
						 Game_countObjectsInCategory(PlayerModelType);
	physicsBodies = (PhysBody *)malloc(physicsBodiesCount * sizeof(PhysBody));
	invariant(physicsBodies);
	initIndex = 0;
	for (i = 0; i < game.worldObjectsCount; ++i)
	{
		obj = game.worldObjects + i;
		{
			ModelTypeCategory category =
				modelTypesProperties[obj->modelType].category;
			if (category == ItemModelType || category == CharacterModelType ||
				category == PlayerModelType)
			{
				invariant(initIndex < physicsBodiesCount);
				Game_initGameObjectPhysBody(physicsBodies + initIndex, obj);
				initIndex++;
			}
		}
	}
#endif

	game.items = items;
	game.itemsCount = itemsCount;
	game.characters = characters;
	game.charactersCount = charactersCount;
	game.physicsBodies = physicsBodies;
	game.physicsBodiesCount = physicsBodiesCount;

	game.pathfindingGraph = NULL;
	game.pathfindingState = NULL;

	game.profTimeCharacters = 0;
	game.profTimePhysics = 0;
	game.profTimeDraw = 0;
	game.profTimePath = 0;
}

// TODO: not used yet, but if we have map transitions we might want this
void Game_destroy()
{
	free(game.items);
	free(game.characters);
	free(game.physicsBodies);
}

Game *Game_get()
{
	return &game;
}

GameObject *Game_getObjectByID(int id)
{
	invariant(id < game.worldObjectsCount);
	return game.worldObjects + id;
}

int Game_countObjectsInCategory(ModelTypeCategory category)
{
	int i;
	int count = 0;

	for (i = 0; i < game.worldObjectsCount; i++)
	{
		if (modelTypesProperties[(game.worldObjects + i)->modelType].category ==
			category)
		{
			count++;
		}
	}
	return count;
}

// finds the first object with a particular modeltype
GameObject *Game_findObjectByType(ModelType modelType)
{
	int i;
	GameObject *obj;

	for (i = 0, obj = game.worldObjects; i < game.worldObjectsCount; i++, obj++)
	{
		if (obj->modelType == modelType)
		{
			return obj;
		}
	}
	return NULL;
}
GameObject *Game_findObjectNByType(ModelType modelType, int n)
{
	int i, found;
	GameObject *obj;

	found = 0;

	for (i = 0, obj = game.worldObjects; i < game.worldObjectsCount; i++, obj++)
	{
		if (obj->modelType == modelType)
		{
			if (found < n)
			{
				found++;
			}
			else
			{
				return obj;
			}
		}
	}
	return NULL;
}

void Game_updateCamera(Game *game, Input *input)
{
	float cameraDist;
	struct Vector3 cameraOffset;
	float desiredZoom, desiredZoomDist;

	// spring to desired zoom level
	desiredZoom = input->zoomIn ? 3.0 : input->zoomOut ? 1.2
													   : 2.0;
	// desiredZoom = 1.0; // zoomed out for debugging
	desiredZoomDist = game->viewZoom - desiredZoom;
	game->viewZoom -= desiredZoomDist * 0.1;

	// vector3Set(&cameraOffset, 0.0F, 0.65F, -0.8F);  // grid aligned aerial
	// vector3Set(&cameraOffset, -0.8F, 0.9F, -0.8F);  // 45 to grid aerial
	// vector3Set(&cameraOffset, 0.0F, 0, -0.8F); // side view
	vector3Set(&cameraOffset, -0.4F, 0.5F, -0.6F); // goose game

	vector3NormalizeSelf(&cameraOffset);

	// cameraDist = 1000.0f / game->viewZoom;  // 45deg fov
	cameraDist = 2500.0f / game->viewZoom; // 15deg fov
	vector3ScaleSelf(&cameraOffset, cameraDist);

	vector3Copy(&game->viewPos, &game->player.player_object->position);
	vector3AddToSelf(&game->viewPos, &cameraOffset);

	// look at goose
	vector3Copy(&game->viewTarget, &game->player.player_object->position);
}

void Game_updatePhysics(Game *game)
{
	int i;
	struct Vector3 positionDelta, physUpdatedPosition;
	PhysBody *body;
	GameObject *obj;
	// for now we need to copy the object's pos to the
	// physbody...
	for (i = 0, body = game->physicsBodies; i < game->physicsBodiesCount;
		 ++i, body++)
	{
		obj = Game_getObjectByID(body->id);

		Game_getObjCenter(obj, &positionDelta);
		vector3SubFromSelf(&positionDelta, &body->position);
		PhysBody_translateWithoutForce(body, &positionDelta);
	}

	// simulate physics
	PhysState_step(&game->physicsState, game->physicsBodies,
				   game->physicsBodiesCount, (float)game->tick / 60.0f * 1000.0f);

	// // ...and copy its position back again
	for (i = 0, body = game->physicsBodies; i < game->physicsBodiesCount;
		 ++i, body++)
	{
		obj = Game_getObjectByID(body->id);
		obj->position = body->position;

		vector3Copy(&physUpdatedPosition, &body->position);
		vector3SubFromSelf(&physUpdatedPosition, &modelTypesProperties[obj->modelType].centroidOffset);
		vector3Copy(&obj->position, &physUpdatedPosition);
	}
}

void Game_update(Input *input)
{
	Game *game;
	int i;
	float profStartPhysics, profStartCharacters, profEndPhysics,
		profEndCharacters;

	game = Game_get();

	if (!game->paused)
	{
		game->tick++;

		//   profStartCharacters = CUR_TIME_MS();
		for (i = 0; i < game->charactersCount; ++i)
		{
			Character_update(&game->characters[i], game);
		}
		//   profEndCharacters = CUR_TIME_MS();
		//   Trace_addEvent(CharactersUpdateTraceEvent, profStartCharacters,
		//                  profEndCharacters);
		Player_update(&game->player, input, game);

		//   profStartPhysics = CUR_TIME_MS();
		Game_updatePhysics(game);
		//   profEndPhysics = CUR_TIME_MS();
		//   Trace_addEvent(PhysUpdateTraceEvent, profStartPhysics, profEndPhysics);

		Game_updateCamera(game, input);

		//   // update windowed (eg. 60 frame) aggregations
		//   game->profTimePhysics += profEndPhysics - profStartPhysics;
		//   game->profTimeCharacters += profEndCharacters - profStartCharacters;
	}
}

#ifndef __N64__
#ifdef __cplusplus

std::vector<RaycastTraceEvent> gameRaycastTrace;

void Game_traceRaycast(RaycastTraceEvent event)
{
	if (gameRaycastTrace.size() > 1000)
	{
		printf("!!! gameRaycastTrace overflowed\n");
		gameRaycastTrace.clear();
	}
	gameRaycastTrace.push_back(event);
}
#endif
#endif

float Game_rayIntersectsSphereDist(struct Vector3 *origin,
								   struct Vector3 *rayDirection,
								   struct Vector3 *objCenter,
								   float objRadius)
{
	struct Vector3 l;
	float tca, d2, radius2, thc, t0, t1;
	// l = objCenter - origin;
	vector3Copy(&l, objCenter);
	vector3SubFromSelf(&l, origin);

	tca = vector3Dot(&l, rayDirection);
	d2 = vector3Dot(&l, &l) - tca * tca;
	radius2 = objRadius * objRadius;

	if (d2 > radius2)
		return -1; // no intersection

	thc = sqrtf(radius2 - d2);

	// t0 = first intersect point - entrance on front of sphere
	t0 = tca - thc;

	// t1 = second intersect point - exit point on back of sphere
	t1 = tca + thc;

	// test to see if both t0 and t1 are behind the ray - if so, return null
	if (t0 < 0 && t1 < 0)
		return -1; // no intersection

	// test to see if t0 is behind the ray:
	// if it is, the ray is inside the sphere, so return the second exit point
	// scaled by t1, in order to always return an intersect point that is in front
	// of the ray.
	if (t0 < 0)
		return t1;

	// else t0 is in front of the ray, so return the first collision point scaled
	// by t0
	return t0;
}

int Game_rayIntersectsSphere(struct Vector3 *origin,
							 struct Vector3 *rayDirection,
							 struct Vector3 *objCenter,
							 float objRadius)
{
	float dist =
		Game_rayIntersectsSphereDist(origin, rayDirection, objCenter, objRadius);

	if (dist < 0)
		return FALSE;

	return TRUE;
}

void Game_getObjCenter(GameObject *obj, struct Vector3 *result)
{
	vector3Copy(result, &obj->position);
	vector3AddToSelf(result, &modelTypesProperties[obj->modelType].centroidOffset);
}

float Game_getObjRadius(GameObject *obj)
{
	return modelTypesProperties[obj->modelType].radius;
}

float Game_distanceToGameObject(struct Vector3 *from, GameObject *to)
{
	struct Vector3 toObjCenter;

	Game_getObjCenter(to, &toObjCenter);

	return vector3Dist(from, &toObjCenter) - modelTypesProperties[to->modelType].radius;
}

/*
Finds the closest object intersecting a cast ray
Method: iterate objects and check ray-sphere intersection. When intersecting,
check if intersection distance is less than previously found intersection (if
any).
Crappy code for debug use only
 */
GameObject *Game_getIntersectingObject(struct Vector3 *raySource, struct Vector3 *rayDirection)
{
	int i, hit;
	float closestObjHitDist, intersectDist;
	GameObject *obj;
	GameObject *closestObjHit;
	struct Vector3 objCenter;

	closestObjHitDist = FLT_MAX;
	closestObjHit = NULL;

	for (i = 0, obj = game.worldObjects; i < game.worldObjectsCount; i++, obj++)
	{
		Game_getObjCenter(obj, &objCenter);

		intersectDist = Game_rayIntersectsSphereDist(
			raySource, rayDirection, &objCenter, Game_getObjRadius(obj));
		hit = intersectDist >= 0;

#ifndef __N64__
		// printf("checking id=%d %s: %s\n", obj->id,
		// ModelTypeStrings[obj->modelType],
		//        hit ? "hit" : "miss");
#endif

		if (hit)
		{
			if (intersectDist < closestObjHitDist)
			{
				// printf("is closest\n");
				closestObjHit = obj;
				closestObjHitDist = intersectDist;
			}
			else
			{
				// printf("is NOT closest\n");
			}
		}
	}

	return closestObjHit;
}

/*
method: check if ray from viewer to target intersects any other object's
bounding sphere at a distance less than the distance to the target's
bounding sphere
*/
int Game_canSeeOtherObject(GameObject *viewer,
						   GameObject *target,
						   float viewerEyeOffset,
						   GameObject *occuludingObjects,
						   int occuludingObjectsCount)
{
	struct Vector3 eye, rayDirection, objCenter;
	int i, canSee;
	float targetDistance, objDistance;
	GameObject *obj;

	canSee = TRUE;
	eye = viewer->position;
	eye.y += viewerEyeOffset; // eye offset
	vector3DirectionTo(&eye, &target->position, &rayDirection);

	targetDistance = Game_distanceToGameObject(&eye, target);

	for (obj = occuludingObjects, i = 0; i < occuludingObjectsCount; obj++, i++)
	{
		if (obj->id == target->id || obj->id == viewer->id)
		{
			// the ray will definitely intersect these, but we care about
			// intersections with other things
			continue;
		}

		objDistance = Game_distanceToGameObject(&eye, obj);

		if (objDistance > targetDistance)
		{
			// ignore objects further than target
			continue;
		}

		Game_getObjCenter(obj, &objCenter);

		if (Game_rayIntersectsSphere(&eye, &rayDirection, &objCenter,
									 modelTypesProperties[obj->modelType].radius))
		{
			canSee = FALSE;
			break;
		}
	}

#ifdef __cplusplus

	Game_traceRaycast({/*int result;*/ canSee,
					   /*struct Vector3 origin;*/ eye,
					   /*struct Vector3 direction;*/ rayDirection,
					   /*GameObject* hit;*/ canSee ? NULL : obj});
#endif
	return canSee;
}
