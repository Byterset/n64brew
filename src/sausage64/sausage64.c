/**
 * @file sausage64.c
 * @author original code by buu342 (https://github.com/buu342/Blender-Sausage64) modified by byterset (https://github.com/byterset)
 * @brief A very simple library that handles Sausage64 models exported by Arabiki64.
 * @date 2023-11-21
 * 
 * 
 */

#include "sausage64.h"

/*********************************
             Globals
*********************************/

static float s64_viewmat[4][4];
static float s64_projmat[4][4];

/*********************************
      Helper Math Functions
*********************************/

/**
 * @brief Clamps a value between two others
 * 
 * @param value The value to clamp
 * @param min The minimum value
 * @param max The maximum value
 * @return f32 The clamped value
 */
static inline f32 s64clamp(f32 value, f32 min, f32 max)
{
    const f32 result = value < min ? min : value;
    return result > max ? max : result;
}


/**
 * @brief Returns the linear interpolation of two values given a fraction
 *
 * @param startValue The first value
 * @param endValue The target value
 * @param fraction The fraction
 * @return f32 The interpolated result
 */
static inline f32 s64lerp(f32 startValue, f32 endValue, f32 fraction)
{
    return startValue + fraction * (endValue - startValue);
}


/*********************************
       Sausage64 Functions
*********************************/

/**
 * @brief Initialize a model helper struct
 * 
 * @param mdl The model helper to initialize
 * @param mdldata The model data 
 * @param matrices The array of matrices for each mesh part
 * @param position The position of the model
 * @param scale The scale of the model
 * @param rotation The rotation of the model
 */
void sausage64_initmodel(s64ModelHelper* mdl, s64ModelData* mdldata, Mtx* matrices, struct Vector3* position, struct Vector3* scale, struct Quaternion* rotation)
{
    mdl->interpolate = TRUE;
    mdl->loop = TRUE;
    mdl->curkeyframe = 0;
    mdl->position = position;
    mdl->scale = scale;
    mdl->rotation = rotation;

    // Set the the first animation if it exists, otherwise set the animation to NULL
    if (mdldata->animcount > 0)
    {
        s64Animation* animdata = &mdldata->anims[0];
        mdl->curanim = animdata;
        mdl->curanimlen = animdata->keyframes[animdata->keyframecount-1].framenumber;
    }
    else
    {
        mdl->curanim = NULL;
        mdl->curanimlen = 0;
    }

    // Initialize the rest of the data
    mdl->animtick = 0;
    mdl->predraw = NULL;
    mdl->postdraw = NULL;
    mdl->animcallback = NULL;
    mdl->mdldata = mdldata;
    mdl->matrix = matrices;


}


/**
 * @brief Sets the camera for Sausage64 to use for billboarding
 * 
 * @param view The view matrix
 * @param projection The projection matrix
 */
void sausage64_set_camera(Mtx* view, Mtx* projection)
{
    guMtxL2F(s64_viewmat, view);
    guMtxL2F(s64_projmat, projection);
}


/**
 * @brief Set a function that gets called before any model is rendered
 * 
 * @param mdl The model helper pointer
 * @param predraw The pre draw function
 */
inline void sausage64_set_predrawfunc(s64ModelHelper* mdl, void (*predraw)(u16))
{
    mdl->predraw = predraw;
}


/**
 * @brief Set a function that gets called after any model is rendered
 * 
 * @param mdl The model helper pointer
 * @param postdraw The post draw function
 */
inline void sausage64_set_postdrawfunc(s64ModelHelper* mdl, void (*postdraw)(u16))
{
    mdl->postdraw = postdraw;
}


/**
 * @brief Set a function that gets called when an animation finishes
 * 
 * @param mdl The model helper pointer
 * @param animcallback The animation end callback function
 */
inline void sausage64_set_animcallback(s64ModelHelper* mdl, void (*animcallback)(u16))
{
    mdl->animcallback = animcallback;
}


/**
 * @brief Updates the animation keyframe based on the animation tick
 * 
 * @param mdl The model helper pointer
 */
static void sausage64_update_anim(s64ModelHelper* mdl)
{
    const s64Animation* anim = mdl->curanim;
    const float curtick = mdl->animtick;
    const u32 curkeyframe = mdl->curkeyframe;
    const u32 curframenum = anim->keyframes[curkeyframe].framenumber;
    const u32 nframes = anim->keyframecount;
    u32 nextkeyframe = (curkeyframe+1)%nframes;
    
    // Check if we changed animation frame
    if (curtick >= anim->keyframes[nextkeyframe].framenumber || curtick < curframenum)
    {
        if (curtick > curframenum) // Animation advanced to next frame
        {
            u32 i=0;
            
            // Cycle through all frames, starting at the one after the current frame
            do
            {
                // Update the keyframe if we've passed this keyframe's number
                if (curtick >= anim->keyframes[nextkeyframe].framenumber)
                {
                    // Go to the next keyframe, and stop
                    mdl->curkeyframe = nextkeyframe;
                    return;
                }
                
                // If that was a failure, go to the next frame
                nextkeyframe = (nextkeyframe+1)%nframes;
                i++;
            }
            while (i<nframes);
        }
        else if (curkeyframe > 0 && curtick < anim->keyframes[1].framenumber) // Animation rolled over to the first frame (special case for speedup reasons)
        {
            mdl->curkeyframe = 0;
        }
        else // Animation is potentially going backwards
        {
            u32 i=0;
            s32 prevkeyframe = curkeyframe-1;
            if (prevkeyframe < 0)
                prevkeyframe = nframes-1;
            
            // Cycle through all frames (backwards), starting at the one before the current frame
            do
            {
                // Update the keyframe if we've passed this keyframe's number
                if (curtick >= anim->keyframes[prevkeyframe].framenumber)
                {
                    // Go to the next keyframe, and stop
                    mdl->curkeyframe = prevkeyframe;
                    return;
                }
                
                // If that was a failure, go to the previous frame
                prevkeyframe--;
                if (prevkeyframe < 0)
                    prevkeyframe = nframes-1;
                i++;
            }
            while (i<nframes);    
        }
    }
}


/**
 * @brief Advances the animation tick by the given amount
 * 
 * @param mdl The model helper pointer
 * @param tickamount The amount to increase the animation tick by
 */
void sausage64_advance_anim(s64ModelHelper* mdl, float tickamount)
{
    char loop = TRUE;
    float division;
    mdl->animtick += tickamount;
    
    // If the animation ended, call the callback function and roll the tick value over
    if (mdl->animtick >= mdl->curanimlen)
    {
        // Execute the animation end callback function
        if (mdl->animcallback != NULL)
            mdl->animcallback(mdl->curanim - &mdl->mdldata->anims[0]);
            
        // If looping is disabled, then stop
        if (!mdl->loop)
        {
            mdl->animtick = (float)mdl->curanimlen;
            mdl->curkeyframe = mdl->curanim->keyframecount-1;
            return;
        }
        
        // Calculate the correct tick         
        division = mdl->animtick/((float)mdl->curanimlen);
        mdl->animtick = (division - ((int)division))*((float)mdl->curanimlen);
    }
    else if (mdl->animtick <= 0)
    {
        // Execute the animation end callback function
        if (mdl->animcallback != NULL)
            mdl->animcallback(mdl->curanim - &mdl->mdldata->anims[0]);
            
        // If looping is disabled, then stop
        if (!mdl->loop)
        {
            mdl->animtick = 0;
            mdl->curkeyframe = 0;
            return;
        }
        
        // Calculate the correct tick       
        division = mdl->animtick/((float)mdl->curanimlen);
        mdl->animtick = (1+(division - ((int)division)))*((float)mdl->curanimlen);
        mdl->curkeyframe = mdl->curanim->keyframecount-1;
    }
    
    // Update the animation
    if (mdl->curanim->keyframecount > 0 && loop)
        sausage64_update_anim(mdl);
}


/**
 * @brief Sets an animation on the model. Does not perform error checking if an invalid animation is given.
 * 
 * @param mdl The model helper pointer
 * @param anim The ANIMATION_* macro to set
 */

void sausage64_set_anim(s64ModelHelper* mdl, u16 anim)
{
    s64Animation* animdata = &mdl->mdldata->anims[anim];
    mdl->curanim = animdata;
    mdl->curanimlen = animdata->keyframes[animdata->keyframecount-1].framenumber;
    mdl->curkeyframe = 0;
    mdl->animtick = 0;
    if (animdata->keyframecount > 0)
        sausage64_update_anim(mdl);
}

/**
 * @brief Renders a part of a Sausage64 model.
 * Calculates the model-view transformation matrix for the part based on
 * position, rotation, and scale in the frame data. Handles interpolation
 * between current and next frame data if enabled.
 *
 * For billboard parts, calculates a rotation matrix based on the current
 * view matrix. For other parts, calculates a rotation matrix from the
 * quaternion in the frame data.
 *
 * Multiplies the matrices together, converts to a N64 matrix, and sets
 * it as the model-view matrix before drawing the display list for the
 * part mesh.
 *
 * @param glistp A pointer to a display list pointer
 * @param mesh The mesh that will be drawn
 * @param cfdata The current frame data
 * @param nfdata The next frame data
 * @param interpolate Whether to interpolate between the current and next frame data
 * @param l The interpolation amount
 * @param matrix The matrix to store the mesh's transformation
 */
static inline void sausage64_drawpart(Gfx **glistp, s64Mesh *mesh, const s64FrameData *cfdata, const s64FrameData *nfdata, const u8 interpolate, const f32 l, Mtx *matrix)
{
    float helper1[4][4];
    float helper2[4][4];
    float test[4][4];
    Quaternion q;
    q.w = cfdata->rot[0];
    q.x = cfdata->rot[1];
    q.y = cfdata->rot[2];
    q.z = cfdata->rot[3];
    // Calculate the transformations on the CPU
    if (interpolate)
    {
        // Setup the quaternions
        if (!mesh->is_billboard)
        {
            Quaternion qn;
            qn.w = nfdata->rot[0];
            qn.x = nfdata->rot[1];
            qn.y = nfdata->rot[2];
            qn.z = nfdata->rot[3];
            // Interpolate the rotation
            quatLerp(&q, &qn, l, &q);
        }

        // Combine the translation and scale matrix
        guTranslateF(helper1,
                     s64lerp(cfdata->pos[0], nfdata->pos[0], l),
                     s64lerp(cfdata->pos[1], nfdata->pos[1], l),
                     s64lerp(cfdata->pos[2], nfdata->pos[2], l));
        // Reposition Catherine to the starting position of the player (temporary)
        // TODO: Big Todo!! include pos, rot, scale from s64ModelHelper in these calculations
        // only calculate the translation matrix of worldtransform once in the drawmodel function
        // then pass it to the drawpart function!
        guTranslateF(test,
                     -1500,
                     125,
                     -1550);
        guMtxCatF(helper1, test, helper1);
        guScaleF(helper2,
                 s64lerp(cfdata->scale[0], nfdata->scale[0], l),
                 s64lerp(cfdata->scale[1], nfdata->scale[1], l),
                 s64lerp(cfdata->scale[2], nfdata->scale[2], l));
        guMtxCatF(helper2, helper1, helper1);
    }
    else
    {
        // Combine the translation and scale matrix
        guTranslateF(helper1,
                     cfdata->pos[0],
                     cfdata->pos[1],
                     cfdata->pos[2]);
        guScaleF(helper2, cfdata->scale[0], cfdata->scale[1], cfdata->scale[2]);
        guMtxCatF(helper2, helper1, helper1);
    }

    // Combine the rotation matrix
    if (!mesh->is_billboard)
    {
        quatToMatrix(&q, &helper2);
        guMtxCatF(helper2, helper1, helper1);
    }
    else
    {
        helper2[0][0] = s64_viewmat[0][0];
        helper2[1][0] = s64_viewmat[0][1];
        helper2[2][0] = s64_viewmat[0][2];
        helper2[3][0] = 0;

        helper2[0][1] = s64_viewmat[1][0];
        helper2[1][1] = s64_viewmat[1][1];
        helper2[2][1] = s64_viewmat[1][2];
        helper2[3][1] = 0;

        helper2[0][2] = s64_viewmat[2][0];
        helper2[1][2] = s64_viewmat[2][1];
        helper2[2][2] = s64_viewmat[2][2];
        helper2[3][2] = 0;

        helper2[0][3] = 0;
        helper2[1][3] = 0;
        helper2[2][3] = 0;
        helper2[3][3] = 1;
        guMtxCatF(helper2, helper1, helper1);
    }

    guMtxF2L(helper1, matrix);

    // Draw the body part
    gSPMatrix((*glistp)++, OS_K0_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPDisplayList((*glistp)++, mesh->dl);
    gSPPopMatrix((*glistp)++, G_MTX_MODELVIEW);
}

/**
 * @brief Renders a Sausage64 model.
 * 
 * @param glistp A pointer to a display list pointer
 * @param mdl The model helper data
 */
void sausage64_drawmodel(Gfx **glistp, s64ModelHelper *mdl)
{
    u16 i;
    const s64ModelData *mdata = mdl->mdldata;
    const u16 mcount = mdata->meshcount;
    const s64Animation *anim = mdl->curanim;

    // If we have a valid animation
    if (anim != NULL)
    {
        const s64KeyFrame *ckframe = &anim->keyframes[mdl->curkeyframe];
        const s64KeyFrame *nkframe = &anim->keyframes[(mdl->curkeyframe + 1) % anim->keyframecount];
        f32 l = 0;

        // Prevent division by zero when calculating the lerp amount
        if (nkframe->framenumber - ckframe->framenumber != 0)
            l = ((f32)(mdl->animtick - ckframe->framenumber)) / ((f32)(nkframe->framenumber - ckframe->framenumber));

        // Iterate through each mesh
        for (i = 0; i < mcount; i++)
        {
            const s64FrameData *cfdata = &ckframe->framedata[i];
            const s64FrameData *nfdata = &nkframe->framedata[i];

            // Call the pre draw function
            if (mdl->predraw != NULL)
                mdl->predraw(i);

            // Draw this part of the model with animations
            sausage64_drawpart(glistp, &mdata->meshes[i], cfdata, nfdata, mdl->interpolate, l, &mdl->matrix[i]);

            // Call the post draw function
            if (mdl->postdraw != NULL)
                mdl->postdraw(i);
        }
    }
    else
    {
        // Iterate through each mesh
        for (i = 0; i < mcount; i++)
        {
            // Call the pre draw function
            if (mdl->predraw != NULL)
                mdl->predraw(i);

            // Draw this part of the model without animations
            gSPDisplayList((*glistp)++, mdata->meshes[i].dl);

            // Call the post draw function
            if (mdl->postdraw != NULL)
                mdl->postdraw(i);
        }
    }
}
