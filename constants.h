
#ifndef CONSTANTS_H
#define CONSTANTS_H

// needs to be at least num characters * num bones per character
#define MAX_ANIM_MESH_PARTS 50
#define MAX_WORLD_OBJECTS 100
// we scale the models up by this much to avoid n64 fixed point precision issues
#define N64_SCALE_FACTOR 30

#define CONST_PI 3.14159265358979323846
#define degToRad(angleInDegrees) ((angleInDegrees)*CONST_PI / 180.0)
#define radToDeg(angleInRadians) ((angleInRadians)*180.0 / CONST_PI)

#ifdef __N64__
#else
#include <stdio.h>
#define debugPrintf(...) printf(__VA_ARGS__)
#endif

#define MEM_HEAP_BYTES 524288

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

#endif /* CONSTANTS_H */
