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
	void init(Allocator &a, const PhysicsSettings *settings)
	{
		CE_UNUSED_2(a, settings);
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

	JointId joint_create(ActorId /*a0*/, ActorId /*a1*/, const JointDesc & /*jd*/)
	{
		return make_joint_instance(UINT32_MAX);
	}

	void joint_destroy(JointId /*joint*/)
	{
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

JointId PhysicsWorld::joint_create(ActorId a0, ActorId a1, const JointDesc &jd)
{
	return _impl->joint_create(a0, a1, jd);
}

void PhysicsWorld::joint_destroy(JointId i)
{
	_impl->joint_destroy(i);
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
