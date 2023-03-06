#include "gameutils.h"
#include <math.h>
#include "game.h"
#include "math/vector2.h"
#include "math/vector3.h"

float GameUtils_lerpDegrees(float start, float end, float amount)
{
	float shortestAngle;
	shortestAngle = fmodf((fmodf((end - start), 360) + 540), 360) - 180;
	return start + fmodf((shortestAngle * amount), 360);
}

float GameUtils_fclamp(float x, float lower, float upper)
{
	return x > upper ? upper : x < lower ? lower
										 : x;
}

float GameUtils_rotateTowardsClamped(float from,
									 float to,
									 float maxSpeed // must be positive
)
{
	float shortestAngle;
	shortestAngle = fmodf((fmodf((to - from), 360.0F) + 540.0F), 360.0F) - 180.0F;
	return from +
		   fmodf(GameUtils_fclamp(shortestAngle, -maxSpeed, maxSpeed), 360.0F);
}

void GameUtils_directionFromTopDownAngle(float angle, struct Vector3 *result)
{
	struct Vector2 direction2d;

	vector2ComplexFromAngle(angle, &direction2d);

	result->x = direction2d.x;
	result->y = 0;
	result->z = -direction2d.y;
}

int GameUtils_inWater(GameObject *obj)
{
	struct Vector3 center;
	Game_getObjCenter(obj, &center);

	if (center.y < WATER_HEIGHT)
	{
		return TRUE;
	}
	return FALSE;
}
