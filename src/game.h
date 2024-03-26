
#ifndef _GAME_H_
#define _GAME_H_

#include "gametypes.h"
#include "controls/input.h"

void Game_init(LevelData *worldObjects,
			   int worldObjectsCount,
			   PhysWorldData *physWorldData);
Game *Game_get();

GameObject *Game_findObjectByType(ModelType modelType);
GameObject *Game_findObjectNByType(ModelType modelType, int n);

int Game_countObjectsInCategory(ModelTypeCategory category);
GameObject *Game_getIntersectingObject(Vector3 *raySource, Vector3 *rayDirection);

float Game_getObjRadius(GameObject *obj);

int Game_rayIntersectsSphere(Vector3 *origin,
							 Vector3 *rayDirection,
							 Vector3 *objCenter,
							 float objRadius);

int Game_canSeeOtherObject(GameObject *viewer,
						   GameObject *target,
						   float viewerEyeOffset,
						   GameObject *occuludingObjects,
						   int occuludingObjectsCount);

void Game_getObjCenter(GameObject *obj, Vector3 *result);
float Game_getObjRadius(GameObject *obj);

void Game_update(Input *input);

#ifndef __N64__
#ifdef __cplusplus

#include <vector>

typedef struct RaycastTraceEvent
{
	int result;
	Vector3 origin;
	Vector3 direction;
	GameObject *hit;
} RaycastTraceEvent;
void Game_traceRaycast(RaycastTraceEvent event);

extern std::vector<RaycastTraceEvent> gameRaycastTrace;
void Game_traceRaycast(RaycastTraceEvent event);
#endif
#endif

#endif /* !_GAME_H_ */
