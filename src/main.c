#include <ultra64.h>
#include <nusys.h>

// this must come after nusys.h
#include "main.h"
#include "malloc.h"

#include <nualstl_n.h>

#include <PR/os_convert.h>
#ifdef ED64
#include "ed64/ed64io.h"
#endif
#include "graphics/graphic.h"
#include "mem_heap.h"
// #include "aud_heap.h"
#include "trace.h"
// #include "audio/audio.h"
// #include "audio/soundplayer.h"

// #define DEBUGSTARTUP

#ifdef DEBUGSTARTUP
#define DBGPRINT debugPrintfSync
#else
#define DBGPRINT(args...)
#endif

/* The global variable  */
NUContData contdata[1]; /* Read data of 1 controller  */
u8 contPattern;         /* The pattern connected to the controller  */

extern char mem_heap[MEM_HEAP_SIZE];
// extern char aud_heap[AUD_HEAP_SIZE];

// OSSched scheduler;
// u64            scheduleStack[OS_SC_STACKSIZE/8];
// OSPiHandle *gPiHandle;
// OSMesgQueue	*schedulerCommandQueue;

EXTERN_SEGMENT_WITH_BSS(memheap);
// EXTERN_SEGMENT_WITH_BSS(audheap);
EXTERN_SEGMENT_WITH_BSS(trace);

int systemHeapMemoryInit(void) {
  int initHeapResult;
  nuPiReadRom((u32)_memheapSegmentRomStart, _memheapSegmentStart,
              (u32)_memheapSegmentRomEnd - (u32)_memheapSegmentRomStart);

  bzero(_memheapSegmentBssStart,
        _memheapSegmentBssEnd - _memheapSegmentBssStart);

  /* Reserve system heap memory */
  initHeapResult = InitHeap(mem_heap, MEM_HEAP_SIZE);

  if (initHeapResult == -1) {
    die("failed to init heap\n");
  } else {
    debugPrintfSync("init heap success, allocated=%d\n", MEM_HEAP_SIZE);
  }

  if (osGetMemSize() == 0x00800000) {
    debugPrintfSync("have expansion pack\n");
    nuPiReadRom((u32)_traceSegmentRomStart, _traceSegmentStart,
                (u32)_traceSegmentRomEnd - (u32)_traceSegmentRomStart);
    bzero(_traceSegmentBssStart, _traceSegmentBssEnd - _traceSegmentBssStart);

    debugPrintfSync("init trace buffer at %p\n", _traceSegmentStart);
  } else {
    die("expansion pack missing\n");
  }
  return 0;
}


// int audioHeapMemoryInit(void) {

//   int initHeapResult;
//     /* init audio heap as zeroed memory */
//   gAudioHeapBuffer = (u8*)_audheapSegmentRomStart;
//   nuPiReadRom((u32)_audheapSegmentRomStart, _audheapSegmentStart,
//               (u32)_audheapSegmentRomEnd - (u32)_audheapSegmentRomStart);

//   u16* memoryEnd = (u16)_audheapSegmentRomEnd;

//   /* init audio heap as zeroed memory */
//   gAudioHeapBuffer = (u8*)memoryEnd - AUD_HEAP_SIZE;
//   /* Reserve audio heap memory */
//   initHeapResult = InitHeap(aud_heap, AUD_HEAP_SIZE);

//   bzero(gAudioHeapBuffer, AUD_HEAP_SIZE);

//   if (initHeapResult == -1) {
//     die("failed to init heap\n");
//   } else {
//     debugPrintfSync("init heap success, allocated=%d\n", AUD_HEAP_SIZE);
//   }
//   return 0;
// }



extern void* __printfunc;
/*------------------------
        Main
--------------------------*/
void mainproc(void) {
#ifdef ED64
  // start everdrive communication
  evd_init();

  // register libultra error handler
  // ed64RegisterOSErrorHandler();

  // start thread which will catch and log errors
  ed64StartFaultHandlerThread(NU_GFX_TASKMGR_THREAD_PRI);

  // overwrite osSyncPrintf impl
  __printfunc = (void*)ed64PrintFuncImpl;
#endif

  DBGPRINT("hello\n");

  DBGPRINT("systemHeapMemoryInit\n");
  systemHeapMemoryInit();

  /* The initialization of the controller manager  */
  contPattern = nuContInit();

  /* The initialization of audio */
  DBGPRINT("nuAuStlInit\n");


  //audioHeapMemoryInit();
  nuAuStlInit();

  /* The initialization of graphic  */
  // nuGfxInit();
  DBGPRINT("gfxInit\n");
  gfxInit();

  // osCreateScheduler(
  //   &scheduler,
  //   (void *)(scheduleStack + OS_SC_STACKSIZE/8),
  //   71,
  //   1,
  //   1
  // );
  // schedulerCommandQueue = osScGetCmdQ(&scheduler);
  // initAudio(60, NU_AU_MGR_THREAD_PRI);
  // soundPlayerInit();
  /* The initialization for stage00()  */
  DBGPRINT("initStage00\n");
  initStage00();
  /* Register call-back  */
  DBGPRINT("nuGfxFuncSet\n");
  nuGfxFuncSet((NUGfxFunc)stage00);

  /* The screen display is ON */
  DBGPRINT("nuGfxDisplayOn\n");
  nuGfxDisplayOn();
  while (1)
    ;
}

/*-----------------------------------------------------------------------------
  The call-back function

  pendingGfx which is passed from Nusystem as the argument of the call-back
  function is the total of RCP tasks that are currently processing and
  waiting for the process.
-----------------------------------------------------------------------------*/
void stage00(int pendingGfx) {
  float skippedGfxTime, profStartUpdate, profStartFrame, profEndFrame;
  profStartFrame = CUR_TIME_MS();
  /* Provide the display process if n or less RCP tasks are processing or
        waiting for the process.  */
  if (nuScRetraceCounter % FRAME_SKIP == 0) {
    if (pendingGfx < GFX_TASKS_PER_MAKEDL * 2) {
      makeDL00();
      Trace_addEvent(MainMakeDisplayListTraceEvent, profStartFrame,
                     CUR_TIME_MS());
    } else {
      skippedGfxTime = CUR_TIME_MS();
      Trace_addEvent(SkippedGfxTaskTraceEvent, skippedGfxTime,
                     skippedGfxTime + 16.0f);
      // debugPrintfSync("dropped frame %d\n", nuScRetraceCounter / FRAME_SKIP);
    }
  }

  profStartUpdate = CUR_TIME_MS();
  /* The process of game progress  */
  updateGame00();
  profEndFrame = CUR_TIME_MS();
  Trace_addEvent(MainCPUTraceEvent, profStartFrame, profEndFrame);
  Trace_addEvent(MainUpdateTraceEvent, profStartUpdate, profEndFrame);
  profilingAccumulated[MainCPUTraceEvent] += profEndFrame - profStartFrame;
  profilingCounts[MainCPUTraceEvent]++;
}
