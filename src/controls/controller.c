
#include "controller.h"
#include "../defs.h"
#include "../util/memory.h"
#include <sched.h>

#define MAX_PLAYERS 4

static u8 validcontrollers = 0;
static u8 cntrlReadInProg = 0;

static OSContStatus gControllerStatus[MAX_PLAYERS];
static OSContPad gControllerData[MAX_PLAYERS];
OSScMsg gControllerMessage;
static u16 gControllerLastButton[MAX_PLAYERS];
static enum ControllerDirection gControllerLastDirection[MAX_PLAYERS];
static int gControllerDeadFrames;

extern OSMesgQueue gfxFrameMsgQ;

#define REMAP_PLAYER_INDEX(index) (index)
// #define REMAP_PLAYER_INDEX(index)   0

void Controller_clearStateAll()
{
	zeroMemory(gControllerData, sizeof(gControllerData));
	zeroMemory(gControllerLastButton, sizeof(gControllerLastButton));
	zeroMemory(gControllerLastDirection, sizeof(gControllerLastDirection));

	gControllerDeadFrames = 30;
}

int Controller_isConnected(int index)
{
	return !gControllerStatus[index].errno;
}

void Controller_init(void)
{
	OSMesgQueue serialMsgQ;
	OSMesg serialMsg;
	s32 i;

	osCreateMesgQueue(&serialMsgQ, &serialMsg, 1);
	osSetEventMesg(OS_EVENT_SI, &serialMsgQ, (OSMesg)1);

	if ((i = osContInit(&serialMsgQ, &validcontrollers, &gControllerStatus[0])) != 0)
		return;

	/**** Set up message and queue, for read completion notification ****/
	gControllerMessage.type = SIMPLE_CONTROLLER_MSG;
	osSetEventMesg(OS_EVENT_SI, &gfxFrameMsgQ, (OSMesg)&gControllerMessage);
}

void Controller_update(void)
{
	unsigned i;
	for (i = 0; i < MAX_PLAYERS; ++i)
	{
		gControllerLastDirection[i] = Controller_getDirection(i);
		gControllerLastButton[i] = gControllerData[i].button;
	}

	osContGetReadData(gControllerData);
	cntrlReadInProg = 0;

	if (gControllerDeadFrames)
	{
		--gControllerDeadFrames;
		zeroMemory(gControllerData, sizeof(gControllerData));
	}

	for (i = 0; i < MAX_PLAYERS; ++i)
	{
		if (gControllerStatus[i].errno & CONT_NO_RESPONSE_ERROR)
		{
			zeroMemory(&gControllerData[i], sizeof(OSContPad));
		}
	}
}

int Controller_hasPendingMsg()
{
	return cntrlReadInProg;
}

#define CONTROLLER_READ_SKIP_NUMBER 10

void Controller_triggerRead(void)
{
	if (validcontrollers && !cntrlReadInProg)
	{
		cntrlReadInProg = CONTROLLER_READ_SKIP_NUMBER;
		osContStartReadData(&gfxFrameMsgQ);
	}
	else if (cntrlReadInProg)
	{
		--cntrlReadInProg;
	}
}

OSContPad *Controller_getControllerData(int index)
{
	return &gControllerData[REMAP_PLAYER_INDEX(index)];
}

u16 Controller_getLastButton(int index)
{
	return gControllerLastButton[REMAP_PLAYER_INDEX(index)];
}

u16 Controller_getButton(int index, u16 button)
{
	return gControllerData[REMAP_PLAYER_INDEX(index)].button & button;
}

u16 Controller_getButtonDown(int index, u16 button)
{
	return gControllerData[REMAP_PLAYER_INDEX(index)].button & ~gControllerLastButton[REMAP_PLAYER_INDEX(index)] & button;
}

u16 Controller_getButtonUp(int index, u16 button)
{
	return ~gControllerData[REMAP_PLAYER_INDEX(index)].button & gControllerLastButton[REMAP_PLAYER_INDEX(index)] & button;
}

enum ControllerDirection Controller_getDirection(int index)
{
	enum ControllerDirection result = 0;

	if (gControllerData[REMAP_PLAYER_INDEX(index)].stick_y > 40 || (gControllerData[REMAP_PLAYER_INDEX(index)].button & U_JPAD) != 0)
	{
		result |= ControllerDirectionUp;
	}

	if (gControllerData[REMAP_PLAYER_INDEX(index)].stick_y < -40 || (gControllerData[REMAP_PLAYER_INDEX(index)].button & D_JPAD) != 0)
	{
		result |= ControllerDirectionDown;
	}

	if (gControllerData[REMAP_PLAYER_INDEX(index)].stick_x > 40 || (gControllerData[REMAP_PLAYER_INDEX(index)].button & R_JPAD) != 0)
	{
		result |= ControllerDirectionRight;
	}

	if (gControllerData[REMAP_PLAYER_INDEX(index)].stick_x < -40 || (gControllerData[REMAP_PLAYER_INDEX(index)].button & L_JPAD) != 0)
	{
		result |= ControllerDirectionLeft;
	}

	return result;
}

enum ControllerDirection Controller_getDirectionDown(int index)
{
	return Controller_getDirection(REMAP_PLAYER_INDEX(index)) & ~gControllerLastDirection[REMAP_PLAYER_INDEX(index)];
}