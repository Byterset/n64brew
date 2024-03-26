#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "vector3.h"
#include "quaternion.h"

typedef struct Transform
{
    struct Vector3 position;
    struct Vector3 scale;
    Quaternion rotation;
} Transform;

extern void transform_set_position (Transform *transform, struct Vector3 position);

extern void transform_translate (Transform *transform, struct Vector3 translation);

extern void transform_set_rotation (Transform *transform, struct Quaternion rotation);

extern void transform_set_rotation_euler_degrees(Transform *transform, struct Vector3 *angles);

extern void transform_look_at (Transform *transform, struct Vector3 target, struct Vector3 up);

extern void transform_rotate (Transform *transform, struct Quaternion rotation);

extern void transform_rotate_euler (Transform *transform, struct Vector3 rotation);

extern void transform_set_scale (Transform *transform, struct Vector3 scale);

extern void transform_scale (Transform *transform, struct Vector3 scale);

#endif /* !TRANSFORM_H */