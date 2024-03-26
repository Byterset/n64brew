
#include "vector3.h"
#include "mathf.h"
#include <math.h>
#include <ultra64.h>

Vector3 gRight = {1.0f, 0.0f, 0.0f};
Vector3 gUp = {0.0f, 1.0f, 0.0f};
Vector3 gForward = {0.0f, 0.0f, 1.0f};
Vector3 gZeroVec = {0.0f, 0.0f, 0.0f};
Vector3 gOneVec = {1.0f, 1.0f, 1.0f};

void vector3Init(Vector3 *self, float x, float y, float z){
	self->x = x;
	self->y = y;
	self->z = z;
}

void vector3Set(Vector3 *self, float x, float y, float z){
	self->x = x;
	self->y = y;
	self->z = z;
}

void vector3Copy(Vector3 *self, Vector3 *other){
	self->x = other->x;
	self->y = other->y;
	self->z = other->z;
}

void vector3Abs(Vector3 *in, Vector3 *out)
{
	out->x = fabsf(in->x);
	out->y = fabsf(in->y);
	out->z = fabsf(in->z);
}

void vector3Negate(Vector3 *in, Vector3 *out)
{
	out->x = -in->x;
	out->y = -in->y;
	out->z = -in->z;
}

void vector3Scale(Vector3 *in, Vector3 *out, float scale)
{
	out->x = in->x * scale;
	out->y = in->y * scale;
	out->z = in->z * scale;
}

void vector3ScaleSelf(Vector3 *self, float scale)
{
	self->x *= scale;
	self->y *= scale;
	self->z *= scale;
}

void vector3DivScalar(Vector3 *self, float scalar){

    if (scalar != 0.0f) {
        self->x /= scalar;
        self->y /= scalar;
        self->z /= scalar;
    } else {
        // Handle division by zero
        self->x = -1;
        self->y = -1;
        self->z = -1;
    }
}

void vector3Add(Vector3 *a, Vector3 *b, Vector3 *out)
{
	out->x = a->x + b->x;
	out->y = a->y + b->y;
	out->z = a->z + b->z;
}

void vector3AddToSelf(Vector3 *self, Vector3 *other)
{
	self->x += other->x;
	self->y += other->y;
	self->z += other->z;
}

void vector3AddScaled(Vector3 *a, Vector3 *normal, float scale, Vector3 *out)
{
	out->x = a->x + normal->x * scale;
	out->y = a->y + normal->y * scale;
	out->z = a->z + normal->z * scale;
}

void vector3Sub(Vector3 *a, Vector3 *b, Vector3 *out)
{
	out->x = a->x - b->x;
	out->y = a->y - b->y;
	out->z = a->z - b->z;
}

void vector3SubFromSelf(Vector3 *self, Vector3 *other)
{
	self->x -= other->x;
	self->y -= other->y;
	self->z -= other->z;
}

void vector3Multiply(Vector3 *a, Vector3 *b, Vector3 *out)
{
	out->x = a->x * b->x;
	out->y = a->y * b->y;
	out->z = a->z * b->z;
}

void vector3Normalize(Vector3 *in, Vector3 *out)
{
	float denom = in->x * in->x + in->y * in->y + in->z * in->z;

	if (denom == 0.0f)
	{
		out->x = 0.0f;
		out->y = 0.0f;
		out->z = 0.0f;
	}
	else
	{
		float invSqrt = 1.0f / sqrtf(denom);
		vector3Scale(in, out, invSqrt);
	}
}

void vector3NormalizeSelf(Vector3 *self)
{
	float magnitude;
	if (self->x == 0.0F && self->y == 0.0F && self->z == 0.0F)
	{
		return;
	}
	magnitude = sqrtf(self->x * self->x + self->y * self->y + self->z * self->z);
	self->x /= magnitude;
	self->y /= magnitude;
	self->z /= magnitude;
}

void vector3Lerp(Vector3 *a, Vector3 *b, float t, Vector3 *out)
{
	// float tFlip = 1.0f - t;
	// out->x = a->x * tFlip + b->x * t;
	// out->y = a->y * tFlip + b->y * t;
	// out->z = a->z * tFlip + b->z * t;

	out->x += (b->x - a->x) * t;
	out->y += (b->y - a->y) * t;
	out->z += (b->z - a->z) * t;
}

float vector3Dot(Vector3 *a, Vector3 *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

float vector3MagSqrd(Vector3 *a)
{
	return a->x * a->x + a->y * a->y + a->z * a->z;
}

float vector3Mag(Vector3 *a)
{
	return sqrtf(a->x * a->x + a->y * a->y + a->z * a->z);
}

float vector3DistSqrd(Vector3 *a, Vector3 *b)
{
	float x = a->x - b->x;
	float y = a->y - b->y;
	float z = a->z - b->z;

	return x * x + y * y + z * z;
}

float vector3Dist(Vector3 *a, Vector3 *b){
	float x = a->x - b->x;
	float y = a->y - b->y;
	float z = a->z - b->z;

	return sqrtf(x * x + y * y + z * z);
}

void vector3Cross(Vector3 *a, Vector3 *b, Vector3 *out)
{
	out->x = a->y * b->z - a->z * b->y;
	out->y = a->z * b->x - a->x * b->z;
	out->z = a->x * b->y - a->y * b->x;
}

void vector3Perp(Vector3 *a, Vector3 *out)
{
	if (fabsf(a->x) > fabsf(a->z))
	{
		vector3Cross(a, &gForward, out);
	}
	else
	{
		vector3Cross(a, &gRight, out);
	}
}

void vector3Project(Vector3 *in, Vector3 *normal, Vector3 *out)
{
	float mag = vector3Dot(in, normal);
	out->x = normal->x * mag;
	out->y = normal->y * mag;
	out->z = normal->z * mag;
}

void vector3ProjectPlane(Vector3 *in, Vector3 *normal, Vector3 *out)
{
	float mag = vector3Dot(in, normal);
	out->x = in->x - normal->x * mag;
	out->y = in->y - normal->y * mag;
	out->z = in->z - normal->z * mag;
}

int vector3MoveTowards(Vector3 *from, Vector3 *towards, float maxDistance, Vector3 *out)
{
	float distance = vector3DistSqrd(from, towards);

	if (distance < maxDistance * maxDistance)
	{
		*out = *towards;
		return 1;
	}
	else
	{
		float scale = maxDistance / sqrtf(distance);
		out->x = (towards->x - from->x) * scale + from->x;
		out->y = (towards->y - from->y) * scale + from->y;
		out->z = (towards->z - from->z) * scale + from->z;
		return 0;
	}
}

void vector3TripleProduct(Vector3 *a, Vector3 *b, Vector3 *c, Vector3 *output)
{
	vector3Scale(b, output, vector3Dot(a, c));
	vector3AddScaled(output, a, -vector3Dot(b, c), output);
}

void vector3DirectionTo(Vector3 *self, Vector3 *other, Vector3 *result){
	vector3Copy(result, other);
	vector3SubFromSelf(result, self);
	vector3NormalizeSelf(result);
}

void vector3Max(Vector3 *a, Vector3 *b, Vector3 *out)
{
	out->x = MAX(a->x, b->x);
	out->y = MAX(a->y, b->y);
	out->z = MAX(a->z, b->z);
}

void vector3Min(Vector3 *a, Vector3 *b, Vector3 *out)
{
	out->x = MIN(a->x, b->x);
	out->y = MIN(a->y, b->y);
	out->z = MIN(a->z, b->z);
}

int vector3IsZero(Vector3 *vector)
{
	return vector->x == 0.0f && vector->y == 0.0f && vector->z == 0.0f;
}

void vector3ToVector3u8(Vector3 *input, Vector3u8 *output)
{
	output->x = floatTos8norm(input->x);
	output->y = floatTos8norm(input->y);
	output->z = floatTos8norm(input->z);
}

float vector3EvalBarycentric1D(Vector3 *baryCoords, float a, float b, float c)
{
	return baryCoords->x * a + baryCoords->y * b + baryCoords->z * c;
}

char *vector3toString(Vector3 *self, char *buffer)
{
	sprintf(buffer, "{x:%.3f, y:%.3f, z:%.3f}", self->x, self->y, self->z);
	return buffer;
}