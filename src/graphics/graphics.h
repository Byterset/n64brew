#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <ultra64.h>
#include "../constants.h"
#include <sched.h>
#include "renderstate.h"
#include "../defs.h"


#if HIGH_RES
#define SCREEN_WD   640
#define SCREEN_HT   480
#define ANTIALIASING 1
#else
#define SCREEN_WD   320
#define SCREEN_HT   240
#define ANTIALIASING 0
#endif

// render mode to write to z buffer but not depth compare when rendering
#define RM_AA_ZUPD_OPA_SURF(clk)                                      \
  AA_EN | Z_UPD | IM_RD | CVG_DST_CLAMP | ZMODE_OPA | ALPHA_CVG_SEL | \
      GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define G_RM_AA_ZUPD_OPA_SURF RM_AA_ZUPD_OPA_SURF(1)
#define G_RM_AA_ZUPD_OPA_SURF2 RM_AA_ZUPD_OPA_SURF(2)

#define RM_ZUPD_OPA_SURF(clk)                        \
  Z_UPD | CVG_DST_FULL | ALPHA_CVG_SEL | ZMODE_OPA | \
      GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define G_RM_ZUPD_OPA_SURF RM_ZUPD_OPA_SURF(1)
#define G_RM_ZUPD_OPA_SURF2 RM_ZUPD_OPA_SURF(2)

struct GraphicsTask {
    struct RenderState renderState;
    OSScTask task;
    OSScMsg msg;
    u16 *framebuffer;
    u16 taskIndex;
};

typedef float ViewportF[4];


/*
rotation/scale is from mtx[0][0] to mtx[2][2]
translation (13-15) is from mtx[3][0] to mtx[3][2]
 */
typedef float MtxF[4][4];

/* The structure of the projection-matrix  */
typedef struct {
  // this stuff gets sent to the RCP so it must be 64bit aligned otherwise
  // everything will fuck up
  Mtx projection;
  Mtx modeling;
  Mtx camera;

  Mtx zUpToYUpCoordinatesRotation;

  Mtx objTransforms[MAX_WORLD_OBJECTS];
} Dynamic;

extern Dynamic gfx_dynamic[];

extern struct GraphicsTask gGraphicsTasks[2];
extern Vp fullscreenViewport;

extern void* gLevelSegment;
extern void* gMaterialSegment;

#define GET_GFX_TYPE(gfx)       (_SHIFTR((gfx)->words.w0, 24, 8))

typedef void (*GraphicsCallback)(void* data, struct RenderState* renderState, struct GraphicsTask* task);

u16* graphicsLayoutScreenBuffers(u16* memoryEnd);
void graphicsCreateTask(struct GraphicsTask* targetTask, GraphicsCallback callback, void* data);

#endif