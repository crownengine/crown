/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/event_stream.h"
#include "core/math/types.h"
#include "resource/types.h"
#include "world/types.h"

namespace crown
{
struct PhysicsWorldImpl;

/// Manages physics objects in a World.
///
/// @ingroup World
struct PhysicsWorld
{
	u32 _marker;
	Allocator* _allocator;
	PhysicsWorldImpl* _impl;

	///
	PhysicsWorld(Allocator& a, ResourceManager& rm, UnitManager& um, DebugLine& dl);

	///
	~PhysicsWorld();

	///
	ColliderInstance collider_create(UnitId id, const ColliderDesc* sd);

	///
	void collider_destroy(ColliderInstance i);

	///
	ColliderInstance collider_first(UnitId id);

	///
	ColliderInstance collider_next(ColliderInstance i);

	///
	ActorInstance actor_create(UnitId id, const ActorResource* ar, const Matrix4x4& tm);

	///
	void actor_destroy(ActorInstance i);

	///
	ActorInstance actor(UnitId id);

	/// Returns the world position of the actor.
	Vector3 actor_world_position(ActorInstance i) const;

	/// Returns the world rotation of the actor.
	Quaternion actor_world_rotation(ActorInstance i) const;

	/// Returns the world pose of the actor.
	Matrix4x4 actor_world_pose(ActorInstance i) const;

	/// Teleports the actor to the given world position.
	void actor_teleport_world_position(ActorInstance i, const Vector3& p);

	/// Teleports the actor to the given world rotation.
	void actor_teleport_world_rotation(ActorInstance i, const Quaternion& r);

	/// Teleports the actor to the given world pose.
	void actor_teleport_world_pose(ActorInstance i, const Matrix4x4& m);

	/// Returns the center of mass of the actor.
	Vector3 actor_center_of_mass(ActorInstance i) const;

	/// Enables gravity for the actor.
	void actor_enable_gravity(ActorInstance i);

	/// Disables gravity for the actor.
	void actor_disable_gravity(ActorInstance i);

	/// Enables collision detection for the actor.
	void actor_enable_collision(ActorInstance i);

	/// Disables collision detection for the actor.
	void actor_disable_collision(ActorInstance i);

	/// Sets the collision filter of the actor.
	void actor_set_collision_filter(ActorInstance i, StringId32 filter);

	/// Sets whether the actor is kinematic or not.
	/// @note This call has no effect on static actors.
	void actor_set_kinematic(ActorInstance i, bool kinematic);

	/// Returns whether the actor is static.
	bool actor_is_static(ActorInstance i) const;

	/// Returns whether the actor is dynamic.
	bool actor_is_dynamic(ActorInstance i) const;

	/// Returns whether the actor is kinematic (keyframed).
	bool actor_is_kinematic(ActorInstance i) const;

	/// Returns whether the actor is nonkinematic (i.e. dynamic and not kinematic).
	bool actor_is_nonkinematic(ActorInstance i) const;

	/// Returns the linear damping of the actor.
	f32 actor_linear_damping(ActorInstance i) const;

	/// Sets the linear damping of the actor.
	void actor_set_linear_damping(ActorInstance i, f32 rate);

	/// Returns the angular damping of the actor.
	f32 actor_angular_damping(ActorInstance i) const;

	/// Sets the angular damping of the actor.
	void actor_set_angular_damping(ActorInstance i, f32 rate);

	/// Returns the linear velocity of the actor.
	Vector3 actor_linear_velocity(ActorInstance i) const;

	/// Sets the linear velocity of the actor.
	/// @note This call only affects nonkinematic actors.
	void actor_set_linear_velocity(ActorInstance i, const Vector3& vel);

	/// Returns the angular velocity of the actor.
	Vector3 actor_angular_velocity(ActorInstance i) const;

	/// Sets the angular velocity of the actor.
	/// @note This call only affects nonkinematic actors.
	void actor_set_angular_velocity(ActorInstance i, const Vector3& vel);

	/// Adds a linear impulse (acting along the center of mass) to the actor.
	/// @note This call only affects nonkinematic actors.
	void actor_add_impulse(ActorInstance i, const Vector3& impulse);

	/// Adds a linear impulse (acting along the world position @a pos) to the actor.
	/// @note This call only affects nonkinematic actors.
	void actor_add_impulse_at(ActorInstance i, const Vector3& impulse, const Vector3& pos);

	/// Adds a torque impulse to the actor.
	void actor_add_torque_impulse(ActorInstance i, const Vector3& imp);

	/// Pushes the actor as if it was hit by a point object with the given @a mass
	/// travelling at the given @a velocity.
	/// @note This call only affects nonkinematic actors.
	void actor_push(ActorInstance i, const Vector3& vel, f32 mass);

	/// Like push() but applies the force at the world position @a pos.
	/// @note This call only affects nonkinematic actors.
	void actor_push_at(ActorInstance i, const Vector3& vel, f32 mass, const Vector3& pos);

	/// Returns whether the actor is sleeping.
	bool actor_is_sleeping(ActorInstance i);

	/// Wakes the actor up.
	void actor_wake_up(ActorInstance i);

	/// Creates joint
	JointInstance joint_create(ActorInstance a0, ActorInstance a1, const JointDesc& jd);

	///
	void joint_destroy(JointInstance i);

	/// Casts a ray into the physics world and returns info about the closest collision if any.
	bool cast_ray(RaycastHit& hit, const Vector3& from, const Vector3& dir, f32 len);

	/// Casts a ray into the physics world and returns info about all the collisions if any.
	bool cast_ray_all(Array<RaycastHit>& hits, const Vector3& from, const Vector3& dir, f32 len);

	/// Casts a sphere into the physics world and returns info about the closest collision if any.
	bool cast_sphere(RaycastHit& hit, const Vector3& from, f32 radius, const Vector3& dir, f32 len);

	/// Casts a box into the physics world and returns info about the closest collision if any.
	bool cast_box(RaycastHit& hit, const Vector3& from, const Vector3& half_extents, const Vector3& dir, f32 len);

	/// Returns the gravity.
	Vector3 gravity() const;

	/// Sets the gravity.
	void set_gravity(const Vector3& g);

	void update_actor_world_poses(const UnitId* begin, const UnitId* end, const Matrix4x4* begin_world);

	/// Updates the physics simulation.
	void update(f32 dt);

	///
	EventStream& events();

	/// Draws debug lines.
	void debug_draw();

	///
	void enable_debug_drawing(bool enable);
};

} // namespace crown
