
#ifndef _GAME_H_
#define _GAME_H_

#include "gameobject.h"
#include "gametypes.h"
#include "controls/input.h"
#include "physics/physics.h"

void Game_init(GameObject* worldObjects,
               int worldObjectsCount,
               PhysWorldData* physWorldData);
Game* Game_get();

GameObject* Game_findObjectByType(ModelType modelType);
GameObject* Game_findObjectNByType(ModelType modelType, int n);

int Game_countObjectsInCategory(ModelTypeCategory category);
GameObject* Game_getIntersectingObject(Vec3d* raySource, Vec3d* rayDirection);

float Game_getObjRadius(GameObject* obj);

int Game_rayIntersectsSphere(Vec3d* origin,
                             Vec3d* rayDirection,
                             Vec3d* objCenter,
                             float objRadius);

int Game_canSeeOtherObject(GameObject* viewer,
                           GameObject* target,
                           float viewerEyeOffset,
                           GameObject* occuludingObjects,
                           int occuludingObjectsCount);

void Game_getObjCenter(GameObject* obj, Vec3d* result);
float Game_getObjRadius(GameObject* obj);

void Game_update(Input* input);

#ifndef __N64__
#ifdef __cplusplus

#include <vector>

typedef struct RaycastTraceEvent {
  int result;
  Vec3d origin;
  Vec3d direction;
  GameObject* hit;
} RaycastTraceEvent;
void Game_traceRaycast(RaycastTraceEvent event);

extern std::vector<RaycastTraceEvent> gameRaycastTrace;
void Game_traceRaycast(RaycastTraceEvent event);
#endif
#endif

#endif /* !_GAME_H_ */
