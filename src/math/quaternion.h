/**
 * @file quaternion.h
 * @author initial: James Lambert, modified: Kevin Reier
 * @brief Quaternion math functions Headerfile
 * @date 2023-11-21
 * 
 */
#ifndef _QUATERNION_H
#define _QUATERNION_H

#include "vector3.h"
#include "vector2.h"

/**
 * @struct Quaternion
 * @brief The Quaternion struct.
 */
typedef struct Quaternion
{
	float x, y, z, w;
} Quaternion;

void quatIdent(struct Quaternion *q);
void quatAxisAngle(Vector3 *axis, float angle, struct Quaternion *out);
void quatAxisComplex(Vector3 *axis, Vector2 *complex, struct Quaternion *out);
void quatConjugate(struct Quaternion *in, struct Quaternion *out);
void quatNegate(struct Quaternion *in, struct Quaternion *out);
void quatRotateVector(struct Quaternion *q, Vector3 *a, Vector3 *out);
void quatRotatedBoundingBoxSize(struct Quaternion *q, Vector3 *halfBoxSize, Vector3 *out);
void quatMultiply(struct Quaternion *a, struct Quaternion *b, struct Quaternion *out);
void quatAdd(struct Quaternion *a, struct Quaternion *b, struct Quaternion *out);
void quatToMatrix(struct Quaternion *q, float out[4][4]);
void quatNormalize(struct Quaternion *q, struct Quaternion *out);
void quatRandom(struct Quaternion *q);
void quatLook(Vector3 *lookDir, Vector3 *up, struct Quaternion *out);
void quatFromEulerRad(Vector3 *angles, struct Quaternion *out);
void quatFromEulerDegrees(Vector3 *angles, struct Quaternion *out);
void quatToEulerDegrees(struct Quaternion *q, Vector3 *out);
void quatToEulerRad(struct Quaternion *q, Vector3 *out);
// cheap approximation of slerp
void quatLerp(struct Quaternion *a, struct Quaternion *b, float t, struct Quaternion *out);
void quatApplyAngularVelocity(struct Quaternion *input, Vector3 *w, float timeStep, struct Quaternion *output);
void quatDecompose(struct Quaternion *input, Vector3 *axis, float *angle);
float quatDotProduct(struct Quaternion *a, struct Quaternion *b);
#endif