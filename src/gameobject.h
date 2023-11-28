
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "animation/animation.h"
#include "modeltype.h"
#include "physics/physics.h"
#include "math/rotation.h"

typedef struct GameObject
{
	int id;
	struct Vector3 position;
	EulerDegrees rotation;
	ModelType modelType;
	int subtype;
	AnimationState *animState;
	PhysBody *physBody;
	int visible;
	int solid;
} GameObject;

GameObject *GameObject_alloc();
GameObject *GameObject_init(GameObject *self, int id, struct Vector3 *initPos);

#endif /* !GAMEOBJECT_H */
