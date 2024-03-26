/**
 * @file animation.c
 * @author James Friend
 * @brief Implements animated models

 * 
 */
#include <math.h>
#ifndef __N64__
#include <assert.h>
#endif

#include "animation.h"
#include "../constants.h"

void AnimationState_init(AnimationState *self)
{
	self->state = 0;
	self->progress = 0.0f;
	AnimationBoneAttachment_init(&self->attachment);
	AnimationBoneSpriteAttachment_init(&self->spriteAttachment);
}

void AnimationBoneAttachment_init(AnimationBoneAttachment *self)
{
	self->boneIndex = 0;
	self->modelType = NoneModel;
	vector3Init(&self->offset, 0.0f, 0.0f, 0.0f);
	vector3Init(&self->rotation, 0.0f, 0.0f, 0.0f);
}

void AnimationBoneSpriteAttachment_init(AnimationBoneSpriteAttachment *self)
{
	self->boneIndex = 0;
	self->spriteType = NoneSprite;
	vector3Init(&self->offset, 0.0f, 0.0f, 0.0f);
	self->startTick = 0;
}

void AnimationInterpolation_calc(AnimationInterpolation *self,
								 AnimationState *state,
								 AnimationRange *animRange)
{
	int animDuration, currentFrame;
	// frames since range start, with intra-frame decimal part
	float integralFrameRel;
	animDuration = animRange->end - animRange->start;
	integralFrameRel = state->progress * animDuration;
	currentFrame = animRange->start + (int)floorf(integralFrameRel);
	self->currentFrame = currentFrame;
	// calculate next frame, wrapping around to range start offset
	self->nextFrame =
		animRange->start + (((int)ceilf(integralFrameRel)) % animDuration);
	self->t = fmodf(integralFrameRel, 1.0);
}

// gets a non-interpolated AnimationFrame for one bone in a rig
void AnimationFrame_get(
	AnimationInterpolation *interp, // result of AnimationInterpolation_calc()
	AnimationFrame *animData,		// pointer to start of AnimationFrame list
									// exported for some rig
	int animDataNumBones,			// num bones in rig used by animData
	int boneIdx,					// index of bone in rig to produce transform for
	AnimationFrame *result			// the resultant animation frame
)
{
	int frameDataOffset;

#ifndef __N64__
	invariant(animDataNumBones <= MAX_ANIM_MESH_PARTS);
#endif

	frameDataOffset = interp->currentFrame * animDataNumBones + boneIdx;
	*result = *(animData + frameDataOffset);

#ifndef __N64__
	invariant(result->object == boneIdx);
#endif
}

// produces an interpolated AnimationFrame for one bone in a rig
void AnimationFrame_lerp(
	AnimationInterpolation *interp, // result of AnimationInterpolation_calc()
	AnimationFrame *animData,		// pointer to start of AnimationFrame list
									// exported for some rig
	int animDataNumBones,			// num bones in rig used by animData
	int boneIdx,					// index of bone in rig to produce transform for
	AnimationFrame *result			// the resultant interpolated animation frame
)
{
	Quaternion quaternionA, quaternionB;
	struct Vector3 radiansA, radiansB, radiansResult;
	int frameDataOffsetA, frameDataOffsetB;
	AnimationFrame *a, *b;

#ifndef __N64__
	invariant(animDataNumBones <= MAX_ANIM_MESH_PARTS);
#endif

	frameDataOffsetA = interp->currentFrame * animDataNumBones + boneIdx;
	frameDataOffsetB = interp->nextFrame * animDataNumBones + boneIdx;
	a = animData + frameDataOffsetA;
	b = animData + frameDataOffsetB;

#ifndef __N64__
	// if either of these fail, the animation data is messed up
	invariant(a->object == boneIdx);
	invariant(b->object == boneIdx);
#endif

	// start with data from A
	*result = *a;
	vector3Lerp(&result->position, &b->position, interp->t, &result->position);

	quatFromEulerDegrees(&a->rotation, &quaternionA);
	quatFromEulerDegrees(&b->rotation, &quaternionB);

	quatLerp(&quaternionA, &quaternionB, interp->t, &quaternionA);

	quatToEulerDegrees(&quaternionA, &result->rotation);


}
