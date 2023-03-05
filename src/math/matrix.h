#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "vector4.h"
#include "vector3.h"
#include <ultra64.h>

/*
rotation/scale is from mtx[0][0] to mtx[2][2]
translation (13-15) is from mtx[3][0] to mtx[3][2]
 */
typedef float MtxF[4][4];

void matrixPerspective(MtxF, unsigned short* perspNorm, float l, float r, float top, float b, float near, float far);

void matrixVec3Mul(MtxF, struct Vector3* input, struct Vector4* output);

void matrixFromBasis(MtxF, struct Vector3* origin, struct Vector3* x, struct Vector3* y, struct Vector3* z);
void matrixFromBasisL(Mtx* matrix, struct Vector3* origin, struct Vector3* x, struct Vector3* y, struct Vector3* z);

void mulMtxFVecF(MtxF matrix, float *in /*[4]*/, float *out /*[4]*/);

void mulMtxFMtxF(MtxF matrix_a, MtxF matrix_b, MtxF *out);

#endif