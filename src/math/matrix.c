#include "matrix.h"
#include "../defs.h"
#include "../constants.h"

void matrixPerspective(float matrix[4][4], unsigned short* perspNorm, float l, float r, float t, float b, float near, float far) {
	guMtxIdentF(matrix);

    matrix[0][0] = 2.0f * near / (r - l);
    matrix[1][1] = 2.0f * near / (t - b);
    matrix[2][0] = (r + l) / (r - l);
    matrix[2][1] = (t + b) / (t - b);
    matrix[2][2] = -(far + near) / (far - near);
    matrix[2][3] = -1;
    matrix[3][2] = -2.0f * far * near / (far - near);
    matrix[3][3] = 0.0f;

	if (perspNorm != (u16 *) NULL) {
	    if (near+far<=2.0) {
		    *perspNorm = (u16) 0xFFFF;
	    } else  {
		    *perspNorm = (u16) ((2.0*65536.0)/(near+far));
            if (*perspNorm<=0) {
                *perspNorm = (u16) 0x0001;
            }
	    }
	}
}

void matrixVec3Mul(float matrix[4][4], struct Vector3* input, struct Vector4* output) {
    output->x = matrix[0][0] * input->x + matrix[1][0] * input->y + matrix[2][0] * input->z + matrix[3][0];
    output->y = matrix[0][1] * input->x + matrix[1][1] * input->y + matrix[2][1] * input->z + matrix[3][1];
    output->z = matrix[0][2] * input->x + matrix[1][2] * input->y + matrix[2][2] * input->z + matrix[3][2];
    output->w = matrix[0][3] * input->x + matrix[1][3] * input->y + matrix[2][3] * input->z + matrix[3][3];
}

void matrixFromBasis(float matrix[4][4], struct Vector3* origin, struct Vector3* x, struct Vector3* y, struct Vector3* z) {
    matrix[0][0] = x->x;
    matrix[0][1] = x->y;
    matrix[0][2] = x->z;
    matrix[0][3] = 0.0f;

    matrix[1][0] = y->x;
    matrix[1][1] = y->y;
    matrix[1][2] = y->z;
    matrix[1][3] = 0.0f;

    matrix[2][0] = z->x;
    matrix[2][1] = z->y;
    matrix[2][2] = z->z;
    matrix[2][3] = 0.0f;

    matrix[3][0] = origin->x * N64_SCALE_FACTOR;
    matrix[3][1] = origin->y * N64_SCALE_FACTOR;
    matrix[3][2] = origin->z * N64_SCALE_FACTOR;
    matrix[3][3] = 1.0f;
}


void matrixFromBasisL(Mtx* matrix, struct Vector3* origin, struct Vector3* x, struct Vector3* y, struct Vector3* z) {
    float fmtx[4][4];
    matrixFromBasis(fmtx, origin, x, y, z);
    guMtxF2L(fmtx, matrix);
}

void mulMtxFVecF(MtxF matrix, float *in /*[4]*/, float *out /*[4]*/)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		out[i] = in[0] * matrix[0][i] + //  0, 1, 2, 3
				 in[1] * matrix[1][i] + //  4, 5, 6, 7,
				 in[2] * matrix[2][i] + //  8, 9,10, 9,
				 in[3] * matrix[3][i];	// 12,13,14,15,
	}
}

void mulMtxFMtxF(MtxF matrix_a, MtxF matrix_b, MtxF *out)
{
	int i;
    int j;
    int k;
    MtxF res;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (k = 0; k < 4; k++) {
                sum += matrix_a[i][k] * matrix_a[k][j];
            }
            res[i][j] = sum;
        }
    }
    out = &res;
}