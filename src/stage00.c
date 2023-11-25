/**
 * @file stage00.c
 * @author Kevin Reier
 * @brief Stage 00, the Main stage that runs when the ROM starts
 * @date 2023-11-21
 * 
 */

#include <assert.h>

#include "audio/clips.h"

#include <math.h>
#include "util/rom.h"
#include "util/memory.h"
// game
#include "controls/controller.h"
#include "animation/animation.h"
#include "constants.h"
#include "math/frustum.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/matrix.h"
#include "game.h"
#include "gameobject.h"
#include "controls/input.h"
#include "main.h"
#include "modeltype.h"
#include "pathfinding/pathfinding.h"
#include "physics/physics.h"
#include "graphics/graphics.h"
#include "graphics/renderer.h"
#include "sprite.h"
#include "util/trace.h"

#include "audio/audio.h"
#include "audio/soundplayer.h"

#include "models.h"
#include "segments.h"

// map
#include "../assets/levels/garden/garden_map.h"
#include "../assets/levels/garden/garden_map_collision.h"
#include "../assets/levels/garden/garden_map_graph.h"
// anim data
#include "actors/gardener/character_anim.h"
#include "goose_anim.h"

#include "ed64/ed64io.h"

#include "util/debug_console.h"

// include animation lib single file
#include "sausage64/sausage64.h"

// include sample data for model data (anim/verts/textures)
#include "actors/catherine/catherineTex.h"
#include "actors/catherine/catherineMdl.h"

/*********************************
        Function Prototypes for catherine Sausage64 Model
*********************************/

void catherine_predraw(u16 part);
void catherine_animcallback(u16 anim);

// Catherine
Mtx catherineMtx[MESHCOUNT_Catherine];
s64ModelHelper catherine;
float catherine_animspeed;



//-----------------------------------------------

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
#define DRAW_OBJECTS 1

static struct Vector3 viewPos;
static struct Vector3 viewRot;
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
static struct Vector3 upVector = {0.0f, 1.0f, 0.0f};

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

	gRenderMode = TextureNoLightingRenderMode;
	nearPlane = DEFAULT_NEARPLANE;
	farPlane = DEFAULT_FARPLANE;
	vector3Init(&viewPos, 0.0F, 0.0F, -400.0F);
	vector3Init(&viewRot, 0.0F, 0.0F, 0.0F);
	Input_init(&input);

	Game_init(garden_map_data, GARDEN_MAP_COUNT, &physWorldData);

	invariant(GARDEN_MAP_COUNT <= MAX_WORLD_OBJECTS);

	game = Game_get();

	game->pathfindingGraph = &garden_map_graph;
	game->pathfindingState = &garden_map_graph_pathfinding_state;
	struct Vector3 catherine_pos = {0.0F, 0.0F, 0.0F};
	struct Vector3 catherine_scale = {1.0F, 1.0F, 1.0F};
	Quaternion catherine_rot = {0.0F, 0.0F, 0.0F, 1.0F};
	// Initialize Catherine
    sausage64_initmodel(&catherine, MODEL_Catherine, catherineMtx, &catherine_pos, &catherine_scale, &catherine_rot);
	sausage64_set_anim(&catherine, ANIMATION_Catherine_Walk);

    // Set catherine's animation speed based on region
    #if TV_TYPE == PAL
        catherine_animspeed = 0.66;
    #else
        catherine_animspeed = 0.5;
    #endif

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

	
}



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
	// int i;
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
	// case for simple gameobjects with no moving sub-parts
	// Gfx *modelDisplayList;
	// gSPDisplayList(renderState->dl++, modelDisplayList);

	sausage64_drawmodel(&renderState->dl, &catherine);
}


/* The game progressing process for stage 0 */
void updateGame00(void)
{
	// Advance Catherine's animation
    sausage64_advance_anim(&catherine, catherine_animspeed);

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
	if (vector2Mag(&input.direction) > 1.0F)
	{
		vector2Normalize(&input.direction, &input.direction);
	}

	Game_update(&input);

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
	int i;
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
#if DRAW_OBJECTS
{
	//setup Pipeline for 3D rendering
	//TODO: add settings to scene that get handled here?
	graphicsSetupPipeline(renderState, dynamicp);
	//set ambient light
	gSPSetLights0(renderState->dl++, amb_light);

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

		gSPClearGeometryMode(renderState->dl++, 0xFFFFFFFF);
		invariant(i < visibleObjectsCount);
		invariant(obj != NULL);

		if (ANTIALIASING)
		{
			gDPSetRenderMode(renderState->dl++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
		}
		else
		{
			gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
		}
		gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);

		int ambientOnly = Renderer_getLightingType(obj) == OnlyAmbientLighting;

		graphicsApplyRenderMode(renderState, gRenderMode, &amb_light, &sun_light, ambientOnly);

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
			// if(obj->modelType == CharacterModelType){
			// 	char *pos[20];
            //     sprintf(pos, "pos:%f,%f,%f", obj->position.x, obj->position.y, obj->position.z);
            //     console_add_msg(pos);
			// }

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
}
#endif

#if DRAW_SPRITES
	{		
		float width = 64;
		float height = 64;
		struct Vector3 center;
		struct Vector3 projected;
		
		
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
	free(intersectingObjects);
	free(visibleObjDistance);
	free(worldObjectsVisibility);
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
	gDPSetPrimColor(renderState->dl++, 0, 0, 255, 255, 255, 255);
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
