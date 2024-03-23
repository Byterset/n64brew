#include "transform.h"

void transform_set_position(Transform *transform, struct Vector3 position) {
    transform->position = position;
}

void transform_translate(Transform *transform, struct Vector3 translation) {
    transform->position.x += translation.x;
    transform->position.y += translation.y;
    transform->position.z += translation.z;
}

void transform_set_rotation(Transform *transform, struct Quaternion rotation) {
    transform->rotation = rotation;
}

void transform_look_at(Transform *transform, struct Vector3 target, struct Vector3 up) {
    struct Vector3 forward;
    Quaternion temp;
    vector3Sub(&target, &transform->position, &forward);
    vector3NormalizeSelf(&forward);
    quatLook(&forward, &up, &temp);
    transform_set_rotation(transform, temp);
}

void transform_rotate(Transform *transform, struct Quaternion rotation) {
    Quaternion temp;
    quatMultiply(&rotation,&transform->rotation, &temp);
    transform_set_rotation(transform, temp);
}

void transform_rotate_euler(Transform *transform, struct Vector3 rotation) {
    Quaternion quat_rotation;
    Quaternion temp;
    struct Vector3 angles = rotation;
    quatEulerAngles(&angles, &quat_rotation);
    quatMultiply(&transform->rotation, &quat_rotation, &temp);
    transform_set_rotation(transform, temp);
}

void transform_set_scale(Transform *transform, struct Vector3 scale) {
    transform->scale = scale;
}

void transform_scale(Transform *transform, struct Vector3 scale) {
    transform->scale.x *= scale.x;
    transform->scale.y *= scale.y;
    transform->scale.z *= scale.z;
}
