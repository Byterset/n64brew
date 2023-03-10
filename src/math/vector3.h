
#ifndef _VECTOR3_H
#define _VECTOR3_H

struct Vector3
{
	float x, y, z;
};

struct Vector3u8
{
	char x, y, z;
};

extern struct Vector3 gRight;
extern struct Vector3 gUp;
extern struct Vector3 gForward;
extern struct Vector3 gZeroVec;
extern struct Vector3 gOneVec;

#define VECTOR3_AS_ARRAY(vector) ((float *)(vector))

void vector3Init(struct Vector3 *self, float x, float y, float z);
void vector3Set(struct Vector3 *self, float x, float y, float z);
void vector3Copy(struct Vector3 *self, struct Vector3 *other);
void vector3Abs(struct Vector3 *in, struct Vector3 *out);
void vector3Negate(struct Vector3 *in, struct Vector3 *out);
void vector3Scale(struct Vector3 *in, struct Vector3 *out, float scale);
void vector3ScaleSelf(struct Vector3 *self, float scale);
void vector3DivScalar(struct Vector3 *self, float scalar);
void vector3Add(struct Vector3 *a, struct Vector3 *b, struct Vector3 *out);
void vector3AddToSelf(struct Vector3 *self, struct Vector3 *other);
void vector3AddScaled(struct Vector3 *a, struct Vector3 *normal, float scale, struct Vector3 *out);
void vector3Sub(struct Vector3 *a, struct Vector3 *b, struct Vector3 *out);
void vector3SubFromSelf(struct Vector3 *self, struct Vector3 *other);
void vector3Multiply(struct Vector3 *a, struct Vector3 *b, struct Vector3 *out);
void vector3Normalize(struct Vector3 *in, struct Vector3 *out);
void vector3NormalizeSelf(struct Vector3 *self);
void vector3Lerp(struct Vector3 *a, struct Vector3 *b, float t, struct Vector3 *out);
float vector3Dot(struct Vector3 *a, struct Vector3 *b);
float vector3MagSqrd(struct Vector3 *a);
float vector3Mag(struct Vector3 *a);
float vector3DistSqrd(struct Vector3 *a, struct Vector3 *b);
float vector3Dist(struct Vector3 *a, struct Vector3 *b);
void vector3Cross(struct Vector3 *a, struct Vector3 *b, struct Vector3 *out);
void vector3Perp(struct Vector3 *a, struct Vector3 *out);
void vector3Project(struct Vector3 *in, struct Vector3 *normal, struct Vector3 *out);
void vector3ProjectPlane(struct Vector3 *in, struct Vector3 *normal, struct Vector3 *out);
int vector3MoveTowards(struct Vector3 *from, struct Vector3 *towards, float maxDistance, struct Vector3 *out);
void vector3TripleProduct(struct Vector3 *a, struct Vector3 *b, struct Vector3 *c, struct Vector3 *output);
void vector3DirectionTo(struct Vector3 *self, struct Vector3 *other, struct Vector3 *result);
void vector3Max(struct Vector3 *a, struct Vector3 *b, struct Vector3 *out);
void vector3Min(struct Vector3 *a, struct Vector3 *b, struct Vector3 *out);

int vector3IsZero(struct Vector3 *vector);

void vector3ToVector3u8(struct Vector3 *input, struct Vector3u8 *output);

float vector3EvalBarycentric1D(struct Vector3 *baryCoords, float a, float b, float c);
char *vector3toString(struct Vector3 *self, char *buffer);
#endif