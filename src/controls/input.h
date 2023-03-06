
#ifndef INPUT_H
#define INPUT_H

#include "../math/vec2d.h"

typedef struct Input
{
	Vec2d direction;
	int run;
	int pickup;
	int zoomIn;
	int zoomOut;
} Input;

void Input_init(Input *self);

#endif /* !INPUT_H */
