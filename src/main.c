#include <ultra64.h>
#include <sched.h>
#include "defs.h"
#include "main.h"
#include "controls/controller.h"
#include "util/memory.h"
#include "util/rom.h"
#include "util/time.h"

#include "graphics/graphics.h"
#include "game.h"
#ifdef ED64
#include "ed64/ed64io.h"
#endif
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
// NUContData contdata[1]; /* Read data of 1 controller  */
u8 contPattern; /* The pattern connected to the controller  */

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

void traceInit(void)
{

	if (osGetMemSize() == 0x00800000)
	{
		// debugPrintfSync("have expansion pack\n");
		romCopy(_traceSegmentRomStart, _traceSegmentStart, (u32)_traceSegmentRomEnd - (u32)_traceSegmentRomStart);
		bzero(_traceSegmentBssStart, _traceSegmentBssEnd - _traceSegmentBssStart);

		// debugPrintfSync("init trace buffer at %p\n", _traceSegmentStart);
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

	// create the initial thread running the initProc. This will set up the memory and other game relevant threads
	osCreateThread(
		&initThread,
		1,
		initProc,
		arg,
		(void *)(initThreadStack + (STACKSIZEBYTES / sizeof(u64))),
		(OSPri)INIT_PRIORITY);
	osStartThread(&initThread);
}

/*------------------------
		Main
Initialize the OS and start the idle thread
--------------------------*/
void initProc(void *arg)
{
	// Create Parallel Interface Manager
	// http://n64devkit.square7.ch/n64man/os/osCreatePiManager.htm
	osCreatePiManager(
		(OSPri)OS_PRIORITY_PIMGR,
		&PiMessageQ,
		PiMessages,
		DMA_QUEUE_SIZE);

	// create and start game thread. This is the main thread that will handle graphics and gameplay
	// at the start of the gameProc memory will be allocated and a seperate audio thread will be started
	osCreateThread(
		&gameThread,
		6,
		gameProc,
		0,
		gameThreadStack + (STACKSIZEBYTES / sizeof(u64)),
		(OSPri)GAME_PRIORITY);
	osStartThread(&gameThread);

	// set the priority of the init/idle thread to the lowest value so it only runs an empty loop
	// if there is nothing else to do
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

	controllersInit();
	initAudio(framerate);
	soundPlayerInit();
	initStage00();
	double lastHonkTime = 0;
	/* MAIN GAME LOOP */
	// soundPlayerPlay(SOUNDS_HONK_1, 1.0f, 1.0f, NULL);
	while (1)
	{
		// double currTime = CUR_TIME_MS();
		// if(currTime - lastHonkTime > 1000){
		// 	soundPlayerPlay(SOUNDS_HONK_1, 1.0f, 1.0f, NULL);
		// 	lastHonkTime = currTime;
		// }
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
				
				graphicsCreateTask(&gGraphicsTasks[drawBufferIndex], (GraphicsCallback)stage00Render, &drawBufferIndex);
				//graphicsCreateTask(&gGraphicsTasks[drawBufferIndex], NULL, NULL);
				drawBufferIndex = drawBufferIndex ^ 1;
				++pendingGFX;
			}
			else if (renderSkip)
			{
				--renderSkip;
			}

			controllersTriggerRead();

			soundPlayerUpdate();
			updateGame00();
			
			timeUpdateDelta();

			break;
		case (OS_SC_DONE_MSG):
			--pendingGFX;
			break;
		case (OS_SC_PRE_NMI_MSG):
			pendingGFX += 2;
			break;
		case SIMPLE_CONTROLLER_MSG:
			controllersUpdate();
			break;
		default:
			// double currTime = CUR_TIME_MS();
			// if (currTime - lastHonkTime > 1000)
			// {
			// 	soundPlayerPlay(SOUNDS_HONK_1, 1.0f, 0.2f, NULL);
			// 	lastHonkTime = currTime;
			// }
			
			// break;
		}
		
	}

	// int materialChunkSize = _material_dataSegmentRomEnd - _material_dataSegmentRomStart;

	// memoryEnd -= materialChunkSize / sizeof(u16);
}
