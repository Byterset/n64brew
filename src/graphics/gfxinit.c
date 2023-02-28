
#include <nusys.h>
#include "graphic.h"

/*
  The viewport structure
  The conversion from (-1,-1,-1)-(1,1,1).  The decimal part of 2-bit.
 */
static Vp vp = {
    SCREEN_WD * 2, SCREEN_HT * 2, G_MAXZ / 2, 0, /* The scale factor  */
    SCREEN_WD * 2, SCREEN_HT * 2, G_MAXZ / 2, 0, /* Move  */
};

static Vp vp_pal = {
    SCREEN_WD * 2, SCREEN_HT_PAL * 2, G_MAXZ / 2, 0, /* The scale factor  */
    SCREEN_WD * 2, SCREEN_HT_PAL * 2, G_MAXZ / 2, 0, /* Move  */
};


Lights1 default_sun_light = gdSPDefLights1(120,
                                           120,
                                           120, /* weak ambient light */
                                           255,
                                           255,
                                           255, /* white light */
                                           80,
                                           80,
                                           0);


