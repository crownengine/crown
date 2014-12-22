/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "math_types.h"
#include "physics_types.h"
#include "id_array.h"
#include "world_types.h"
#include "resource_types.h"

#include "PxPhysics.h"
#include "PxScene.h"
#include "PxRigidActor.h"
#include "PxCooking.h"

using physx::PxRigidActor;

namespace crown
{

#define MAX_PHYSX_VERTICES 256

struct Unit;
struct SceneGraph;

/// Represents a rigid body.
///
/// @ingroup Physics
struct Actor
{
	Actor(PhysicsWorld& pw, const ActorResource* ar, SceneGraph& sg, int32_t node, UnitId unit_id);
	~Actor();

	/// Returns the world position of the actor.
	Vector3 world_position() const;

	/// Returns the world rotation of the actor.
	Quaternion world_rotation() const;

	/// Returns the world pose of the actor.
	Matrix4x4 world_pose() const;

	/// Teleports the actor to the given world position.
	void teleport_world_position(const Vector3& p);

	/// Teleports the actor to the given world rotation.
	void teleport_world_rotation(const Quaternion& r);

	/// Teleports the actor to the given world pose.
	void teleport_world_pose(const Matrix4x4& m);

	/// Returns the center of mass of the actor.
	Vector3 center_of_mass() const;

	/// Enables gravity for the actor.
	void enable_gravity();

	/// Disables gravity for the actor.
	void disable_gravity();

	/// Enables collision detection for the actor.
	void enable_collision();

	/// Disables collision detection for the actor.
	void disable_collision();

	/// Sets the collision filter of the actor.
	void set_collision_filter(const char* filter);
	void set_collision_filter(StringId32 filter);

	/// Sets whether the actor is kinematic or not.
	/// @note This call has no effect on static actors.
	void set_kinematic(bool kinematic);

	/// Moves the actor to @a pos
	/// @note This call only affects nonkinematic actors.
	void move(const Vector3& pos);

	/// Returns whether the actor is static.
	bool is_static() const;

	/// Returns whether the actor is dynamic.
	bool is_dynamic() const;

	/// Returns whether the actor is kinematic (keyframed).
	bool is_kinematic() const;

	/// Returns whether the actor is nonkinematic (i.e. dynamic and not kinematic).
	bool is_nonkinematic() const;

	/// Returns the linear damping of the actor.
	float linear_damping() const;

	/// Sets the linear damping of the actor.
	void set_linear_damping(float rate);

	/// Returns the angular damping of the actor.
	float angular_damping() const;

	/// Sets the angular damping of the actor.
	void set_angular_damping(float rate);

	/// Returns the linear velocity of the actor.
	Vector3 linear_velocity() const;

	/// Sets the linear velocity of the actor.
	/// @note This call only affects nonkinematic actors.
	void set_linear_velocity(const Vector3& vel);

	/// Returns the angular velocity of the actor.
	Vector3 angular_velocity() const;

	/// Sets the angular velocity of the actor.
	/// @note This call only affects nonkinematic actors.
	void set_angular_velocity(const Vector3& vel);

	/// Adds a linear impulse (acting along the center of mass) to the actor.
	/// @note This call only affects nonkinematic actors.
	void add_impulse(const Vector3& impulse);

	/// Adds a linear impulse (acting along the world position @a pos) to the actor.
	/// @note This call only affects nonkinematic actors.
	void add_impulse_at(const Vector3& impulse, const Vector3& pos);

	/// Adds a torque impulse to the actor.
	void add_torque_impulse(const Vector3& i);

	/// Pushes the actor as if it was hit by a point object with the given @a mass
	/// travelling at the given @a velocity.
	/// @note This call only affects nonkinematic actors.
	void push(const Vector3& vel, float mass);

	/// Like push() but applies the force at the world position @a pos.
	/// @note This call only affects nonkinematic actors.
	void push_at(const Vector3& vel, float mass, const Vector3& pos);

	/// Returns whether the actor is sleeping.
	bool is_sleeping();

	/// Wakes the actor up.
	void wake_up();

	/// Returns the id of the unit that owns the actor.
	UnitId unit_id() const;

	/// Returns the unit that owns the actor.
	Unit* unit();

	const ActorResource* resource() const { return m_resource; }

private:

	void create_objects();
	void destroy_objects();

	void update(const Matrix4x4& pose);

public:

	PhysicsWorld& m_world;
	const ActorResource* m_resource;
	PxRigidActor* m_actor;

	SceneGraph& m_scene_graph;
	int32_t m_node;

	UnitId m_unit;

private:

	friend class PhysicsWorld;
};

} // namespace crown
