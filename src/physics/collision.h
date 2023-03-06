
#ifndef _COLLISION_H_
#define _COLLISION_H_

#include "../math/vector3.h"

typedef struct Triangle
{
	struct Vector3 a;
	struct Vector3 b;
	struct Vector3 c;
} Triangle;

typedef struct AABB
{
	struct Vector3 min;
	struct Vector3 max;
} AABB;

typedef struct SphereTriangleCollision
{
	int index;
	float distance;
	struct Vector3 posInTriangle;
	Triangle *triangle;
	AABB triangleAABB;
} SphereTriangleCollision;

typedef struct SpatialHashBucket
{
	int size;
	int *data;
} SpatialHashBucket;

typedef struct SpatialHash
{
	int numBuckets;
	float gridCellSize;
	int cellsInDimension;
	int cellOffsetInDimension;
	SpatialHashBucket **data;
} SpatialHash;

#ifndef __N64__
#ifdef __cplusplus

#include <map>
extern int testCollisionResult;
extern int testCollisionTrace;

extern std::map<int, SphereTriangleCollision> testCollisionResults;
#endif
#endif

void Triangle_getCentroid(Triangle *triangle, struct Vector3 *result);
void Triangle_getNormal(Triangle *triangle, struct Vector3 *result);
float Triangle_comparePoint(Triangle *triangle, struct Vector3 *point);

void AABB_fromTriangle(Triangle *triangle, AABB *result);

int Collision_intersectAABBAABB(AABB *a, AABB *b);

int Collision_sphereTriangleIsSeparated(Triangle *triangle,
										struct Vector3 *sphereCenter,
										double sphereRadius);

void Collision_distancePointTriangleExact(struct Vector3 *point,
										  Triangle *triangle,
										  struct Vector3 *closest);

int Collision_testMeshSphereCollision(Triangle *triangles,
									  int trianglesLength,
									  struct Vector3 *objCenter,
									  float objRadius,
									  SpatialHash *spatialHash,
									  SphereTriangleCollision *result);

int Collision_testSegmentAABBCollision(struct Vector3 *p0, struct Vector3 *p1, AABB *b);

int SpatialHash_unitsToGridForDimension(float unitsPos,
										SpatialHash *spatialHash);

float SpatialHash_unitsToGridFloatForDimension(float unitsPos,
											   SpatialHash *spatialHash);
float SpatialHash_gridToUnitsForDimension(float unitsPos,
										  SpatialHash *spatialHash);

SpatialHashBucket *SpatialHash_getBucket(float x,
										 float y,
										 SpatialHash *spatialHash);

typedef void (*SpatialHashRaycastCallback)(int, int, void *);

void SpatialHash_raycast(float x0,
						 float y0,
						 float x1,
						 float y1,
						 SpatialHashRaycastCallback traversalVisitor,
						 void *traversalState);

int SpatialHash_getTriangles(struct Vector3 *position,
							 float radius,
							 SpatialHash *spatialHash,
							 int *results,
							 int maxResults);

int SpatialHash_getTrianglesForRaycast(struct Vector3 *rayStart,
									   struct Vector3 *rayEnd,
									   SpatialHash *spatialHash,
									   int *results,
									   int maxResults);
#endif /* !_COLLISION_H_ */
