/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Types.h"
#include "MathTypes.h"
#include "PhysicsTypes.h"
#include "IdArray.h"
#include "WorldTypes.h"

#include "PxPhysics.h"
#include "PxScene.h"
#include "PxRigidActor.h"
#include "PxCooking.h"

using physx::PxRigidActor;

namespace crown
{

#define MAX_PHYSX_VERTICES 256

struct PhysicsResource;
struct PhysicsConfigResource;
struct Quaternion;
struct Matrix4x4;
struct Unit;
class SceneGraph;

/// Represents a rigid body.
///
/// @ingroup Physics
struct Actor
{
	Actor(PhysicsWorld& pw, const PhysicsResource* res, uint32_t actor_idx, SceneGraph& sg, int32_t node, UnitId unit_id);
	~Actor();

	/// Makes the actor subject to gravity
	void enable_gravity();

	/// Makes the actor unsubject to gravity
	void disable_gravity();

	void enable_collision();
	void disable_collision();

	/// Makes the actor kinematic (keyframed)
	/// @note
	/// Works only for dynamic actors
	void set_kinematic();

	/// Makes the actor dynamic
	/// @note
	/// Works only for kinematic actors
	void clear_kinematic();

	/// Moves the actor to @a pos
	/// @note
	/// Works only for kinematic actors
	void move(const Vector3& pos);

	/// Returns whether the actor is static (i.e. immovable).
	bool is_static() const;

	/// Returns whether the actor is dynamic (i.e. driven dy physics).
	bool is_dynamic() const;

	/// Returns whether the actor is kinematic (i.e. driven by the user).
	bool is_kinematic() const;

	/// Returns the rate at which rigid bodies dissipate linear momentum
	float linear_damping() const;

	/// Sets the rate at which rigid bodies dissipate linear momentum
	void set_linear_damping(float rate);

	/// Returns the rate at which rigid bodies dissipate angular momentum
	float angular_damping() const;

	/// Sets the rate at which rigid bodies dissipate angular momentum
	void set_angular_damping(float rate);
 
	/// Returns linear velocity of the actor
	/// @note
	/// If actor is sleeping, linear velocity must be 0
	Vector3 linear_velocity() const;

	/// Sets linear velocity of the actor
	/// @note
	/// If actor is sleeping, this will wake it up
	void set_linear_velocity(const Vector3& vel);

	/// Returns angular velocity of the actor
	/// @note
	/// If actor is sleeping, angular velocity must be 0
	Vector3 angular_velocity() const;

	/// Sets angular velocity of the actor
	/// @note
	/// If actor is sleeping, this will wake it up
	void set_angular_velocity(const Vector3& vel);

	/// Applies a force (or impulse) defined in the global coordinate frame, acting at a particular point in global coordinates, to the actor.
	/// @note
	/// If the force does not act along the center of mass of the actor, this will also add the corresponding torque.
	/// Because forces are reset at the end of every timestep, you can maintain a total external force on an object by calling this once every frame.
	void add_impulse(const Vector3& impulse);

	/// Applies a force (or impulse) defined in the global coordinate frame, acting at a particular point in local coordinates, to the actor.
	/// @note
	/// If the force does not act along the center of mass of the actor, this will also add the corresponding torque.
	/// Because forces are reset at the end of every timestep, you can maintain a total external force on an object by calling this once every frame. 
	void add_impulse_at(const Vector3& impulse, const Vector3& pos);

	/// Applies a force, evaluated by actor's @a mass and @a velocity that will be achieved, to the actor
	void push(const Vector3& vel, const float mass);

	/// Returns true if tha actor is sleeping, false otherwise
	bool is_sleeping();

	/// Forces the actor to wake up
	void wake_up();

	/// Returns actor's name
	StringId32 name();

	/// Returns the unit that owns the actor.
	Unit* unit();

	const PhysicsResource* resource() const { return m_resource; }

private:

	void create_objects();
	void destroy_objects();

	void update(const Matrix4x4& pose);
	Matrix4x4 get_kinematic_pose() const;

public:

	PhysicsWorld& m_world;
	const PhysicsResource* m_resource;
	uint32_t m_index;
	PxRigidActor* m_actor;

	SceneGraph& m_scene_graph;
	int32_t m_node;

	UnitId m_unit;

private:

	friend class PhysicsWorld;
};

} // namespace crown
