#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "vector3.h"
#include "quaternion.h"

typedef struct Transform
{
    Vector3 position;
    Vector3 scale;
    Quaternion rotation;
} Transform;

extern void transform_set_position (Transform *transform, Vector3 position);

extern void transform_translate (Transform *transform, Vector3 translation);

extern void transform_set_rotation (Transform *transform, struct Quaternion rotation);

extern void transform_set_rotation_euler_degrees(Transform *transform, Vector3 *angles);

extern void transform_look_at (Transform *transform, Vector3 target, Vector3 up);

extern void transform_rotate (Transform *transform, struct Quaternion rotation);

extern void transform_rotate_euler (Transform *transform, Vector3 rotation);

extern void transform_set_scale (Transform *transform, Vector3 scale);

extern void transform_scale (Transform *transform, Vector3 scale);

#endif /* !TRANSFORM_H */