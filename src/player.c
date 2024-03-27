#include <math.h>

#include "game.h"
#include "gameutils.h"
#include "gooseanimtypes.h"
#include "item.h"
#include "player.h"
#include "util/time.h"
#include "constants.h"
#include "util/time.h"

// max speed, slightly slower than character speed
#define PLAYER_SPEED 3.0F
#define PLAYER_WALK_SPEED_RATIO 0.5
#define PLAYER_FORCE 5000.0F
#define PLAYER_MAX_TURN_SPEED 15.0f
#define PLAYER_NEAR_OBJ_DIST 100.0f
#define PLAYER_PICKUP_COOLDOWN 10
#define PLAYER_WALK_ANIM_MOVEMENT_DIVISOR 100.0
#define PLAYER_PHYS_WALK_ANIM_MOVEMENT_DIVISOR 5200.0

static Vector3 playerItemOffset = {0.0F, 80.0F, 0.0F};

void Player_init(Player *self, GameObject *obj)
{
	ItemHolder_init(&self->itemHolder, PlayerItemHolder, (void *)self);
	AnimationState_init(&self->animState);
	self->player_object = obj;
	obj->animState = &self->animState;
	// setup picked-up object attachment point
	self->animState.attachment.boneIndex = (int)goosehead_gooseheadmesh;
	self->animState.attachment.offset.x = 14;
	self->animState.attachment.offset.z = -2;
	self->animState.attachment.rotation.x = 90;
	self->animState.spriteAttachment.offset.x = 14;
	self->animState.spriteAttachment.offset.z = -2;
	self->lastPickupTick = 0;
}

int Player_debounceInput(unsigned int lastTrigger, unsigned int cooldown)
{
	if (Game_get()->tick > lastTrigger + cooldown)
	{
		return TRUE;
	}
	return FALSE;
}

void Player_setVisibleItemAttachment(Player *self, ModelType modelType)
{
	self->animState.attachment.modelType = modelType;
}

float Player_move(Player *self, Input *input, Game *game)
{
	Vector3 inputDirection, updatedHeading, playerMovement;
	float movementMagnitude, movementSpeedRatio,
		resultantMovementSpeed, angleRad;
	GameObject *player_object;

	player_object = self->player_object;

	movementSpeedRatio = (input->run ? 1.3 : PLAYER_WALK_SPEED_RATIO);

	vector3Init(&inputDirection, input->direction.x, 0.0F, input->direction.y);
	movementMagnitude = vector3Mag(&inputDirection);
	// keep the magnitude to re-apply after we get the updated heading
	
	// apply rotation (less if running) and get updated heading

	if (vector2MagSqr(&input->direction) > 0)
	{

		// Get the Angle in Rad of the input direction and flip it to get the correct angle
		angleRad = -vector2Angle(&input->direction);

		// Calculate the rotation quaternion
		Quaternion rotationQuat;
		quatAxisAngle(&(Vector3){0.0F, 1.0F, 0.0F}, angleRad, &rotationQuat);

		// Calculate the rotation speed based on PLAYER_MAX_TURN_SPEED
		float rotationSpeed = (input->run ? 0.1 : 1.0) * PLAYER_MAX_TURN_SPEED;

		// Get the current rotation quaternion of the player object's transform
		Quaternion currentRotationQuat = player_object->transform.rotation;

		// Interpolate between the current rotation and the target rotation
		Quaternion targetRotationQuat;
		quatLerp(&currentRotationQuat, &rotationQuat, rotationSpeed * gDeltaTimeSec, &targetRotationQuat);

		// Set the y rotation of the player object's transform
		transform_set_rotation(&(player_object->transform), targetRotationQuat);

		
	}

	// Calculate the heading vector based on the resulting quaternion rotation
	Vector3 forward = {1.0F, 0.0F,0.0F};
	vector3Init(&updatedHeading, 0.0F, 0.0F, 0.0F);
	quatGetHeadingDir(&player_object->transform.rotation, &forward, &updatedHeading);

	// move based on heading
	playerMovement = updatedHeading;
	// prevent moving too fast diagonally
	vector3NormalizeSelf(&playerMovement);

	// movement
	vector3ScaleSelf(&playerMovement,
					movementMagnitude * PLAYER_SPEED * movementSpeedRatio * (60 * gDeltaTimeSec));
	
	// vector3AddToSelf(&player_object->transform.position, &playerMovement);
	transform_translate(&player_object->transform, playerMovement);
	resultantMovementSpeed = vector3Mag(&playerMovement);
	resultantMovementSpeed /= PLAYER_WALK_ANIM_MOVEMENT_DIVISOR;

	return resultantMovementSpeed;
}

void Player_update(Player *self, Input *input, Game *game)
{
	float resultantMovementSpeed;
	GameObject *player_object;
	int i;
	Item *item;
	player_object = self->player_object;
	resultantMovementSpeed = Player_move(self, input, game);

	// update animation
	if (resultantMovementSpeed > 0.001)
	{
		if (self->animState.state != goose_walk_anim)
		{
			// enter walk anim
			self->animState.progress = 0.0;
		}
		else
		{
			// advance walk anim
			self->animState.progress = fmodf(
				self->animState.progress +
					resultantMovementSpeed * (GameUtils_inWater(player_object)
												  ? 0.5
												  : 1.0), // slower anim in water
				1.0);
		}
		self->animState.state = goose_walk_anim;
	}
	else
	{
		self->animState.state = goose_idle_anim;
		self->animState.progress = 0.0;
	}

	// update held item visual attachment
	if (self->itemHolder.heldItem != NULL)
	{
		Player_setVisibleItemAttachment(self,
										self->itemHolder.heldItem->obj->modelType);
	}
	else
	{
		Player_setVisibleItemAttachment(self, NoneModel);
	}

	if (self->itemHolder.heldItem)
	{
		// bring item with you
		self->itemHolder.heldItem->obj->transform.position = self->player_object->transform.position;
		vector3AddToSelf(&self->itemHolder.heldItem->obj->transform.position, &playerItemOffset);
	}

	if (input->pickup &&
		Player_debounceInput(self->lastPickupTick, PLAYER_PICKUP_COOLDOWN))
	{
		if (self->itemHolder.heldItem)
		{
			// drop item
			Item_drop(self->itemHolder.heldItem);
			self->lastPickupTick = game->tick;
		}
		else
		{
			// TODO: should have a concept of target so goose can look at items to
			// pickup
			for (i = 0, item = game->items; i < game->itemsCount; i++, item++)
			{
				if (vector3Dist(&self->player_object->transform.position, &item->obj->transform.position) <
					PLAYER_NEAR_OBJ_DIST)
				{
					// yes, pick up
					Item_take(item, &self->itemHolder);
					if (self->itemHolder.heldItem == item)
					{
						self->lastPickupTick = game->tick;
						// one's enough
						break;
					}
				}
			}
		}
	}
}

void Player_haveItemTaken(Player *self, Item *item)
{
	// react to item being taken
}

#ifndef __N64__
#include <stdio.h>
void Player_print(Player *self)
{
	printf("Player heldItem=%s pos=",
		   self->itemHolder.heldItem
			   ? ModelTypeStrings[self->itemHolder.heldItem->obj->modelType]
			   : "none");
	// Vec3d_print(&self->player_object->transform.position);
}

void Player_toString(Player *self, char *buffer)
{
	char pos[60];
	char vel[60];
	vector3toString(&self->player_object->transform.position, pos);
	vector3toString(&self->player_object->physBody->nonIntegralVelocity, vel);
	sprintf(buffer, "Player id=%d pos=%s vel=%s heldItem=%s", self->player_object->id,
			pos, vel,
			self->itemHolder.heldItem
				? ModelTypeStrings[self->itemHolder.heldItem->obj->modelType]
				: "none");
}
#endif
