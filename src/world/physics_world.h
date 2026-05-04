/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/types.h"
#include "core/event_stream.h"
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
	void collider_destroy(ColliderId collider);

	///
	ColliderId collider_instance(UnitId unit);

	///
	void actor_create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	/// Destroys the @a actor.
	void actor_destroy(ActorId actor);

	/// Returns the ID of the actor owned by the *unit*.
	ActorId actor(UnitId unit);

	/// Returns the world position of the @a actor.
	Vector3 actor_world_position(ActorId actor) const;

	/// Returns the world rotation of the @a actor.
	Quaternion actor_world_rotation(ActorId actor) const;

	/// Returns the world pose of the @a actor.
	Matrix4x4 actor_world_pose(ActorId actor) const;

	/// Teleports the @a actor to the given world position.
	void actor_teleport_world_position(ActorId actor, const Vector3 &p);

	/// Teleports the @a actor to the given world rotation.
	void actor_teleport_world_rotation(ActorId actor, const Quaternion &r);

	/// Teleports the @a actor to the given world pose.
	void actor_teleport_world_pose(ActorId actor, const Matrix4x4 &m);

	/// Returns the center of mass of the @a actor.
	Vector3 actor_center_of_mass(ActorId actor) const;

	/// Enables gravity for the @a actor.
	void actor_enable_gravity(ActorId actor);

	/// Disables gravity for the @a actor.
	void actor_disable_gravity(ActorId actor);

	/// Enables collision detection for the @a actor.
	void actor_enable_collision(ActorId actor);

	/// Disables collision detection for the @a actor.
	void actor_disable_collision(ActorId actor);

	/// Sets the collision filter of the @a actor.
	void actor_set_collision_filter(ActorId actor, StringId32 filter);

	/// Sets whether the @a actor is kinematic or not.
	/// @note This call has no effect on static actors.
	void actor_set_kinematic(ActorId actor, bool kinematic);

	/// Returns whether the @a actor is static.
	bool actor_is_static(ActorId actor) const;

	/// Returns whether the @a actor is dynamic.
	bool actor_is_dynamic(ActorId actor) const;

	/// Returns whether the @a actor is kinematic (keyframed).
	bool actor_is_kinematic(ActorId actor) const;

	/// Returns whether the @a actor is nonkinematic (i.e. dynamic and not kinematic).
	bool actor_is_nonkinematic(ActorId actor) const;

	/// Returns the linear damping of the @a actor.
	f32 actor_linear_damping(ActorId actor) const;

	/// Sets the linear damping of the @a actor.
	void actor_set_linear_damping(ActorId actor, f32 rate);

	/// Returns the angular damping of the @a actor.
	f32 actor_angular_damping(ActorId actor) const;

	/// Sets the angular damping of the @a actor.
	void actor_set_angular_damping(ActorId actor, f32 rate);

	/// Returns the linear velocity of the @a actor.
	Vector3 actor_linear_velocity(ActorId actor) const;

	/// Sets the linear velocity of the @a actor.
	/// @note This call only affects nonkinematic actors.
	void actor_set_linear_velocity(ActorId actor, const Vector3 &vel);

	/// Returns the angular velocity of the @a actor.
	Vector3 actor_angular_velocity(ActorId actor) const;

	/// Sets the angular velocity of the @a actor.
	/// @note This call only affects nonkinematic actors.
	void actor_set_angular_velocity(ActorId actor, const Vector3 &vel);

	/// Adds a linear impulse (acting along the center of mass) to the @a actor.
	/// @note This call only affects nonkinematic actors.
	void actor_add_impulse(ActorId actor, const Vector3 &impulse);

	/// Adds a linear impulse (acting along the world position @a pos) to the @a actor.
	/// @note This call only affects nonkinematic actors.
	void actor_add_impulse_at(ActorId actor, const Vector3 &impulse, const Vector3 &pos);

	/// Adds a torque impulse to the @a actor.
	void actor_add_torque_impulse(ActorId actor, const Vector3 &imp);

	/// Pushes the @a actor as if it was hit by a point object with the given @a mass
	/// travelling at the given @a velocity.
	/// @note This call only affects nonkinematic actors.
	void actor_push(ActorId actor, const Vector3 &vel, f32 mass);

	/// Like push() but applies the force at the world position @a pos.
	/// @note This call only affects nonkinematic actors.
	void actor_push_at(ActorId actor, const Vector3 &vel, f32 mass, const Vector3 &pos);

	/// Returns whether the @a actor is sleeping.
	bool actor_is_sleeping(ActorId actor);

	/// Wakes the @a actor up.
	void actor_wake_up(ActorId actor);

	/// Adds the @a actor's debug geometry to @a lines.
	void actor_debug_draw(ActorId actor, DebugLine *lines, const Color4 &color = COLOR4_ORANGE);

	///
	void mover_create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	/// Creates a new mover instance for the @a unit.
	MoverId mover_create(UnitId unit, const MoverDesc *desc);

	/// Destroys the @a mover.
	void mover_destroy(MoverId mover);

	/// Returns the ID of the mover owned by the *unit*.
	MoverId mover(UnitId unit);

	/// Sets the @a height of the @a mover capsule.
	void mover_set_height(MoverId mover, float height);

	/// Returns the radius of the @a mover capsule.
	f32 mover_radius(MoverId mover);

	/// Sets the @a radius of the @a mover capsule.
	void mover_set_radius(MoverId mover, float radius);

	/// Returns the max slope angle of the @a mover.
	f32 mover_max_slope_angle(MoverId mover);

	/// Sets the max slope @a angle of the @a mover.
	void mover_set_max_slope_angle(MoverId mover, f32 angle);

	/// Returns the step height of the @a mover.
	f32 mover_step_height(MoverId mover);

	/// Sets the step @a height of the @a mover.
	void mover_set_step_height(MoverId mover, f32 height);

	/// Sets the collision @a filter of the @a mover.
	void mover_set_collision_filter(MoverId mover, StringId32 filter);

	/// Returns the position of the @a mover.
	Vector3 mover_position(MoverId mover);

	/// Teleports the @a mover to the specified @a position.
	void mover_set_position(MoverId mover, const Vector3 &position);

	/// Returns the center of the *mover* relative to the transform's position.
	Vector3 mover_center(MoverId mover);

	/// Sets the center of the *mover* relative to the transform's position.
	void mover_set_center(MoverId mover, const Vector3 &center);

	/// Attempts to move the @a mover by the specified @a delta vector.
	/// The @a mover will slide against physical actors.
	void mover_move(MoverId mover, const Vector3 &delta);

	///
	bool mover_separate(MoverId mover, Vector3 &separation_delta);

	/// Returns whether the @a mover can be placed at @a position without colliding with any surrounding actors.
	bool mover_fits_at(MoverId mover, const Vector3 &position);

	/// Returns whether the @a mover collides sideways.
	bool mover_collides_sides(MoverId mover);

	/// Returns whether the @a mover collides upwards.
	bool mover_collides_up(MoverId mover);

	/// Returns whether the @a mover collides downwards.
	bool mover_collides_down(MoverId mover);

	/// Returns the actor currently colliding with the @a mover in the downwards direction, if any.
	ActorId mover_actor_colliding_down(MoverId mover);

	/// Adds the @a mover's debug geometry to @a lines.
	void mover_debug_draw(MoverId actor, DebugLine *lines, const Color4 &color = COLOR4_ORANGE);

	///
	void joint_create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	/// Creates a new joint between @a actor and @a other_actor.
	JointId joint_create(JointType::Enum type
		, ActorId actor
		, const Matrix4x4 &pose
		, ActorId other_actor
		, const Matrix4x4 &other_pose
		);

	/// Returns the ID of the joint owned by @a unit.
	JointId joint_instance(UnitId unit);

	/// Destroys the @a joint.
	void joint_destroy(JointId joint);

	/// Sets the break @a force of the @a joint. Use FLT_MAX to disable breaking.
	void joint_set_break_force(JointId joint, f32 force);

	/// Returns the spring stiffness and damping of the @a joint.
	void joint_spring_params(f32 &stiffness, f32 &damping, JointId joint);

	/// Sets the spring @a stiffness and @a damping of the @a joint.
	void joint_spring_set_params(JointId joint, f32 stiffness, f32 damping);

	/// Returns the current hinge angle of the @a joint.
	f32 joint_hinge_angle(JointId joint);

	/// Returns whether the hinge motor is enabled and its max motor impulse.
	void joint_hinge_motor(bool &enabled, f32 &max_motor_impulse, JointId joint);

	/// Enables or disables the hinge motor and sets its max motor impulse.
	void joint_hinge_set_motor(JointId joint, bool enabled, f32 max_motor_impulse);

	/// Returns the hinge motor target velocity.
	f32 joint_hinge_target_velocity(JointId joint);

	/// Sets the hinge motor target @a velocity.
	void joint_hinge_set_target_velocity(JointId joint, f32 velocity);

	/// Returns whether hinge limits are enabled, the lower limit, upper limit and bounciness.
	void joint_hinge_limits(bool &enabled, f32 &lower_limit, f32 &upper_limit, f32 &bounciness, JointId joint);

	/// Enables or disables hinge limits and sets the lower limit, upper limit and bounciness.
	void joint_hinge_set_limits(JointId joint, bool enabled, f32 lower_limit, f32 upper_limit, f32 bounciness);

	/// Returns the current limb twist angle of the @a joint.
	f32 joint_limb_twist_angle(JointId joint);

	/// Returns the current limb swing angle around Y of the @a joint.
	f32 joint_limb_swing_y_angle(JointId joint);

	/// Returns the current limb swing angle around Z of the @a joint.
	f32 joint_limb_swing_z_angle(JointId joint);

	/// Returns the twist, swing Y and swing Z motion settings.
	void joint_limb_motion(D6Motion::Enum &twist_motion, D6Motion::Enum &swing_y_motion, D6Motion::Enum &swing_z_motion, JointId joint);

	/// Sets the twist, swing Y and swing Z motion settings.
	void joint_limb_set_motion(JointId joint, D6Motion::Enum twist_motion, D6Motion::Enum swing_y_motion, D6Motion::Enum swing_z_motion);

	/// Returns the lower and upper twist limits.
	void joint_limb_twist_limit(f32 &lower_limit, f32 &upper_limit, JointId joint);

	/// Sets the lower and upper twist limits.
	void joint_limb_set_twist_limit(JointId joint, f32 lower_limit, f32 upper_limit);

	/// Returns the swing Y and swing Z limits.
	void joint_limb_swing_limit(f32 &y_limit, f32 &z_limit, JointId joint);

	/// Sets the swing Y and swing Z limits.
	void joint_limb_set_swing_limit(JointId joint, f32 y_limit, f32 z_limit);

	/// Returns the D6 linear motion for @a axis of the @a joint.
	D6Motion::Enum joint_d6_linear_motion(JointId joint, D6Axis::Enum axis);

	/// Sets the D6 linear @a motion for @a axis of the @a joint.
	void joint_d6_set_linear_motion(JointId joint, D6Axis::Enum axis, D6Motion::Enum motion);

	/// Returns the D6 angular motion for @a axis of the @a joint.
	D6Motion::Enum joint_d6_angular_motion(JointId joint, D6Axis::Enum axis);

	/// Sets the D6 angular @a motion for @a axis of the @a joint.
	void joint_d6_set_angular_motion(JointId joint, D6Axis::Enum axis, D6Motion::Enum motion);

	/// Returns the lower and upper D6 linear limits for @a axis of the @a joint.
	void joint_d6_linear_limit(f32 &lower, f32 &upper, JointId joint, D6Axis::Enum axis);

	/// Sets the lower and upper D6 linear limits for @a axis of the @a joint.
	void joint_d6_set_linear_limit(JointId joint, D6Axis::Enum axis, f32 lower, f32 upper);

	/// Returns the lower and upper D6 angular limits for @a axis of the @a joint.
	void joint_d6_angular_limit(f32 &lower, f32 &upper, JointId joint, D6Axis::Enum axis);

	/// Sets the lower and upper D6 angular limits for @a axis of the @a joint.
	void joint_d6_set_angular_limit(JointId joint, D6Axis::Enum axis, f32 lower, f32 upper);

	/// Returns the D6 linear motor, linear max force, angular motor and angular max force for @a axis of the @a joint.
	void joint_d6_motor(D6MotorMode::Enum &linear_motor, f32 &linear_max_force, D6MotorMode::Enum &angular_motor, f32 &angular_max_force, JointId joint, D6Axis::Enum axis);

	/// Sets the D6 linear and angular motors and max forces for @a axis of the @a joint.
	void joint_d6_set_motor(JointId joint, D6Axis::Enum axis, D6MotorMode::Enum linear_motor, f32 linear_max_force, D6MotorMode::Enum angular_motor, f32 angular_max_force);

	/// Returns the D6 linear and angular target velocities for @a axis of the @a joint.
	void joint_d6_target_velocity(f32 &linear, f32 &angular, JointId joint, D6Axis::Enum axis);

	/// Sets the D6 linear and angular target velocities for @a axis of the @a joint.
	void joint_d6_set_target_velocity(JointId joint, D6Axis::Enum axis, f32 linear, f32 angular);

	/// Returns the D6 linear and angular target positions for @a axis of the @a joint.
	void joint_d6_target_position(f32 &linear, f32 &angular, JointId joint, D6Axis::Enum axis);

	/// Sets the D6 linear and angular target positions for @a axis of the @a joint.
	void joint_d6_set_target_position(JointId joint, D6Axis::Enum axis, f32 linear, f32 angular);

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
