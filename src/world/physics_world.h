/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/types.h"
#include "resource/types.h"
#include "world/event_stream.h"
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
	Allocator *_allocator;
	PhysicsWorldImpl *_impl;

	///
	PhysicsWorld(Allocator &a, ResourceManager &rm, UnitManager &um, SceneGraph &sg, DebugLine &dl);

	///
	~PhysicsWorld();

	///
	PhysicsWorld(const PhysicsWorld &) = delete;

	///
	PhysicsWorld &operator=(const PhysicsWorld &) = delete;

	///
	void collider_create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	///
	void collider_destroy(ColliderInstance collider);

	///
	ColliderInstance collider_instance(UnitId unit);

	///
	void actor_create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	/// Destroys the @a actor.
	void actor_destroy(ActorInstance actor);

	/// Returns the ID of the actor owned by the *unit*.
	ActorInstance actor(UnitId unit);

	/// Returns the world position of the @a actor.
	Vector3 actor_world_position(ActorInstance actor) const;

	/// Returns the world rotation of the @a actor.
	Quaternion actor_world_rotation(ActorInstance actor) const;

	/// Returns the world pose of the @a actor.
	Matrix4x4 actor_world_pose(ActorInstance actor) const;

	/// Teleports the @a actor to the given world position.
	void actor_teleport_world_position(ActorInstance actor, const Vector3 &p);

	/// Teleports the @a actor to the given world rotation.
	void actor_teleport_world_rotation(ActorInstance actor, const Quaternion &r);

	/// Teleports the @a actor to the given world pose.
	void actor_teleport_world_pose(ActorInstance actor, const Matrix4x4 &m);

	/// Returns the center of mass of the @a actor.
	Vector3 actor_center_of_mass(ActorInstance actor) const;

	/// Enables gravity for the @a actor.
	void actor_enable_gravity(ActorInstance actor);

	/// Disables gravity for the @a actor.
	void actor_disable_gravity(ActorInstance actor);

	/// Enables collision detection for the @a actor.
	void actor_enable_collision(ActorInstance actor);

	/// Disables collision detection for the @a actor.
	void actor_disable_collision(ActorInstance actor);

	/// Sets the collision filter of the @a actor.
	void actor_set_collision_filter(ActorInstance actor, StringId32 filter);

	/// Sets whether the @a actor is kinematic or not.
	/// @note This call has no effect on static actors.
	void actor_set_kinematic(ActorInstance actor, bool kinematic);

	/// Returns whether the @a actor is static.
	bool actor_is_static(ActorInstance actor) const;

	/// Returns whether the @a actor is dynamic.
	bool actor_is_dynamic(ActorInstance actor) const;

	/// Returns whether the @a actor is kinematic (keyframed).
	bool actor_is_kinematic(ActorInstance actor) const;

	/// Returns whether the @a actor is nonkinematic (i.e. dynamic and not kinematic).
	bool actor_is_nonkinematic(ActorInstance actor) const;

	/// Returns the linear damping of the @a actor.
	f32 actor_linear_damping(ActorInstance actor) const;

	/// Sets the linear damping of the @a actor.
	void actor_set_linear_damping(ActorInstance actor, f32 rate);

	/// Returns the angular damping of the @a actor.
	f32 actor_angular_damping(ActorInstance actor) const;

	/// Sets the angular damping of the @a actor.
	void actor_set_angular_damping(ActorInstance actor, f32 rate);

	/// Returns the linear velocity of the @a actor.
	Vector3 actor_linear_velocity(ActorInstance actor) const;

	/// Sets the linear velocity of the @a actor.
	/// @note This call only affects nonkinematic actors.
	void actor_set_linear_velocity(ActorInstance actor, const Vector3 &vel);

	/// Returns the angular velocity of the @a actor.
	Vector3 actor_angular_velocity(ActorInstance actor) const;

	/// Sets the angular velocity of the @a actor.
	/// @note This call only affects nonkinematic actors.
	void actor_set_angular_velocity(ActorInstance actor, const Vector3 &vel);

	/// Adds a linear impulse (acting along the center of mass) to the @a actor.
	/// @note This call only affects nonkinematic actors.
	void actor_add_impulse(ActorInstance actor, const Vector3 &impulse);

	/// Adds a linear impulse (acting along the world position @a pos) to the @a actor.
	/// @note This call only affects nonkinematic actors.
	void actor_add_impulse_at(ActorInstance actor, const Vector3 &impulse, const Vector3 &pos);

	/// Adds a torque impulse to the @a actor.
	void actor_add_torque_impulse(ActorInstance actor, const Vector3 &imp);

	/// Pushes the @a actor as if it was hit by a point object with the given @a mass
	/// travelling at the given @a velocity.
	/// @note This call only affects nonkinematic actors.
	void actor_push(ActorInstance actor, const Vector3 &vel, f32 mass);

	/// Like push() but applies the force at the world position @a pos.
	/// @note This call only affects nonkinematic actors.
	void actor_push_at(ActorInstance actor, const Vector3 &vel, f32 mass, const Vector3 &pos);

	/// Returns whether the @a actor is sleeping.
	bool actor_is_sleeping(ActorInstance actor);

	/// Wakes the @a actor up.
	void actor_wake_up(ActorInstance actor);

	///
	void mover_create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	/// Creates a new mover instance for the @a unit.
	MoverInstance mover_create(UnitId unit, const MoverDesc *desc);

	/// Destroys the @a mover.
	void mover_destroy(MoverInstance mover);

	/// Returns the ID of the mover owned by the *unit*.
	MoverInstance mover(UnitId unit);

	/// Returns the radius of the @a mover capsule.
	f32 mover_radius(MoverInstance mover);

	/// Returns the max slope angle of the @a mover.
	f32 mover_max_slope_angle(MoverInstance mover);

	/// Sets the max slope @a angle of the @a mover.
	void mover_set_max_slope_angle(MoverInstance mover, f32 angle);

	/// Sets the collision @a filter of the @a mover.
	void mover_set_collision_filter(MoverInstance mover, StringId32 filter);

	/// Returns the position of the @a mover.
	Vector3 mover_position(MoverInstance mover);

	/// Teleports the @a mover to the specified @a position.
	void mover_set_position(MoverInstance mover, const Vector3 &position);

	/// Attempts to move the @a mover by the specified @a delta vector.
	/// The @a mover will slide against physical actors.
	void mover_move(MoverInstance mover, const Vector3 &delta);

	/// Returns whether the @a mover collides sideways.
	bool mover_collides_sides(MoverInstance mover);

	/// Returns whether the @a mover collides upwards.
	bool mover_collides_up(MoverInstance mover);

	/// Returns whether the @a mover collides downwards.
	bool mover_collides_down(MoverInstance mover);

	/// Creates joint
	JointInstance joint_create(ActorInstance a0, ActorInstance a1, const JointDesc &jd);

	/// Destroys the @a joint.
	void joint_destroy(JointInstance joint);

	/// Casts a ray into the physics world and returns info about the closest collision if any.
	bool cast_ray(RaycastHit &hit, const Vector3 &from, const Vector3 &dir, f32 len);

	/// Casts a ray into the physics world and returns info about all the collisions if any.
	bool cast_ray_all(Array<RaycastHit> &hits, const Vector3 &from, const Vector3 &dir, f32 len);

	/// Casts a sphere into the physics world and returns info about the closest collision if any.
	bool cast_sphere(RaycastHit &hit, const Vector3 &from, f32 radius, const Vector3 &dir, f32 len);

	/// Casts a box into the physics world and returns info about the closest collision if any.
	bool cast_box(RaycastHit &hit, const Vector3 &from, const Vector3 &half_extents, const Vector3 &dir, f32 len);

	/// Returns the gravity.
	Vector3 gravity() const;

	/// Sets the gravity.
	void set_gravity(const Vector3 &g);

	///
	void update_actor_world_poses(const UnitId *begin, const UnitId *end, const Matrix4x4 *begin_world);

	/// Updates the physics simulation.
	void update(f32 dt);

	///
	EventStream &events();

	/// Draws debug lines.
	void debug_draw();

	///
	void enable_debug_drawing(bool enable);
};

} // namespace crown
