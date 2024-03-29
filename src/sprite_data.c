#include "sprite.h"

#include "assets/sprites/honk1spr.h"
#include "assets/sprites/honk2spr.h"
#include "assets/sprites/honk3spr.h"
#include "assets/sprites/testspr.h"

unsigned short *HonkSpriteAnimFrames[] = {
	Sprite_honk1spr,
	Sprite_honk2spr,
	Sprite_honk3spr,
};

unsigned short *getSpriteForSpriteType(SpriteType spriteType, int frame)
{
	switch (spriteType)
	{
	case HonkSprite:
		return HonkSpriteAnimFrames[frame];
	default:
		return Sprite_testspr;
	}
}
