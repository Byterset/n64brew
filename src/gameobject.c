
#include <math.h>
#include <stdlib.h>

#include "util/memory.h"
#include "constants.h"
#include "gameobject.h"
#include "math/rotation.h"
#include "math/vector3.h"

GameObject *GameObject_alloc()
{
	GameObject *obj = (GameObject *)malloc(sizeof(GameObject));
	return obj;
}

GameObject *GameObject_init(GameObject *self, int id, struct Vector3 *initPos)
{
	self->id = id;
	vector3Init(&self->position, 0.0f, 0.0f, 0.0f);
	EulerDegrees_origin(&self->rotation);
	self->modelType = NoneModel;
	self->animState = NULL;
	self->visible = TRUE;
	self->solid = TRUE;

	if (initPos)
	{
		vector3Copy(&self->position, initPos);
	}
	return self;
}

#ifndef __N64__
#include <stdio.h>
void GameObject_print(GameObject *self)
{
	printf("%s pos=", ModelTypeStrings[self->modelType]);
	// Vec3d_print(&self->position);
}
#endif
