
#include "frustum.h"
#include <math.h>
#include "../constants.h"

char *FrustumTestResultStrings[MAX_FRUSTUM_TEST_RESULT] = {
	"Inside",
	"Outside",
	"Intersecting",
};

char *FrustumPlanesStrings[NUM_FRUSTUM_PLANES] = {
	"NearFrustumPlane",	  //
	"FarFrustumPlane",	  //
	"TopFrustumPlane",	  //
	"BottomFrustumPlane", //
	"LeftFrustumPlane",	  //
	"RightFrustumPlane",  //
};

void Plane_setNormalAndPoint(Plane *self, struct Vector3 *normal, struct Vector3 *point)
{
	self->normal = *normal;
	vector3NormalizeSelf(&self->normal);
	self->point = *point;

	self->d = -(vector3Dot(&self->normal, &self->point));
}

void Plane_set3Points(Plane *self, struct Vector3 *v1, struct Vector3 *v2, struct Vector3 *v3)
{
	struct Vector3 aux1, aux2;

	aux1 = *v1;
	vector3SubFromSelf(&aux1, v2);
	aux2 = *v3;
	vector3SubFromSelf(&aux2, v2);

	vector3Cross(&aux2, &aux1, &self->normal);

	vector3NormalizeSelf(&self->normal);
	self->point = *v2;
	self->d = -(vector3Dot(&self->normal, &self->point));
}

float Plane_distance(Plane *self, struct Vector3 *p)
{
	return (self->d + vector3Dot(&self->normal, p));
}

void Plane_pointClosestPoint(Plane *p, struct Vector3 *q, struct Vector3 *result)
{
	// float t = Dot(p.n, q) - p.d;
	// return q - t * p.n;

	struct Vector3 tmp;
	float t = vector3Dot(&p->normal, q) + p->d;

	*result = *q;
	tmp = p->normal;
	vector3ScaleSelf(&tmp, t);
	vector3SubFromSelf(result, &tmp);
}

float Plane_distPointToPlane(Plane *p, struct Vector3 *q)
{
	// return Dot(q, p.n) - p.d; if plane equation normalized (||p.n||==1)
	return (vector3Dot(&p->normal, q) + p->d) / vector3Dot(&p->normal, &p->normal);
}

// based on
// http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-implementation/

// This function takes exactly the same parameters as the function
// guPerspective/gluPerspective. Each time the perspective definitions change,
// for instance when a window is resized, this function should be called as
// well.
void Frustum_setCamInternals(Frustum *self,
							 float fovy,
							 float aspect,
							 float nearD,
							 float farD)
{
	self->aspect = aspect;
	self->fovy = fovy;
	self->nearD = nearD;
	self->farD = farD;

	self->tang = (float)tanf(degToRad(fovy) * 0.5f);
	self->nh = nearD * self->tang;
	self->nw = self->nh * aspect;
	self->fh = farD * self->tang;
	self->fw = self->fh * aspect;
}

// This function takes three vectors that contain the information for the
// guLookAt/gluLookAt function: the position of the camera (p), a point to where
// the camera is pointing (l) and the up vector (u). Each time the camera
// position or orientation changes, this function should be called as well.
void Frustum_setCamDef(Frustum *self, struct Vector3 *p, struct Vector3 *l, struct Vector3 *u)
{
	struct Vector3 nc, fc, X, Y, Z;
	// compute the Z axis of camera
	// this axis points in the opposite direction from
	// the looking direction
	// Z = p - l;
	Z = *p;
	vector3SubFromSelf(&Z, l);
	vector3NormalizeSelf(&Z);

	// X axis of camera with given "up" vector and Z axis
	// X = u * Z;
	vector3Cross(u, &Z, &X);
	vector3NormalizeSelf(&X);

	// the real "up" vector is the cross product of Z and X
	// Y = Z * X;
	vector3Cross(&Z, &X, &Y);

	// compute the centers of the near and far planes
	{
		// nc = p - Z * nearD;
		struct Vector3 ZNearD;
		ZNearD = Z;
		vector3ScaleSelf(&ZNearD, self->nearD);
		nc = *p;
		vector3SubFromSelf(&nc, &ZNearD);
	}

	{
		// fc = p - Z * farD;
		struct Vector3 ZFarD;
		ZFarD = Z;
		vector3ScaleSelf(&ZFarD, self->farD);
		fc = *p;
		vector3SubFromSelf(&fc, &ZFarD);
	}

	{
		struct Vector3 Ynh, Xnw, Yfh, Xfw;

		Ynh = Y;
		vector3ScaleSelf(&Ynh, self->nh);
		Xnw = X;
		vector3ScaleSelf(&Xnw, self->nw);
		Yfh = Y;
		vector3ScaleSelf(&Yfh, self->fh);
		Xfw = X;
		vector3ScaleSelf(&Xfw, self->fw);

		// ntl = nc + Ynh - Xnw;
		self->ntl = nc;
		vector3AddToSelf(&self->ntl, &Ynh);
		vector3SubFromSelf(&self->ntl, &Xnw);

		// ntr = nc + Ynh + Xnw;
		self->ntr = nc;
		vector3AddToSelf(&self->ntr, &Ynh);
		vector3AddToSelf(&self->ntr, &Xnw);

		// nbl = nc - Ynh - Xnw;
		self->nbl = nc;
		vector3SubFromSelf(&self->nbl, &Ynh);
		vector3SubFromSelf(&self->nbl, &Xnw);

		// nbr = nc - Ynh + Xnw;
		self->nbr = nc;
		vector3SubFromSelf(&self->nbr, &Ynh);
		vector3AddToSelf(&self->nbr, &Xnw);

		// ftl = fc + Yfh - Xfw;
		self->ftl = fc;
		vector3AddToSelf(&self->ftl, &Yfh);
		vector3SubFromSelf(&self->ftl, &Xfw);

		// ftr = fc + Yfh + Xfw;
		self->ftr = fc;
		vector3AddToSelf(&self->ftr, &Yfh);
		vector3AddToSelf(&self->ftr, &Xfw);

		// fbl = fc - Yfh - Xfw;
		self->fbl = fc;
		vector3SubFromSelf(&self->fbl, &Yfh);
		vector3SubFromSelf(&self->fbl, &Xfw);

		// fbr = fc - Yfh + Xfw;
		self->fbr = fc;
		vector3SubFromSelf(&self->fbr, &Yfh);
		vector3AddToSelf(&self->fbr, &Xfw);
	}

	Plane_set3Points(&self->planes[TopFrustumPlane], &self->ntr, &self->ntl,
					 &self->ftl);
	Plane_set3Points(&self->planes[BottomFrustumPlane], &self->nbl, &self->nbr,
					 &self->fbr);
	Plane_set3Points(&self->planes[LeftFrustumPlane], &self->ntl, &self->nbl,
					 &self->fbl);
	Plane_set3Points(&self->planes[RightFrustumPlane], &self->nbr, &self->ntr,
					 &self->fbr);
	Plane_set3Points(&self->planes[NearFrustumPlane], &self->ntl, &self->ntr,
					 &self->nbr);
	Plane_set3Points(&self->planes[FarFrustumPlane], &self->ftr, &self->ftl,
					 &self->fbl);
}

void Frustum_setCamDef2(Frustum *self, struct Vector3 *p, struct Vector3 *l, struct Vector3 *u)
{
	struct Vector3 aux, normal;

	struct Vector3 nc, fc, X, Y, Z;
	// compute the Z axis of camera
	// this axis points in the opposite direction from
	// the looking direction
	// Z = p - l;
	Z = *p;
	vector3SubFromSelf(&Z, l);
	vector3NormalizeSelf(&Z);

	// X axis of camera with given "up" vector and Z axis
	// X = u * Z;
	vector3Cross(u, &Z, &X);
	vector3NormalizeSelf(&X);

	// the real "up" vector is the cross product of Z and X
	// Y = Z * X;
	vector3Cross(&Z, &X, &Y);

	// compute the centers of the near and far planes
	{
		// nc = p - Z * nearD;
		struct Vector3 tempZ;
		tempZ = Z;
		vector3ScaleSelf(&tempZ, self->nearD);
		nc = *p;
		vector3SubFromSelf(&nc, &tempZ);
	}

	{
		// fc = p - Z * farD;
		struct Vector3 tempZ;
		tempZ = Z;
		vector3ScaleSelf(&tempZ, self->farD);
		fc = *p;
		vector3SubFromSelf(&fc, &tempZ);
	}

	// near
	{
		struct Vector3 negZ;
		vector3Copy(&Z, &negZ);
		// negZ = Z;
		vector3ScaleSelf(&negZ, -1.0);
		Plane_setNormalAndPoint(&self->planes[NearFrustumPlane], &negZ, &nc);
	}
	// far
	Plane_setNormalAndPoint(&self->planes[FarFrustumPlane], &Z, &fc);

	{
		struct Vector3 Ynh;
		Ynh = Y;
		vector3ScaleSelf(&Ynh, self->nh);
		// top
		{
			// aux = (nc + Y * nh) - p;
			struct Vector3 nsAddYnh;
			nsAddYnh = nc;
			vector3AddToSelf(&nsAddYnh, &Ynh);
			aux = nsAddYnh;
			vector3SubFromSelf(&aux, p);
			vector3NormalizeSelf(&aux);
			// normal = aux * X;
			vector3Cross(&aux, &X, &normal);
			Plane_setNormalAndPoint(&self->planes[TopFrustumPlane], &normal,
									&nsAddYnh);
		}
		// bottom
		{
			// aux = (nc - Y * nh) - p;
			struct Vector3 ncSubYnh;
			ncSubYnh = nc;
			vector3SubFromSelf(&ncSubYnh, &Ynh);
			aux = ncSubYnh;
			vector3SubFromSelf(&aux, p);
			vector3NormalizeSelf(&aux);
			// normal = X * aux;
			vector3Cross(&aux, &X, &normal);
			Plane_setNormalAndPoint(&self->planes[BottomFrustumPlane], &normal,
									&ncSubYnh);
		}
	}
	{
		struct Vector3 Xnw;
		Xnw = X;
		vector3ScaleSelf(&Xnw, self->nw);
		// left
		{
			// aux = (nc - X * nw) - p;
			struct Vector3 ncSubXnw;
			ncSubXnw = nc;
			vector3SubFromSelf(&ncSubXnw, &Xnw);

			aux = ncSubXnw;
			vector3SubFromSelf(&aux, p);
			vector3NormalizeSelf(&aux);
			vector3Cross(&aux, &Y, &normal);
			Plane_setNormalAndPoint(&self->planes[LeftFrustumPlane], &normal,
									&ncSubXnw);
		}
		// right
		{
			// aux = (nc + X * nw) - p;
			struct Vector3 ncAddXnw;
			ncAddXnw = nc;
			vector3AddToSelf(&ncAddXnw, &Xnw);
			aux = ncAddXnw;
			vector3SubFromSelf(&aux, p);
			vector3NormalizeSelf(&aux);
			vector3Cross(&aux, &Y, &normal);
			Plane_setNormalAndPoint(&self->planes[RightFrustumPlane], &normal,
									&ncAddXnw);
		}
	}
}

void Frustum_getAABBVertexP(AABB *self, struct Vector3 *normal, struct Vector3 *result)
{
	*result = self->min;

	if (normal->x >= 0)
		result->x = self->max.x;

	if (normal->y >= 0)
		result->y = self->max.y;

	if (normal->z >= 0)
		result->z = self->max.z;
}

void Frustum_getAABBVertexN(AABB *self, struct Vector3 *normal, struct Vector3 *result)
{
	*result = self->max;

	if (normal->x >= 0)
		result->x = self->min.x;

	if (normal->y >= 0)
		result->y = self->min.y;

	if (normal->z >= 0)
		result->z = self->min.z;
}

// FrustumTestResult Frustum_boxInFrustum(Frustum* frustum, AABB* aabb) {
//   int i;
//   float r, s;
//   struct Vector3 center, positiveExtents;
//   Plane* plane;
//   FrustumTestResult result;
//   result = InsideFrustum;
//   // for each plane do ...
//   for (i = 0; i < NUM_FRUSTUM_PLANES; i++) {
//     plane = &frustum->planes[i];
//     // Compute AABB center
//     center = aabb->max;
//     vector3AddToSelf(&center, &aabb->min);
//     vector3ScaleSelf(&center, 0.5f);

//     // Compute positive extents
//     positiveExtents = aabb->max;
//     vector3SubFromSelf(&positiveExtents, &center);

//     // Compute the projection interval radius of b onto L(t) = aabb->c + t *
//     p.n r = positiveExtents.x * fabsf(plane->normal.x) +
//         positiveExtents.y * fabsf(plane->normal.y) +
//         positiveExtents.z * fabsf(plane->normal.z);
//     // Compute distance of box center from plane
//     s = vector3Dot(&plane->normal, &center) - plane->d;
//     // Intersection occurs when distance s falls within [-r,+r] interval
//     // fabsf(s) <= r;

//     if (fabsf(s) <= r) {
//       result = IntersectingFrustum;
//     } else if (s < -r) {
//       return OutsideFrustum;
//     }
//   }

//   return result;
// }

// from Real Time Collision Detection ch 5.2.3
// currently doesn't work, maybe plane representation is wrong?
FrustumTestResult Frustum_boxFrustumPlaneTestRTCD(Frustum *frustum,
												  AABB *aabb,
												  int planeIdx)
{
	float r, s;
	struct Vector3 center, positiveExtents;
	Plane *plane;
	FrustumTestResult result = InsideFrustum;
	// for each plane do ...
	plane = &frustum->planes[planeIdx];
	// Compute AABB center
	// center = (min + max) * 0.5
	center = aabb->min;
	vector3AddToSelf(&center, &aabb->max);
	vector3ScaleSelf(&center, 0.5f);

	// Compute positive extents
	// extents = max - center
	positiveExtents = aabb->max;
	vector3SubFromSelf(&positiveExtents, &center);

	// Compute the projection interval radius of b onto L(t) = center + t *
	// plane.normal
	r = positiveExtents.x * fabsf(plane->normal.x) +
		positiveExtents.y * fabsf(plane->normal.y) +
		positiveExtents.z * fabsf(plane->normal.z);
	// Compute distance of box center from plane
	s = vector3Dot(&plane->normal, &center) - plane->d;
	// Intersection occurs when distance s falls within [-r,+r] interval
	// fabsf(s) <= r;

	if (s + r > 0)
	{
		result = IntersectingFrustum;
	}
	else if (s - r >= 0)
	{
		// fully inside
	}
	else
	{
		return OutsideFrustum;
	}

	return result;
}

FrustumTestResult Frustum_boxFrustumPlaneTestPN(Frustum *frustum,
												AABB *aabb,
												int planeIdx)
{
	struct Vector3 vertexP, vertexN;
	FrustumTestResult result = InsideFrustum;
	Frustum_getAABBVertexP(aabb, &frustum->planes[planeIdx].normal, &vertexP);
	Frustum_getAABBVertexN(aabb, &frustum->planes[planeIdx].normal, &vertexN);
	// is the positive vertex outside?
	if (Plane_distance(&frustum->planes[planeIdx], &vertexP) < 0)
	{
		return OutsideFrustum;
		// is the negative vertex outside?
	}
	else if (Plane_distance(&frustum->planes[planeIdx], &vertexN) < 0)
	{
		result = IntersectingFrustum;
	}
	return result;
}

FrustumTestResult Frustum_boxInFrustum(Frustum *frustum, AABB *aabb)
{
	FrustumTestResult result = InsideFrustum;
	FrustumTestResult planeResult;
	int i;
	// for each plane do ...
	for (i = 0; i < NUM_FRUSTUM_PLANES; i++)
	{
		planeResult = Frustum_boxFrustumPlaneTestPN(frustum, aabb, i);

		if (planeResult == OutsideFrustum)
		{
			return OutsideFrustum;
		}
		else if (planeResult == IntersectingFrustum)
		{
			result = planeResult;
		}
	}
	return result;
}

void Frustum_getAABBVertex(AABB *aabb, int vertex, struct Vector3 *result)
{
	switch (vertex)
	{
	case 0:
		*result = (struct Vector3){aabb->min.x, aabb->min.y, aabb->min.z};
		return;
	case 1:
		*result = (struct Vector3){aabb->max.x, aabb->min.y, aabb->min.z};
		return;
	case 2:
		*result = (struct Vector3){aabb->min.x, aabb->max.y, aabb->min.z};
		return;
	case 3:
		*result = (struct Vector3){aabb->min.x, aabb->min.y, aabb->max.z};
		return;
	case 4:
		*result = (struct Vector3){aabb->max.x, aabb->max.y, aabb->max.z};
		return;
	case 5:
		*result = (struct Vector3){aabb->min.x, aabb->max.y, aabb->max.z};
		return;
	case 6:
		*result = (struct Vector3){aabb->max.x, aabb->min.y, aabb->max.z};
		return;
	case 7:
		*result = (struct Vector3){aabb->max.x, aabb->max.y, aabb->min.z};
		return;
	}
}

FrustumTestResult Frustum_boxInFrustumNaive(Frustum *frustum, AABB *aabb)
{
	int i, k;
	int out;
	int in;
	FrustumTestResult result = InsideFrustum;
	struct Vector3 vertex;

	// for each plane do ...
	for (i = 0; i < NUM_FRUSTUM_PLANES; i++)
	{
		// reset counters for corners in and out
		out = 0;
		in = 0;
		// for each corner of the box do ...
		// get out of the cycle as soon as a box as corners
		// both inside and out of the frustum
		for (k = 0; k < 8 && (in == 0 || out == 0); k++)
		{
			Frustum_getAABBVertex(aabb, k, &vertex);
			// is the corner outside or inside
			if (Plane_distance(&frustum->planes[i], &vertex) < 0)
			{
				out++;
			}
			else
			{
				in++;
			}
		}
		// if all corners are out
		if (!in)
		{
			return OutsideFrustum;
			// if some corners are out and others are in
		}
		else if (out)
		{
			result = IntersectingFrustum;
		}
	}
	return result;
}
