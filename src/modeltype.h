
#ifndef MODELTYPE_H
#define MODELTYPE_H

#include "math/vector3.h"

// when updating this, also update ModelTypeStrings[] and modelTypesProperties[]
// and add new meshes to getModelDisplayList() in stage00.c and add a
// loadModel() call in glgoose.cpp
typedef enum ModelType
{
	NoneModel,
	GooseModel,
	BookItemModel,
	HomeworkItemModel,
	CakeItemModel,
	UniBldgModel,
	UniFloorModel,
	BushModel,
	FlagpoleModel,
	GardenerCharacterModel,
	WallModel,
	PlanterModel,
	GroundModel,
	WaterModel,
	RockModel,
	WatergrassModel,
	ReedModel,
	LilypadModel,
	CatherineModel,
	MAX_MODEL_TYPE
} ModelType;

typedef enum ModelTypeCategory
{
	GenericModelType,
	ItemModelType,
	CharacterModelType,
	PlayerModelType,
	MAX_MODEL_TYPE_CATEGORY,
} ModelTypeCategory;

typedef struct ModelProperties
{
	float mass;
	float radius;
	struct Vector3 centroidOffset;
	float scale;
	ModelTypeCategory category;
} ModelProperties;

extern ModelProperties modelTypesProperties[];

extern char *ModelTypeStrings[];

#endif /* !MODELTYPE_H */
