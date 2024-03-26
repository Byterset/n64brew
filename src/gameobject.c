
#include <math.h>
#include <stdlib.h>

#include "util/memory.h"
#include "gameobject.h"
#include "math/vector3.h"

GameObject *GameObject_alloc()
{
	GameObject *obj = (GameObject *)malloc(sizeof(GameObject));
	return obj;
}

/**
 * Initializes a GameObject with the given ID and initial position.
 *
 * @param self      Pointer to the GameObject instance.
 * @param id        The ID of the GameObject.
 * @param initPos   Pointer to the initial position of the GameObject.
 * @return          Pointer to the initialized GameObject.
 */
GameObject *GameObject_init(GameObject *self, int id, struct Vector3 *initPos, struct Vector3 *initRot)
{
	self->id = id;
	vector3Init(&self->transform.position, 0.0f, 0.0f, 0.0f);
	quatIdent(&self->transform.rotation);
	self->modelType = NoneModel;
	self->animState = NULL;
	self->visible = TRUE;
	self->solid = TRUE;

	if (initPos)
	{
		vector3Copy(&self->transform.position, initPos);
	}
	if (initRot)
	{
		quatFromEulerRad(initRot, &self->transform.rotation);
	}
	return self;
}

#ifndef __N64__
#include <stdio.h>

/**
 * Prints the GameObject's information.
 *
 * @param self The GameObject instance to print.
 */
void GameObject_print(GameObject *self)
{
	printf("%s pos=", ModelTypeStrings[self->modelType]);
}
#endif
