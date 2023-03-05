
#include "renderer.h"
#include <math.h>
#ifndef __N64__
#include <stdio.h>
#endif
#ifdef __N64__
#include <PR/os_internal.h>
#endif
#include <assert.h>

#include <stdlib.h>

#include "../util/memory.h"
#include "../ed64/ed64io_usb.h"
#include "../math/frustum.h"
#include "../math/vec3d.h"
#include "../math/vec2d.h"
#include "../math/vector3.h"
#include "../math/vector4.h"
#include "../game.h"
#include "../gameobject.h"


#define RENDERER_FRUSTUM_CULLING 1

int Renderer_isDynamicObject(GameObject *obj)
{
	return obj->physBody != NULL;
}

int Renderer_isZBufferedGameObject(GameObject *obj)
{
	if (Renderer_isDynamicObject(obj))
		return TRUE;
	if (Renderer_isAnimatedGameObject(obj))
		return TRUE;

	switch (obj->modelType)
	{
	case BushModel:
		// case WatergrassModel:
		// case ReedModel:
		return TRUE;
	default:
		return FALSE;
	}
}
int Renderer_isZWriteGameObject(GameObject *obj)
{
	switch (obj->modelType)
	{
	// case GroundModel:
	case WaterModel:
		return TRUE;
	default:
		return FALSE;
	}
}

int Renderer_isBackgroundGameObject(GameObject *obj)
{
	switch (obj->modelType)
	{
	case GroundModel:
	case WaterModel:
		return TRUE;
	default:
		return FALSE;
	}
}

int Renderer_isLitGameObject(GameObject *obj)
{
	switch (obj->modelType)
	{
	case GroundModel:
	case WaterModel:
	case WallModel:
	case PlanterModel:
		return TRUE;
	default:
		return FALSE;
	}
}

int Renderer_isAnimatedGameObject(GameObject *obj)
{
	switch (obj->modelType)
	{
	case GooseModel:
	case GardenerCharacterModel:
		return TRUE;
	default:
		return FALSE;
	}
}

float Renderer_gameobjectSortDist(GameObject *obj, Vec3d *viewPos)
{
	if (Renderer_isBackgroundGameObject(obj))
	{
		// always consider this far away
		return 10000.0F - obj->id; // add object id to achieve stable sorting
	}

	return Vec3d_distanceTo(&obj->position, viewPos);
}

void Renderer_closestPointOnAABB(AABB *b,
								 /* sourcePoint*/ Vec3d *p,
								 /* result */ Vec3d *q)
{
	float v;
	v = p->x;
	if (v < b->min.x)
		v = b->min.x; // v = max(v, b->min.x)
	if (v > b->max.x)
		v = b->max.x; // v = min(v, b->max.x)
	q->x = v;
	v = p->y;
	if (v < b->min.y)
		v = b->min.y; // v = max(v, b->min.y)
	if (v > b->max.y)
		v = b->max.y; // v = min(v, b->max.y)
	q->y = v;
	v = p->z;
	if (v < b->min.z)
		v = b->min.z; // v = max(v, b->min.z)
	if (v > b->max.z)
		v = b->max.z; // v = min(v, b->max.z)
	q->z = v;
}

void Renderer_getSeparatingPlane(Vec3d *a, Vec3d *b, Plane *separatingPlane)
{
	Vec3d halfwayPoint, aToBDirection;
	halfwayPoint = *a;
	Vec3d_add(&halfwayPoint, b);
	Vec3d_divScalar(&halfwayPoint, 2);

	Vec3d_directionTo(a, b, &aToBDirection);
	aToBDirection.y = 0; // only separate on x,z
	if (fabsf(aToBDirection.x) > fabsf(aToBDirection.z))
	{
		aToBDirection.z = 0;
	}
	else
	{
		aToBDirection.x = 0;
	}

	Plane_setNormalAndPoint(separatingPlane, &aToBDirection, &halfwayPoint);
}

// currently this somehow causes the game to crash when touching a BushModel ??
int Renderer_isCloserBySeparatingPlane(RendererSortDistance *a,
									   RendererSortDistance *b,
									   Vec3d *viewPos)
{
	Plane separatingPlane;
	Vec3d aCenter, bCenter, aClosestPoint, bClosestPoint, aReallyClosestPoint,
		bReallyClosestPoint;
	float planeToADist, planeToBDist, planeToViewDist;
	invariant(a->obj != NULL);
	invariant(b->obj != NULL);

	Game_getObjCenter(a->obj, &aCenter);
	Game_getObjCenter(b->obj, &bCenter);

	// dumb heuristic
	if (Renderer_isDynamicObject(a->obj))
	{
		aClosestPoint = aCenter;
	}
	else
	{
		Renderer_closestPointOnAABB(&a->worldAABB, &bCenter, &aClosestPoint);
	}
	if (Renderer_isDynamicObject(b->obj))
	{
		bClosestPoint = bCenter;
	}
	else
	{
		Renderer_closestPointOnAABB(&b->worldAABB, &aCenter, &bClosestPoint);
	}

	if (Renderer_isDynamicObject(a->obj))
	{
		aReallyClosestPoint = aCenter;
	}
	else
	{
		Renderer_closestPointOnAABB(&a->worldAABB, &bClosestPoint,
									&aReallyClosestPoint);
	}
	if (Renderer_isDynamicObject(b->obj))
	{
		bReallyClosestPoint = bCenter;
	}
	else
	{
		Renderer_closestPointOnAABB(&b->worldAABB, &aClosestPoint,
									&bReallyClosestPoint);
	}

	Renderer_getSeparatingPlane(&aReallyClosestPoint, &bReallyClosestPoint,
								&separatingPlane);

	planeToADist = Plane_distance(&separatingPlane, &aCenter);
	planeToBDist = Plane_distance(&separatingPlane, &bCenter);
	planeToViewDist = Plane_distance(&separatingPlane, viewPos);

	if ((planeToADist < 0.0) == (planeToBDist < 0.0))
	{
		// if A is on the same side of plane as B, probably intersecting
		return 0;
	}
	else
	{
		if ((planeToADist < 0.0) == (planeToViewDist < 0.0))
		{
			// if A is on the same side of plane as the view, A closer than B
			return -1;
		}
		else
		{
			// B closer than A
			return 1;
		}
	}
}

// global variable because qsort's API sucks lol
Vec3d sortWorldComparatorFn_viewPos;
int sortIterations = 0;
int Renderer_sortWorldComparatorFnPaintersSeparatingPlane(const void *a,
														  const void *b)
{
	RendererSortDistance *sortA = (RendererSortDistance *)a;
	RendererSortDistance *sortB = (RendererSortDistance *)b;
#ifdef DEBUG
	sortIterations++;
	invariant(sortIterations < 1000);
#endif
	// sort far to near for painters algorithm
	if (Renderer_isBackgroundGameObject(sortA->obj) ||
		Renderer_isBackgroundGameObject(sortB->obj))
	{
		return sortB->distance - sortA->distance;
	}

	return -Renderer_isCloserBySeparatingPlane(sortA, sortB,
											   &sortWorldComparatorFn_viewPos);
}

int Renderer_sortWorldComparatorFnPaintersSimple(const void *a, const void *b)
{
	RendererSortDistance *sortA = (RendererSortDistance *)a;
	RendererSortDistance *sortB = (RendererSortDistance *)b;

	// sort far to near for painters algorithm
	return sortB->distance - sortA->distance;
}

int Renderer_sortWorldComparatorFnZBuffer(const void *a, const void *b)
{
	RendererSortDistance *sortA = (RendererSortDistance *)a;
	RendererSortDistance *sortB = (RendererSortDistance *)b;
	// sort near to far, so we benefit from zbuffer fast bailout
	return sortA->distance - sortB->distance;
}

AABB Renderer_getWorldAABB(AABB *localAABBs, GameObject *obj)
{
	AABB *localAABB = localAABBs + obj->id;
	AABB worldAABB = *localAABB;

	Vec3d_add(&worldAABB.min, &obj->position);
	Vec3d_add(&worldAABB.max, &obj->position);
	return worldAABB;
}

typedef struct GameObjectAABB
{
	int index;
	AABB aabb;
} GameObjectAABB;

void Renderer_calcIntersecting(
	int *objectsIntersecting, // the result, keyed by index in sorted objects
	int objectsCount,
	RendererSortDistance *sortedObjects,
	AABB *localAABBs)
{
#if RENDERER_PAINTERS_ALGORITHM
	int i, k;
	int zWriteObjectsCount, zBufferedObjectsCount;
	GameObjectAABB *zWriteObjects;
	GameObjectAABB *zBufferedObjects;
	GameObject *obj;
	GameObjectAABB *zWriteAABB;
	GameObjectAABB *zBufferedAABB;
	GameObjectAABB *otherZBufferedAABB;

	zWriteObjectsCount = 0;
	zBufferedObjectsCount = 0;
	zWriteObjects =
		(GameObjectAABB *)malloc((objectsCount) * sizeof(GameObjectAABB));
	zBufferedObjects =
		(GameObjectAABB *)malloc((objectsCount) * sizeof(GameObjectAABB));
	invariant(zWriteObjects);
	invariant(zBufferedObjects);

	for (i = 0; i < objectsCount; ++i)
	{
		obj = (sortedObjects + i)->obj;
		objectsIntersecting[i] = FALSE;
		if (Renderer_isZWriteGameObject(obj))
		{
			zWriteObjects[zWriteObjectsCount] =
				(GameObjectAABB){i, Renderer_getWorldAABB(localAABBs, obj)};
			zWriteObjectsCount++;
		}
		else if (Renderer_isZBufferedGameObject(obj))
		{
			zBufferedObjects[zBufferedObjectsCount] =
				(GameObjectAABB){i, Renderer_getWorldAABB(localAABBs, obj)};
			zBufferedObjectsCount++;
		}
	}
	for (i = 0; i < zBufferedObjectsCount; ++i)
	{
		zBufferedAABB = &zBufferedObjects[i];

		for (k = 0; k < zWriteObjectsCount; ++k)
		{
			zWriteAABB = &zWriteObjects[k];
			if (Collision_intersectAABBAABB(&zBufferedAABB->aabb,
											&zWriteAABB->aabb))
			{
				objectsIntersecting[zBufferedAABB->index] = TRUE;
				objectsIntersecting[zWriteAABB->index] = TRUE;
			}
		}
		for (k = 0; k < zBufferedObjectsCount; ++k)
		{
			if (i == k)
				continue;
			otherZBufferedAABB = &zBufferedObjects[k];
			if (Collision_intersectAABBAABB(&zBufferedAABB->aabb,
											&otherZBufferedAABB->aabb))
			{
				objectsIntersecting[zBufferedAABB->index] = TRUE;
				objectsIntersecting[otherZBufferedAABB->index] = TRUE;
			}
		}
	}

	free(zWriteObjects);
	free(zBufferedObjects);
#else
	int i;
	// no-op impl which just marks all objects as potentially intersecting
	for (i = 0; i < objectsCount; ++i)
	{
		objectsIntersecting[i] = TRUE;
	}
#endif
}

int Renderer_frustumCull(GameObject *worldObjects,
							int worldObjectsCount,
							int *worldObjectsVisibility,
							Frustum *frustum,
							AABB *localAABBs)
{
	GameObject *obj;
	int i;
	int visibilityCulled = 0;
	for (i = 0; i < worldObjectsCount; i++)
	{
		obj = worldObjects + i;
		// cull all objects that have no model or are set to invisible
		if (obj->modelType == NoneModel || !obj->visible)
		{
			worldObjectsVisibility[i] = FALSE;
			visibilityCulled++;
			continue;
		}
// cull all objects that are not in the camera frustum
#if RENDERER_FRUSTUM_CULLING
		{
			FrustumTestResult frustumTestResult;

			// transform the local bounding box of the object into world space
			AABB *localAABB = localAABBs + i;
			AABB worldAABB = *localAABB;
			Vec3d_add(&worldAABB.min, &obj->position);
			Vec3d_add(&worldAABB.max, &obj->position);

			// check if the box is inside the given frustum
			frustumTestResult = Frustum_boxInFrustum(frustum, &worldAABB);
			// printf("%d: %s", i, FrustumTestResultStrings[frustumTestResult]);
			if (frustumTestResult == OutsideFrustum)
			{
				// cull this object
				visibilityCulled++;
				worldObjectsVisibility[i] = FALSE;
				continue;
			}
		}
#endif
		// if none of the above is the case, the object is visible
		worldObjectsVisibility[i] = TRUE;
	}

	return visibilityCulled;
}

int Renderer_occlusionCull(GameObject *worldObjects,
						   int worldObjectsCount,
						   int *worldObjectsVisibility,
						   MtxF modelViewMatrix,
						   MtxF projMatrix,
						   ViewportF viewport,
						   Frustum *frustum,
						   AABB *localAABBs)
{
	GameObject *obj;
	MtxF mvp_matrix;

	int a;
    int b;
    int c;
    MtxF res;
    for (a = 0; a < 4; a++) {
        for (b = 0; b < 4; b++) {
            float sum = 0.0f;
            for (c = 0; c < 4; c++) {
                sum += modelViewMatrix[a][c] * projMatrix[c][b];
            }
            mvp_matrix[a][b] = sum;
        }
    }
	// mulMtxFMtxF(&projMatrix, &modelViewMatrix, &mvp_matrix);
	int i;
	int visibilityCulled = 0;
	for (i = 0; i < worldObjectsCount; i++)
	{
		// if this object is already culled by the frustum check, continue with the next
		if (worldObjectsVisibility[i] == FALSE)
		{
			continue;
		}
		obj = worldObjects + i;

		AABB *localAABB = localAABBs + i;
		Vec3d aabb_min = localAABB->min;
		Vec3d aabb_max = localAABB->max;
		struct Vector4 corners[8] = {
            {aabb_min.x, aabb_min.y, aabb_min.z, 1},
            {aabb_min.x, aabb_min.y, aabb_max.z, 1},
            {aabb_min.x, aabb_max.y, aabb_min.z, 1},
            {aabb_min.x, aabb_max.y, aabb_max.z, 1},
            {aabb_max.x, aabb_min.y, aabb_min.z, 1},
            {aabb_max.x, aabb_min.y, aabb_max.z, 1},
            {aabb_max.x, aabb_max.y, aabb_min.z, 1},
            {aabb_max.x, aabb_max.y, aabb_max.z, 1}
		};

		Mtx objTransform;
		// set the transform in world space for the gameobject to render
		guPosition(&objTransform,
				   obj->rotation.x,									   // rot x
				   obj->rotation.y,							   // rot y
				   obj->rotation.z,								   // rot z
				   modelTypesProperties[obj->modelType].scale, // scale
				   obj->position.x,							   // pos x
				   obj->position.y,							   // pos y
				   obj->position.z							   // pos z
		);
		int j;
		float corner[4];
		float translated[4];
		float clip_coord[4];
		MtxF objTransformF;
		guMtxL2F(objTransformF, &objTransform);
		Vec2d screen_corners[8];
		Vec2d win_size = {(float)SCREEN_WD, (float)SCREEN_HT};
		for(j = 0; j < 8; j++){
			corner[0] = corners[i].x;
			corner[1] = corners[i].y;
			corner[2] = corners[i].z;
			corner[3] = corners[i].w;
			mulMtxFVecF(objTransformF, &corner, &translated);
			mulMtxFVecF(mvp_matrix, &translated, &clip_coord);
			Vec3d clip3d = {clip_coord[0], clip_coord[1], clip_coord[2]};

			Vec3d_divScalar(&clip3d, clip_coord[3]);

			
			// sprintf(translated_str, "clip3d_w: %f", clip_coord[3]);
			
			Vec2d ndc_2d = {clip3d.x, clip3d.y};
			Vec2d screen_corner = ndc_2d;
			Vec2d_mulScalar(&screen_corner, 0.5f);
			Vec2d_add(&screen_corner, &win_size);
			screen_corners[j] = screen_corner;

		}
		corner[0] = obj->position.x;
		corner[1] = obj->position.y;
		corner[2] = obj->position.z;
		corner[3] = 1.0f;
		mulMtxFVecF(objTransformF, &corners[0], &translated);
		mulMtxFVecF(mvp_matrix, &translated, &clip_coord);
		// char *translated_str[20];
		// sprintf(translated_str, "screenCn: (%f,%f,%f)", clip_coord[0], clip_coord[1], clip_coord[2] );
		// console_add_msg(translated_str);

		Vec2d screen_aabb_min = screen_corners[0];
        Vec2d screen_aabb_max = screen_corners[0];
		// char *translated_str[20];
		// sprintf(translated_str, "corner: (%f,%f)", screen_aabb_min.x, screen_aabb_min.y);
		// console_add_msg(translated_str);
		int k;
        for (k = 1; k < 8; k++) {
			Vec2d current = screen_corners[k];

			// char *translated_str[20];
			// sprintf(translated_str, "screenCn: (%f,%f)", screen_corners[k].x, screen_corners[k].y);
			// console_add_msg(translated_str);

			screen_aabb_min.x = screen_aabb_min.x < screen_corners[k].x ? screen_aabb_min.x : screen_corners[k].x;
			// screen_aabb_min.y = screen_aabb_min.y < screen_corners[k].y ? screen_aabb_min.y : screen_corners[k].y;

			// screen_aabb_max.x = screen_aabb_max.x > screen_corners[k].x ? screen_aabb_max.x : screen_corners[k].x;
			// screen_aabb_max.y = screen_aabb_max.y > screen_corners[k].y ? screen_aabb_max.y : screen_corners[k].y;
			// Vec2d_min(&screen_aabb_min, &current);
            // Vec2d_max(&screen_aabb_max, &current);
        }

		// sprintf(screen_bb, "ScreenBB: min(%d,%d), max(%d,%d)", screen_aabb_min.x, screen_aabb_min.y, screen_aabb_max.x, screen_aabb_max.y);
		// console_add_msg(screen_bb);
	}
	return 0;
}

int Renderer_screenProject(Vec3d *obj,
				  MtxF modelMatrix,
				  MtxF projMatrix,
				  ViewportF viewport,
				  Vec3d *win)
{
	float in[4];
	float out[4];

	in[0] = obj->x;
	in[1] = obj->y;
	in[2] = obj->z;
	in[3] = 1.0;
	mulMtxFVecF(modelMatrix, in, out);
	mulMtxFVecF(projMatrix, out, in);

	if (in[3] == 0.0)
		return FALSE;
	in[0] /= in[3];
	in[1] /= in[3];
	in[2] /= in[3];
	/* Map x, y and z to range 0-1 */
	in[0] = in[0] * 0.5 + 0.5;
	in[1] = in[1] * 0.5 + 0.5;
	in[2] = in[2] * 0.5 + 0.5;

	/* Map x,y to viewport */
	in[0] = in[0] * viewport[2] + viewport[0];
	in[1] = in[1] * viewport[3] + viewport[1];

	win->x = in[0];
	win->y = in[1];
	win->z = in[2];
	return TRUE;
}


void Renderer_sortVisibleObjects(GameObject *worldObjects,
								 int worldObjectsCount,
								 int *worldObjectsVisibility,
								 int visibleObjectsCount,
								 RendererSortDistance *result,
								 Vec3d *viewPos,
								 AABB *localAABBs)
{
	int i;
	RendererSortDistance *sortDist; //obj, distance, worldAABB

	int visibleObjectIndex = 0;
	for (i = 0; i < worldObjectsCount; ++i)
	{
		// only add visible objects, compacting results array
		if (worldObjectsVisibility[i])
		{
			// results array is only as long as num visible objects
			invariant(visibleObjectIndex < visibleObjectsCount);
			sortDist = result + visibleObjectIndex;
			sortDist->obj = worldObjects + i;
			sortDist->distance = Renderer_gameobjectSortDist(sortDist->obj, viewPos);

#if RENDERER_PAINTERS_ALGORITHM
			sortDist->worldAABB = Renderer_getWorldAABB(localAABBs, sortDist->obj);
#endif

			visibleObjectIndex++;
		}
	}

	invariant(visibleObjectIndex == visibleObjectsCount);

	// used for painters algo
	sortWorldComparatorFn_viewPos = *viewPos;
#if RENDERER_PAINTERS_ALGORITHM
#if 1
	sortIterations = 0;
	qsort(result, visibleObjectsCount, sizeof(RendererSortDistance),
		  Renderer_sortWorldComparatorFnPaintersSeparatingPlane);
#else
	qsort(result, visibleObjectsCount, sizeof(RendererSortDistance),
		  Renderer_sortWorldComparatorFnPaintersSimple);
#endif
#else
	qsort(result, visibleObjectsCount, sizeof(RendererSortDistance),
		  Renderer_sortWorldComparatorFnZBuffer);
#endif
}
