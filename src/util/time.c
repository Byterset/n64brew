
#include "time.h"

double lastTime = 0;
extern double gDeltaTimeMS = 0;
extern float gDeltaTimeSec = 0.0f;
void timeUpdateDelta()
{
	double currTime = CUR_TIME_MS();
	gDeltaTimeMS = currTime - lastTime;
	gDeltaTimeSec = gDeltaTimeMS / 1000.0f;
	lastTime = currTime;
}