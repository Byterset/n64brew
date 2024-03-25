#include "graphics.h"
#include "initgfx.h"
#include "../util/memory.h"
#include "../util/time.h"
#include "../controls/controller.h"
#include "../audio/soundplayer.h"
// #include "../font/font_ext.h"
struct GraphicsTask gGraphicsTasks[2];
Dynamic gfx_dynamic[2];
extern OSMesgQueue gfxFrameMsgQ;
extern OSMesgQueue *schedulerCommandQueue;

extern RenderMode gRenderMode = ToonFlatShadingRenderMode;

void *gLevelSegment;
void *gMaterialSegment;

#if WITH_GFX_VALIDATOR
#include "../../gfxvalidator/validator.h"
#endif

#define FORCE_VALIDATOR 1

#if WITH_DEBUGGER
#include "../../debugger/debugger.h"
#include "../../debugger/serial.h"

void graphicsOutputMessageToDebugger(char *message, unsigned len)
{
	gdbSendMessage(GDBDataTypeText, message, len);
}

#endif

#define RDP_OUTPUT_SIZE 0x4000

u64 *rdpOutput;
u64 __attribute__((aligned(16))) dram_stack[SP_DRAM_STACK_SIZE64 + 1];
u64 __attribute__((aligned(16))) gfxYieldBuf2[OS_YIELD_DATA_SIZE / sizeof(u64)];
u32 firsttime = 1;

u16 __attribute__((aligned(64))) zbuffer[SCREEN_HT * SCREEN_WD];

u16 *graphicsLayoutScreenBuffers(u16 *memoryEnd)
{
	gGraphicsTasks[0].framebuffer = memoryEnd - SCREEN_WD * SCREEN_HT;
	gGraphicsTasks[0].taskIndex = 0;
	gGraphicsTasks[0].msg.type = OS_SC_DONE_MSG;

	gGraphicsTasks[1].framebuffer = gGraphicsTasks[0].framebuffer - SCREEN_WD * SCREEN_HT;
	gGraphicsTasks[1].taskIndex = 1;
	gGraphicsTasks[1].msg.type = OS_SC_DONE_MSG;

	rdpOutput = (u64 *)(gGraphicsTasks[1].framebuffer - RDP_OUTPUT_SIZE / sizeof(u16));
	zeroMemory(rdpOutput, RDP_OUTPUT_SIZE);
	return (u16 *)rdpOutput;
}

#define CLEAR_COLOR GPACK_RGBA5551(0x32, 0x5D, 0x79, 1)

int fpsSlowdown = 0;
int fps = 0;
int heapCheckCounter = 200;


void graphicsCreateTask(struct GraphicsTask *targetTask, GraphicsCallback callback, void *data)
{
	struct RenderState *renderState = &targetTask->renderState;

	renderStateInit(renderState, targetTask->framebuffer, zbuffer);
	gSPSegment(renderState->dl++, 0, 0);
	// gSPSegment(renderState->dl++, LEVEL_SEGMENT, gLevelSegment);
	// gSPSegment(renderState->dl++, MATERIAL_SEGMENT, gMaterialSegment);

	gSPDisplayList(renderState->dl++, setup_rspstate);
	if (firsttime)
	{
		gSPDisplayList(renderState->dl++, rdpstateinit_dl);
		firsttime = 0;
	}
	gSPDisplayList(renderState->dl++, setup_rdpstate);

	// clear the zbuffer
	gDPSetDepthImage(renderState->dl++, osVirtualToPhysical(zbuffer));
	gDPPipeSync(renderState->dl++);
	gDPSetCycleType(renderState->dl++, G_CYC_FILL);
	gDPSetColorImage(renderState->dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, osVirtualToPhysical(zbuffer));
	gDPSetFillColor(renderState->dl++, (GPACK_ZDZ(G_MAXFBZ, 0) << 16 | GPACK_ZDZ(G_MAXFBZ, 0)));
	gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);

	// clear the framebuffer
	gDPPipeSync(renderState->dl++);
	gDPSetColorImage(renderState->dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, osVirtualToPhysical(targetTask->framebuffer));
	gDPSetFillColor(renderState->dl++, GPACK_RGBA5551(0, 0, 0, 1) << 16 | GPACK_RGBA5551(0, 0, 0, 1));
	gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
	//gDPSetScissor(renderState->dl++, G_SC_NON_INTERLACE, 0, 2, SCREEN_WD, SCREEN_HT - 2);

	// execute the render graphics callback
	// this is where the displaylists for the level, models, ui etc get added depending on the callback
	if (callback)
	{
		callback(data, renderState, targetTask);
	}
	gDPPipeSync(renderState->dl++);

	/*
	 * Draw Text
	 */
	gDPSetScissor(renderState->dl++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);
	font_init(&renderState->dl);
	font_set_transparent(1);

	font_set_scale(0.75, 0.75);
	font_set_win(200, 1);
	if(fpsSlowdown < 10){
		fpsSlowdown++;
	}
	else{
		fpsSlowdown = 0;
		fps = (int)(1000.0f/gDeltaTimeMS);
	}
	//uncomment to track free heap in on-screen debug console
	// if(heapCheckCounter < 200){
	// 	heapCheckCounter++;
	// }
	// else{
	// 	heapCheckCounter = 0;
	// 	char *heapfree[20];
	// 	sprintf(heapfree, "free heap:%d", calculateBytesFree());
	// 	console_add_msg(heapfree);
	// }
	char *renderMode_str[20];
	switch (gRenderMode){
	case ToonFlatShadingRenderMode:
		sprintf(renderMode_str, "Mode: Toon");
		break;
	case TextureAndLightingRenderMode:
		sprintf(renderMode_str, "Mode: TexAndLight");
		break;
	case TextureNoLightingRenderMode:
		sprintf(renderMode_str, "Mode: TexNoLight");
		break;
	case LightingNoTextureRenderMode:
		sprintf(renderMode_str, "Mode: LightNoTex");
		break; 
	case WireframeRenderMode:
		sprintf(renderMode_str, "Mode: Wireframe");
		break;
	};
	

	char *fps_str[7];
	sprintf(fps_str, "fps:%d", fps);
	SHOWFONT(&renderState->dl, fps_str, SCREEN_WD-60, 10, 50,50,50);
	SHOWFONT(&renderState->dl, renderMode_str, SCREEN_WD-300, SCREEN_HT - 20, 50, 50, 50);
	console_print_all(renderState);

	font_finish(&renderState->dl);

	/*
	 *
	 */

	gDPFullSync(renderState->dl++);
	gSPEndDisplayList(renderState->dl++);

	renderStateFlushCache(renderState);

	OSScTask *scTask = &targetTask->task;

	OSTask_t *task = &scTask->list.t;

	task->data_ptr = (u64 *)renderState->glist;
	task->data_size = (s32)renderState->dl - (s32)renderState->glist;
	task->type = M_GFXTASK;
	task->flags = OS_TASK_LOADABLE;
	task->ucode_boot = (u64 *)rspbootTextStart;
	task->ucode_boot_size = (u32)rspbootTextEnd - (u32)rspbootTextStart;
	if (gRenderMode == WireframeRenderMode)
	{
		/* use L3DEX2 microcode to render wireframe*/
		task->ucode = (u64 *)gspL3DEX2_fifoTextStart;
		task->ucode_data = (u64 *)gspL3DEX2_fifoDataStart;
	}
	else
	{
		/* use default F3DEX2 Microcode*/
		task->ucode = (u64 *)gspF3DEX2_fifoTextStart;
		task->ucode_data = (u64 *)gspF3DEX2_fifoDataStart;
	}

	// task->ucode = (u64 *)gspF3DEX2_xbusTextStart;
	// task->ucode_data = (u64 *)gspF3DEX2_xbusDataStart;
	

	task->output_buff = (u64 *)rdpOutput;
	task->output_buff_size = (u64 *)rdpOutput + RDP_OUTPUT_SIZE / sizeof(u64);
	task->ucode_data_size = SP_UCODE_DATA_SIZE;
	task->dram_stack = (u64 *)dram_stack;
	task->dram_stack_size = SP_DRAM_STACK_SIZE8;
	task->yield_data_ptr = (u64 *)gfxYieldBuf2;
	task->yield_data_size = OS_YIELD_DATA_SIZE;

	scTask->flags =
		OS_SC_NEEDS_RSP |
		OS_SC_NEEDS_RDP |
		OS_SC_LAST_TASK |
		OS_SC_SWAPBUFFER;
	scTask->framebuffer = targetTask->framebuffer;
	scTask->msg = &targetTask->msg;
	scTask->msgQ = &gfxFrameMsgQ;
	scTask->next = 0;
	scTask->state = 0;

#if WITH_GFX_VALIDATOR
#if WITH_DEBUGGER || FORCE_VALIDATOR
	struct GFXValidationResult validationResult;
	zeroMemory(&validationResult, sizeof(struct GFXValidationResult));

	if (gfxValidate(&scTask->list, MAX_DL_LENGTH, &validationResult) != GFXValidatorErrorNone)
	{

#if WITH_DEBUGGER
		gfxGenerateReadableMessage(&validationResult, graphicsOutputMessageToDebugger);
		gdbBreak();
#endif
	}

#endif // WITH_DEBUGGER
#endif // WITH_GFX_VALIDATOR

	osSendMesg(schedulerCommandQueue, (OSMesg)scTask, OS_MESG_BLOCK);
}

/**
 * @brief Setup Render Pipeline by clearing the previous GeometryMode, setting RenderMode as zBuffered, aliased, opaque Triangles
 * 
 * @param renderState 
 * @param dynamicp 
 */
void graphicsSetupPipeline(struct RenderState *renderState, Dynamic *dynamicp){
	gSPClearGeometryMode(renderState->dl++, 0xFFFFFFFF);
	gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE);
	// z-buffered, antialiased triangles
	gDPSetRenderMode(renderState->dl++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
	gSPSetGeometryMode(renderState->dl++, G_ZBUFFER); //is this needed?

	// setup view
	gSPMatrix(renderState->dl++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)),
			  G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
	gSPMatrix(renderState->dl++, OS_K0_TO_PHYSICAL(&(dynamicp->camera)),
			  G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
}

void graphicsApplyRenderMode(struct RenderState *renderState, Lights0 *amb_light, Lights1 *sun_light, int ambientOnly)
{
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
		
		if (ambientOnly == TRUE)
		{
			gSPSetLights0(renderState->dl++, (*amb_light));
		}
		else
		{
			gSPSetLights0(renderState->dl++, (*amb_light));
			gSPSetLights1(renderState->dl++, (*sun_light));
		}
		gSPSetGeometryMode(renderState->dl++, G_SHADE | G_SHADING_SMOOTH | G_LIGHTING | G_CULL_BACK);
		gDPSetCombineMode(renderState->dl++, G_CC_MODULATERGB, G_CC_MODULATERGB);
		
		break;
	case LightingNoTextureRenderMode:
		
		if (ambientOnly == TRUE)
		{
			gSPSetLights0(renderState->dl++, (*amb_light));
		}
		else
		{
			gSPSetLights1(renderState->dl++, (*sun_light));
		}
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
}