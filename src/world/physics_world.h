/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "event_stream.h"
#include "math_types.h"
#include "resource_types.h"
#include "world_types.h"

namespace crown
{
/// Manages physics objects in a World.
///
/// @ingroup World
class PhysicsWorld
{
public:

	PhysicsWorld() {}
	virtual ~PhysicsWorld() {}

	virtual ColliderInstance create_collider(UnitId id, const ShapeDesc* sd) = 0;
	virtual ColliderInstance first_collider(UnitId id) = 0;
	virtual ColliderInstance next_collider(ColliderInstance i) = 0;

	virtual ActorInstance create_actor(UnitId id, const ActorResource* ar, const Matrix4x4& tm) = 0;
	virtual void destroy_actor(ActorInstance i) = 0;
	virtual ActorInstance actor(UnitId id) = 0;

	/// Returns the world position of the actor.
	virtual Vector3 actor_world_position(ActorInstance i) const = 0;

	/// Returns the world rotation of the actor.
	virtual Quaternion actor_world_rotation(ActorInstance i) const = 0;

	/// Returns the world pose of the actor.
	virtual Matrix4x4 actor_world_pose(ActorInstance i) const = 0;

	/// Teleports the actor to the given world position.
	virtual void teleport_actor_world_position(ActorInstance i, const Vector3& p) = 0;

	/// Teleports the actor to the given world rotation.
	virtual void teleport_actor_world_rotation(ActorInstance i, const Quaternion& r) = 0;

	/// Teleports the actor to the given world pose.
	virtual void teleport_actor_world_pose(ActorInstance i, const Matrix4x4& m) = 0;

	/// Returns the center of mass of the actor.
	virtual Vector3 actor_center_of_mass(ActorInstance i) const = 0;

	/// Enables gravity for the actor.
	virtual void enable_actor_gravity(ActorInstance i) = 0;

	/// Disables gravity for the actor.
	virtual void disable_actor_gravity(ActorInstance i) = 0;

	/// Enables collision detection for the actor.
	virtual void enable_actor_collision(ActorInstance i) = 0;

	/// Disables collision detection for the actor.
	virtual void disable_actor_collision(ActorInstance i) = 0;

	/// Sets the collision filter of the actor.
	virtual void set_actor_collision_filter(ActorInstance i, StringId32 filter) = 0;

	/// Sets whether the actor is kinematic or not.
	/// @note This call has no effect on static actors.
	virtual void set_actor_kinematic(ActorInstance i, bool kinematic) = 0;

	/// Moves the actor to @a pos
	/// @note This call only affects nonkinematic actors.
	virtual void move_actor(ActorInstance i, const Vector3& pos) = 0;

	/// Returns whether the actor is static.
	virtual bool is_static(ActorInstance i) const = 0;

	/// Returns whether the actor is dynamic.
	virtual bool is_dynamic(ActorInstance i) const = 0;

	/// Returns whether the actor is kinematic (keyframed).
	virtual bool is_kinematic(ActorInstance i) const = 0;

	/// Returns whether the actor is nonkinematic (i.e. dynamic and not kinematic).
	virtual bool is_nonkinematic(ActorInstance i) const = 0;

	/// Returns the linear damping of the actor.
	virtual float actor_linear_damping(ActorInstance i) const = 0;

	/// Sets the linear damping of the actor.
	virtual void set_actor_linear_damping(ActorInstance i, float rate) = 0;

	/// Returns the angular damping of the actor.
	virtual float actor_angular_damping(ActorInstance i) const = 0;

	/// Sets the angular damping of the actor.
	virtual void set_actor_angular_damping(ActorInstance i, float rate) = 0;

	/// Returns the linear velocity of the actor.
	virtual Vector3 actor_linear_velocity(ActorInstance i) const = 0;

	/// Sets the linear velocity of the actor.
	/// @note This call only affects nonkinematic actors.
	virtual void set_actor_linear_velocity(ActorInstance i, const Vector3& vel) = 0;

	/// Returns the angular velocity of the actor.
	virtual Vector3 actor_angular_velocity(ActorInstance i) const = 0;

	/// Sets the angular velocity of the actor.
	/// @note This call only affects nonkinematic actors.
	virtual void set_actor_angular_velocity(ActorInstance i, const Vector3& vel) = 0;

	/// Adds a linear impulse (acting along the center of mass) to the actor.
	/// @note This call only affects nonkinematic actors.
	virtual void add_actor_impulse(ActorInstance i, const Vector3& impulse) = 0;

	/// Adds a linear impulse (acting along the world position @a pos) to the actor.
	/// @note This call only affects nonkinematic actors.
	virtual void add_actor_impulse_at(ActorInstance i, const Vector3& impulse, const Vector3& pos) = 0;

	/// Adds a torque impulse to the actor.
	virtual void add_actor_torque_impulse(ActorInstance i, const Vector3& imp) = 0;

	/// Pushes the actor as if it was hit by a point object with the given @a mass
	/// travelling at the given @a velocity.
	/// @note This call only affects nonkinematic actors.
	virtual void push_actor(ActorInstance i, const Vector3& vel, float mass) = 0;

	/// Like push() but applies the force at the world position @a pos.
	/// @note This call only affects nonkinematic actors.
	virtual void push_actor_at(ActorInstance i, const Vector3& vel, float mass, const Vector3& pos) = 0;

	/// Returns whether the actor is sleeping.
	virtual bool is_sleeping(ActorInstance i) = 0;

	/// Wakes the actor up.
	virtual void wake_up(ActorInstance i) = 0;

	virtual ControllerInstance create_controller(UnitId id, const ControllerDesc& cd, const Matrix4x4& tm) = 0;
	virtual void destroy_controller(ControllerInstance id) = 0;
	virtual ControllerInstance controller(UnitId id) = 0;

	/// Returns the position of the controller.
	virtual Vector3 position(ControllerInstance i) const = 0;

	/// Moves the controller to @a pos.
	virtual void move_controller(ControllerInstance i, const Vector3& pos) = 0;

	/// Sets the contoller height.
	virtual void set_height(ControllerInstance i, float height) = 0;

	/// Returns whether the contoller collides upwards.
	virtual bool collides_up(ControllerInstance i) const = 0;

	/// Returns whether the controller collides downwards.
	virtual bool collides_down(ControllerInstance i) const = 0;

	/// Returns whether the controller collides sidewards.
	virtual bool collides_sides(ControllerInstance i) const = 0;

	/// Creates joint
	virtual JointInstance create_joint(ActorInstance a0, ActorInstance a1, const JointDesc& jd) = 0;
	virtual void destroy_joint(JointInstance i) = 0;

	/// Performs a raycast.
	virtual void raycast(const Vector3& from, const Vector3& dir, float len, RaycastMode::Enum mode, Array<RaycastHit>& hits) = 0;

	/// Returns the gravity.
	virtual Vector3 gravity() const = 0;

	/// Sets the gravity.
	virtual void set_gravity(const Vector3& g) = 0;

	virtual void update_actor_world_poses(const UnitId* begin, const UnitId* end, const Matrix4x4* begin_world) = 0;

	/// Updates the physics simulation.
	virtual void update(float dt) = 0;

	virtual EventStream& events() = 0;

	/// Draws debug lines.
	virtual void draw_debug() = 0;

	virtual void enable_debug_drawing(bool enable) = 0;

	static PhysicsWorld* create(Allocator& a, ResourceManager& rm, UnitManager& um, DebugLine& dl);
	static void destroy(Allocator& a, PhysicsWorld* pw);
};

} // namespace crown
