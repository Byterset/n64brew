#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "../constants.h"

#include "collision.h"
#include "../trace.h"
#include "../math/vector3.h"

#ifndef __N64__
#include "float.h"
// otherwise this stuff is in constants.h
#endif

void Triangle_getCentroid(Triangle *triangle, struct Vector3 *result)
{
	*result = triangle->a;
	vector3AddToSelf(result, &triangle->b);
	vector3AddToSelf(result, &triangle->c);
	vector3DivScalar(result, 3.0);
}

void Triangle_getNormal(Triangle *triangle, struct Vector3 *result)
{
	struct Vector3 edgeAB, edgeAC;
	edgeAB = triangle->b;
	vector3SubFromSelf(&edgeAB, &triangle->a);
	edgeAC = triangle->c;
	vector3SubFromSelf(&edgeAC, &triangle->a);

	vector3Cross(&edgeAB, &edgeAC, result);
	vector3NormalizeSelf(result);
}

// if result > 0: point is in front of triangle
// if result = 0: point is coplanar with triangle
// if result < 0: point is behind triangle
float Triangle_comparePoint(Triangle *triangle, struct Vector3 *point)
{
	struct Vector3 normal, toPoint;

	// normal . (point - triangleVert)
	Triangle_getNormal(triangle, &normal);
	toPoint = *point;
	vector3SubFromSelf(&toPoint, &triangle->a);
	return vector3Dot(&normal, &toPoint);
}

void AABB_fromSphere(struct Vector3 *sphereCenter, float sphereRadius, AABB *result)
{
	result->min = *sphereCenter;
	result->min.x -= sphereRadius;
	result->min.y -= sphereRadius;
	result->min.z -= sphereRadius;

	result->max = *sphereCenter;
	result->max.x += sphereRadius;
	result->max.y += sphereRadius;
	result->max.z += sphereRadius;
}

void AABB_fromTriangle(Triangle *triangle, AABB *result)
{
	result->min = triangle->a;
	result->min.x = MIN(result->min.x, triangle->b.x);
	result->min.x = MIN(result->min.x, triangle->c.x);
	result->min.y = MIN(result->min.y, triangle->b.y);
	result->min.y = MIN(result->min.y, triangle->c.y);
	result->min.z = MIN(result->min.z, triangle->b.z);
	result->min.z = MIN(result->min.z, triangle->c.z);

	result->max = triangle->a;
	result->max.x = MAX(result->max.x, triangle->b.x);
	result->max.x = MAX(result->max.x, triangle->c.x);
	result->max.y = MAX(result->max.y, triangle->b.y);
	result->max.y = MAX(result->max.y, triangle->c.y);
	result->max.z = MAX(result->max.z, triangle->b.z);
	result->max.z = MAX(result->max.z, triangle->c.z);
}

void AABB_expandByPoint(AABB *self, struct Vector3 *point)
{
	self->min.x = MIN(self->min.x, point->x);
	self->min.y = MIN(self->min.y, point->y);
	self->min.z = MIN(self->min.z, point->z);

	self->max.x = MAX(self->max.x, point->x);
	self->max.y = MAX(self->max.y, point->y);
	self->max.z = MAX(self->max.z, point->z);
}

int Collision_intersectAABBAABB(AABB *a, AABB *b)
{
	// Exit with no intersection if separated along an axis
	if (a->max.x < b->min.x || a->min.x > b->max.x)
		return FALSE;
	if (a->max.y < b->min.y || a->min.y > b->max.y)
		return FALSE;
	if (a->max.z < b->min.z || a->min.z > b->max.z)
		return FALSE; // Overlapping on all axes means AABBs are intersecting
	return TRUE;
}

// not tested
int Collision_intersectRayTriangle(struct Vector3 *pt,
								   struct Vector3 *dir,
								   Triangle *tri,
								   struct Vector3 *out)
{
	struct Vector3 edge1, edge2, tvec, pvec, qvec;
	float det, u, v, t;

	edge1 = tri->b;

	vector3SubFromSelf(&edge1, &tri->a);
	edge2 = tri->c;
	vector3SubFromSelf(&edge2, &tri->a);

	vector3Cross(dir, &edge2, &pvec);
	det = vector3Dot(&edge1, &pvec);

	if (det < FLT_EPSILON)
		return FALSE;

	tvec = *pt;
	vector3SubFromSelf(&tvec, &tri->a);
	u = vector3Dot(&tvec, &pvec);
	if (u < 0 || u > det)
		return FALSE;
	vector3Cross(&tvec, &edge1, &qvec);
	v = vector3Dot(dir, &qvec);
	if (v < 0 || u + v > det)
		return FALSE;

	t = vector3Dot(&edge2, &qvec) / det;
	out->x = pt->x + t * dir->x;
	out->y = pt->y + t * dir->y;
	out->z = pt->z + t * dir->z;

	return TRUE;
}

// http://realtimecollisiondetection.net/blog/?p=103
int Collision_sphereTriangleIsSeparated(Triangle *triangle,
										struct Vector3 *P,
										double r)
{
	double rr, d, e, aa, ab, ac, bb, bc, cc, d1, e1, d2, e2, d3, e3;
	int sep1, sep2, sep3, sep4, sep5, sep6, sep7;
	struct Vector3 A, B, C;
	struct Vector3 V, BSubA, CSubA;
	struct Vector3 AB, BC, CA;
	struct Vector3 Q1, ABd1;
	struct Vector3 QC;
	struct Vector3 Q2, BCd2;
	struct Vector3 QA;
	struct Vector3 Q3, CAd3;
	struct Vector3 QB;
	// Translate problem so sphere is centered at origin
	// A = A - P
	A = triangle->a;
	vector3SubFromSelf(&A, P);
	// B = B - P
	B = triangle->b;
	vector3SubFromSelf(&B, P);
	// C = C - P
	C = triangle->c;
	vector3SubFromSelf(&C, P);

	rr = r * r;

	// Testing if sphere lies outside the triangle plane
	{
		// Compute a vector normal to triangle plane (V), normalize it
		// V = (B - A).cross(C - A)
		BSubA = B;
		vector3SubFromSelf(&BSubA, &A);
		CSubA = C;
		vector3SubFromSelf(&CSubA, &A);
		vector3Cross(&BSubA, &CSubA, &V);
		// Compute distance d of sphere center to triangle plane
		d = vector3Dot(&A, &V);
		e = vector3Dot(&V, &V);
		// d > r
		sep1 = d * d > rr * e;

		if (sep1)
			return TRUE;
	}
	// Testing if sphere lies outside a triangle vertex
	{
		// for triangle vertex A

		// Compute distance between sphere center and vertex A
		aa = vector3Dot(&A, &A);
		ab = vector3Dot(&A, &B);
		ac = vector3Dot(&A, &C);

		sep2 =
			// The plane through A with normal A ("A - P") separates sphere if:
			// (1) A lies outside the sphere, and
			(aa > rr) &
			// (2) if B and C lie on the opposite side of the plane w.r.t. the
			// sphere center
			(ab > aa) & (ac > aa);

		if (sep2)
			return TRUE;
	}
	{
		// for triangle vertex B
		bb = vector3Dot(&B, &B);
		bc = vector3Dot(&B, &C);
		sep3 = (bb > rr) & (ab > bb) & (bc > bb);

		if (sep3)
			return TRUE;
	}
	{
		// for triangle vertex C
		cc = vector3Dot(&C, &C);
		sep4 = (cc > rr) & (ac > cc) & (bc > cc);

		if (sep4)
			return TRUE;
	}

	// Testing if sphere lies outside a triangle edge
	{
		// AB = B - A
		AB = B;
		vector3SubFromSelf(&AB, &A);

		d1 = ab - aa;
		e1 = vector3Dot(&AB, &AB);

		// Q1 = A * e1 - AB * d1
		Q1 = A;
		vector3ScaleSelf(&Q1, e1);
		ABd1 = AB;
		vector3ScaleSelf(&ABd1, d1);
		vector3SubFromSelf(&Q1, &ABd1);

		// QC = C * e1 - Q1
		QC = C;
		vector3ScaleSelf(&QC, e1);
		vector3SubFromSelf(&QC, &Q1);

		sep5 = (vector3Dot(&Q1, &Q1) > rr * e1 * e1) & (vector3Dot(&Q1, &QC) > 0);

		if (sep5)
			return TRUE;
	}
	{
		// BC = C - B
		BC = C;
		vector3SubFromSelf(&BC, &B);

		d2 = bc - bb;
		e2 = vector3Dot(&BC, &BC);

		// Q2 = B * e2 - BC * d2
		Q2 = B;
		vector3ScaleSelf(&Q2, e2);
		BCd2 = BC;
		vector3ScaleSelf(&BCd2, d2);
		vector3SubFromSelf(&Q2, &BCd2);

		// QA = A * e2 - Q2
		QA = A;
		vector3ScaleSelf(&QA, e2);
		vector3SubFromSelf(&QA, &Q2);

		sep6 = (vector3Dot(&Q2, &Q2) > rr * e2 * e2) & (vector3Dot(&Q2, &QA) > 0);

		if (sep6)
			return TRUE;
	}
	{
		// CA = A - C
		CA = A;
		vector3SubFromSelf(&CA, &C);

		d3 = ac - cc;
		e3 = vector3Dot(&CA, &CA);

		// Q3 = C * e3 - CA * d3
		Q3 = C;
		vector3ScaleSelf(&Q3, e3);
		CAd3 = CA;
		vector3ScaleSelf(&CAd3, d3);
		vector3SubFromSelf(&Q3, &CAd3);

		// QB = B * e3 - Q3
		QB = B;
		vector3ScaleSelf(&QB, e3);
		vector3SubFromSelf(&QB, &Q3);

		sep7 = (vector3Dot(&Q3, &Q3) > rr * e3 * e3) & (vector3Dot(&Q3, &QB) > 0);

		if (sep7)
			return TRUE;
	}
	return FALSE;
}

void Collision_distancePointTriangleExact(struct Vector3 *point,
										  Triangle *triangle,
										  struct Vector3 *closest)
{
	struct Vector3 diff, edge0, edge1, t0edge0, t1edge1;
	double a00, a01, a11, b0, b1, zero, one, det, t0, t1;
	double invDet;
	double tmp0, tmp1, numer, denom;
	// diff = point - triangle->a
	diff = *point;
	vector3SubFromSelf(&diff, &triangle->a);
	// edge0 = triangle->b - triangle->a
	edge0 = triangle->b;
	vector3SubFromSelf(&edge0, &triangle->a);

	// edge1 = triangle->c - triangle->a
	edge1 = triangle->c;
	vector3SubFromSelf(&edge1, &triangle->a);

	a00 = vector3Dot(&edge0, &edge0);
	a01 = vector3Dot(&edge0, &edge1);
	a11 = vector3Dot(&edge1, &edge1);
	b0 = -vector3Dot(&diff, &edge0);
	b1 = -vector3Dot(&diff, &edge1);
	zero = (double)0;
	one = (double)1;
	det = a00 * a11 - a01 * a01;
	t0 = a01 * b1 - a11 * b0;
	t1 = a01 * b0 - a00 * b1;

	if (t0 + t1 <= det)
	{
		if (t0 < zero)
		{
			if (t1 < zero) // region 4
			{
				if (b0 < zero)
				{
					t1 = zero;
					if (-b0 >= a00) // V1
					{
						t0 = one;
					}
					else // E01
					{
						t0 = -b0 / a00;
					}
				}
				else
				{
					t0 = zero;
					if (b1 >= zero) // V0
					{
						t1 = zero;
					}
					else if (-b1 >= a11) // V2
					{
						t1 = one;
					}
					else // E20
					{
						t1 = -b1 / a11;
					}
				}
			}
			else // region 3
			{
				t0 = zero;
				if (b1 >= zero) // V0
				{
					t1 = zero;
				}
				else if (-b1 >= a11) // V2
				{
					t1 = one;
				}
				else // E20
				{
					t1 = -b1 / a11;
				}
			}
		}
		else if (t1 < zero) // region 5
		{
			t1 = zero;
			if (b0 >= zero) // V0
			{
				t0 = zero;
			}
			else if (-b0 >= a00) // V1
			{
				t0 = one;
			}
			else // E01
			{
				t0 = -b0 / a00;
			}
		}
		else // region 0, interior
		{
			invDet = det == 0 ? 0 : one / det;
			t0 *= invDet;
			t1 *= invDet;
		}
	}
	else
	{
		if (t0 < zero) // region 2
		{
			tmp0 = a01 + b0;
			tmp1 = a11 + b1;
			if (tmp1 > tmp0)
			{
				numer = tmp1 - tmp0;
				denom = a00 - ((double)2) * a01 + a11;
				if (numer >= denom) // V1
				{
					t0 = one;
					t1 = zero;
				}
				else // E12
				{
					t0 = numer / denom;
					t1 = one - t0;
				}
			}
			else
			{
				t0 = zero;
				if (tmp1 <= zero) // V2
				{
					t1 = one;
				}
				else if (b1 >= zero) // V0
				{
					t1 = zero;
				}
				else // E20
				{
					t1 = -b1 / a11;
				}
			}
		}
		else if (t1 < zero) // region 6
		{
			tmp0 = a01 + b1;
			tmp1 = a00 + b0;
			if (tmp1 > tmp0)
			{
				numer = tmp1 - tmp0;
				denom = a00 - ((double)2) * a01 + a11;
				if (numer >= denom) // V2
				{
					t1 = one;
					t0 = zero;
				}
				else // E12
				{
					t1 = numer / denom;
					t0 = one - t1;
				}
			}
			else
			{
				t1 = zero;
				if (tmp1 <= zero) // V1
				{
					t0 = one;
				}
				else if (b0 >= zero) // V0
				{
					t0 = zero;
				}
				else // E01
				{
					t0 = -b0 / a00;
				}
			}
		}
		else // region 1
		{
			numer = a11 + b1 - a01 - b0;
			if (numer <= zero) // V2
			{
				t0 = zero;
				t1 = one;
			}
			else
			{
				denom = a00 - ((double)2) * a01 + a11;
				if (numer >= denom) // V1
				{
					t0 = one;
					t1 = zero;
				}
				else // 12
				{
					t0 = numer / denom;
					t1 = one - t0;
				}
			}
		}
	}

	// closest = triangle->a + t0 * edge0 + t1 * edge1;
	t0edge0 = edge0;
	vector3ScaleSelf(&t0edge0, t0);
	t1edge1 = edge1;
	vector3ScaleSelf(&t1edge1, t1);
	*closest = triangle->a;
	vector3AddToSelf(closest, &t0edge0);
	vector3AddToSelf(closest, &t1edge1);

	if (closest->x != closest->x)
	{
#ifndef __N64__
		debugPrintf("got NAN\n");
		// Collision_distancePointTriangleExact(point, triangle, closest);
#endif
		vector3Init(closest, 0.0f, 0.0f, 0.0f);
	}

	// other things we could calculate:
	// parameter[0] = one - t0 - t1;
	// parameter[1] = t0;
	// parameter[2] = t1;
	// diff = point - closest;
	// sqrDistance = vector3Dot(diff, diff);
}

#ifndef __N64__
#ifdef __cplusplus

#include <map>
int testCollisionResult;
int testCollisionTrace = FALSE; // set to true to capture trace

std::map<int, SphereTriangleCollision> testCollisionResults;
#endif
#endif

#define COLLISION_SPATIAL_HASH_MAX_RESULTS 100
#define COLLISION_SPATIAL_HASH_PRUNING_ENABLED 1

float Collision_sqDistancePointAABB(struct Vector3 *p, AABB *b)
{
	float v, dist;
	float sqDist = 0.0f;
	// For each axis count any excess distance outside box extents
	v = p->x;
	if (v < b->min.x)
	{
		dist = (b->min.x - v);
		sqDist += dist * dist;
	}
	if (v > b->max.x)
	{
		dist = (v - b->max.x);
		sqDist += dist * dist;
	}

	v = p->y;
	if (v < b->min.y)
	{
		dist = (b->min.y - v);
		sqDist += dist * dist;
	}
	if (v > b->max.y)
	{
		dist = (v - b->max.y);
		sqDist += dist * dist;
	}

	v = p->z;
	if (v < b->min.z)
	{
		dist = (b->min.z - v);
		sqDist += dist * dist;
	}
	if (v > b->max.z)
	{
		dist = (v - b->max.z);
		sqDist += dist * dist;
	}
	return sqDist;
}

// Returns true if sphere intersects AABB, false otherwise
int Collision_testSphereAABBCollision(struct Vector3 *sphereCenter,
									  float sphereRadius,
									  AABB *aabb)
{
	// Compute squared distance between sphere center and AABB
	float sqDist = Collision_sqDistancePointAABB(sphereCenter, aabb);
	// Sphere and AABB intersect if the (squared) distance
	// between them is less
	// than the (squared) sphere radius
	return sqDist <= sphereRadius * sphereRadius;
}

int Collision_testMeshSphereCollision(Triangle *triangles,
									  int trianglesLength,
									  struct Vector3 *objCenter,
									  float objRadius,
									  SpatialHash *spatialHash,
									  SphereTriangleCollision *result)
{
	int i, k;
	Triangle *tri;
	struct Vector3 closestPointOnTriangle;

	float closestHitDistSq;
	float hitDistSq;
	float objRadiusSq;
	AABB triangleAABB;
	// AABB sphereAABB;
	int hit, closestHitTriangleIndex, spatialHashResultsCount;
	int spatialHashResults[COLLISION_SPATIAL_HASH_MAX_RESULTS];
	// float profStartTriangleExact = CUR_TIME_MS();
	closestHitDistSq = FLT_MAX;
	closestHitTriangleIndex = -1;
	objRadiusSq = objRadius * objRadius;

	// AABB_fromSphere(objCenter, objRadius, &sphereAABB);

#ifndef __N64__
#ifdef __cplusplus
	if (testCollisionTrace)
	{
		testCollisionResults.clear();
	}
#endif
#endif

	spatialHashResultsCount = SpatialHash_getTriangles(
		objCenter, objRadius, spatialHash, spatialHashResults,
		COLLISION_SPATIAL_HASH_MAX_RESULTS);

#if COLLISION_SPATIAL_HASH_PRUNING_ENABLED
	for (k = 0; k < spatialHashResultsCount; k++)
	{
		i = spatialHashResults[k];
		tri = triangles + i;
#else
	for (i = 0, tri = triangles; i < trianglesLength; i++, tri++)
	{
#endif
		// as an optimization, first test AABB overlap
		AABB_fromTriangle(tri, &triangleAABB);
		if (!Collision_testSphereAABBCollision(objCenter, objRadius,
											   &triangleAABB))
		{
			continue;
		}
		// if (!Collision_intersectAABBAABB(&sphereAABB, &triangleAABB)) {
		//   continue;
		// }

		// then test triangle intersection
		// hit = !Collision_sphereTriangleIsSeparated(tri, objCenter, objRadius);
		hit = TRUE;

		if (hit)
		{
			Collision_distancePointTriangleExact(objCenter, tri,
												 &closestPointOnTriangle);

			hitDistSq = vector3DistSqrd(objCenter, &closestPointOnTriangle);
			if (hitDistSq > objRadiusSq)
			{
				// not really a collision, separating axis test fucked up
				continue;
			}

#ifndef __N64__
#ifdef __cplusplus
			if (testCollisionTrace)
			{
				SphereTriangleCollision debugResult = {
					i, hitDistSq, closestPointOnTriangle, tri, triangleAABB};
				testCollisionResults.insert(
					std::pair<int, SphereTriangleCollision>(i, debugResult));
			}
#endif
#endif

			if (hitDistSq < closestHitDistSq)
			{
				closestHitDistSq = hitDistSq;
				closestHitTriangleIndex = i;

				result->index = i;
				result->distance = sqrtf(closestHitDistSq);
				result->triangle = tri;
				result->posInTriangle = closestPointOnTriangle;
				result->triangleAABB = triangleAABB;
			}
		}
	}

#ifndef __N64__
#ifdef __cplusplus
	if (testCollisionTrace)
	{
		testCollisionResult = closestHitTriangleIndex;
	}
#endif
#endif

	// Trace_addEvent(CollisionTestMeshSphereTraceEvent, profStartTriangleExact,
	//                CUR_TIME_MS());
	return closestHitTriangleIndex > -1;
}

// Test if segment specified by points p0 and p1 intersects AABB b
// from Real Time Collision Detection ch5.3
int Collision_testSegmentAABBCollision(struct Vector3 *p0, struct Vector3 *p1, AABB *b)
{
	struct Vector3 c;
	struct Vector3 e;
	struct Vector3 m;
	struct Vector3 d;
	float adx;
	float ady;
	float adz;

	// Box center-point
	// c = (b->min + b->max) * 0.5f;
	c = b->min;
	vector3AddToSelf(&c, &b->max);
	vector3ScaleSelf(&c, 0.5f);

	// Box halflength extents
	// e = b->max - c;
	e = b->max;
	vector3SubFromSelf(&e, &c);

	// Segment midpoint
	// (p0 + p1) * 0.5f;
	m = *p0;
	vector3AddToSelf(&m, p1);
	vector3ScaleSelf(&m, 0.5f);

	// Segment halflength vector
	// d = p1 - m;
	d = *p1;
	vector3SubFromSelf(&d, &m);

	// Translate box and segment to origin
	// m = m - c;
	vector3SubFromSelf(&m, &c);

	// Try world coordinate axes as separating axes
	adx = fabsf(d.x);
	if (fabsf(m.x) > e.x + adx)
		return FALSE;
	ady = fabsf(d.y);
	if (fabsf(m.y) > e.y + ady)
		return FALSE;
	adz = fabsf(d.z);
	if (fabsf(m.z) > e.z + adz)
		return FALSE;
	// Add in an epsilon term to counteract arithmetic errors when segment is
	// (near) parallel to a coordinate axis (see text for detail)
	adx += FLT_EPSILON;
	ady += FLT_EPSILON;
	adz += FLT_EPSILON;
	// Try cross products of segment direction vector with coordinate axes
	if (fabsf(m.y * d.z - m.z * d.y) > e.y * adz + e.z * ady)
		return FALSE;
	if (fabsf(m.z * d.x - m.x * d.z) > e.x * adz + e.z * adx)
		return FALSE;
	if (fabsf(m.x * d.y - m.y * d.x) > e.x * ady + e.y * adx)
		return FALSE; // No separating axis found; segment must be overlapping AABB
	return TRUE;
}

int SpatialHash_getBucketIndex(int cellX, int cellY, int cellsInDimension)
{
	// layout is rows then columns (row-major)
	// (x, y) is a cell pos, not unit pos
	return cellY * cellsInDimension + cellX;
}

// quantize world pos to containing grid cell
int SpatialHash_unitsToGridForDimension(float unitsPos,
										SpatialHash *spatialHash)
{
	return floorf(unitsPos / spatialHash->gridCellSize) +
		   spatialHash->cellOffsetInDimension;
}

// convert world pos to integral pos in grid cell
float SpatialHash_unitsToGridFloatForDimension(float unitsPos,
											   SpatialHash *spatialHash)
{
	return unitsPos / spatialHash->gridCellSize +
		   spatialHash->cellOffsetInDimension;
}

// localize grid cell to world pos of bottom of cell
float SpatialHash_gridToUnitsForDimension(float gridCell,
										  SpatialHash *spatialHash)
{
	return (gridCell - spatialHash->cellOffsetInDimension) *
		   spatialHash->gridCellSize;
}

void SpatialHash_raycast(float x0,
						 float y0,
						 float x1,
						 float y1,
						 SpatialHashRaycastCallback traversalVisitor,
						 void *traversalState)
{
	float dx;
	float dy;
	int x;
	int y;
	int n;
	int x_inc, y_inc;
	float error;

	dx = fabs(x1 - x0);
	dy = fabs(y1 - y0);

	x = (int)(floor(x0));
	y = (int)(floor(y0));

	n = 1;

	if (dx == 0)
	{
		x_inc = 0;
		error = FLT_MAX;
	}
	else if (x1 > x0)
	{
		x_inc = 1;
		n += (int)(floor(x1)) - x;
		error = (floor(x0) + 1 - x0) * dy;
	}
	else
	{
		x_inc = -1;
		n += x - (int)(floor(x1));
		error = (x0 - floor(x0)) * dy;
	}

	if (dy == 0)
	{
		y_inc = 0;
		error -= FLT_MAX;
	}
	else if (y1 > y0)
	{
		y_inc = 1;
		n += (int)(floor(y1)) - y;
		error -= (floor(y0) + 1 - y0) * dx;
	}
	else
	{
		y_inc = -1;
		n += y - (int)(floor(y1));
		error -= (y0 - floor(y0)) * dx;
	}

	for (; n > 0; --n)
	{
		// visit(x, y);
		traversalVisitor(x, y, traversalState);

		if (error > 0)
		{
			y += y_inc;
			error -= dx;
		}
		else
		{
			x += x_inc;
			error += dy;
		}
	}
}

SpatialHashBucket *SpatialHash_getBucket(float x,
										 float y,
										 SpatialHash *spatialHash)
{
	int bucketIndex, cellX, cellY;

	cellX = SpatialHash_unitsToGridForDimension(x, spatialHash);
	cellY = SpatialHash_unitsToGridForDimension(y, spatialHash);
	bucketIndex =
		SpatialHash_getBucketIndex(cellX, cellY, spatialHash->cellsInDimension);

	invariant(bucketIndex < spatialHash->numBuckets);

	return *(spatialHash->data + bucketIndex);
}

typedef struct GetTrianglesVisitBucketState
{
	SpatialHash *spatialHash;
	int *results;
	int maxResults;
	int resultsFound;
} GetTrianglesVisitBucketState;

void SpatialHash_getTrianglesVisitBucket(int cellX,
										 int cellY,
										 GetTrianglesVisitBucketState *state)
{
	int bucketIndex, bucketItemIndex, resultIndex;
	SpatialHashBucket *bucket;
	int *bucketItem, *currentResult;

	bucketIndex = SpatialHash_getBucketIndex(
		cellX, cellY, state->spatialHash->cellsInDimension);

	invariant(bucketIndex < state->spatialHash->numBuckets);

	bucket = *(state->spatialHash->data + bucketIndex);
	if (!bucket)
	{
		// nothing in this bucket
		return;
	}
	// collect results from this bucket
	for (bucketItemIndex = 0; bucketItemIndex < bucket->size; ++bucketItemIndex)
	{
		bucketItem = bucket->data + bucketItemIndex;

		// look through results and add if not duplicate
		// TODO: optimize this, as it's currently O(n^2)
		// could qsort then remove duplicates to get to O(n log n)
		for (resultIndex = 0; resultIndex < state->maxResults; ++resultIndex)
		{
			currentResult = state->results + resultIndex;
			if (resultIndex < state->resultsFound)
			{
				if (*currentResult == *bucketItem)
				{
					// already have this triangle in the results
					break;
				}
				else
				{
					continue;
				}
			}
			else
			{
				// at end of found results and this result is not already in the
				// list
				*currentResult = *bucketItem;
				state->resultsFound++;
				break; // continue to next item in bucket
			}
		}
	}
}

int SpatialHash_getTrianglesForRaycast(struct Vector3 *rayStart,
									   struct Vector3 *rayEnd,
									   SpatialHash *spatialHash,
									   int *results,
									   int maxResults)
{
	GetTrianglesVisitBucketState traversalState;
	traversalState.spatialHash = spatialHash;
	traversalState.results = results;
	traversalState.maxResults = maxResults;
	traversalState.resultsFound = 0;
	SpatialHash_raycast(
		SpatialHash_unitsToGridFloatForDimension(rayStart->x, spatialHash),
		SpatialHash_unitsToGridFloatForDimension(-rayStart->z, spatialHash),
		SpatialHash_unitsToGridFloatForDimension(rayEnd->x, spatialHash),
		SpatialHash_unitsToGridFloatForDimension(-rayEnd->z, spatialHash),
		(SpatialHashRaycastCallback)&SpatialHash_getTrianglesVisitBucket,
		(void *)&traversalState);

#ifndef __N64__
	if (traversalState.resultsFound == maxResults)
	{
		debugPrintf("possibly ran out of space in results array\n");
	}
#endif

	return traversalState.resultsFound;
}

int SpatialHash_getTriangles(struct Vector3 *position,
							 float radius,
							 SpatialHash *spatialHash,
							 int *results,
							 int maxResults)
{
	int minCellX, minCellY, maxCellX, maxCellY, cellX, cellY;
	GetTrianglesVisitBucketState traversalState;
	// float profStartCollisionGetTriangles = CUR_TIME_MS();

	traversalState.spatialHash = spatialHash;
	traversalState.results = results;
	traversalState.maxResults = maxResults;
	traversalState.resultsFound = 0;

	minCellX =
		SpatialHash_unitsToGridForDimension(position->x - radius, spatialHash);
	minCellY =
		SpatialHash_unitsToGridForDimension(-position->z - radius, spatialHash);
	maxCellX =
		SpatialHash_unitsToGridForDimension(position->x + radius, spatialHash) +
		1;
	maxCellY =
		SpatialHash_unitsToGridForDimension(-position->z + radius, spatialHash) +
		1;

	// walk range of overlapping buckets and collect (unique) set of triangles
	for (cellX = minCellX; cellX < maxCellX; ++cellX)
	{
		for (cellY = minCellY; cellY < maxCellY; ++cellY)
		{
			SpatialHash_getTrianglesVisitBucket(cellX, cellY, &traversalState);
		}
	}

#ifndef __N64__
	if (traversalState.resultsFound == maxResults)
	{
		debugPrintf("possibly ran out of space in results array\n");
	}
#endif

	// Trace_addEvent(CollisionGetTrianglesTraceEvent,
	//                profStartCollisionGetTriangles, CUR_TIME_MS());
	return traversalState.resultsFound;
}
