
#ifndef RENDERER_H
#define RENDERER_H

#include "../math/frustum.h"
#include "../math/matrix.h"
#include "../gameobject.h"
#include "graphics.h"

// when painter's alg enabled we change render order and disable z buffer for
// some objects that don't need it
#define RENDERER_PAINTERS_ALGORITHM 0

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
								 Vec3d *viewPos,
								 AABB *localAABBs);

int Renderer_frustumCull(GameObject *worldObjects,
							int worldObjectsCount,
							int *worldObjectsVisibility,
							Frustum *frustum,
							AABB *localAABBs);

int Renderer_occlusionCull(GameObject *worldObjects,
							int worldObjectsCount,
							int *worldObjectsVisibility,
							MtxF modelViewMatrix,
							MtxF projMatrix,
							ViewportF viewport,
							Frustum *frustum,
							AABB *localAABBs);

void Renderer_calcIntersecting(int *objectsIntersecting,
							   int objectsCount,
							   RendererSortDistance *sortedObjects,
							   AABB *localAABBs);

void Renderer_getSeparatingPlane(Vec3d *a, Vec3d *b, Plane *separatingPlane);

int Renderer_isCloserBySeparatingPlane(RendererSortDistance *a,
									   RendererSortDistance *b,
									   Vec3d *viewPos);

int Renderer_screenProject(Vec3d *obj,  MtxF modelMatrix,  MtxF projMatrix,  ViewportF viewport,  Vec3d *win);

void Renderer_closestPointOnAABB(AABB *b,
								 /* sourcePoint*/ Vec3d *p,
								 /* result */ Vec3d *q);

AABB Renderer_getWorldAABB(AABB *localAABBs, GameObject *obj);
#endif /* !RENDERER_H_ */
