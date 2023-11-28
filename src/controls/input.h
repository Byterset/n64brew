
#ifndef INPUT_H
#define INPUT_H

#include "../math/vector2.h"
#include "controller.h"

typedef struct Input
{
	struct Vector2 direction;
	int run;
	int pickup;
	int zoomIn;
	int zoomOut;
	int advanceRenderMode;
	int playMusic;
	int playHonk;
} Input;

void Input_init(Input *self);

void Input_update(Input *self, int controller_id);

#endif /* !INPUT_H */
