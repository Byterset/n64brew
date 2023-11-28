
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
#include "../math/vector2.h"
#include "../math/vector3.h"
#include "../math/vector4.h"
#include "../game.h"


#define RENDERER_FRUSTUM_CULLING 1

int Renderer_isDynamicObject(GameObject *obj)
{
	return obj->physBody != NULL;
}

LightingType Renderer_getLightingType(GameObject *obj)
{
	switch (obj->modelType)
	{
	case UniFloorModel:
		return OnlyAmbientLighting;
	default:
		return SunLighting;
	}
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

float Renderer_gameobjectSortDist(GameObject *obj, struct Vector3 *viewPos)
{
	if (Renderer_isBackgroundGameObject(obj))
	{
		// always consider this far away
		return 10000.0F - obj->id; // add object id to achieve stable sorting
	}

	return vector3Dist(&obj->position, viewPos);
}

void Renderer_closestPointOnAABB(AABB *b,
								 /* sourcePoint*/ struct Vector3 *p,
								 /* result */ struct Vector3 *q)
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

void Renderer_getSeparatingPlane(struct Vector3 *a, struct Vector3 *b, Plane *separatingPlane)
{
	struct Vector3 halfwayPoint, aToBDirection;
	halfwayPoint = *a;
	vector3AddToSelf(&halfwayPoint, b);
	vector3DivScalar(&halfwayPoint, 2);

	vector3DirectionTo(a, b, &aToBDirection);
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
									   struct Vector3 *viewPos)
{
	Plane separatingPlane;
	struct Vector3 aCenter, bCenter, aClosestPoint, bClosestPoint, aReallyClosestPoint,
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

	vector3AddToSelf(&worldAABB.min, &obj->position);
	vector3AddToSelf(&worldAABB.max, &obj->position);
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

	int i;
	// no-op impl which just marks all objects as potentially intersecting
	for (i = 0; i < objectsCount; ++i)
	{
		objectsIntersecting[i] = TRUE;
	}

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
			vector3AddToSelf(&worldAABB.min, &obj->position);
			vector3AddToSelf(&worldAABB.max, &obj->position);

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

int Renderer_screenProject(struct Vector3 *obj,
				  MtxF modelMatrix,
				  MtxF projMatrix,
				  ViewportF viewport,
				  struct Vector3 *win)
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
								 struct Vector3 *viewPos,
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
			visibleObjectIndex++;
		}
	}

	invariant(visibleObjectIndex == visibleObjectsCount);
	qsort(result, visibleObjectsCount, sizeof(RendererSortDistance),
		  Renderer_sortWorldComparatorFnZBuffer);
}
