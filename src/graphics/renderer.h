
#ifndef RENDERER_H
#define RENDERER_H

#include "../math/frustum.h"
#include "../math/matrix.h"
#include "../gameobject.h"
#include "graphics.h"

typedef struct RendererSortDistance
{
	GameObject *obj;
	float distance;
	AABB worldAABB;
} RendererSortDistance;

typedef enum LightingType
{
	SunLighting,
	OnlyAmbientLighting,
	MAX_LIGHTING_TYPE
} LightingType;

int Renderer_isDynamicObject(GameObject *obj);

LightingType Renderer_getLightingType(GameObject *obj);
int Renderer_isZBufferedGameObject(GameObject *obj);
int Renderer_isZWriteGameObject(GameObject *obj);
int Renderer_isBackgroundGameObject(GameObject *obj);

int Renderer_isLitGameObject(GameObject *obj);
int Renderer_isAnimatedGameObject(GameObject *obj);

void Renderer_sortVisibleObjects(GameObject *worldObjects,
								 int worldObjectsCount,
								 int *worldObjectsVisibility,
								 int visibleObjectsCount,
								 RendererSortDistance *result,
								 Vector3 *viewPos,
								 AABB *localAABBs);

int Renderer_frustumCull(GameObject *worldObjects,
							int worldObjectsCount,
							int *worldObjectsVisibility,
							Frustum *frustum,
							AABB *localAABBs);

void Renderer_calcIntersecting(int *objectsIntersecting,
							   int objectsCount,
							   RendererSortDistance *sortedObjects,
							   AABB *localAABBs);

void Renderer_getSeparatingPlane(Vector3 *a, Vector3 *b, Plane *separatingPlane);

int Renderer_isCloserBySeparatingPlane(RendererSortDistance *a,
									   RendererSortDistance *b,
									   Vector3 *viewPos);

int Renderer_screenProject(Vector3 *obj,  MtxF modelMatrix,  MtxF projMatrix,  ViewportF viewport,  Vector3 *win);

void Renderer_closestPointOnAABB(AABB *b,
								 /* sourcePoint*/ Vector3 *p,
								 /* result */ Vector3 *q);

AABB Renderer_getWorldAABB(AABB *localAABBs, GameObject *obj);
#endif /* !RENDERER_H_ */
