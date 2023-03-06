
#ifndef PHYSICS_H
#define PHYSICS_H

#include "collision.h"
#include "../constants.h"
#include "../math/vector3.h"

#define PHYS_MAX_STEPS 4

#define PHYS_TIMESTEP 1.0 / 60.0

typedef struct PhysWorldData {
  Triangle* worldMeshTris;
  int worldMeshTrisLength;
  SpatialHash* worldMeshSpatialHash;
  float gravity;
  float viscosity;
  float waterHeight;
} PhysWorldData;

typedef struct PhysState {
  float accumulatedTime;
  float clock;
  float viscosity;
  float simulationRate;
  float timeScale;
  int dynamicTimestep;  // boolean
  PhysWorldData* worldData;
} PhysState;

typedef struct PhysBody {
  int id;
  float mass;
  float massInverse;
  float radius;
  float radiusSquared;
  float restitution;
  int enabled;
  int controlled;  // controlled bodies have no inertia
  // in verlet, this is used to derive the velocity (pos - prevPos) so changing
  // pos explicitly without changing prevPos will result in acceleration when
  // velocity is derived
  struct Vector3 prevPosition;
  struct Vector3 position;
  // in verlet this is derived.
  // after integration, it represents the velocity for dt
  struct Vector3 velocity;
  // after integration, this represents the velocity for 1 second
  struct Vector3 nonIntegralVelocity;
  // this is really force (eg. in newtons) until after integration, when it
  // becomes acceleration
  struct Vector3 acceleration;
  // after integration, this represents the acceleration for 1 second
  struct Vector3 nonIntegralAcceleration;
  // this is the resultant acceleration from the 2x previous timestep
  struct Vector3 prevAcceleration;
} PhysBody;

void PhysState_init(PhysState* self, PhysWorldData* worldData);

void PhysState_step(PhysState* physics,
					PhysBody* bodies,
					int numBodies,
					float now);

void PhysBody_init(PhysBody* self,
				   float mass,
				   float radius,
				   struct Vector3* position,
				   int id);

void PhysBody_applyForce(PhysBody* body, struct Vector3* force);
void PhysBody_translateWithoutForce(PhysBody* body, struct Vector3* translation);
void PhysBody_setEnabled(PhysBody* body, int enabled);

#ifndef __N64__
#include <stdio.h>

void PhysBody_toString(PhysBody* self, char* buffer);
#endif

#endif /* !PHYSICS_H */
