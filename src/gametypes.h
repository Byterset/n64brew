
#ifndef _GAMETYPES_H_
#define _GAMETYPES_H_

#include "animation/animation.h"
#include "characterstate.h"
#include "gameobject.h"
#include "pathfinding.h"
#include "physics/physics.h"

typedef enum ItemHolderType
{
	PlayerItemHolder,
	CharacterItemHolder,
} ItemHolderType;

struct ItemStruct;

// composable struct for players and characters which hold things
typedef struct ItemHolder
{
	ItemHolderType itemHolderType;
	void *owner;

	unsigned int acquiredTick;
	struct ItemStruct *heldItem;
} ItemHolder;

typedef struct Player
{
	ItemHolder itemHolder;

	GameObject *goose;
	AnimationState animState;

	unsigned int lastPickupTick;
} Player;

typedef struct Character
{
	ItemHolder itemHolder;

	GameObject *obj;
	AnimationState animState;

	struct ItemStruct *targetItem;
	struct ItemStruct *defaultActivityItem;
	struct Vector3 defaultActivityLocation;
	struct Vector3 movementTarget; // immediate goal for local movement/steering
	struct Vector3 targetLocation; // high level movement goal (eg. last seen/heard loc)
	CharacterTarget targetType;
	CharacterState state;
	PathfindingState *pathfindingResult;
	// index of target node in path, or pathlength+1 for final target
	int pathProgress;
	float pathSegmentProgress; // param between segments, used by path smoothing
	float speedScaleForHeading;
	float speedMultiplier;
	float speedScaleForArrival;
	float turningSpeedScaleForHeading;

	unsigned int enteredStateTick;
	unsigned int startedActivityTick;
} Character;

typedef struct ItemStruct
{
	GameObject *obj;
	ItemHolder *holder;
	unsigned int lastPickedUpTick;
	struct Vector3 initialLocation;
} Item;

// this is here because of mutual dependency between game methods and objects
// with update methods which take Game arg
typedef struct Game
{
	unsigned int tick; // gets incremented every frame, with 60fps this will overflow if you run the game for 829 days straight
	int paused;
	struct Vector3 viewPos;
	struct Vector3 viewRot;
	struct Vector3 viewTarget;
	float viewZoom;
	int freeView;
	GameObject *worldObjects;
	int worldObjectsCount;
	Item *items;
	int itemsCount;
	Character *characters;
	int charactersCount;
	PhysBody *physicsBodies;
	int physicsBodiesCount;

	Player player;
	PhysState physicsState;
	Graph *pathfindingGraph;
	PathfindingState *pathfindingState;

	// profiling
	float profTimeCharacters;
	float profTimePhysics;
	float profTimeDraw;
	float profTimePath;
} Game;

#endif /* !_GAMETYPES_H_ */
