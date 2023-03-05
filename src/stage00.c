/*
   stage00.c

  the main game file
*/

#include <assert.h>

#include "../build/src/audio/clips.h"

#include <math.h>
#include <string.h>
#include "util/rom.h"
#include "util/memory.h"
// game
#include "controls/controller.h"
#include "animation/animation.h"
#include "constants.h"
#include "math/frustum.h"
#include "math/vec2d.h"
#include "math/vec3d.h"
#include "math/matrix.h"
#include "game.h"
#include "gameobject.h"
#include "controls/input.h"
#include "main.h"
#include "modeltype.h"
#include "pathfinding.h"
#include "physics/physics.h"
#include "graphics/graphics.h"
#include "graphics/renderer.h"
#include "sprite.h"
#include "trace.h"

#include "audio/audio.h"
#include "audio/soundplayer.h"

#include "models.h"
#include "segments.h"

// map
#include "../assets/levels/garden_map.h"
#include "../assets/levels/garden_map_collision.h"
#include "garden_map_graph.h"
// anim data
#include "character_anim.h"
#include "goose_anim.h"

#include "ed64/ed64io.h"

#include "util/debug_console.h"

#define CONSOLE_ED64LOG_DEBUG 0
#define CONSOLE_SHOW_PROFILING 0
#define CONSOLE_SHOW_TRACING 0
#define CONSOLE_SHOW_CULLING 0
#define CONSOLE_SHOW_CAMERA 0
#define CONSOLE_SHOW_SOUND 0
#define CONSOLE_SHOW_RCP_TASKS 0
#define LOG_TRACES 0
#define CONTROLLER_DEAD_ZONE 0.1
#define DRAW_SPRITES 1

static Vec3d viewPos;
static Vec3d viewRot;
static Input input;

static u16 perspNorm;
static u32 nearPlane; /* Near Plane */
static u32 farPlane;  /* Far Plane */
static Frustum frustum;
#if HIGH_RESOLUTION && HIGH_RESOLUTION_HALF_Y
static float aspect = (f32)SCREEN_WD / (f32)(SCREEN_HT * 2);
#else
static float aspect = (f32)SCREEN_WD / (f32)SCREEN_HT;
#endif
static float fovy = DEFAULT_FOVY;
static Vec3d upVector = {0.0f, 1.0f, 0.0f};

/* frame counter */
float frameCounterLastTime;
int frameCounterCurFrames;
int frameCounterLastFrames;

/* profiling */
int totalUpdates;
float profAvgCharacters;
float profAvgPhysics;
float profAvgDraw;
float profAvgPath;
// float lastFrameTime;
float profilingAverages[MAX_TRACE_EVENT_TYPE];

static int usbEnabled;
static int usbResult;
static UsbLoggerState usbLoggerState;

static int logTraceStartOffset = 0;
static int loggingTrace = FALSE;

static int twoCycleMode;
// static RenderMode renderModeSetting;
PhysWorldData physWorldData;

void drawWorldObjects(Dynamic *dynamicp, struct RenderState *renderState);
void drawSprite(unsigned short *sprData,
				int sprWidth,
				int sprHeight,
				int x,
				int y,
				int width,
				int height,
				int centered,
				struct RenderState *renderState);

#define OBJ_START_VAL 1000

Lights1 sun_light = gdSPDefLights1(120,
								   120,
								   120, /* weak ambient light */
								   255,
								   255,
								   255, /* white light */
								   80,
								   80,
								   0);

Lights0 amb_light = gdSPDefLights0(200, 200, 200 /*  ambient light */);

static float sndPitch = 10.5; // i don't fucking know :((
static int sndNumber = 0;
static int seqPlaying = FALSE;
s16 seqId = -1;

/* The initialization of stage 0 */
void initStage00()
{
	Game *game;
	int i;

	// load in the models segment into higher memory
	romCopy(_modelsSegmentRomStart, _modelsSegmentStart, (_modelsSegmentRomEnd - _modelsSegmentRomStart));
	// load in the sprites segment into higher memory
	romCopy(_spritesSegmentRomStart, _spritesSegmentStart, (_spritesSegmentRomEnd - _spritesSegmentRomStart));
	// load in the collision segment into higher memory
	romCopy(_collisionSegmentRomStart, _collisionSegmentStart, (_collisionSegmentRomEnd - _collisionSegmentRomStart));

	physWorldData = (PhysWorldData){garden_map_collision_collision_mesh,
									GARDEN_MAP_COLLISION_LENGTH,
									&garden_map_collision_collision_mesh_hash,
									/*gravity*/ -9.8 * N64_SCALE_FACTOR,
									/*viscosity*/ 0.05,
									/*waterHeight*/ WATER_HEIGHT};

	usbEnabled = FALSE;
	usbResult = 0;

	frameCounterLastTime = 0;
	frameCounterCurFrames = 0;
	totalUpdates = 0;
	profAvgCharacters = 0;
	profAvgPhysics = 0;
	profAvgDraw = 0;
	profAvgPath = 0;

	loggingTrace = FALSE;

	twoCycleMode = FALSE;
	gRenderMode = TextureNoLightingRenderMode;
	nearPlane = DEFAULT_NEARPLANE;
	farPlane = DEFAULT_FARPLANE;
	Vec3d_init(&viewPos, 0.0F, 0.0F, -400.0F);
	Vec3d_init(&viewRot, 0.0F, 0.0F, 0.0F);
	Input_init(&input);

	Game_init(garden_map_data, GARDEN_MAP_COUNT, &physWorldData);

	invariant(GARDEN_MAP_COUNT <= MAX_WORLD_OBJECTS);

	game = Game_get();

	game->pathfindingGraph = &garden_map_graph;
	game->pathfindingState = &garden_map_graph_pathfinding_state;

	// lastFrameTime = CUR_TIME_MS();

	// for (i = 0; i < MAX_TRACE_EVENT_TYPE; ++i) {
	//   profilingAverages[i] = 0;
	//   profilingAccumulated[i] = 0;
	//   profilingCounts[i] = 0;
	// }

#ifdef ED64
	// start a thread to watch the totalUpdates value
	// if it doesn't change each second, we've crashed
	ed64StartWatchdogThread(&totalUpdates, 1000);

	// debugPrintfSync("getModelDisplayList at %p\n", getModelDisplayList);
#endif

	// debugPrintf("good morning\n");
}

// void debugPrintVec3d(int x, int y, char* label, Vec3d* vec) {
//   char conbuf[100];
//   nuDebConTextPos(0, x, y);
//   sprintf(conbuf, "%s {%5.0f,%5.0f,%5.0f}", label, vec->x, vec->y, vec->z);
//   nuDebConCPuts(0, conbuf);
// }

// void debugPrintFloat(int x, int y, char* format, float value) {
//   char conbuf[100];
//   nuDebConTextPos(0, x, y);
//   sprintf(conbuf, format, value);
//   nuDebConCPuts(0, conbuf);
// }

#ifdef NU_DEBUG
void traceRCP()
{
	int i;
	float longestTaskTime = 0;
	// s64 retraceTime;

	// retraceTime = nuDebTaskPerfPtr->retraceTime;
	// debugPrintf("rt=%f ", retraceTime / (1000000.0));
	for (i = 0; i < nuDebTaskPerfPtr->gfxTaskCnt; i++)
	{
		// debugPrintf(
		//     "[t%d: st=%f rsp=%f,rdp=%f] ", i,
		//     (nuDebTaskPerfPtr->gfxTaskTime[i].rspStart ) / 1000.0,
		//     (nuDebTaskPerfPtr->gfxTaskTime[i].rspEnd -
		//      nuDebTaskPerfPtr->gfxTaskTime[i].rspStart) /
		//         1000.0,
		//     (nuDebTaskPerfPtr->gfxTaskTime[i].rdpEnd -
		//      nuDebTaskPerfPtr->gfxTaskTime[i].rspStart) /
		//         1000.0);

		// pulled out to a variable so the modern compiler doesn't
		// generate broken code
		NUDebTaskTime gfxTaskTime = nuDebTaskPerfPtr->gfxTaskTime[i];

		Trace_addEvent(RSPTaskTraceEvent, gfxTaskTime.rspStart / 1000.0f,
					   gfxTaskTime.rspEnd / 1000.0f);

		Trace_addEvent(RDPTaskTraceEvent, gfxTaskTime.rspStart / 1000.0f,
					   gfxTaskTime.rdpEnd / 1000.0f);

		longestTaskTime = MAX(longestTaskTime, ((gfxTaskTime.rdpEnd) / 1000.0f -
												(gfxTaskTime.rspStart / 1000.0f)));
	}
	// debugPrintf("\n");
	profilingAccumulated[RDPTaskTraceEvent] += longestTaskTime;
	profilingCounts[RDPTaskTraceEvent]++;
}
#endif

/* Make the display list and activate the task */
void stage00Render(u32 *data, struct RenderState *renderState, struct GraphicsTask *task)
{
	Dynamic *dynamicp;
	Game *game;
	// int consoleOffset;
	// float curTime;
	int i;
	// float profStartDraw, profEndDraw, profStartDebugDraw;

	game = Game_get();
	// // consoleOffset = 20;

	// curTime = CUR_TIME_MS();
	frameCounterCurFrames++;

	// profStartDraw = curTime;

	// if (curTime - frameCounterLastTime >= 1000.0) {
	//   frameCounterLastFrames = frameCounterCurFrames;
	//   frameCounterCurFrames = 0;
	//   frameCounterLastTime += 1000.0;
	// }

	/* Specify the display list buffer */
	dynamicp = &gfx_dynamic[*data];
	// // glistp = &gfx_glist[gfx_gtask_no][0];

	Frustum_setCamInternals(&frustum, fovy, aspect, nearPlane, farPlane);

	/* projection, viewing, modeling matrix set */
	guPerspective(&dynamicp->projection, &perspNorm, fovy, aspect, nearPlane,
				  farPlane, 1.0);
	gSPPerspNormalize(renderState->dl++, perspNorm);

	Frustum_setCamDef(&frustum, &game->viewPos, &game->viewTarget, &upVector);

	if (game->freeView)
	{
		guPosition(&dynamicp->camera,
				   viewRot.x, // roll
				   viewRot.y, // pitch
				   viewRot.z, // yaw
				   1.0F,	  // scale
				   viewPos.x, viewPos.y, viewPos.z);
	}
	else
	{
		guLookAt(&dynamicp->camera, game->viewPos.x, game->viewPos.y,
				 game->viewPos.z, game->viewTarget.x, game->viewTarget.y,
				 game->viewTarget.z, upVector.x, upVector.y, upVector.z);
	}

	drawWorldObjects(dynamicp, renderState);

	/* Activate the task and (maybe)
	   switch display buffers */
	// nuGfxTaskStart(&gfx_glist[gfx_gtask_no][0],
	//                (s32)(glistp - gfx_glist[gfx_gtask_no]) * sizeof(Gfx),
	//                gRenderMode == WireframeRenderMode ? NU_GFX_UCODE_L3DEX2
	//                                                         : NU_GFX_UCODE_F3DEX,

	//   profEndDraw = CUR_TIME_MS();
	//   Trace_addEvent(DrawTraceEvent, profStartDraw, profEndDraw);
	//   game->profTimeDraw += profEndDraw - profStartDraw;

	//   profStartDebugDraw = CUR_TIME_MS();

	// #if NU_PERF_BAR
	//   nuDebTaskPerfBar1(1, 200, NU_SC_SWAPBUFFER);
	// #endif

	// // debug text overlay
	// #if CONSOLE

	// #if NU_PERF_BAR
	// #error "can't enable NU_PERF_BAR and CONSOLE at the same time"
	// #endif

	//   if (contPattern & 0x1) {
	//     nuDebConClear(0);
	// #if CONSOLE_SHOW_CAMERA
	//     consoleOffset = 16;
	// #else
	//     consoleOffset = 21;
	// #endif

	//     debugPrintFloat(4, consoleOffset++, "frame=%3.2fms",
	//                     1000.0 / frameCounterLastFrames);

	//     if (game->freeView) {
	//       debugPrintVec3d(4, consoleOffset++, "viewPos", &viewPos);
	//     } else {
	// #if CONSOLE_SHOW_PROFILING
	//       // debugPrintFloat(4, consoleOffset++, "char=%3.2fms", profAvgCharacters);
	//       // debugPrintFloat(4, consoleOffset++, "phys=%3.2fms", profAvgPhysics);
	//       // debugPrintFloat(4, consoleOffset++, "draw=%3.2fms", profAvgDraw);
	//       // debugPrintFloat(0, consoleOffset++, "path=%3.2fms", profAvgPath);
	//       debugPrintFloat(4, consoleOffset++, "cpu=%3.2fms",
	//                       profilingAverages[MainCPUTraceEvent]);
	//       debugPrintFloat(4, consoleOffset++, "rdp=%3.2fms",
	//                       profilingAverages[RDPTaskTraceEvent]);
	// #endif
	// #if CONSOLE_SHOW_TRACING
	//       nuDebConTextPos(0, 4, consoleOffset++);
	//       sprintf(conbuf, "trace rec=%d,log=%d,evs=%d,lgd=%d", Trace_isTracing(),
	//               loggingTrace, Trace_getEventsCount(), logTraceStartOffset);
	//       nuDebConCPuts(0, conbuf);
	// #endif
	// #if CONSOLE_SHOW_RCP_TASKS
	// #ifdef NU_DEBUG
	//       {
	//         int tskIdx;
	//         for (tskIdx = 0; tskIdx < nuDebTaskPerfPtr->gfxTaskCnt; tskIdx++) {
	//           nuDebConTextPos(0, 4, consoleOffset++);
	//           sprintf(conbuf, "[t%d:  rsp=%.2f,rdp=%.2f] ", tskIdx,
	//                   (nuDebTaskPerfPtr->gfxTaskTime[tskIdx].rspEnd -
	//                    nuDebTaskPerfPtr->gfxTaskTime[tskIdx].rspStart) /
	//                       1000.0,
	//                   (nuDebTaskPerfPtr->gfxTaskTime[tskIdx].rdpEnd -
	//                    nuDebTaskPerfPtr->gfxTaskTime[tskIdx].rspStart) /
	//                       1000.0);
	//           nuDebConCPuts(0, conbuf);
	//         }
	//       }
	// #endif
	// #endif
	// #if CONSOLE_SHOW_CULLING
	//       nuDebConTextPos(0, 4, consoleOffset++);
	//       sprintf(conbuf, "culled=%d", objectsCulled);
	//       nuDebConCPuts(0, conbuf);
	// #endif
	// #if CONSOLE_SHOW_CAMERA
	//       debugPrintVec3d(4, consoleOffset++, "viewPos", &game->viewPos);
	//       debugPrintVec3d(4, consoleOffset++, "viewTarget", &game->viewTarget);
	//       debugPrintFloat(4, consoleOffset++, "viewZoom=%1.1f", game->viewZoom);
	//       debugPrintFloat(4, consoleOffset++, "fovy=%3.1f", fovy);
	//       debugPrintFloat(4, consoleOffset++, "nearPlane=%.2f", nearPlane);
	//       debugPrintFloat(4, consoleOffset++, "farPlane=%.2f", farPlane);
	// #endif

	// #if CONSOLE_ED64LOG_DEBUG
	// #ifdef ED64
	//       usbLoggerGetState(&usbLoggerState);
	//       nuDebConTextPos(0, 4, consoleOffset++);
	//       sprintf(conbuf, "usb=%d,res=%d,st=%d,id=%d,mqsz=%d", usbEnabled,
	//               usbResult, usbLoggerState.fifoWriteState, usbLoggerState.msgID,
	//               usbLoggerState.msgQSize);
	//       nuDebConCPuts(0, conbuf);
	//       nuDebConTextPos(0, 4, consoleOffset++);
	//       sprintf(conbuf, "off=%4d,flu=%d,ovf=%d,don=%d,err=%d",
	//               usbLoggerState.usbLoggerOffset, usbLoggerState.usbLoggerFlushing,
	//               usbLoggerState.usbLoggerOverflow, usbLoggerState.countDone,
	//               usbLoggerState.writeError);
	//       nuDebConCPuts(0, conbuf);
	// #endif
	// #endif

	// #if CONSOLE_SHOW_SOUND
	//       debugPrintFloat(4, consoleOffset++, "snd=%.0f", sndNumber);
	//       debugPrintFloat(4, consoleOffset++, "pitch=%f", sndPitch);
	// #endif
	//     }
	//     nuDebConTextPos(0, 4, consoleOffset++);
	//     sprintf(conbuf, "retrace=%lu", nuScRetraceCounter);
	//     nuDebConCPuts(0, conbuf);
	//     debugPrintVec3d(4, consoleOffset++, "pos", &game->player.goose->position);
	//   } else {
	//     nuDebConTextPos(0, 9, 24);
	//     nuDebConCPuts(0, "Controller1 not connected");
	//   }

	//   /* Display characters on the frame buffer */
	//   nuDebConDisp(NU_SC_SWAPBUFFER);
	// #endif  // #if CONSOLE

	//   Trace_addEvent(DebugDrawTraceEvent, profStartDebugDraw, CUR_TIME_MS());
}

void logTraceChunk()
{
	int i;
	int printedFirstItem;
	// #if ED64

	//   printedFirstItem = FALSE;
	//   if (usbLoggerBufferRemaining() < 120) {
	//     return;
	//   }
	//   debugPrintf("TRACE=[");
	//   for (i = logTraceStartOffset; i < Trace_getEventsCount(); i++) {
	//     // check we have room for more data
	//     if (usbLoggerBufferRemaining() < 40) {
	//       break;
	//     }
	//     debugPrintf("%s[%.2f,%.2f,%d]", printedFirstItem ? "," : "",
	//                 traceEvents[i].start, traceEvents[i].end, traceEvents[i].type);
	//     printedFirstItem = TRUE;
	//     logTraceStartOffset = i;
	//   }
	//   debugPrintf("]\n");

	//   if (logTraceStartOffset == Trace_getEventsCount() - 1) {
	//     // finished
	//     loggingTrace = FALSE;
	//     logTraceStartOffset = 0;
	//     Trace_clear();
	//   }
	// #endif
}

// void startRecordingTrace() {
//   Trace_clear();
//   Trace_start();
// }

// void finishRecordingTrace() {
//   Trace_stop();
//   loggingTrace = TRUE;
// }

/* The game progressing process for stage 0 */
void updateGame00(void)
{
	int i;
	Game *game;

	game = Game_get();

	// Vec2d_origin(&input.direction);
	Input_init(&input);
	/* Data reading of controller 1 */
	OSContPad *controller_input = controllersGetControllerData(0);

	if (controllerGetButtonUp(0, START_BUTTON))
	{
		controller_input->button;
		gRenderMode++;
		if (gRenderMode >= MAX_RENDER_MODE)
		{
			gRenderMode = 0;
		}
	}

	if (controllerGetButton(0, A_BUTTON))
	{
		input.run = TRUE;
	}
	if (controllerGetButtonDown(0, B_BUTTON))
	{
		input.pickup = TRUE;
	}
	if (controllerGetButton(0, Z_TRIG))
	{
		input.zoomIn = TRUE;
	}
	if (controllerGetButton(0, L_TRIG))
	{
		input.zoomIn = TRUE;
	}
	if (controllerGetButton(0, R_TRIG))
	{
		input.zoomOut = TRUE;
	}

	if (controllerGetButtonDown(0, L_CBUTTONS))
	{

		soundPlayerPlay(SOUNDS_HONK_1, 1.0f, 1.0f, NULL);
	}

	if(!soundPlayerIsPlaying(seqId))
	{
		seqPlaying = FALSE;
	}
	if (controllerGetButtonDown(0, R_CBUTTONS))
	{
		if (seqPlaying)
		{
			// debugPrintf("stop playing seq\n");
			if (seqId != -1)
			{
				soundPlayerStop(seqId);
				seqId = -1;
			}
			seqPlaying = FALSE;
		}
		else
		{
			// debugPrintf("start playing seq\n");
			seqId = soundPlayerPlay(SOUNDS_JAH_SPOOKS, 1.0f, 1.0f, NULL);
			seqPlaying = TRUE;
		}
	}

	input.direction.x = -controller_input->stick_x / 61.0F;
	input.direction.y = controller_input->stick_y / 61.0F;
	if (fabsf(input.direction.x) < CONTROLLER_DEAD_ZONE)
		input.direction.x = 0;
	if (fabsf(input.direction.y) < CONTROLLER_DEAD_ZONE)
		input.direction.y = 0;
	if (Vec2d_length(&input.direction) > 1.0F)
	{
		Vec2d_normalise(&input.direction);
	}
	// }

	//   // if (Trace_getEventsCount() == TRACE_EVENT_BUFFER_SIZE) {
	//   //   finishRecordingTrace();
	//   // }

	//   if (usbEnabled) {
	// #if LOG_TRACES
	//     if (loggingTrace) {
	//       logTraceChunk();
	//     }
	// #endif
	//   }

	Game_update(&input);

	//   // if (totalUpdates % 60 == 0) {
	//   //   debugPrintfSync("retrace=%d\n", nuScRetraceCounter);
	//   // }

	//   if (usbEnabled) {
	// #if LOG_TRACES
	//     if (loggingTrace) {
	//       logTraceChunk();
	//     }
	// #endif
	// #ifdef ED64
	//     usbResult = ed64AsyncLoggerFlush();
	// #endif
	//   }

	//-----------------Trace-------------------
	// if (totalUpdates % 60 == 0) {
	//   // calc averages for last 60 updates
	//   profAvgCharacters = game->profTimeCharacters / 60;
	//   game->profTimeCharacters = 0.0f;
	//   profAvgPhysics = game->profTimePhysics / 60;
	//   game->profTimePhysics = 0.0f;
	//   profAvgDraw = game->profTimeDraw / 60;
	//   game->profTimeDraw = 0.0f;
	//   profAvgPath = game->profTimePath / 60;
	//   game->profTimePath = 0.0f;

	//   for (i = 0; i < MAX_TRACE_EVENT_TYPE; ++i) {
	//     profilingAverages[i] =
	//         profilingCounts[i] == 0
	//             ? 0
	//             : profilingAccumulated[i] / (float)profilingCounts[i];
	//     profilingAccumulated[i] = 0;
	//     profilingCounts[i] = 0;
	//   }
	// }
}

typedef enum LightingType
{
	SunLighting,
	OnlyAmbientLighting,
	MAX_LIGHTING_TYPE
} LightingType;

LightingType getLightingType(GameObject *obj)
{
	switch (obj->modelType)
	{
	case UniFloorModel:
		return OnlyAmbientLighting;
	default:
		return SunLighting;
	}
}

int getAnimationNumModelMeshParts(ModelType modelType)
{
	switch (modelType)
	{
	case GooseModel:
		return MAX_GOOSE_MESH_TYPE;
	default:
		return MAX_CHARACTER_MESH_TYPE;
	}
}

int shouldLerpAnimation(ModelType modelType)
{
	switch (modelType)
	{
	case GooseModel:
		return TRUE;
	default:
		return FALSE;
	}
}

AnimationRange *getCurrentAnimationRange(GameObject *obj)
{
	if (obj->modelType == GooseModel)
	{
		return &goose_anim_ranges[(GooseAnimType)obj->animState->state];
	}
	else
	{
		return &character_anim_ranges[(CharacterAnimType)obj->animState->state];
	}
}

AnimationFrame *getAnimData(ModelType modelType)
{
	switch (modelType)
	{
	case GooseModel:
		return goose_anim_data;
	default:
		return character_anim_data;
	}
}

void drawWorldObjects(Dynamic *dynamicp, struct RenderState *renderState)
{
	Game *game;
	GameObject *obj;
	int i, useZBuffering;
	int modelMeshIdx;
	int modelMeshParts;
	Gfx *modelDisplayList;
	Triangle *worldCollisionTris;
	AnimationFrame animFrame;
	AnimationInterpolation animInterp;
	AnimationRange *curAnimRange;
	AnimationBoneAttachment *attachment;
	RendererSortDistance *visibleObjDistance;
	int *worldObjectsVisibility;
	int *intersectingObjects;
	int visibleObjectsCount;
	int frustumCulled = 0;
	int occlusionCulled = 0;
	ViewportF viewport = {0, 0, SCREEN_WD, SCREEN_HT};
	MtxF modelViewMtxF;
	MtxF projectionMtxF;

	//get the camera view matrix as well as the projection matrix
	guMtxL2F(modelViewMtxF, &dynamicp->camera);
	guMtxL2F(projectionMtxF, &dynamicp->projection);

	//get the current gamestate
	game = Game_get();

	worldCollisionTris = game->physicsState.worldData->worldMeshTris;
	worldObjectsVisibility = (int *)malloc(game->worldObjectsCount * sizeof(int));
	invariant(worldObjectsVisibility);

	//cull objects that are outside of the view frustum
	frustumCulled = Renderer_frustumCull(
		game->worldObjects, game->worldObjectsCount, worldObjectsVisibility,
		&frustum, garden_map_bounds);
	char str[15];
	sprintf(str, "# of obj culled %d", frustumCulled);
	console_add_msg(str);

	//of the objects that are in the frustum, exclude those that are fully occluded
	occlusionCulled = Renderer_occlusionCull(
		game->worldObjects, game->worldObjectsCount, worldObjectsVisibility,
		modelViewMtxF, projectionMtxF, viewport, &frustum, garden_map_bounds);

	// only alloc space for num visible objects
	visibleObjectsCount = game->worldObjectsCount - frustumCulled;
	visibleObjDistance = (RendererSortDistance *)malloc(
		(visibleObjectsCount) * sizeof(RendererSortDistance));
	invariant(visibleObjDistance);

	// profStartSort = CUR_TIME_MS();
	Renderer_sortVisibleObjects(game->worldObjects, game->worldObjectsCount,
								worldObjectsVisibility, visibleObjectsCount,
								visibleObjDistance, &game->viewPos,
								garden_map_bounds);

	// boolean of whether an object intersects another (for z buffer optimization)
	intersectingObjects = (int *)malloc((visibleObjectsCount) * sizeof(int));
	invariant(intersectingObjects);
	Renderer_calcIntersecting(intersectingObjects, visibleObjectsCount,
							  visibleObjDistance, garden_map_bounds);

	gSPClearGeometryMode(renderState->dl++, 0xFFFFFFFF);
	gDPSetCycleType(renderState->dl++, twoCycleMode ? G_CYC_2CYCLE : G_CYC_1CYCLE);

	// z-buffered, antialiased triangles
#if !RENDERER_PAINTERS_ALGORITHM
	gDPSetRenderMode(renderState->dl++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
	gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);
#endif

	gSPSetLights0(renderState->dl++, amb_light);

	// setup view
	gSPMatrix(renderState->dl++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)),
			  G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
	gSPMatrix(renderState->dl++, OS_K0_TO_PHYSICAL(&(dynamicp->camera)),
			  G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	//   profStartIter = CUR_TIME_MS();
	// render world objects
	for (i = 0; i < visibleObjectsCount; i++)
	{
		// iterate visible objects far to near
		obj = (visibleObjDistance + i)->obj;

		// render textured models
		gSPTexture(renderState->dl++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);
		gDPSetTextureFilter(renderState->dl++, G_TF_BILERP);
		gDPSetTexturePersp(renderState->dl++, G_TP_PERSP);

		switch (gRenderMode)
		{
		case TextureAndLightingRenderMode:
		case LightingNoTextureRenderMode:
			if (getLightingType(obj) == OnlyAmbientLighting)
			{
				gSPSetLights0(renderState->dl++, amb_light);
			}
			else
			{
				gSPSetLights1(renderState->dl++, sun_light);
			}
			break;
		default:
			break;
		}

		gSPClearGeometryMode(renderState->dl++, 0xFFFFFFFF);
		invariant(i < visibleObjectsCount);
		invariant(obj != NULL);
		if (!RENDERER_PAINTERS_ALGORITHM || // always z buffer
			intersectingObjects[i] ||
			// animated game objects have concave shapes, need z buffering
			Renderer_isAnimatedGameObject(obj))
		{
			if (!RENDERER_PAINTERS_ALGORITHM || // always z buffer
				Renderer_isZBufferedGameObject(obj))
			{
				if (ANTIALIASING)
				{
					gDPSetRenderMode(renderState->dl++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
				}
				else
				{
					gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
				}
				gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);
			}
			else /*if (Renderer_isZWriteGameObject(obj))*/
			{
				if (ANTIALIASING)
				{
					gDPSetRenderMode(renderState->dl++, G_RM_AA_ZUPD_OPA_SURF,
									 G_RM_AA_ZUPD_OPA_SURF2);
				}
				else
				{
					gDPSetRenderMode(renderState->dl++, G_RM_ZUPD_OPA_SURF, G_RM_ZUPD_OPA_SURF2);
				}
				gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);
			}
		}
		else
		{
			if (ANTIALIASING)
			{
				gDPSetRenderMode(renderState->dl++, G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
			}
			else
			{
				gDPSetRenderMode(renderState->dl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
			}
		}

		switch (gRenderMode)
		{
		case ToonFlatShadingRenderMode:
			gSPSetGeometryMode(renderState->dl++, G_CULL_BACK);
			gDPSetCombineMode(renderState->dl++, G_CC_DECALRGB, G_CC_DECALRGB);
			break;
		case TextureNoLightingRenderMode:
		case WireframeRenderMode:
			gSPSetGeometryMode(renderState->dl++, G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK);
			gDPSetCombineMode(renderState->dl++, G_CC_DECALRGB, G_CC_DECALRGB);
			break;
		case TextureAndLightingRenderMode:
			gSPSetGeometryMode(
				renderState->dl++, G_SHADE | G_SHADING_SMOOTH | G_LIGHTING | G_CULL_BACK);
			gDPSetCombineMode(renderState->dl++, G_CC_MODULATERGB, G_CC_MODULATERGB);
			break;
		case LightingNoTextureRenderMode:
			gSPSetGeometryMode(
				renderState->dl++, G_SHADE | G_SHADING_SMOOTH | G_LIGHTING | G_CULL_BACK);
			gDPSetCombineMode(renderState->dl++, G_CC_SHADE, G_CC_SHADE);
			break;
		default: // NoTextureNoLightingRenderMode
			gDPSetPrimColor(renderState->dl++, 0, 0, /*r*/ 180, /*g*/ 180, /*b*/ 180,
							/*a*/ 255);
			gSPSetGeometryMode(renderState->dl++, G_CULL_BACK);
			gDPSetCombineMode(renderState->dl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
		}

		// set the transform in world space for the gameobject to render
		guPosition(&dynamicp->objTransforms[i],
				   obj->rotation.x,									   // rot x
				   obj->rotation.y,							   // rot y
				   obj->rotation.z,								   // rot z
				   modelTypesProperties[obj->modelType].scale, // scale
				   obj->position.x,							   // pos x
				   obj->position.y,							   // pos y
				   obj->position.z							   // pos z
		);
		gSPMatrix(
			renderState->dl++, OS_K0_TO_PHYSICAL(&(dynamicp->objTransforms[i])),
			G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH); // gameobject mtx start

		if (Renderer_isAnimatedGameObject(obj))
		{
			// case for multi-part objects using rigid body animation

			modelMeshParts = getAnimationNumModelMeshParts(obj->modelType);
			curAnimRange = getCurrentAnimationRange(obj);
			AnimationInterpolation_calc(&animInterp, obj->animState, curAnimRange);

			for (modelMeshIdx = 0; modelMeshIdx < modelMeshParts; ++modelMeshIdx)
			{
				//         // lerping takes about 0.2ms per bone
				if (shouldLerpAnimation(obj->modelType))
				{
					AnimationFrame_lerp(
						&animInterp, // result of AnimationInterpolation_calc()
						getAnimData(
							obj->modelType), // pointer to start of AnimationFrame list
						modelMeshParts,		 // num bones in rig used by animData
						modelMeshIdx,		 // index of bone in rig to produce transform for
						&animFrame			 // the resultant interpolated animation frame
					);
				}
				else
				{
					AnimationFrame_get(
						&animInterp, // result of AnimationInterpolation_calc()
						getAnimData(
							obj->modelType), // pointer to start of AnimationFrame list
						modelMeshParts,		 // num bones in rig used by animData
						modelMeshIdx,		 // index of bone in rig to produce transform for
						&animFrame			 // the resultant interpolated animation frame
					);
				}

				// push matrix with the blender to n64 coord rotation, then mulitply
				// it by the model's rotation and offset

				// rotate from z-up (blender) to y-up (opengl) coords
				// TODO: move as many of these transformations as possible to
				// be precomputed in animation data
				guRotate(&dynamicp->zUpToYUpCoordinatesRotation, -90.0f, 1, 0, 0);
				gSPMatrix(renderState->dl++,
						  OS_K0_TO_PHYSICAL(&(dynamicp->zUpToYUpCoordinatesRotation)),
						  G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

				guPosition(&obj->animState->animMeshTransform[modelMeshIdx],
						   animFrame.rotation.x, // roll
						   animFrame.rotation.y, // pitch
						   animFrame.rotation.z, // yaw
						   1.0F,				 // scale
						   animFrame.position.x, // pos x
						   animFrame.position.y, // pos y
						   animFrame.position.z	 // pos z
				);
				gSPMatrix(renderState->dl++,
						  OS_K0_TO_PHYSICAL(
							  &(obj->animState->animMeshTransform[modelMeshIdx])),
						  G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

				gSPDisplayList(renderState->dl++, getMeshDisplayListForModelMeshPart(
													  obj->modelType, animFrame.object));

				attachment = &obj->animState->attachment;
				if (attachment->modelType != NoneModel &&
					attachment->boneIndex == modelMeshIdx)
				{
					guPosition(&obj->animState->attachmentTransform,
							   attachment->rotation.x, // roll
							   attachment->rotation.y, // pitch
							   attachment->rotation.z, // yaw
							   1.0F,				   // scale
							   attachment->offset.x,   // pos x
							   attachment->offset.y,   // pos y
							   attachment->offset.z	   // pos z
					);
					gSPMatrix(renderState->dl++,
							  OS_K0_TO_PHYSICAL(&(obj->animState->attachmentTransform)),
							  G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
					modelDisplayList = getModelDisplayList(attachment->modelType, 0);
					gSPDisplayList(renderState->dl++, modelDisplayList);
					gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
				}

				gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
			}
		}
		else
		{
			// case for simple gameobjects with no moving sub-parts
			modelDisplayList = getModelDisplayList(obj->modelType, obj->subtype);

			gSPDisplayList(renderState->dl++, modelDisplayList);
		}

		gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW); // gameobject mtx end

		gDPPipeSync(renderState->dl++);
	}

#if DRAW_SPRITES
	{		
		float width = 64;
		float height = 64;
		Vec3d center;
		Vec3d projected;

		for (i = 0; i < visibleObjectsCount; i++)
		{
			obj = visibleObjDistance[i].obj;
			Game_getObjCenter(obj, &center);
			center.y += 75;
			if (!obj->animState)
			{
				continue;
			}
			Renderer_screenProject(&center, modelViewMtxF, projectionMtxF, viewport,
						  &projected);

			// render sprites
			drawSprite(getSpriteForSpriteType(HonkSprite,
											  Sprite_frameCycle(HONK_SPRITE_FRAMES,
																/*sprite frame
																duration*/
																10, game->tick)),

					   32, 32, // texture dimensions
					   projected.x,
					   SCREEN_HT - projected.y, // draw at pos, invert y
					   RES_SCALE_X(width), RES_SCALE_Y(height), TRUE,
					   renderState

			);
		}
	}
#endif

	stackMallocFree(intersectingObjects);
	stackMallocFree(visibleObjDistance);
	stackMallocFree(worldObjectsVisibility);
}

void drawSprite(unsigned short *sprData,
				int sprWidth,
				int sprHeight,
				int atX,
				int atY,
				int width,
				int height,
				int centered,
				struct RenderState *renderState)
{
	int x = atX + (centered ? -(width / 2) : 0);
	int y = atY + (centered ? -(height / 2) : 0);

	gDPLoadTextureBlock(renderState->dl++, sprData, G_IM_FMT_RGBA, G_IM_SIZ_16b, sprWidth,
						sprHeight, 0, G_TX_WRAP | G_TX_NOMIRROR,
						G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
	gDPSetTexturePersp(renderState->dl++, G_TP_NONE);
	gDPSetAlphaCompare(renderState->dl++, G_AC_THRESHOLD);
	gDPSetCombineMode(renderState->dl++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
	gDPSetRenderMode(renderState->dl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	// TODO: clip rectangle and offset+scale tex coords so we can render partly
	// offscreen textures
	gSPScisTextureRectangle(
		renderState->dl++,
		(int)(x) << 2,			// upper left x,
		(int)(y) << 2,			// upper left y
		(int)(x + width) << 2,	// lower right x,
		(int)(y + height) << 2, // lower right y,
		G_TX_RENDERTILE,		// tile
		// upper left s, t
		// when using gSPTextureRectangleFlip, these args are flipped
		0 << 5, 0 << 5,
		// change in s,t for each change in screen space x,y.
		// smaller values = larger scaling
		// when using gSPTextureRectangleFlip, these args are flipped
		((float)sprWidth / (float)width) * (1 << 10),
		((float)sprHeight / (float)height) * (1 << 10));

	gDPPipeSync(renderState->dl++);
}

// like gluProject()
