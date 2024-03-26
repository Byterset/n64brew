
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "animation/animation.h"
#include "modeltype.h"
#include "physics/physics.h"
#include "math/quaternion.h"
#include "math/transform.h"


/**
 * @brief Represents a game object in the game world.
 */
//TODO: Remove legacy pos & rot and replace usage everywhere with SPMatrix Push and Transform info
typedef struct GameObject
{
	int id; /**< The identifier of the game object. */
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
	Vector3 position; /**< The position of the game object in 3D space. */
	Vector3 rotation; /**< The rotation of the game object in Euler angles. */
	ModelType modelType; /**< The type of model used for rendering the game object. */
	int subtype; /**< The subtype of the game object. */
} LevelData;


GameObject *GameObject_alloc();
GameObject *GameObject_init(GameObject *self, int id, Vector3 *initPos, Vector3 *initRot);

#endif /* !GAMEOBJECT_H */
