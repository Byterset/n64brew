
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "animation/animation.h"
#include "modeltype.h"
#include "physics/physics.h"
#include "math/rotation.h"
#include "math/transform.h"


/**
 * @brief Represents a game object in the game world.
 */
typedef struct GameObject
{
	int id; /**< The identifier of the game object. */
	struct Vector3 position; /**< The position of the game object in 3D space. */
	EulerDegrees rotation; /**< The rotation of the game object in Euler angles. */
	ModelType modelType; /**< The type of model used for rendering the game object. */
	int subtype; /**< The subtype of the game object. */
	AnimationState *animState; /**< The animation state of the game object. */
	PhysBody *physBody; /**< The physical body of the game object. */
	int visible; /**< Flag indicating if the game object is visible. */
	int solid; /**< Flag indicating if the game object is solid. */
	struct Transform transform; /**< The transform of the game object. */
} GameObject;

typedef struct LevelData
{
	int id; /**< The identifier of the game object. */
	struct Vector3 position; /**< The position of the game object in 3D space. */
	EulerDegrees rotation; /**< The rotation of the game object in Euler angles. */
	ModelType modelType; /**< The type of model used for rendering the game object. */
	int subtype; /**< The subtype of the game object. */
} LevelData;


GameObject *GameObject_alloc();
GameObject *GameObject_init(GameObject *self, int id, struct Vector3 *initPos, Euler *initRot);

#endif /* !GAMEOBJECT_H */
