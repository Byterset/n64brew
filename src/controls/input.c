#include "input.h"
#include "../constants.h"

void Input_init(Input *self)
{
	vector2Init(&self->direction, 0.0f, 0.0f);
	self->run = FALSE;
	self->pickup = FALSE;
	self->zoomIn = FALSE;
	self->zoomOut = FALSE;
}
