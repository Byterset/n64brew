#ifndef MAIN_H
#define MAIN_H

#ifdef _LANGUAGE_C
#include <ultra64.h>
#include "constants.h"
#include "graphics/renderstate.h"
#include "graphics/graphics.h"

// extern NUContData contdata[1]; /* Read data of the controller  */
extern u8 contPattern; /* The pattern of the connected controller  */

#endif /* _LANGUAGE_C */
#endif /* MAIN_H */

void initStage00(void);
void stage00Render(u32 *data, struct RenderState *renderState, struct GraphicsTask *task);
// void makeDL00(struct RenderState *renderState);
void updateGame00(void);