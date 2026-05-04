/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PHYSICS_NOOP
#include "core/containers/array.inl"
#include "core/math/constants.h"
#include "core/memory/memory.inl"
#include "world/physics.h"
#include "world/physics_world.h"

namespace crown
{
namespace physics_globals
{
	void init(Allocator &a, Allocator &heap, ConsoleServer &cs, const PhysicsSettings *settings)
	{
		CE_UNUSED_4(a, heap, cs, settings);
	}

	void shutdown(Allocator &a)
	{
		CE_UNUSED(a);
	}

} // namespace physics_globals

struct PhysicsWorldImpl
{
	EventStream _events;

	explicit PhysicsWorldImpl(Allocator &a)
		: _events(a)
	{
	}

	~PhysicsWorldImpl()
	{
	}

	ColliderId collider_create(UnitId /*unit*/, const ColliderDesc * /*sd*/, const Vector3 & /*scl*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	void collider_destroy(ColliderId /*collider*/)
	{
	}

	ColliderId collider_first(UnitId /*unit*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	ColliderId collider_next(ColliderId /*collider*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	ActorId actor_create(UnitId /*unit*/, const ActorResource * /*ar*/, const Matrix4x4 & /*tm*/)
	{
		return make_actor_instance(UINT32_MAX);
	}

	void actor_destroy(ActorId /*actor*/)
	{
	}

	ActorId actor(UnitId /*unit*/)
	{
		return make_actor_instance(UINT32_MAX);
	}

	Vector3 actor_world_position(ActorId /*actor*/) const
	{
		return VECTOR3_ZERO;
	}

	Quaternion actor_world_rotation(ActorId /*actor*/) const
	{
		return QUATERNION_IDENTITY;
	}

	Matrix4x4 actor_world_pose(ActorId /*actor*/) const
	{
		return MATRIX4X4_IDENTITY;
	}

	void actor_teleport_world_position(ActorId /*actor*/, const Vector3 & /*p*/)
	{
	}

	void actor_teleport_world_rotation(ActorId /*actor*/, const Quaternion & /*r*/)
	{
	}

	void actor_teleport_world_pose(ActorId /*actor*/, const Matrix4x4 & /*m*/)
	{
	}

	Vector3 actor_center_of_mass(ActorId /*actor*/) const
	{
		return VECTOR3_ZERO;
	}

	void actor_enable_gravity(ActorId /*actor*/)
	{
	}

	void actor_disable_gravity(ActorId /*actor*/)
	{
	}

	void actor_enable_collision(ActorId /*actor*/)
	{
	}

	void actor_disable_collision(ActorId /*actor*/)
	{
	}

	void actor_set_collision_filter(ActorId /*actor*/, StringId32 /*filter*/)
	{
	}

	void actor_set_kinematic(ActorId /*actor*/, bool /*kinematic*/)
	{
	}

	bool actor_is_static(ActorId /*actor*/) const
	{
		return false;
	}

	bool actor_is_dynamic(ActorId /*actor*/) const
	{
		return false;
	}

	bool actor_is_kinematic(ActorId /*actor*/) const
	{
		return false;
	}

	bool actor_is_nonkinematic(ActorId /*actor*/) const
	{
		return false;
	}

	f32 actor_linear_damping(ActorId /*actor*/) const
	{
		return 0.0f;
	}

	void actor_set_linear_damping(ActorId /*actor*/, f32 /*rate*/)
	{
	}

	f32 actor_angular_damping(ActorId /*actor*/) const
	{
		return 0.0f;
	}

	void actor_set_angular_damping(ActorId /*actor*/, f32 /*rate*/)
	{
	}

	Vector3 actor_linear_velocity(ActorId /*actor*/) const
	{
		return VECTOR3_ZERO;
	}

	void actor_set_linear_velocity(ActorId /*actor*/, const Vector3 & /*vel*/)
	{
	}

	Vector3 actor_angular_velocity(ActorId /*actor*/) const
	{
		return VECTOR3_ZERO;
	}

	void actor_set_angular_velocity(ActorId /*actor*/, const Vector3 & /*vel*/)
	{
	}

	void actor_add_impulse(ActorId /*actor*/, const Vector3 & /*impulse*/)
	{
	}

	void actor_add_impulse_at(ActorId /*actor*/, const Vector3 & /*impulse*/, const Vector3 & /*pos*/)
	{
	}

	void actor_add_torque_impulse(ActorId /*actor*/, const Vector3 & /*imp*/)
	{
	}

	void actor_push(ActorId /*actor*/, const Vector3 & /*vel*/, f32 /*mass*/)
	{
	}

	void actor_push_at(ActorId /*actor*/, const Vector3 & /*vel*/, f32 /*mass*/, const Vector3 & /*pos*/)
	{
	}

	bool actor_is_sleeping(ActorId /*actor*/)
	{
		return false;
	}

	void actor_wake_up(ActorId /*actor*/)
	{
	}

	MoverId mover_create(UnitId unit, const MoverDesc *desc, const Matrix4x4 &tm)
	{
		CE_UNUSED_3(unit, desc, tm);
		return make_mover_instance(UINT32_MAX);
	}

	void mover_destroy(MoverId mover)
	{
		CE_UNUSED(mover);
	}

	MoverId mover(UnitId unit)
	{
		CE_UNUSED(unit);
		return make_mover_instance(UINT32_MAX);
	}

	f32 mover_radius(MoverId mover)
	{
		CE_UNUSED(mover);
		return 0.0f;
	}

	f32 mover_max_slope_angle(MoverId mover)
	{
		CE_UNUSED(mover);
		return 0.0f;
	}

	void mover_set_max_slope_angle(MoverId mover, f32 angle)
	{
		CE_UNUSED_2(mover, angle);
	}

	void mover_set_collision_filter(MoverId mover, StringId32 filter)
	{
		CE_UNUSED_2(mover, filter);
	}

	Vector3 mover_position(MoverId mover)
	{
		CE_UNUSED(mover);
		return VECTOR3_ZERO;
	}

	void mover_set_position(MoverId mover, const Vector3 &position)
	{
		CE_UNUSED_2(mover, position);
	}

	void mover_move(MoverId mover, const Vector3 &delta)
	{
		CE_UNUSED_2(mover, delta);
	}

	bool mover_separate(MoverId mover, Vector3 &separation_delta)
	{
		CE_UNUSED(mover);
		separation_delta = VECTOR3_ZERO;
		return false;
	}

	bool mover_fits_at(MoverId mover, const Vector3 &position)
	{
		CE_UNUSED_2(mover, position);
		return true;
	}

	bool mover_collides_sides(MoverId mover)
	{
		CE_UNUSED(mover);
		return false;
	}

	bool mover_collides_up(MoverId mover)
	{
		CE_UNUSED(mover);
		return false;
	}

	bool mover_collides_down(MoverId mover)
	{
		CE_UNUSED(mover);
		return false;
	}

	ActorId mover_actor_colliding_down(MoverId mover)
	{
		CE_UNUSED(mover);
		return make_actor_instance(UINT32_MAX);
	}

	JointId joint_create(JointType::Enum type
		, ActorId actor
		, const Matrix4x4 &pose
		, ActorId other_actor
		, const Matrix4x4 &other_pose
		)
	{
		CE_UNUSED_5(type, actor, pose, other_actor, other_pose);
		return make_joint_instance(UINT32_MAX);
	}

	void joint_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
	{
		CE_UNUSED_4(components_data, num, unit_lookup, unit_index);
	}

	JointId joint_instance(UnitId unit)
	{
		CE_UNUSED(unit);
		return make_joint_instance(UINT32_MAX);
	}

	void joint_destroy(JointId /*joint*/)
	{
	}

	void joint_set_break_force(JointId joint, f32 force)
	{
		CE_UNUSED_2(joint, force);
	}

	void joint_spring_params(f32 &stiffness, f32 &damping, JointId joint)
	{
		CE_UNUSED(joint);
		stiffness = 0.0f;
		damping = 0.0f;
	}

	void joint_spring_set_params(JointId joint, f32 stiffness, f32 damping)
	{
		CE_UNUSED_3(joint, stiffness, damping);
	}

	f32 joint_hinge_angle(JointId joint)
	{
		CE_UNUSED(joint);
		return 0.0f;
	}

	void joint_hinge_motor(bool &enabled, f32 &max_motor_impulse, JointId joint)
	{
		CE_UNUSED(joint);
		enabled = false;
		max_motor_impulse = 0.0f;
	}

	void joint_hinge_set_motor(JointId joint, bool enabled, f32 max_motor_impulse)
	{
		CE_UNUSED_3(joint, enabled, max_motor_impulse);
	}

	f32 joint_hinge_target_velocity(JointId joint)
	{
		CE_UNUSED(joint);
		return 0.0f;
	}

	void joint_hinge_set_target_velocity(JointId joint, f32 velocity)
	{
		CE_UNUSED_2(joint, velocity);
	}

	void joint_hinge_limits(bool &enabled, f32 &lower_limit, f32 &upper_limit, f32 &bounciness, JointId joint)
	{
		CE_UNUSED(joint);
		enabled = false;
		lower_limit = 0.0f;
		upper_limit = 0.0f;
		bounciness = 0.0f;
	}

	void joint_hinge_set_limits(JointId joint, bool enabled, f32 lower_limit, f32 upper_limit, f32 bounciness)
	{
		CE_UNUSED_5(joint, enabled, lower_limit, upper_limit, bounciness);
	}

	f32 joint_limb_twist_angle(JointId joint)
	{
		CE_UNUSED(joint);
		return 0.0f;
	}

	f32 joint_limb_swing_y_angle(JointId joint)
	{
		CE_UNUSED(joint);
		return 0.0f;
	}

	f32 joint_limb_swing_z_angle(JointId joint)
	{
		CE_UNUSED(joint);
		return 0.0f;
	}

	void joint_limb_motion(D6Motion::Enum &twist_motion, D6Motion::Enum &swing_y_motion, D6Motion::Enum &swing_z_motion, JointId joint)
	{
		CE_UNUSED(joint);
		twist_motion = D6Motion::LOCKED;
		swing_y_motion = D6Motion::LOCKED;
		swing_z_motion = D6Motion::LOCKED;
	}

	void joint_limb_set_motion(JointId joint, D6Motion::Enum twist_motion, D6Motion::Enum swing_y_motion, D6Motion::Enum swing_z_motion)
	{
		CE_UNUSED_4(joint, twist_motion, swing_y_motion, swing_z_motion);
	}

	void joint_limb_twist_limit(f32 &lower_limit, f32 &upper_limit, JointId joint)
	{
		CE_UNUSED(joint);
		lower_limit = 0.0f;
		upper_limit = 0.0f;
	}

	void joint_limb_set_twist_limit(JointId joint, f32 lower_limit, f32 upper_limit)
	{
		CE_UNUSED_3(joint, lower_limit, upper_limit);
	}

	void joint_limb_swing_limit(f32 &y_limit, f32 &z_limit, JointId joint)
	{
		CE_UNUSED(joint);
		y_limit = 0.0f;
		z_limit = 0.0f;
	}

	void joint_limb_set_swing_limit(JointId joint, f32 y_limit, f32 z_limit)
	{
		CE_UNUSED_3(joint, y_limit, z_limit);
	}

	D6Motion::Enum joint_d6_linear_motion(JointId joint, D6Axis::Enum axis)
	{
		CE_UNUSED_2(joint, axis);
		return D6Motion::LOCKED;
	}

	void joint_d6_set_linear_motion(JointId joint, D6Axis::Enum axis, D6Motion::Enum motion)
	{
		CE_UNUSED_3(joint, axis, motion);
	}

	D6Motion::Enum joint_d6_angular_motion(JointId joint, D6Axis::Enum axis)
	{
		CE_UNUSED_2(joint, axis);
		return D6Motion::LOCKED;
	}

	void joint_d6_set_angular_motion(JointId joint, D6Axis::Enum axis, D6Motion::Enum motion)
	{
		CE_UNUSED_3(joint, axis, motion);
	}

	void joint_d6_linear_limit(f32 &lower, f32 &upper, JointId joint, D6Axis::Enum axis)
	{
		CE_UNUSED_2(joint, axis);
		lower = 0.0f;
		upper = 0.0f;
	}

	void joint_d6_set_linear_limit(JointId joint, D6Axis::Enum axis, f32 lower, f32 upper)
	{
		CE_UNUSED_4(joint, axis, lower, upper);
	}

	void joint_d6_angular_limit(f32 &lower, f32 &upper, JointId joint, D6Axis::Enum axis)
	{
		CE_UNUSED_2(joint, axis);
		lower = 0.0f;
		upper = 0.0f;
	}

	void joint_d6_set_angular_limit(JointId joint, D6Axis::Enum axis, f32 lower, f32 upper)
	{
		CE_UNUSED_4(joint, axis, lower, upper);
	}

	void joint_d6_motor(D6MotorMode::Enum &linear_motor, f32 &linear_max_force, D6MotorMode::Enum &angular_motor, f32 &angular_max_force, JointId joint, D6Axis::Enum axis)
	{
		CE_UNUSED_2(joint, axis);
		linear_motor = D6MotorMode::OFF;
		linear_max_force = 0.0f;
		angular_motor = D6MotorMode::OFF;
		angular_max_force = 0.0f;
	}

	void joint_d6_set_motor(JointId joint, D6Axis::Enum axis, D6MotorMode::Enum linear_motor, f32 linear_max_force, D6MotorMode::Enum angular_motor, f32 angular_max_force)
	{
		CE_UNUSED_6(joint, axis, linear_motor, linear_max_force, angular_motor, angular_max_force);
	}

	void joint_d6_target_velocity(f32 &linear, f32 &angular, JointId joint, D6Axis::Enum axis)
	{
		CE_UNUSED_2(joint, axis);
		linear = 0.0f;
		angular = 0.0f;
	}

	void joint_d6_set_target_velocity(JointId joint, D6Axis::Enum axis, f32 linear, f32 angular)
	{
		CE_UNUSED_4(joint, axis, linear, angular);
	}

	void joint_d6_target_position(f32 &linear, f32 &angular, JointId joint, D6Axis::Enum axis)
	{
		CE_UNUSED_2(joint, axis);
		linear = 0.0f;
		angular = 0.0f;
	}

	void joint_d6_set_target_position(JointId joint, D6Axis::Enum axis, f32 linear, f32 angular)
	{
		CE_UNUSED_4(joint, axis, linear, angular);
	}

	bool cast_ray(RaycastHit & /*hit*/, const Vector3 & /*from*/, const Vector3 & /*dir*/, f32 /*len*/)
	{
		return false;
	}

	bool cast_ray_all(Array<RaycastHit> & /*hits*/, const Vector3 & /*from*/, const Vector3 & /*dir*/, f32 /*len*/)
	{
		return false;
	}

	bool cast_sphere(RaycastHit & /*hit*/, const Vector3 & /*from*/, f32 /*radius*/, const Vector3 & /*dir*/, f32 /*len*/)
	{
		return false;
	}

	bool cast_box(RaycastHit & /*hit*/, const Vector3 & /*from*/, const Vector3 & /*half_extents*/, const Vector3 & /*dir*/, f32 /*len*/)
	{
		return false;
	}

	Vector3 gravity() const
	{
		return VECTOR3_ZERO;
	}

	void set_gravity(const Vector3 & /*g*/)
	{
	}

	void update_actor_world_poses(const UnitId * /*begin*/, const UnitId * /*end*/, const Matrix4x4 * /*begin_world*/)
	{
	}

	void update(f32 /*dt*/)
	{
	}

	EventStream &events()
	{
		return _events;
	}

	void debug_draw()
	{
	}

	void enable_debug_drawing(bool /*enable*/)
	{
	}

	ColliderId make_collider_instance(u32 i)
	{
		ColliderId inst = { i }; return inst;
	}

	ActorId make_actor_instance(u32 i)
	{
		ActorId inst = { i }; return inst;
	}

	MoverId make_mover_instance(u32 i)
	{
		MoverId inst = { i }; return inst;
	}

	JointId make_joint_instance(u32 i)
	{
		JointId inst = { i }; return inst;
	}
};

PhysicsWorld::PhysicsWorld(Allocator &a, ResourceManager & /*rm*/, UnitManager & /*um*/, DebugLine & /*dl*/)
	: _marker(PHYSICS_WORLD_MARKER)
	, _allocator(&a)
	, _impl(NULL)
{
	_impl = CE_NEW(*_allocator, PhysicsWorldImpl)(a);
}

PhysicsWorld::~PhysicsWorld()
{
	CE_DELETE(*_allocator, _impl);
	_marker = 0;
}

ColliderId PhysicsWorld::collider_create(UnitId unit, const ColliderDesc *sd, const Vector3 &scl)
{
	return _impl->collider_create(unit, sd, scl);
}

void PhysicsWorld::collider_destroy(ColliderId i)
{
	_impl->collider_destroy(i);
}

ColliderId PhysicsWorld::collider_first(UnitId unit)
{
	return _impl->collider_first(unit);
}

ColliderId PhysicsWorld::collider_next(ColliderId i)
{
	return _impl->collider_next(i);
}

ActorId PhysicsWorld::actor_create(UnitId unit, const ActorResource *ar, const Matrix4x4 &tm)
{
	return _impl->actor_create(unit, ar, tm);
}

void PhysicsWorld::actor_destroy(ActorId actor)
{
	_impl->actor_destroy(actor);
}

ActorId PhysicsWorld::actor(UnitId unit)
{
	return _impl->actor(unit);
}

Vector3 PhysicsWorld::actor_world_position(ActorId actor) const
{
	return _impl->actor_world_position(actor);
}

Quaternion PhysicsWorld::actor_world_rotation(ActorId actor) const
{
	return _impl->actor_world_rotation(actor);
}

Matrix4x4 PhysicsWorld::actor_world_pose(ActorId actor) const
{
	return _impl->actor_world_pose(actor);
}

void PhysicsWorld::actor_teleport_world_position(ActorId actor, const Vector3 &p)
{
	_impl->actor_teleport_world_position(actor, p);
}

void PhysicsWorld::actor_teleport_world_rotation(ActorId actor, const Quaternion &r)
{
	_impl->actor_teleport_world_rotation(actor, r);
}

void PhysicsWorld::actor_teleport_world_pose(ActorId actor, const Matrix4x4 &m)
{
	_impl->actor_teleport_world_pose(actor, m);
}

Vector3 PhysicsWorld::actor_center_of_mass(ActorId actor) const
{
	return _impl->actor_center_of_mass(actor);
}

void PhysicsWorld::actor_enable_gravity(ActorId actor)
{
	_impl->actor_enable_gravity(actor);
}

void PhysicsWorld::actor_disable_gravity(ActorId actor)
{
	_impl->actor_disable_gravity(actor);
}

void PhysicsWorld::actor_enable_collision(ActorId actor)
{
	_impl->actor_enable_collision(actor);
}

void PhysicsWorld::actor_disable_collision(ActorId actor)
{
	_impl->actor_disable_collision(actor);
}

void PhysicsWorld::actor_set_collision_filter(ActorId actor, StringId32 filter)
{
	_impl->actor_set_collision_filter(actor, filter);
}

void PhysicsWorld::actor_set_kinematic(ActorId actor, bool kinematic)
{
	_impl->actor_set_kinematic(actor, kinematic);
}

bool PhysicsWorld::actor_is_static(ActorId actor) const
{
	return _impl->actor_is_static(actor);
}

bool PhysicsWorld::actor_is_dynamic(ActorId actor) const
{
	return _impl->actor_is_dynamic(actor);
}

bool PhysicsWorld::actor_is_kinematic(ActorId actor) const
{
	return _impl->actor_is_kinematic(actor);
}

bool PhysicsWorld::actor_is_nonkinematic(ActorId actor) const
{
	return _impl->actor_is_nonkinematic(actor);
}

f32 PhysicsWorld::actor_linear_damping(ActorId actor) const
{
	return _impl->actor_linear_damping(actor);
}

void PhysicsWorld::actor_set_linear_damping(ActorId actor, f32 rate)
{
	_impl->actor_set_linear_damping(actor, rate);
}

f32 PhysicsWorld::actor_angular_damping(ActorId actor) const
{
	return _impl->actor_angular_damping(actor);
}

void PhysicsWorld::actor_set_angular_damping(ActorId actor, f32 rate)
{
	_impl->actor_set_angular_damping(actor, rate);
}

Vector3 PhysicsWorld::actor_linear_velocity(ActorId actor) const
{
	return _impl->actor_linear_velocity(actor);
}

void PhysicsWorld::actor_set_linear_velocity(ActorId actor, const Vector3 &vel)
{
	_impl->actor_set_linear_velocity(actor, vel);
}

Vector3 PhysicsWorld::actor_angular_velocity(ActorId actor) const
{
	return _impl->actor_angular_velocity(actor);
}

void PhysicsWorld::actor_set_angular_velocity(ActorId actor, const Vector3 &vel)
{
	_impl->actor_set_angular_velocity(actor, vel);
}

void PhysicsWorld::actor_add_impulse(ActorId actor, const Vector3 &impulse)
{
	_impl->actor_add_impulse(actor, impulse);
}

void PhysicsWorld::actor_add_impulse_at(ActorId actor, const Vector3 &impulse, const Vector3 &pos)
{
	_impl->actor_add_impulse_at(actor, impulse, pos);
}

void PhysicsWorld::actor_add_torque_impulse(ActorId actor, const Vector3 &imp)
{
	_impl->actor_add_torque_impulse(actor, imp);
}

void PhysicsWorld::actor_push(ActorId actor, const Vector3 &vel, f32 mass)
{
	_impl->actor_push(actor, vel, mass);
}

void PhysicsWorld::actor_push_at(ActorId actor, const Vector3 &vel, f32 mass, const Vector3 &pos)
{
	_impl->actor_push_at(actor, vel, mass, pos);
}

bool PhysicsWorld::actor_is_sleeping(ActorId actor)
{
	return _impl->actor_is_sleeping(actor);
}

void PhysicsWorld::actor_wake_up(ActorId actor)
{
	_impl->actor_wake_up(actor);
}

MoverId PhysicsWorld::mover_create(UnitId unit, const MoverDesc *desc, const Matrix4x4 &tm)
{
	return _impl->mover_create(unit, desc, tm);
}

void PhysicsWorld::mover_destroy(MoverId actor)
{
	_impl->mover_destroy(actor);
}

MoverId PhysicsWorld::mover(UnitId unit)
{
	return _impl->mover(unit);
}

f32 PhysicsWorld::mover_radius(MoverId mover)
{
	return _impl->mover_radius(mover);
}

f32 PhysicsWorld::mover_max_slope_angle(MoverId mover)
{
	return _impl->mover_max_slope_angle(mover);
}

void PhysicsWorld::mover_set_max_slope_angle(MoverId mover, f32 angle)
{
	_impl->mover_set_max_slope_angle(mover, angle);
}

void PhysicsWorld::mover_set_collision_filter(MoverId mover, StringId32 filter)
{
	_impl->mover_set_collision_filter(mover, filter);
}

Vector3 PhysicsWorld::mover_position(MoverId mover)
{
	return _impl->mover_position(mover);
}

void PhysicsWorld::mover_set_position(MoverId mover, const Vector3 &position)
{
	return _impl->mover_set_position(mover, position);
}

void PhysicsWorld::mover_move(MoverId mover, const Vector3 &delta)
{
	_impl->mover_move(mover, delta);
}

bool PhysicsWorld::mover_separate(MoverId mover, Vector3 &separation_delta)
{
	return _impl->mover_separate(mover, separation_delta);
}

bool PhysicsWorld::mover_fits_at(MoverId mover, const Vector3 &position)
{
	return _impl->mover_fits_at(mover, position);
}

bool PhysicsWorld::mover_collides_sides(MoverId mover)
{
	return _impl->mover_collides_sides(mover);
}

bool PhysicsWorld::mover_collides_up(MoverId mover)
{
	return _impl->mover_collides_up(mover);
}

bool PhysicsWorld::mover_collides_down(MoverId mover)
{
	return _impl->mover_collides_down(mover);
}

ActorId PhysicsWorld::mover_actor_colliding_down(MoverId mover)
{
	return _impl->mover_actor_colliding_down(mover);
}

JointId PhysicsWorld::joint_create(JointType::Enum type
	, ActorId actor
	, const Matrix4x4 &pose
	, ActorId other_actor
	, const Matrix4x4 &other_pose
	)
{
	return _impl->joint_create(type, actor, pose, other_actor, other_pose);
}

void PhysicsWorld::joint_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
{
	_impl->joint_create_instances(components_data, num, unit_lookup, unit_index);
}

JointId PhysicsWorld::joint_instance(UnitId unit)
{
	return _impl->joint_instance(unit);
}

void PhysicsWorld::joint_destroy(JointId i)
{
	_impl->joint_destroy(i);
}

void PhysicsWorld::joint_set_break_force(JointId joint, f32 force)
{
	_impl->joint_set_break_force(joint, force);
}

void PhysicsWorld::joint_spring_params(f32 &stiffness, f32 &damping, JointId joint)
{
	_impl->joint_spring_params(stiffness, damping, joint);
}

void PhysicsWorld::joint_spring_set_params(JointId joint, f32 stiffness, f32 damping)
{
	_impl->joint_spring_set_params(joint, stiffness, damping);
}

f32 PhysicsWorld::joint_hinge_angle(JointId joint)
{
	return _impl->joint_hinge_angle(joint);
}

void PhysicsWorld::joint_hinge_motor(bool &enabled, f32 &max_motor_impulse, JointId joint)
{
	_impl->joint_hinge_motor(enabled, max_motor_impulse, joint);
}

void PhysicsWorld::joint_hinge_set_motor(JointId joint, bool enabled, f32 max_motor_impulse)
{
	_impl->joint_hinge_set_motor(joint, enabled, max_motor_impulse);
}

f32 PhysicsWorld::joint_hinge_target_velocity(JointId joint)
{
	return _impl->joint_hinge_target_velocity(joint);
}

void PhysicsWorld::joint_hinge_set_target_velocity(JointId joint, f32 velocity)
{
	_impl->joint_hinge_set_target_velocity(joint, velocity);
}

void PhysicsWorld::joint_hinge_limits(bool &enabled, f32 &lower_limit, f32 &upper_limit, f32 &bounciness, JointId joint)
{
	_impl->joint_hinge_limits(enabled, lower_limit, upper_limit, bounciness, joint);
}

void PhysicsWorld::joint_hinge_set_limits(JointId joint, bool enabled, f32 lower_limit, f32 upper_limit, f32 bounciness)
{
	_impl->joint_hinge_set_limits(joint, enabled, lower_limit, upper_limit, bounciness);
}

f32 PhysicsWorld::joint_limb_twist_angle(JointId joint)
{
	return _impl->joint_limb_twist_angle(joint);
}

f32 PhysicsWorld::joint_limb_swing_y_angle(JointId joint)
{
	return _impl->joint_limb_swing_y_angle(joint);
}

f32 PhysicsWorld::joint_limb_swing_z_angle(JointId joint)
{
	return _impl->joint_limb_swing_z_angle(joint);
}

void PhysicsWorld::joint_limb_motion(D6Motion::Enum &twist_motion, D6Motion::Enum &swing_y_motion, D6Motion::Enum &swing_z_motion, JointId joint)
{
	_impl->joint_limb_motion(twist_motion, swing_y_motion, swing_z_motion, joint);
}

void PhysicsWorld::joint_limb_set_motion(JointId joint, D6Motion::Enum twist_motion, D6Motion::Enum swing_y_motion, D6Motion::Enum swing_z_motion)
{
	_impl->joint_limb_set_motion(joint, twist_motion, swing_y_motion, swing_z_motion);
}

void PhysicsWorld::joint_limb_twist_limit(f32 &lower_limit, f32 &upper_limit, JointId joint)
{
	_impl->joint_limb_twist_limit(lower_limit, upper_limit, joint);
}

void PhysicsWorld::joint_limb_set_twist_limit(JointId joint, f32 lower_limit, f32 upper_limit)
{
	_impl->joint_limb_set_twist_limit(joint, lower_limit, upper_limit);
}

void PhysicsWorld::joint_limb_swing_limit(f32 &y_limit, f32 &z_limit, JointId joint)
{
	_impl->joint_limb_swing_limit(y_limit, z_limit, joint);
}

void PhysicsWorld::joint_limb_set_swing_limit(JointId joint, f32 y_limit, f32 z_limit)
{
	_impl->joint_limb_set_swing_limit(joint, y_limit, z_limit);
}

D6Motion::Enum PhysicsWorld::joint_d6_linear_motion(JointId joint, D6Axis::Enum axis)
{
	return _impl->joint_d6_linear_motion(joint, axis);
}

void PhysicsWorld::joint_d6_set_linear_motion(JointId joint, D6Axis::Enum axis, D6Motion::Enum motion)
{
	_impl->joint_d6_set_linear_motion(joint, axis, motion);
}

D6Motion::Enum PhysicsWorld::joint_d6_angular_motion(JointId joint, D6Axis::Enum axis)
{
	return _impl->joint_d6_angular_motion(joint, axis);
}

void PhysicsWorld::joint_d6_set_angular_motion(JointId joint, D6Axis::Enum axis, D6Motion::Enum motion)
{
	_impl->joint_d6_set_angular_motion(joint, axis, motion);
}

void PhysicsWorld::joint_d6_linear_limit(f32 &lower, f32 &upper, JointId joint, D6Axis::Enum axis)
{
	_impl->joint_d6_linear_limit(lower, upper, joint, axis);
}

void PhysicsWorld::joint_d6_set_linear_limit(JointId joint, D6Axis::Enum axis, f32 lower, f32 upper)
{
	_impl->joint_d6_set_linear_limit(joint, axis, lower, upper);
}

void PhysicsWorld::joint_d6_angular_limit(f32 &lower, f32 &upper, JointId joint, D6Axis::Enum axis)
{
	_impl->joint_d6_angular_limit(lower, upper, joint, axis);
}

void PhysicsWorld::joint_d6_set_angular_limit(JointId joint, D6Axis::Enum axis, f32 lower, f32 upper)
{
	_impl->joint_d6_set_angular_limit(joint, axis, lower, upper);
}

void PhysicsWorld::joint_d6_motor(D6MotorMode::Enum &linear_motor, f32 &linear_max_force, D6MotorMode::Enum &angular_motor, f32 &angular_max_force, JointId joint, D6Axis::Enum axis)
{
	_impl->joint_d6_motor(linear_motor, linear_max_force, angular_motor, angular_max_force, joint, axis);
}

void PhysicsWorld::joint_d6_set_motor(JointId joint, D6Axis::Enum axis, D6MotorMode::Enum linear_motor, f32 linear_max_force, D6MotorMode::Enum angular_motor, f32 angular_max_force)
{
	_impl->joint_d6_set_motor(joint, axis, linear_motor, linear_max_force, angular_motor, angular_max_force);
}

void PhysicsWorld::joint_d6_target_velocity(f32 &linear, f32 &angular, JointId joint, D6Axis::Enum axis)
{
	_impl->joint_d6_target_velocity(linear, angular, joint, axis);
}

void PhysicsWorld::joint_d6_set_target_velocity(JointId joint, D6Axis::Enum axis, f32 linear, f32 angular)
{
	_impl->joint_d6_set_target_velocity(joint, axis, linear, angular);
}

void PhysicsWorld::joint_d6_target_position(f32 &linear, f32 &angular, JointId joint, D6Axis::Enum axis)
{
	_impl->joint_d6_target_position(linear, angular, joint, axis);
}

void PhysicsWorld::joint_d6_set_target_position(JointId joint, D6Axis::Enum axis, f32 linear, f32 angular)
{
	_impl->joint_d6_set_target_position(joint, axis, linear, angular);
}

bool PhysicsWorld::cast_ray(RaycastHit &hit, const Vector3 &from, const Vector3 &dir, f32 len)
{
	return _impl->cast_ray(hit, from, dir, len);
}

bool PhysicsWorld::cast_ray_all(Array<RaycastHit> &hits, const Vector3 &from, const Vector3 &dir, f32 len)
{
	return _impl->cast_ray_all(hits, from, dir, len);
}

bool PhysicsWorld::cast_sphere(RaycastHit &hit, const Vector3 &from, f32 radius, const Vector3 &dir, f32 len)
{
	return _impl->cast_sphere(hit, from, radius, dir, len);
}

bool PhysicsWorld::cast_box(RaycastHit &hit, const Vector3 &from, const Vector3 &half_extents, const Vector3 &dir, f32 len)
{
	return _impl->cast_box(hit, from, half_extents, dir, len);
}

Vector3 PhysicsWorld::gravity() const
{
	return _impl->gravity();
}

void PhysicsWorld::set_gravity(const Vector3 &g)
{
	_impl->set_gravity(g);
}

void PhysicsWorld::update_actor_world_poses(const UnitId *begin, const UnitId *end, const Matrix4x4 *begin_world)
{
	_impl->update_actor_world_poses(begin, end, begin_world);
}

void PhysicsWorld::update(f32 dt)
{
	_impl->update(dt);
}

EventStream &PhysicsWorld::events()
{
	return _impl->events();
}

void PhysicsWorld::debug_draw()
{
	_impl->debug_draw();
}

void PhysicsWorld::enable_debug_drawing(bool enable)
{
	_impl->enable_debug_drawing(enable);
}

} // namespace crown

#endif // if CROWN_PHYSICS_NOOP
