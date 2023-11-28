#include "input.h"

void Input_init(Input *self)
{
	vector2Init(&self->direction, 0.0f, 0.0f);
	self->run = FALSE;
	self->pickup = FALSE;
	self->zoomIn = FALSE;
	self->zoomOut = FALSE;
	self->advanceRenderMode = FALSE;
	self->playMusic = FALSE;
	self->playHonk = FALSE;
}

void Input_update(Input *self, int controller_id){
	Input_init(self);
	/* Data reading of controller 1 */
	OSContPad *controller_input = Controller_getControllerData(0);
	if (Controller_getButtonUp(0, START_BUTTON))
	{
		controller_input->button;
		self->advanceRenderMode = TRUE;
	}
	if (Controller_getButton(0, A_BUTTON))
	{
		self->run = TRUE;
	}
	if (Controller_getButtonDown(0, B_BUTTON))
	{
		self->pickup = TRUE;
	}
	if (Controller_getButton(0, Z_TRIG))
	{
		self->zoomIn = TRUE;
	}
	if (Controller_getButton(0, L_TRIG))
	{
		self->zoomIn = TRUE;
	}
	if (Controller_getButton(0, R_TRIG))
	{
		self->zoomOut = TRUE;
	}
	if (Controller_getButtonDown(0, L_CBUTTONS))
	{
		self->playHonk = TRUE;
	}
	if (Controller_getButtonDown(0, R_CBUTTONS))
	{
		self->playMusic = TRUE;
	}

	self->direction.x = -controller_input->stick_x / 61.0F;
	self->direction.y = controller_input->stick_y / 61.0F;
}
