#ifndef _TIME_H
#define _TIME_H

#include <ultra64.h>
#include "../constants.h"
extern float gTimePassed;
extern OSTime gLastTime;

#define FIXED_DELTA_TIME ((1.0f + FRAME_SKIP) / 60.0f)

void timeUpdateDelta();

#endif