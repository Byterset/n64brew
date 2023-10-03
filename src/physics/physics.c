
#include <math.h>
#include <stdlib.h>

#include "collision.h"
#include "../constants.h"
#include "physics.h"
#include "../util/trace.h"

#define PHYS_DEBUG 0
#define PHYS_MIN_MOVEMENT 0.5
#define PHYS_COLLISION_MIN_SEPARATION 0.001
#define PHYSICS_MOTION_DAMPENING 0
#define PHYSICS_USE_VERLET_INTEGRATION 0
#define PHYS_MAX_COLLISION_ITERATIONS 10
#define PHYS_DEBUG_PRINT_COLLISIONS 0

void PhysState_init(PhysState *self, PhysWorldData *worldData)
{
	self->accumulatedTime = 0.0;
	self->clock = 0.0;
	self->simulationRate = 1.0;
	self->timeScale = 1.0;
	self->dynamicTimestep = TRUE;
	self->worldData = worldData;
}

void PhysBody_init(PhysBody *self,
				   float mass,
				   float radius,
				   struct Vector3 *position,
				   int id)
{
	self->id = id;
	self->mass = mass;
	self->massInverse = 1.0 / mass;
	self->radius = radius;
	self->radiusSquared = radius * radius;
	self->restitution = 1.0;
	self->enabled = TRUE;
	self->position = *position;
	self->prevPosition = *position;

	vector3Init(&self->velocity, 0.0f, 0.0f, 0.0f);
	vector3Init(&self->nonIntegralVelocity, 0.0f, 0.0f, 0.0f);
	vector3Init(&self->acceleration, 0.0f, 0.0f, 0.0f);
	vector3Init(&self->nonIntegralAcceleration, 0.0f, 0.0f, 0.0f);
	vector3Init(&self->prevAcceleration, 0.0f, 0.0f, 0.0f);
}

void PhysBehavior_floorBounce(PhysBody *body, float floorHeight)
{
	float opposite;
	struct Vector3 response;

	opposite = (-1.0) * body->mass;
	if (body->position.y - body->radius < floorHeight)
	{
		body->position.y = floorHeight + body->radius;
		vector3Init(&response, 0.0, body->nonIntegralVelocity.y * opposite, 0.0);
		vector3AddToSelf(&body->acceleration, &response);
	}
}

void PhysBehavior_floorClamp(PhysBody *body, float floorHeight)
{
	if (body->position.y - body->radius < floorHeight)
	{
		body->position.y = floorHeight + body->radius;
	}
}

void PhysBehavior_waterBuoyancy(PhysBody *body,
								float waterHeight,
								struct Vector3 *gravity)
{
	struct Vector3 response;
	// float maxDepth = -58.0;

	if (body->position.y < waterHeight)
	{
		// float buoyancyRatio = CLAMP(
		//     (body->position.y - waterHeight) / (maxDepth - waterHeight), -2.0,
		//     0.0);
		body->position.y = waterHeight;
		vector3Init(&response, 0.0,
				   -gravity->y, // gravity->y * buoyancyRatio
				   0.0);
		PhysBody_applyForce(body, &response);
	}
}

int PhysBehavior_worldCollisionResponseStep(PhysBody *body,
											PhysWorldData *world)
{
	int hasCollision;
	float distanceToIntersect, responseDistance, bodyInFrontOfTriangle;
	SphereTriangleCollision collision;
	struct Vector3 response, beforePos;

	hasCollision = Collision_testMeshSphereCollision(
		world->worldMeshTris, world->worldMeshTrisLength, &body->position,
		body->radius, world->worldMeshSpatialHash, &collision);

	if (!hasCollision)
	{
		return FALSE;
	}

#ifndef PHYS_DEBUG
	// if (body->id == 2) {
	//   printf("player collided\n");
	// }
	debugPrintf("body %d collided\n", body->id);
#endif
	distanceToIntersect = collision.distance;

	bodyInFrontOfTriangle =
		Triangle_comparePoint(collision.triangle, &body->position);

	// move away by radius
	Triangle_getNormal(collision.triangle, &response);

	responseDistance = 0;

	if (bodyInFrontOfTriangle >= 0)
	{
		// center of body is in front of or on face
		responseDistance =
			body->radius + PHYS_COLLISION_MIN_SEPARATION - distanceToIntersect;
	}
	else
	{
		// center of body is behind face
		responseDistance =
			body->radius + PHYS_COLLISION_MIN_SEPARATION + distanceToIntersect;
	}

	beforePos = body->position;
#if PHYSICS_USE_VERLET_INTEGRATION
	vector3ScaleSelf(&response, responseDistance);
	PhysBody_translateWithoutForce(body, &response);
#else
	vector3ScaleSelf(&response, responseDistance);
	PhysBody_translateWithoutForce(body, &response);
	vector3Init(&body->nonIntegralVelocity, 0.0f, 0.0f, 0.0f);
#endif

#if PHYS_DEBUG_PRINT_COLLISIONS
	if (body->id == 2)
	{
		printf("player collided with world\n");
	}
	if (body->id == 22)
	{
		printf("groundskeeper collided with world\n");
	}
#ifdef __cplusplus
	// printf(
	// 	"PhysBody id=%d hasCollision tri=%d distanceToIntersect=%f beforePos=%s "
	// 	"afterPos=%s "
	// 	"posInTriangle=%s response=%s "
	// 	"responseDistance=%f bodyInFrontOfTriangle=%f",
	// 	body->id, collision.index, distanceToIntersect,
	// 	Vec3d_toStdString(&beforePos).c_str(),
	// 	Vec3d_toStdString(&body->position).c_str(),
	// 	Vec3d_toStdString(&collision.posInTriangle).c_str(),
	// 	Vec3d_toStdString(&response).c_str(), responseDistance,
	// 	bodyInFrontOfTriangle

	// );
#endif
#endif

	return TRUE;
}

void PhysBehavior_collisionSeparationOffset(struct Vector3 *result,
											struct Vector3 *pos,
											float overlap,
											float separationForce)
{
	vector3Copy(result, pos);
	vector3NormalizeSelf(result);
	vector3ScaleSelf(result, overlap * separationForce);
}

int PhysBehavior_bodyBodyCollisionResponse(PhysBody *body,
										   PhysBody *pool,
										   int numInPool)
{
	struct Vector3 delta, direction, collisionSeparationOffset;
	int i, hasCollision;
	float distanceSquared, radii, distance, overlap, mt, bodySeparationForce,
		otherBodySeparationForce;
	PhysBody *otherBody;

	hasCollision = FALSE;

	vector3Init(&delta, 0.0f, 0.0f, 0.0f);
	vector3Init(&direction, 0.0f, 0.0f, 0.0f);

	for (i = 0, otherBody = pool; i < numInPool; i++, otherBody++)
	{
		if (body != otherBody && otherBody->enabled)
		{
			vector3Copy(&delta, &otherBody->position);
			vector3SubFromSelf(&delta, &body->position);
			distanceSquared = vector3MagSqrd(&delta);
			vector3Copy(&direction, &delta);
			vector3NormalizeSelf(&direction);
			radii = body->radius + otherBody->radius;
			if (distanceSquared <= radii * radii)
			{ // collision
				hasCollision = TRUE;

				distance = sqrtf(distanceSquared);
				overlap = radii - distance - 0.5;
				/* Total mass. */
				mt = body->mass + otherBody->mass;
				/* Distribute collision responses. */
				bodySeparationForce = otherBody->mass / mt;
				otherBodySeparationForce = body->mass / mt;

				/* Move particles so they no longer overlap.*/
				PhysBehavior_collisionSeparationOffset(
					&collisionSeparationOffset, &delta, overlap, -bodySeparationForce);
				vector3AddToSelf(&body->position, &collisionSeparationOffset);

				PhysBehavior_collisionSeparationOffset(&collisionSeparationOffset,
													   &delta, overlap,
													   otherBodySeparationForce);
				vector3AddToSelf(&otherBody->position, &collisionSeparationOffset);
			}
		}
	}
	return hasCollision;
}

int PhysBehavior_collisionResponseStep(PhysBody *body,
									   PhysWorldData *world,
									   PhysBody *pool,
									   int numInPool)
{
	int hasCollision;

	hasCollision = FALSE;
	hasCollision =
		hasCollision || PhysBehavior_worldCollisionResponseStep(body, world);
	return hasCollision;
}

void PhysBehavior_collisionResponse(PhysWorldData *world,
									PhysBody *bodies,
									int numBodies)
{
	int i, k, hasAnyCollision;
	PhysBody *body;
	// float profStartObjCollision;
	// float profStartWorldCollision;
	// int floorHeight = 0.0;

	// profStartObjCollision = CUR_TIME_MS();
	for (k = 0, body = bodies; k < numBodies; k++, body++)
	{
		if (body->enabled)
		{
			// PhysBehavior_floorBounce(body, floorHeight);
			// PhysBehavior_floorClamp(body, floorHeight);
			PhysBehavior_bodyBodyCollisionResponse(body, bodies, numBodies);
		}
	}
	// Trace_addEvent(PhysObjCollisionTraceEvent, profStartObjCollision,
	//                CUR_TIME_MS());

	// run multiple iterations, because the response to a collision can create
	// another collision
	for (i = 0; i < PHYS_MAX_COLLISION_ITERATIONS; ++i)
	{
		hasAnyCollision = FALSE;
		// profStartWorldCollision = CUR_TIME_MS();
		for (k = 0, body = bodies; k < numBodies; k++, body++)
		{
			if (body->enabled)
			{
				hasAnyCollision =
					hasAnyCollision ||
					PhysBehavior_collisionResponseStep(body, world, bodies, numBodies);
			}
		}
		// Trace_addEvent(PhysWorldCollisionTraceEvent, profStartWorldCollision,
		//                CUR_TIME_MS());
		if (!hasAnyCollision)
		{
			break;
		}
	}
	// #ifndef PHYS_DEBUG
	//   if (i > 0) {
	//     debugPrintf("collision response took %d iters\n", i);
	//   }
	//   if (hasAnyCollision) {
	//     debugPrintf(
	//         "hit PHYS_MAX_COLLISION_ITERATIONS and ended collision response with "
	//         "collisions remaining\n");
	//   }
	// #endif
}

void PhysBody_setEnabled(PhysBody *body, int enabled)
{
	if (enabled)
	{
		body->enabled = TRUE;
		// prevent velocity from movement while disabled
		vector3Copy(&body->prevPosition, &body->position);
	}
	else
	{
		body->enabled = FALSE;
	}
}

void PhysBehavior_constantForce(PhysBody *body, struct Vector3 force)
{
	vector3AddToSelf(&body->acceleration, &force);
}

void PhysBody_applyForce(PhysBody *body, struct Vector3 *force)
{
	vector3AddToSelf(&body->acceleration, force);
}

// move but don't affect velocity
void PhysBody_translateWithoutForce(PhysBody *body, struct Vector3 *translation)
{
	vector3AddToSelf(&body->position, translation);
	vector3AddToSelf(&body->prevPosition, translation);
}

void PhysBody_update(PhysBody *self,
					 float dt,
					 float drag,
					 PhysBody *pool,
					 int numInPool,
					 PhysState *physics)
{
	struct Vector3 gravity;
	vector3Init(&gravity, 0, physics->worldData->gravity * self->mass, 0);
	// do behaviours
	PhysBehavior_constantForce(self, gravity); // apply gravity

	PhysBehavior_waterBuoyancy(self, physics->worldData->waterHeight, &gravity);
}

void PhysBody_dampenSmallMovements(PhysBody *body)
{
	// dampen small movements
	if (vector3Dist(&body->position, &body->prevPosition) <
		PHYS_MIN_MOVEMENT)
	{
		body->position = body->prevPosition;
		vector3Init(&body->velocity, 0.0f, 0.0f, 0.0f);
		vector3Init(&body->nonIntegralVelocity, 0.0f, 0.0f, 0.0f);
		vector3Init(&body->acceleration, 0.0f, 0.0f, 0.0f);
		vector3Init(&body->prevAcceleration, 0.0f, 0.0f, 0.0f);
	}
}

void PhysBody_integrateMotionVerlet(PhysBody *body, float dt, float drag)
{
	struct Vector3 newPosition;
	vector3Init(&newPosition, 0.0f, 0.0f, 0.0f);
	/* Scale force to mass. */
	vector3ScaleSelf(&body->acceleration, body->massInverse);
	/* Derive velocity. */
	vector3Copy(&body->velocity, &body->position);
	vector3SubFromSelf(&body->velocity, &body->prevPosition);
	/* Apply friction. */
	vector3ScaleSelf(&body->velocity, drag);
	/* Apply acceleration force to new position. */
	/* Get integral acceleration, apply to velocity, then apply updated
	   velocity to position */
	vector3Copy(&newPosition, &body->position);
	vector3ScaleSelf(&body->acceleration, dt);
	vector3AddToSelf(&body->velocity, &body->acceleration);
	vector3AddToSelf(&newPosition, &body->velocity);

	/* Store old position, update position to new position. */
	vector3Copy(&body->prevPosition, &body->position);
	vector3Copy(&body->position, &newPosition);

#if PHYSICS_MOTION_DAMPENING
	PhysBody_dampenSmallMovements(&body);
#endif

	/* Reset acceleration force. */
	vector3Copy(&body->prevAcceleration, &body->acceleration);
	vector3Init(&body->acceleration, 0.0f, 0.0f, 0.0f);
	/* store velocity for use in acc calculations by user code */
	vector3Copy(&body->nonIntegralVelocity, &body->velocity);
	vector3ScaleSelf(&body->nonIntegralVelocity, 1.0 / dt);
}
void PhysBody_integrateMotionSemiImplicitEuler(PhysBody *body,
											   float dt,
											   float drag)
{
	// acceleration = force / mass
	// accelerationForDT = acceleration * dt
	// velocity = velocity + accelerationForDT
	// velocityForDT = velocity * dt
	// position = position + velocityForDT

	struct Vector3 newPosition;
	/* Scale force by mass to calculate actual acceleration */
	// acceleration = ( force / mass )
	vector3ScaleSelf(&body->acceleration, body->massInverse);
	body->nonIntegralAcceleration = body->acceleration; // for debugging
	// accelerationForDT = acceleration * dt
	vector3ScaleSelf(&body->acceleration, dt);

	// velocity = velocity + accelerationForDT
	vector3AddToSelf(&body->nonIntegralVelocity, &body->acceleration);

	// velocityForDT = velocity * dt
	body->velocity = body->nonIntegralVelocity;
	vector3ScaleSelf(&body->velocity, dt);

	/* Apply friction. */
	vector3ScaleSelf(&body->velocity, drag);

	// position = position + velocityForDT;
	newPosition = body->position;
	vector3AddToSelf(&newPosition, &body->velocity);

	/* Store old position, update position to new position. */
	body->prevPosition = body->position;
	body->position = newPosition;

#if PHYSICS_MOTION_DAMPENING
	PhysBody_dampenSmallMovements(body);
#endif

	/* Reset acceleration force. */
	body->prevAcceleration = body->acceleration;
	vector3Init(&body->acceleration, 0.0f, 0.0f, 0.0f);
}

void PhysBody_integrateBodies(PhysBody *bodies,
							  int numBodies,
							  float dt,
							  float drag,
							  PhysState *physics)
{
	PhysBody *body;
	int i;
	for (i = 0, body = bodies; i < numBodies; i++, body++)
	{
		if (body->enabled)
		{
			PhysBody_update(body, dt, drag, bodies, numBodies, physics);
		}
	}

	for (i = 0, body = bodies; i < numBodies; i++, body++)
	{
		if (body->enabled /*&& !body->controlled*/)
		{
#if PHYSICS_USE_VERLET_INTEGRATION
			PhysBody_integrateMotionVerlet(body, dt, drag);
#else
			PhysBody_integrateMotionSemiImplicitEuler(body, dt, drag);
#endif
		}
	}

	// do this after so we can fix any world penetration resulting from motion
	// integration
	PhysBehavior_collisionResponse(physics->worldData, bodies, numBodies);
}

void PhysState_step(PhysState *physics,
					PhysBody *bodies,
					int numBodies,
					float now)
{
	float time;
	int i;
	float timestep;
	float delta;
	float drag;
	/* Initialise the clock on first step. */
	if (physics->clock == 0.0)
	{
		physics->clock = now;
	};
	/* Compute delta time since last step. */
	time = now;
	/* fixed delta for debugging */
	time = physics->dynamicTimestep
			   ? time
			   : physics->clock +
					 16.667 * physics->timeScale * physics->simulationRate;
	delta = time - physics->clock;
	/* sufficient change. */
	if (delta > 0.0)
	{
		/* Convert time to seconds. */
		delta = delta * 0.001;
		/* Drag is inversely proportional to viscosity. */
		drag = 1.0 - physics->worldData->viscosity;
		/* Update the clock. */
		physics->clock = time;
		/* Increment time accumulatedTime.
		   Don't accumulate any additional time if we're already more than 1 second
		   behind. This happens when the tab is backgrounded, and if this grows
		   large enough we won't be able to ever catch up.
		   */
		if (physics->accumulatedTime < 1.0)
		{
			physics->accumulatedTime = physics->accumulatedTime + delta;
		}
		else
		{
			// #ifndef PHYS_DEBUG
			//       debugPrintf(
			//           "Physics: accumulated too much time, not accumulating any more\n");
			// #endif
		};
		/* Integrate until the accumulatedTime is empty or until the */
		/* maximum amount of iterations per step is reached. */
		i = 0;
		timestep = PHYS_TIMESTEP * physics->timeScale;

		while (physics->accumulatedTime >= timestep && i < PHYS_MAX_STEPS)
		{
			/* Integrate bodies by fixed timestep. */
			PhysBody_integrateBodies(bodies, numBodies, timestep, drag, physics);
			/* Reduce accumulatedTime by one timestep. */
			physics->accumulatedTime = physics->accumulatedTime - timestep;
			i++;

			// TODO: updating physics step multiple times without running game update
			// step is probably wrong, as forces are reset after each step
			break;
		}
		// #ifndef PHYS_DEBUG
		//     debugPrintf("Physics: ran %d timesteps\n", i + 1);
		// #endif
	}
}

#ifndef __N64__
#include <stdio.h>

void PhysBody_toString(PhysBody *self, char *buffer)
{
	char pos[60];
	char vel[60];
	vector3toString(&self->position, pos);
	vector3toString(&self->nonIntegralVelocity, vel);
	sprintf(buffer, "PhysBody id=%d pos=%s vel=%s", self->id, pos, vel);
}
#endif
