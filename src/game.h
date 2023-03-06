
#ifndef _GAME_H_
#define _GAME_H_

#include "gameobject.h"
#include "gametypes.h"
#include "controls/input.h"
#include "physics/physics.h"

void Game_init(GameObject *worldObjects,
			   int worldObjectsCount,
			   PhysWorldData *physWorldData);
Game *Game_get();

GameObject *Game_findObjectByType(ModelType modelType);
GameObject *Game_findObjectNByType(ModelType modelType, int n);

int Game_countObjectsInCategory(ModelTypeCategory category);
GameObject *Game_getIntersectingObject(struct Vector3 *raySource, struct Vector3 *rayDirection);

float Game_getObjRadius(GameObject *obj);

int Game_rayIntersectsSphere(struct Vector3 *origin,
							 struct Vector3 *rayDirection,
							 struct Vector3 *objCenter,
							 float objRadius);

int Game_canSeeOtherObject(GameObject *viewer,
						   GameObject *target,
						   float viewerEyeOffset,
						   GameObject *occuludingObjects,
						   int occuludingObjectsCount);

void Game_getObjCenter(GameObject *obj, struct Vector3 *result);
float Game_getObjRadius(GameObject *obj);

void Game_update(Input *input);

#ifndef __N64__
#ifdef __cplusplus

#include <vector>

typedef struct RaycastTraceEvent
{
	int result;
	struct Vector3 origin;
	struct Vector3 direction;
	GameObject *hit;
} RaycastTraceEvent;
void Game_traceRaycast(RaycastTraceEvent event);

extern std::vector<RaycastTraceEvent> gameRaycastTrace;
void Game_traceRaycast(RaycastTraceEvent event);
#endif
#endif

#endif /* !_GAME_H_ */
