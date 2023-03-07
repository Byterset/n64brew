
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

int Renderer_isDynamicObject(GameObject *obj);
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
								 struct Vector3 *viewPos,
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

void Renderer_getSeparatingPlane(struct Vector3 *a, struct Vector3 *b, Plane *separatingPlane);

int Renderer_isCloserBySeparatingPlane(RendererSortDistance *a,
									   RendererSortDistance *b,
									   struct Vector3 *viewPos);

int Renderer_screenProject(struct Vector3 *obj,  MtxF modelMatrix,  MtxF projMatrix,  ViewportF viewport,  struct Vector3 *win);

void Renderer_closestPointOnAABB(AABB *b,
								 /* sourcePoint*/ struct Vector3 *p,
								 /* result */ struct Vector3 *q);

AABB Renderer_getWorldAABB(AABB *localAABBs, GameObject *obj);
#endif /* !RENDERER_H_ */
