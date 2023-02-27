#ifndef MAIN_H
#define MAIN_H

#ifdef _LANGUAGE_C
#include <nusys.h>
#include "constants.h"

extern NUContData contdata[1]; /* Read data of the controller  */
extern u8 contPattern;         /* The pattern of the connected controller  */
extern s32 gAudioHeapSize;      /* The size of the Audio Heap*/
#if CONSOLE || NU_PERF_BAR
#define GFX_TASKS_PER_MAKEDL 2
#else
#define GFX_TASKS_PER_MAKEDL 1
#endif

#endif /* _LANGUAGE_C */
#endif /* MAIN_H */

void stage00(int);
void initStage00(void);
void makeDL00(void);
void updateGame00(void);