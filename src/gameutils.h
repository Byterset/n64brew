
#ifndef GAMEUTILS_H
#define GAMEUTILS_H
#include "constants.h"
#include "gameobject.h"
#include "math/vector3.h"

float GameUtils_lerpDegrees(float start, float end, float amount);
float GameUtils_fclamp(float x, float lower, float upper);
float GameUtils_rotateTowardsClamped(float from,
									 float to,
									 float maxSpeed // must be positive
);

void GameUtils_directionFromTopDownAngle(float angle, struct Vector3 *result);

int GameUtils_inWater(GameObject *obj);

#endif /* !GAMEUTILS_H */
