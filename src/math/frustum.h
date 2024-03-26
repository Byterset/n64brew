
#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "../physics/collision.h"

typedef struct Plane
{
	Vector3 normal; // Plane normal. Points x on the plane satisfy Dot(n,x) = -d
	Vector3 point;
	float d; // d = -dot(n,p) for a given point p on the plane
} Plane;

typedef enum FrustumPlanes
{
	NearFrustumPlane,
	FarFrustumPlane,
	TopFrustumPlane,
	BottomFrustumPlane,
	LeftFrustumPlane,
	RightFrustumPlane,
	NUM_FRUSTUM_PLANES
} FrustumPlanes;

typedef enum FrustumTestResult
{
	InsideFrustum,
	OutsideFrustum,
	IntersectingFrustum,
	MAX_FRUSTUM_TEST_RESULT
} FrustumTestResult;

extern char *FrustumTestResultStrings[MAX_FRUSTUM_TEST_RESULT];
extern char *FrustumPlanesStrings[NUM_FRUSTUM_PLANES];

typedef struct Frustum
{
	Plane planes[NUM_FRUSTUM_PLANES];
	Vector3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
	float nearD, farD, aspect, fovy, tang;
	float nw, nh, fw, fh;
} Frustum;

float Plane_distance(Plane *self, Vector3 *p);

void Plane_setNormalAndPoint(Plane *self, Vector3 *normal, Vector3 *point);

float Plane_distPointToPlane(Plane *p, Vector3 *q);
void Plane_pointClosestPoint(Plane *p, Vector3 *q, Vector3 *result);

void Frustum_setCamInternals(Frustum *self,
							 float fovy,
							 float aspect,
							 float nearD,
							 float farD);

void Frustum_setCamDef(Frustum *self, Vector3 *p, Vector3 *l, Vector3 *u);
FrustumTestResult Frustum_boxInFrustum(Frustum *frustum, AABB *aabb);
FrustumTestResult Frustum_boxInFrustumNaive(Frustum *frustum, AABB *aabb);

FrustumTestResult Frustum_boxFrustumPlaneTestRTCD(Frustum *frustum,
												  AABB *aabb,
												  int planeIdx);
FrustumTestResult Frustum_boxFrustumPlaneTestPN(Frustum *frustum,
												AABB *aabb,
												int planeIdx);

void Frustum_getAABBVertexP(AABB *self, Vector3 *normal, Vector3 *result);
void Frustum_getAABBVertexN(AABB *self, Vector3 *normal, Vector3 *result);

#endif /* !FRUSTUM_H_ */
