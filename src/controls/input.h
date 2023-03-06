
#ifndef INPUT_H
#define INPUT_H

#include "../math/vector2.h"

typedef struct Input
{
	struct Vector2 direction;
	int run;
	int pickup;
	int zoomIn;
	int zoomOut;
} Input;

void Input_init(Input *self);

#endif /* !INPUT_H */
