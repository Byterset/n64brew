#include <ultra64.h>
#include <sched.h>
#include "defs.h"

// this must come after nusys.h
#include "main.h"
// #include "malloc.h"
#include "util/memory.h"
#include "util/rom.h"
// #include <nualstl_n.h>
#include "util/time.h"
#include <PR/os_convert.h>
#ifdef ED64
#include "ed64/ed64io.h"
#endif
#include "graphics/graphic.h"
#include "mem_heap.h"
#include "trace.h"
#include "audio/audio.h"
#include "audio/soundplayer.h"
#include "../build/src/audio/clips.h"

// #define DEBUGSTARTUP

#ifdef DEBUGSTARTUP
#define DBGPRINT debugPrintfSync
#else
#define DBGPRINT(args...)
#endif

/* The global variable  */
NUContData contdata[1]; /* Read data of 1 controller  */
u8 contPattern;			/* The pattern connected to the controller  */

OSPiHandle *gPiHandle;
static OSMesg PiMessages[DMA_QUEUE_SIZE];
static OSMesgQueue PiMessageQ;

extern char mem_heap[MEM_HEAP_SIZE];

// extern char aud_heap[AUD_HEAP_SIZE];

u64 mainStack[STACKSIZEBYTES / sizeof(u64)];

OSThread initThread;
u64 initThreadStack[STACKSIZEBYTES / sizeof(u64)];

OSThread gameThread;
u64 gameThreadStack[STACKSIZEBYTES / sizeof(u64)];

OSMesgQueue gfxFrameMsgQ;
static OSMesg gfxFrameMsgBuf[MAX_FRAME_BUFFER_MESGS];
static OSScClient gfxClient;

OSSched scheduler;
u64 scheduleStack[OS_SC_STACKSIZE / 8];
OSMesgQueue *schedulerCommandQueue;

void initProc(void *arg);
void gameProc(void *arg);

EXTERN_SEGMENT_WITH_BSS(memheap);
EXTERN_SEGMENT_WITH_BSS(trace);

void systemHeapMemoryInit(void *heapend)
{
	int initHeapResult;
	romCopy(_memheapSegmentRomStart, _memheapSegmentStart, (u32)_memheapSegmentRomEnd - (u32)_memheapSegmentRomStart);
	heapInit(_memheapSegmentBssStart, _memheapSegmentBssEnd);

	if (osGetMemSize() == 0x00800000)
	{
		debugPrintfSync("have expansion pack\n");
		romCopy(_traceSegmentRomStart, _traceSegmentStart, (u32)_traceSegmentRomEnd - (u32)_traceSegmentRomStart);
		bzero(_traceSegmentBssStart, _traceSegmentBssEnd - _traceSegmentBssStart);

		debugPrintfSync("init trace buffer at %p\n", _traceSegmentStart);
	}
	else
	{
		die("expansion pack missing\n");
	}
}

extern void *__printfunc;

/*------------------------
		Main
Initialize the OS and start the idle thread
--------------------------*/
void mainproc(void *arg)
{
	osInitialize();
	gPiHandle = osCartRomInit();
#ifdef ED64
	// start everdrive communication
	evd_init();

	// register libultra error handler
	// ed64RegisterOSErrorHandler();

	// start thread which will catch and log errors
	ed64StartFaultHandlerThread(NU_GFX_TASKMGR_THREAD_PRI);

	// overwrite osSyncPrintf impl
	__printfunc = (void *)ed64PrintFuncImpl;
#endif

	//create the initial thread running the initProc. This will set up the memory and other game relevant threads
	osCreateThread(
		&initThread,
		1,
		initProc,
		arg,
		(void *)(initThreadStack + (STACKSIZEBYTES / sizeof(u64))),
		(OSPri)INIT_PRIORITY);
	osStartThread(&initThread);

	// TODO: re-implement this in the appropriate place without using nusys

	// DBGPRINT("gfxInit\n");
	// gfxInit();

	// /* The initialization for stage00()  */
	// DBGPRINT("initStage00\n");
	// initStage00();


	// /* Register call-back  */
	// DBGPRINT("nuGfxFuncSet\n");
	// nuGfxFuncSet((NUGfxFunc)stage00);


	// /* The screen display is ON */
	// DBGPRINT("nuGfxDisplayOn\n");
	// nuGfxDisplayOn();

}

/*------------------------
		Main
Initialize the OS and start the idle thread
--------------------------*/
void initProc(void *arg)
{
	//Create Parallel Interface Manager
	//http://n64devkit.square7.ch/n64man/os/osCreatePiManager.htm
	osCreatePiManager(
		(OSPri)OS_PRIORITY_PIMGR,
		&PiMessageQ,
		PiMessages,
		DMA_QUEUE_SIZE);

	//create and start game thread. This is the main thread that will handle graphics and gameplay
	//at the start of the gameProc memory will be allocated and a seperate audio thread will be started
	osCreateThread(
		&gameThread,
		6,
		gameProc,
		0,
		gameThreadStack + (STACKSIZEBYTES / sizeof(u64)),
		(OSPri)GAME_PRIORITY);
	osStartThread(&gameThread);

	//set the priority of the init/idle thread to the lowest value so it only runs an empty loop
	//if there is nothing else to do
	osSetThreadPri(NULL, 0);
	for (;;)
		;
}

extern OSMesgQueue dmaMessageQ;

extern char _memheapSegmentRomStart[];

void gameProc(void *arg)
{
	u8 schedulerMode = OS_VI_NTSC_HPF1;
	int framerate = 60;

	switch (osTvType)
	{
	case 0: // PAL
		schedulerMode = HIGH_RESOLUTION ? OS_VI_PAL_HPF1 : OS_VI_PAL_LPF1;
		framerate = 50;
		break;
	case 1: // NTSC
		schedulerMode = HIGH_RESOLUTION ? OS_VI_NTSC_HPF1 : OS_VI_NTSC_LPF1;
		break;
	case 2: // MPAL
		schedulerMode = HIGH_RESOLUTION ? OS_VI_MPAL_HPF1 : OS_VI_MPAL_LPF1;
		framerate = 50;
		break;
	}

	osCreateScheduler(
		&scheduler,
		(void *)(scheduleStack + OS_SC_STACKSIZE / 8),
		SCHEDULER_PRIORITY,
		schedulerMode,
		1);

	schedulerCommandQueue = osScGetCmdQ(&scheduler);

	osCreateMesgQueue(&gfxFrameMsgQ, gfxFrameMsgBuf, MAX_FRAME_BUFFER_MESGS);
	osScAddClient(&scheduler, &gfxClient, &gfxFrameMsgQ);

	osViSetSpecialFeatures(OS_VI_GAMMA_OFF |
						   OS_VI_GAMMA_DITHER_OFF |
						   OS_VI_DIVOT_OFF |
						   OS_VI_DITHER_FILTER_OFF);

	u32 pendingGFX = 0;
	u32 drawBufferIndex = 0;
	int frameControl = 0;

	u16 *memoryEnd = graphicsLayoutScreenBuffers((u16 *)PHYS_TO_K0(osMemSize));

	gAudioHeapBuffer = (u8 *)memoryEnd - AUDIO_HEAP_SIZE;
	zeroMemory(gAudioHeapBuffer, AUDIO_HEAP_SIZE);
	memoryEnd = (u16 *)gAudioHeapBuffer;
	heapInit(_memheapSegmentStart, memoryEnd);
	romInit();

	initAudio(framerate);
	soundPlayerInit();
	soundPlayerPlay(SOUNDS_JAH_SPOOKS, 1.0f, 1.0f, NULL);
	soundPlayerPlay(SOUNDS_HONK_1, 1.0f, 1.0f, NULL);
	while (1)
	{
		OSScMsg *msg = NULL;
		osRecvMesg(&gfxFrameMsgQ, (OSMesg *)&msg, OS_MESG_BLOCK);
		switch (msg->type)
		{
		case (OS_SC_RETRACE_MSG):
			// control the framerate
			frameControl = (frameControl + 1) % (FRAME_SKIP + 1);
			if (frameControl != 0)
			{
				break;
			}
			static int renderSkip = 1;

			if (pendingGFX < 2 && !renderSkip)
			{
				//graphicsCreateTask(&gGraphicsTasks[drawBufferIndex], (GraphicsCallback)sceneRender, &gScene);
				drawBufferIndex = drawBufferIndex ^ 1;
				++pendingGFX;
			}
			else if (renderSkip)
			{
				--renderSkip;
			}
			soundPlayerUpdate();

			timeUpdateDelta();
			break;
		case (OS_SC_DONE_MSG):
			--pendingGFX;
			break;
		case (OS_SC_PRE_NMI_MSG):
			pendingGFX += 2;
			break;
		case SIMPLE_CONTROLLER_MSG:
			// controllersUpdate();
			break;
		}
	}

	// int materialChunkSize = _material_dataSegmentRomEnd - _material_dataSegmentRomStart;

	// memoryEnd -= materialChunkSize / sizeof(u16);
}


//TODO: rewrite this in non-nusys way
/*-----------------------------------------------------------------------------
  The call-back function

  pendingGfx which is passed from Nusystem as the argument of the call-back
  function is the total of RCP tasks that are currently processing and
  waiting for the process.
-----------------------------------------------------------------------------*/
void stage00(int pendingGfx)
{
	float skippedGfxTime, profStartUpdate, profStartFrame, profEndFrame;
	profStartFrame = CUR_TIME_MS();
	/* Provide the display process if n or less RCP tasks are processing or
		  waiting for the process.  */
	if (nuScRetraceCounter % FRAME_SKIP == 0)
	{
		if (pendingGfx < GFX_TASKS_PER_MAKEDL * 2)
		{
			makeDL00();
			Trace_addEvent(MainMakeDisplayListTraceEvent, profStartFrame,
						   CUR_TIME_MS());
		}
		else
		{
			skippedGfxTime = CUR_TIME_MS();
			Trace_addEvent(SkippedGfxTaskTraceEvent, skippedGfxTime,
						   skippedGfxTime + 16.0f);
			// debugPrintfSync("dropped frame %d\n", nuScRetraceCounter / FRAME_SKIP);
		}
	}

	profStartUpdate = CUR_TIME_MS();
	/* The process of game progress  */
	updateGame00();
	// soundPlayerUpdate();
	profEndFrame = CUR_TIME_MS();
	Trace_addEvent(MainCPUTraceEvent, profStartFrame, profEndFrame);
	Trace_addEvent(MainUpdateTraceEvent, profStartUpdate, profEndFrame);
	profilingAccumulated[MainCPUTraceEvent] += profEndFrame - profStartFrame;
	profilingCounts[MainCPUTraceEvent]++;
}
