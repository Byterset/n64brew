#ifndef _TIME_H
#define _TIME_H

#include <ultra64.h>
#include "../constants.h"
extern double gDeltaTimeMS;
extern float gDeltaTimeSec;

#define FIXED_DELTA_TIME ((1.0f + FRAME_SKIP) / 60.0f)

#ifdef __N64__
// this include needs to be here or this macro will break sometimes
#include <PR/os.h>
#include "../n64compat.h"
#define CUR_TIME_MS() OS_CYCLES_TO_USEC(osGetTime()) / 1000.0
#else
#include "../compat.h"
#define CUR_TIME_MS() getElapsedTimeMS()
#endif

void timeUpdateDelta();

#endif