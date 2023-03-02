
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX_WORLD_OBJECTS 100
// we scale the models up by this much to avoid n64 fixed point precision issues
#define N64_SCALE_FACTOR 30
#define DEFAULT_FOVY 15.0f
#define DEFAULT_NEARPLANE 100
#define DEFAULT_FARPLANE 4000
// enable high resolution rendering, essentially double x and y resolution
// looks really crisp but there is a tradeoff where the FRAME_SKIP is set to 2 and the framerate practically halves
#define HIGH_RESOLUTION 0
// if enabled will limit y resolution, this gets mitigated by adjusting aspect ratio
#define HIGH_RESOLUTION_HALF_Y 0
// without antialiasing gaps between floor tiles become apparent
// TODO: think of a way to mask this issue or do not partition the ground
#define ANTIALIASING 1 

#define CONSOLE 1
#define NU_PERF_BAR 0

#ifdef RELEASE
#undef CONSOLE
#undef NU_PERF_BAR
#endif

// default water height.
// TODO: make water level-specific or better create water-body system to place bodies of water anywhere in the scene
#define WATER_HEIGHT -70.0

// who doesn't like pie?
#define CONST_PI 3.14159265358979323846
#define degToRad(angleInDegrees) ((angleInDegrees)*CONST_PI / 180.0)
#define radToDeg(angleInRadians) ((angleInRadians)*180.0 / CONST_PI)

//basically if the high resolution mode is active, render less frames
// TODO: switch based on region (VSYNC_FPS should be 25 and 50 for PAL systems)
#if HIGH_RESOLUTION
#if HIGH_RESOLUTION_HALF_Y
#define VSYNC_FPS 30
#define FRAME_SKIP 2
#else
#define VSYNC_FPS 30
#define FRAME_SKIP 2
// #define VSYNC_FPS 15
// #define FRAME_SKIP 4
#endif
#else  // low resolution
#define VSYNC_FPS 60
#define FRAME_SKIP 1
#endif

// dumb
#ifdef __N64__
#ifdef __cplusplus // only for clang linter
#include "ed64/ed64io_usb.h"
#endif
#else
#include <stdio.h>
#endif

#ifdef __N64__
#include <PR/gu.h>
#ifndef RAND
#define RAND(x) (guRandom() % (x)) /* random number between 0 to x */
#endif
#else
#define RAND(x) (rand() % (x)) /* random number between 0 to x */
#endif

#ifdef __N64__
// this include needs to be here or this macro will break sometimes
#include <PR/os.h>
#include "n64compat.h"
#define CUR_TIME_MS() OS_CYCLES_TO_USEC(osGetTime()) / 1000.0
#else
#include "compat.h"
#define CUR_TIME_MS() getElapsedTimeMS()
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef CLAMP
#define CLAMP(x, low, high) \
	(((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif
#define INTFLOOR(x) (int)x
#define INTCEIL(x) ((int)x) + 1

#ifdef __N64__
// hard coded arch-dependent values for n64
#include "mathdef.h"

#ifndef FLT_EPSILON
#define FLT_EPSILON 0.0000001192
#endif

#ifndef FLT_MAX
#define FLT_MAX 3.402823e+38
#endif

#endif

#ifdef __N64__
#ifdef ED64
#include "ed64/ed64io.h"
#define debugPrintf ed64Printf
#else
#define debugPrintf(args...)
#endif
#else
#define debugPrintf printf
#endif

#ifdef __N64__

#ifdef ED64
#define debugPrintfSync ed64PrintfSync
#else
#define debugPrintfSync(args...)
#endif
#else
#define debugPrintfSync printf
#endif

#ifdef __N64__
#define die(msg)                                                        \
	debugPrintfSync("died: \"%s\" in %s at %s:%d\n", msg, __FUNCTION__, \
					__FILE__, __LINE__);                                \
	while (TRUE)                                                        \
	{                                                                   \
	}

#else

#define die(msg)                                                         \
	printf("died: \"%s\" in %s at %s:%d\n", msg, __FUNCTION__, __FILE__, \
		   __LINE__);                                                    \
	assert(FALSE);
#endif

#define beacon() debugPrintfSync("at %d in %s\n", __LINE__, __FUNCTION__);

#ifdef __N64__
#define invariant(expression)                                              \
	if (!(expression))                                                     \
	{                                                                      \
		debugPrintfSync("assertion failed in %s at %s:%d\n", __FUNCTION__, \
						__FILE__, __LINE__);                               \
		{                                                                  \
			u64 intentionallyCrash;                                        \
			intentionallyCrash = *(u64 *)1;                                \
		}                                                                  \
	}
#else
#define invariant(expression) assert(expression)
#endif

#define EXTERN_SEGMENT(name)                                    \
	extern char _##name##SegmentStart[], _##name##SegmentEnd[], \
		_##name##SegmentRomStart[], _##name##SegmentRomEnd[]

#define EXTERN_SEGMENT_WITH_BSS(name)                           \
	extern char _##name##SegmentStart[], _##name##SegmentEnd[], \
		_##name##SegmentRomStart[], _##name##SegmentRomEnd[],   \
		_##name##SegmentBssStart[], _##name##SegmentBssEnd[]

#endif /* CONSTANTS_H */
