#include "transform.h"

void transform_set_position(Transform *transform, Vector3 position) {
    transform->position = position;
}

void transform_translate(Transform *transform, Vector3 translation) {
    transform->position.x += translation.x;
    transform->position.y += translation.y;
    transform->position.z += translation.z;
}

void transform_set_rotation(Transform *transform, struct Quaternion rotation) {
    transform->rotation = rotation;
}

void transform_set_rotation_euler_degrees(Transform *transform, Vector3 *angles){
    Quaternion quat;
    quatFromEulerRad(angles, &quat);
    transform_set_rotation(transform, quat);
}

void transform_look_at(Transform *transform, Vector3 target, Vector3 up) {
    Vector3 forward;
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

void transform_rotate_euler(Transform *transform, Vector3 rotation) {
    Quaternion quat_rotation;
    Quaternion temp;
    Vector3 angles = rotation;
    quatFromEulerRad(&angles, &quat_rotation);
    quatMultiply(&transform->rotation, &quat_rotation, &temp);
    transform_set_rotation(transform, temp);
}

void transform_set_scale(Transform *transform, Vector3 scale) {
    transform->scale = scale;
}

void transform_scale(Transform *transform, Vector3 scale) {
    transform->scale.x *= scale.x;
    transform->scale.y *= scale.y;
    transform->scale.z *= scale.z;
}
