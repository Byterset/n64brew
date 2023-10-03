#include <ultra64.h>
#include <sched.h>
#include "defs.h"
#include "main.h"
#include "controls/controller.h"
#include "util/memory.h"
#include "util/rom.h"
#include "util/time.h"
#include "font/font_ext.h"

#include "graphics/graphics.h"
#include "game.h"
#ifdef ED64
#include "ed64/ed64io.h"
#endif
#include "util/mem_heap.h"
#include "util/trace.h"
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
	OSViMode viMode;
	OSIntMask im;
	u8 schedulerMode = OS_VI_NTSC_HPF1;
	u32 xScale;
	xScale = (u32)((SCREEN_WD * XSCALE_MAX) / SCREEN_WD_MAX);
	int framerate = 60;

	//not sure if all this is the best way to set that but it works
	switch (osTvType)
	{
	case 0: // PAL
		schedulerMode = HIGH_RESOLUTION ? 
						(ANTIALIASING ? OS_VI_PAL_HAF1 : OS_VI_PAL_HPF1) : OS_VI_PAL_LPF1;
		schedulerMode = (HIGH_RESOLUTION && HIGH_RESOLUTION_HALF_Y) ? OS_VI_PAL_LPF1: schedulerMode;
		framerate = 50;
		break;
	case 1: // NTSC
		schedulerMode = HIGH_RESOLUTION ? 
						(ANTIALIASING ? OS_VI_NTSC_HAF1 : OS_VI_NTSC_HPF1) : OS_VI_NTSC_LPF1;
		schedulerMode = (HIGH_RESOLUTION && HIGH_RESOLUTION_HALF_Y) ? OS_VI_NTSC_LPF1: schedulerMode;
		break;
	case 2: // MPAL
		schedulerMode = HIGH_RESOLUTION ? 
						(ANTIALIASING ? OS_VI_MPAL_HAF1 : OS_VI_MPAL_HPF1) : OS_VI_MPAL_LPF1;
		schedulerMode = (HIGH_RESOLUTION && HIGH_RESOLUTION_HALF_Y) ? OS_VI_MPAL_LPF1: schedulerMode;
		framerate = 50;
		break;
	}

	viMode = osViModeTable[schedulerMode];

#if HIGH_RESOLUTION_HALF_Y
	viMode = osViModeTable[OS_VI_NTSC_LAN1];
	/* Change width, xScale, and origin */
	im = osSetIntMask(OS_IM_VI);
	viMode.comRegs.width = SCREEN_WD;
	viMode.comRegs.xScale = xScale;
	viMode.fldRegs[0].origin = SCREEN_WD * 2;
	viMode.fldRegs[1].origin = SCREEN_WD * 2;
	(void)osSetIntMask(im);
#endif

	osCreateScheduler(
		&scheduler,
		(void *)(scheduleStack + OS_SC_STACKSIZE / 8),
		SCHEDULER_PRIORITY,
		schedulerMode,
		1);

	/* Set VI */
	osViSetMode(&viMode);
	// when osViSetMode was called these flags were reset to their default values
	osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON | OS_VI_GAMMA_OFF |
						   OS_VI_GAMMA_DITHER_OFF | OS_VI_DIVOT_ON);


	schedulerCommandQueue = osScGetCmdQ(&scheduler);

	osCreateMesgQueue(&gfxFrameMsgQ, gfxFrameMsgBuf, MAX_FRAME_BUFFER_MESGS);
	osScAddClient(&scheduler, &gfxClient, &gfxFrameMsgQ);

	osViSetSpecialFeatures(OS_VI_GAMMA_OFF |
						   OS_VI_GAMMA_DITHER_OFF |
						   OS_VI_DIVOT_OFF |
						   OS_VI_DITHER_FILTER_ON);

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
	controllersClearState();
	initAudio(framerate);
	soundPlayerInit();
	initStage00();
	double lastHonkTime = 0;
	/* MAIN GAME LOOP */

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
				/*Do all the rendering and creation of the DisplayLists here!*/
				// TODO: decide which level to load
				// TODO: generalize render pipeline for multiple levels, move the render code away from stage00
				// TODO: render UI (Intro, Main Menu etc)
				graphicsCreateTask(&gGraphicsTasks[drawBufferIndex], (GraphicsCallback)stage00Render, &drawBufferIndex);
				// graphicsCreateTask(&gGraphicsTasks[drawBufferIndex], NULL, NULL);
				drawBufferIndex = drawBufferIndex ^ 1;
				++pendingGFX;
			}
			else if (renderSkip)
			{
				--renderSkip;
			}

			//read controller Input
			controllersTriggerRead();

			//update the soundplayer logic and the active sounds (actual playing is handled in seperate thread!)
			soundPlayerUpdate();

			//Update Input, Positions, Gamelogic and Physics
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

			break;
		}
	}

	// int materialChunkSize = _material_dataSegmentRomEnd - _material_dataSegmentRomStart;

	// memoryEnd -= materialChunkSize / sizeof(u16);
}
