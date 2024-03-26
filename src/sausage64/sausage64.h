#ifndef SAUSAGE64_H
#define SAUSAGE64_H

#include "../math/transform.h"
#include "../util/time.h"

#ifndef _ULTRA64_H_
#include <ultra64.h>
#endif
#define s64Gfx Gfx

/*********************************
        Sausage64 Structs
*********************************/

/**
 * @brief The data for a single frame of a single mesh.
 * @note pos & scale are f32[3] (Vector3), rot is f32[4] (Quaternion).
 */
typedef struct
{
    f32 pos[3];
    f32 rot[4];
    f32 scale[3];
} s64FrameData;

/**
 * @brief A single keyframe of a Sausage64 Animation in a Model holding the Framedata of all Meshes in the Model.
 * 
 */
typedef struct
{
    u32 framenumber;
    s64FrameData *framedata;
} s64KeyFrame;

/**
 * @brief A Sausage64 Animation. Holds a list of KeyFrames, the number of KeyFrames and the name of the Animation.
 * 
 */
typedef struct
{
    const char *name;
    u32 keyframecount;
    s64KeyFrame *keyframes;
} s64Animation;

/**
 * @brief A Sausage64 Mesh. Holds the name of the Mesh, the Display List of the Mesh and if it is a Billboard or not.
 * 
 */
typedef struct
{
    const char *name;
    const u32 is_billboard;
    s64Gfx *dl;
} s64Mesh;

/**
 * @brief Sausage64 Model Data. Holds the number of Meshes and the number of Animations, a pointer to the Meshes and Animations and their names
 * 
 */
typedef struct
{
    u16 meshcount;
    u16 animcount;
    s64Mesh *meshes;
    s64Animation *anims;
} s64ModelData;

/**
 * @brief S64 Model Helper. Holds the model data, the current animation, the current keyframe, the current animation tick, the current matrix, the pre draw and post draw functions and the model data.
 * @note This is used to actually create and hold a Sausage64 Model and it's data.
 * Initialize it using sausage64_initmodel
 */
typedef struct
{
    u8 interpolate;
    u8 loop;
    s64Animation *curanim;
    u32 curanimlen;
    float animtick;
    u32 curkeyframe;
    Mtx *matrix;
    void (*predraw)(u16);
    void (*postdraw)(u16);
    void (*animcallback)(u16);
    s64ModelData *mdldata;
    Transform transform;
} s64ModelHelper;

/*********************************
       Sausage64 Functions
*********************************/

/*==============================
    sausage64_initmodel
    Initialize a model helper struct
    @param The model helper to initialize
    @param The model data
    @param An array of matrices for each mesh
           part
==============================*/

extern void sausage64_initmodel(s64ModelHelper *mdl, s64ModelData *mdldata, Mtx *matrices, struct Vector3 position, struct Vector3 scale, struct Quaternion rotation);

/*==============================
    sausage64_set_camera
    Sets the camera for Sausage64 to use for billboarding
    @param The view matrix
    @param The projection matrix
==============================*/

extern void sausage64_set_camera(Mtx *view, Mtx *projection);

/*==============================
    sausage64_set_anim
    Sets an animation on the model. 
    Performs a small sanity check if there are any animations in the mdlData and if the given anim is in the range of anims in the model
    @param The model helper pointer
    @param The ANIMATION_* macro to set
==============================*/

extern void sausage64_set_anim(s64ModelHelper *mdl, u16 anim);

/*==============================
    sausage64_set_animcallback
    Set a function that gets called when an animation finishes
    @param The model helper pointer
    @param The animation end callback function
==============================*/

extern void sausage64_set_animcallback(s64ModelHelper *mdl, void (*animcallback)(u16));

/*==============================
    sausage64_set_predrawfunc
    Set a function that gets called before any mesh is rendered
    @param The model helper pointer
    @param The pre draw function
==============================*/

extern void sausage64_set_predrawfunc(s64ModelHelper *mdl, void (*predraw)(u16));

/*==============================
    sausage64_set_postdrawfunc
    Set a function that gets called after any mesh is rendered
    @param The model helper pointer
    @param The post draw function
==============================*/

extern void sausage64_set_postdrawfunc(s64ModelHelper *mdl, void (*postdraw)(u16));

/*==============================
    sausage64_advance_anim
    Advances the animation tick by the given amount
    @param The model helper pointer
    @param The amount to increase the animation tick by
==============================*/

void sausage64_advance_anim(s64ModelHelper *mdl, float tickamount);


/**
 * @brief Renders a Sausage64 model
 * 
 * @param glistp A pointer to a display list pointer
 * @param mdl The model helper data
 */
void sausage64_drawmodel(Gfx **glistp, s64ModelHelper *mdl);

#endif