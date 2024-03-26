#ifndef _MATH_VECTOR2_H
#define _MATH_VECTOR2_H

typedef struct Vector2
{
	float x, y;
} Vector2;

extern Vector2 gRight2;
extern Vector2 gUp2;
extern Vector2 gZeroVec2;
extern Vector2 gOneVec2;

void vector2Init(Vector2 *a, float x, float y);
void vector2ComplexMul(Vector2 *a, Vector2 *b, Vector2 *out);
void vector2ComplexConj(Vector2 *a, Vector2 *out);
int vector2RotateTowards(Vector2 *from, Vector2 *towards, Vector2 *max, Vector2 *out);
void vector2ComplexFromAngle(float radians, Vector2 *out);
void vector2Rotate90(Vector2 *input, Vector2 *out);
float vector2Cross(Vector2 *a, Vector2 *b);
float vector2Dot(Vector2 *a, Vector2 *b);
float vector2MagSqr(Vector2 *a);
float vector2Mag(Vector2 *a);
float vector2DistSqr(Vector2 *a, Vector2 *b);
float vector2Dist(Vector2 *a, Vector2 *b);
void vector2Add(Vector2 *a, Vector2 *b, Vector2 *out);
void vector2Scale(Vector2 *a, float scale, Vector2 *out);
int vector2Normalize(Vector2 *a, Vector2 *out);
void vector2Sub(Vector2 *a, Vector2 *b, Vector2 *out);
void vector2Negate(Vector2 *a, Vector2 *out);
float vector2Angle(Vector2 *a);

void vector2Min(Vector2 *a, Vector2 *b, Vector2 *out);
void vector2Max(Vector2 *a, Vector2 *b, Vector2 *out);

void vector2Lerp(Vector2 *a, Vector2 *b, float lerp, Vector2 *out);

#endif