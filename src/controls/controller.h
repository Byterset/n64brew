#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <ultra64.h>

void Controller_init(void);
void Controller_update(void);
void Controller_triggerRead(void);

enum ControllerDirection
{
	ControllerDirectionUp = (1 << 0),
	ControllerDirectionRight = (1 << 1),
	ControllerDirectionDown = (1 << 2),
	ControllerDirectionLeft = (1 << 3),
};

void Controller_clearStateAll();
int Controller_hasPendingMsg();
int Controller_isConnected(int index);
OSContPad *Controller_getControllerData(int index);
u16 Controller_getLastButton(int index);
u16 Controller_getButton(int index, u16 button);
u16 Controller_getButtonDown(int index, u16 button);
u16 Controller_getButtonUp(int index, u16 button);
enum ControllerDirection Controller_getDirection(int index);
enum ControllerDirection Controller_getDirectionDown(int index);

#endif