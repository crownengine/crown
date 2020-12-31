/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PHYSICS_NOOP

#include "core/containers/array.inl"
#include "core/math/constants.h"
#include "core/memory/memory.inl"
#include "world/physics_world.h"

namespace crown
{
namespace physics_globals
{
	void init(Allocator& /*a*/)
	{
	}

	void shutdown(Allocator& /*a*/)
	{
	}

} // namespace physics_globals

struct PhysicsWorldImpl
{
	EventStream _events;

	explicit PhysicsWorldImpl(Allocator& a)
		: _events(a)
	{
	}

	~PhysicsWorldImpl()
	{
	}

	ColliderInstance collider_create(UnitId /*unit*/, const ColliderDesc* /*sd*/, const Vector3& /*scl*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	void collider_destroy(ColliderInstance /*collider*/)
	{
	}

	ColliderInstance collider_first(UnitId /*unit*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	ColliderInstance collider_next(ColliderInstance /*collider*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	ActorInstance actor_create(UnitId /*unit*/, const ActorResource* /*ar*/, const Matrix4x4& /*tm*/)
	{
		return make_actor_instance(UINT32_MAX);
	}

	void actor_destroy(ActorInstance /*actor*/)
	{
	}

	ActorInstance actor(UnitId /*unit*/)
	{
		return make_actor_instance(UINT32_MAX);
	}

	Vector3 actor_world_position(ActorInstance /*actor*/) const
	{
		return VECTOR3_ZERO;
	}

	Quaternion actor_world_rotation(ActorInstance /*actor*/) const
	{
		return QUATERNION_IDENTITY;
	}

	Matrix4x4 actor_world_pose(ActorInstance /*actor*/) const
	{
		return MATRIX4X4_IDENTITY;
	}

	void actor_teleport_world_position(ActorInstance /*actor*/, const Vector3& /*p*/)
	{
	}

	void actor_teleport_world_rotation(ActorInstance /*actor*/, const Quaternion& /*r*/)
	{
	}

	void actor_teleport_world_pose(ActorInstance /*actor*/, const Matrix4x4& /*m*/)
	{
	}

	Vector3 actor_center_of_mass(ActorInstance /*actor*/) const
	{
		return VECTOR3_ZERO;
	}

	void actor_enable_gravity(ActorInstance /*actor*/)
	{
	}

	void actor_disable_gravity(ActorInstance /*actor*/)
	{
	}

	void actor_enable_collision(ActorInstance /*actor*/)
	{
	}

	void actor_disable_collision(ActorInstance /*actor*/)
	{
	}

	void actor_set_collision_filter(ActorInstance /*actor*/, StringId32 /*filter*/)
	{
	}

	void actor_set_kinematic(ActorInstance /*actor*/, bool /*kinematic*/)
	{
	}

	bool actor_is_static(ActorInstance /*actor*/) const
	{
		return false;
	}

	bool actor_is_dynamic(ActorInstance /*actor*/) const
	{
		return false;
	}

	bool actor_is_kinematic(ActorInstance /*actor*/) const
	{
		return false;
	}

	bool actor_is_nonkinematic(ActorInstance /*actor*/) const
	{
		return false;
	}

	f32 actor_linear_damping(ActorInstance /*actor*/) const
	{
		return 0.0f;
	}

	void actor_set_linear_damping(ActorInstance /*actor*/, f32 /*rate*/)
	{
	}

	f32 actor_angular_damping(ActorInstance /*actor*/) const
	{
		return 0.0f;
	}

	void actor_set_angular_damping(ActorInstance /*actor*/, f32 /*rate*/)
	{
	}

	Vector3 actor_linear_velocity(ActorInstance /*actor*/) const
	{
		return VECTOR3_ZERO;
	}

	void actor_set_linear_velocity(ActorInstance /*actor*/, const Vector3& /*vel*/)
	{
	}

	Vector3 actor_angular_velocity(ActorInstance /*actor*/) const
	{
		return VECTOR3_ZERO;
	}

	void actor_set_angular_velocity(ActorInstance /*actor*/, const Vector3& /*vel*/)
	{
	}

	void actor_add_impulse(ActorInstance /*actor*/, const Vector3& /*impulse*/)
	{
	}

	void actor_add_impulse_at(ActorInstance /*actor*/, const Vector3& /*impulse*/, const Vector3& /*pos*/)
	{
	}

	void actor_add_torque_impulse(ActorInstance /*actor*/, const Vector3& /*imp*/)
	{
	}

	void actor_push(ActorInstance /*actor*/, const Vector3& /*vel*/, f32 /*mass*/)
	{
	}

	void actor_push_at(ActorInstance /*actor*/, const Vector3& /*vel*/, f32 /*mass*/, const Vector3& /*pos*/)
	{
	}

	bool actor_is_sleeping(ActorInstance /*actor*/)
	{
		return false;
	}

	void actor_wake_up(ActorInstance /*actor*/)
	{
	}

	JointInstance joint_create(ActorInstance /*a0*/, ActorInstance /*a1*/, const JointDesc& /*jd*/)
	{
		return make_joint_instance(UINT32_MAX);
	}

	void joint_destroy(JointInstance /*joint*/)
	{
	}

	bool cast_ray(RaycastHit& /*hit*/, const Vector3& /*from*/, const Vector3& /*dir*/, f32 /*len*/)
	{
		return false;
	}

	bool cast_ray_all(Array<RaycastHit>& /*hits*/, const Vector3& /*from*/, const Vector3& /*dir*/, f32 /*len*/)
	{
		return false;
	}

	bool cast_sphere(RaycastHit& /*hit*/, const Vector3& /*from*/, f32 /*radius*/, const Vector3& /*dir*/, f32 /*len*/)
	{
		return false;
	}

	bool cast_box(RaycastHit& /*hit*/, const Vector3& /*from*/, const Vector3& /*half_extents*/, const Vector3& /*dir*/, f32 /*len*/)
	{
		return false;
	}

	Vector3 gravity() const
	{
		return VECTOR3_ZERO;
	}

	void set_gravity(const Vector3& /*g*/)
	{
	}

	void update_actor_world_poses(const UnitId* /*begin*/, const UnitId* /*end*/, const Matrix4x4* /*begin_world*/)
	{
	}

	void update(f32 /*dt*/)
	{
	}

	EventStream& events()
	{
		return _events;
	}

	void debug_draw()
	{
	}

	void enable_debug_drawing(bool /*enable*/)
	{
	}

	ColliderInstance make_collider_instance(u32 i) { ColliderInstance inst = { i }; return inst; }
	ActorInstance make_actor_instance(u32 i) { ActorInstance inst = { i }; return inst; }
	JointInstance make_joint_instance(u32 i) { JointInstance inst = { i }; return inst; }
};

PhysicsWorld::PhysicsWorld(Allocator& a, ResourceManager& /*rm*/, UnitManager& /*um*/, DebugLine& /*dl*/)
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

ColliderInstance PhysicsWorld::collider_create(UnitId unit, const ColliderDesc* sd, const Vector3& scl)
{
	return _impl->collider_create(unit, sd, scl);
}

void PhysicsWorld::collider_destroy(ColliderInstance i)
{
	_impl->collider_destroy(i);
}

ColliderInstance PhysicsWorld::collider_first(UnitId unit)
{
	return _impl->collider_first(unit);
}

ColliderInstance PhysicsWorld::collider_next(ColliderInstance i)
{
	return _impl->collider_next(i);
}

ActorInstance PhysicsWorld::actor_create(UnitId unit, const ActorResource* ar, const Matrix4x4& tm)
{
	return _impl->actor_create(unit, ar, tm);
}

void PhysicsWorld::actor_destroy(ActorInstance actor)
{
	_impl->actor_destroy(actor);
}

ActorInstance PhysicsWorld::actor(UnitId unit)
{
	return _impl->actor(unit);
}

Vector3 PhysicsWorld::actor_world_position(ActorInstance actor) const
{
	return _impl->actor_world_position(actor);
}

Quaternion PhysicsWorld::actor_world_rotation(ActorInstance actor) const
{
	return _impl->actor_world_rotation(actor);
}

Matrix4x4 PhysicsWorld::actor_world_pose(ActorInstance actor) const
{
	return _impl->actor_world_pose(actor);
}

void PhysicsWorld::actor_teleport_world_position(ActorInstance actor, const Vector3& p)
{
	_impl->actor_teleport_world_position(actor, p);
}

void PhysicsWorld::actor_teleport_world_rotation(ActorInstance actor, const Quaternion& r)
{
	_impl->actor_teleport_world_rotation(actor, r);
}

void PhysicsWorld::actor_teleport_world_pose(ActorInstance actor, const Matrix4x4& m)
{
	_impl->actor_teleport_world_pose(actor, m);
}

Vector3 PhysicsWorld::actor_center_of_mass(ActorInstance actor) const
{
	return _impl->actor_center_of_mass(actor);
}

void PhysicsWorld::actor_enable_gravity(ActorInstance actor)
{
	_impl->actor_enable_gravity(actor);
}

void PhysicsWorld::actor_disable_gravity(ActorInstance actor)
{
	_impl->actor_disable_gravity(actor);
}

void PhysicsWorld::actor_enable_collision(ActorInstance actor)
{
	_impl->actor_enable_collision(actor);
}

void PhysicsWorld::actor_disable_collision(ActorInstance actor)
{
	_impl->actor_disable_collision(actor);
}

void PhysicsWorld::actor_set_collision_filter(ActorInstance actor, StringId32 filter)
{
	_impl->actor_set_collision_filter(actor, filter);
}

void PhysicsWorld::actor_set_kinematic(ActorInstance actor, bool kinematic)
{
	_impl->actor_set_kinematic(actor, kinematic);
}

bool PhysicsWorld::actor_is_static(ActorInstance actor) const
{
	return _impl->actor_is_static(actor);
}

bool PhysicsWorld::actor_is_dynamic(ActorInstance actor) const
{
	return _impl->actor_is_dynamic(actor);
}

bool PhysicsWorld::actor_is_kinematic(ActorInstance actor) const
{
	return _impl->actor_is_kinematic(actor);
}

bool PhysicsWorld::actor_is_nonkinematic(ActorInstance actor) const
{
	return _impl->actor_is_nonkinematic(actor);
}

f32 PhysicsWorld::actor_linear_damping(ActorInstance actor) const
{
	return _impl->actor_linear_damping(actor);
}

void PhysicsWorld::actor_set_linear_damping(ActorInstance actor, f32 rate)
{
	_impl->actor_set_linear_damping(actor, rate);
}

f32 PhysicsWorld::actor_angular_damping(ActorInstance actor) const
{
	return _impl->actor_angular_damping(actor);
}

void PhysicsWorld::actor_set_angular_damping(ActorInstance actor, f32 rate)
{
	_impl->actor_set_angular_damping(actor, rate);
}

Vector3 PhysicsWorld::actor_linear_velocity(ActorInstance actor) const
{
	return _impl->actor_linear_velocity(actor);
}

void PhysicsWorld::actor_set_linear_velocity(ActorInstance actor, const Vector3& vel)
{
	_impl->actor_set_linear_velocity(actor, vel);
}

Vector3 PhysicsWorld::actor_angular_velocity(ActorInstance actor) const
{
	return _impl->actor_angular_velocity(actor);
}

void PhysicsWorld::actor_set_angular_velocity(ActorInstance actor, const Vector3& vel)
{
	_impl->actor_set_angular_velocity(actor, vel);
}

void PhysicsWorld::actor_add_impulse(ActorInstance actor, const Vector3& impulse)
{
	_impl->actor_add_impulse(actor, impulse);
}

void PhysicsWorld::actor_add_impulse_at(ActorInstance actor, const Vector3& impulse, const Vector3& pos)
{
	_impl->actor_add_impulse_at(actor, impulse, pos);
}

void PhysicsWorld::actor_add_torque_impulse(ActorInstance actor, const Vector3& imp)
{
	_impl->actor_add_torque_impulse(actor, imp);
}

void PhysicsWorld::actor_push(ActorInstance actor, const Vector3& vel, f32 mass)
{
	_impl->actor_push(actor, vel, mass);
}

void PhysicsWorld::actor_push_at(ActorInstance actor, const Vector3& vel, f32 mass, const Vector3& pos)
{
	_impl->actor_push_at(actor, vel, mass, pos);
}

bool PhysicsWorld::actor_is_sleeping(ActorInstance actor)
{
	return _impl->actor_is_sleeping(actor);
}

void PhysicsWorld::actor_wake_up(ActorInstance actor)
{
	_impl->actor_wake_up(actor);
}

JointInstance PhysicsWorld::joint_create(ActorInstance a0, ActorInstance a1, const JointDesc& jd)
{
	return _impl->joint_create(a0, a1, jd);
}

void PhysicsWorld::joint_destroy(JointInstance i)
{
	_impl->joint_destroy(i);
}

bool PhysicsWorld::cast_ray(RaycastHit& hit, const Vector3& from, const Vector3& dir, f32 len)
{
	return _impl->cast_ray(hit, from, dir, len);
}

bool PhysicsWorld::cast_ray_all(Array<RaycastHit>& hits, const Vector3& from, const Vector3& dir, f32 len)
{
	return _impl->cast_ray_all(hits, from, dir, len);
}

bool PhysicsWorld::cast_sphere(RaycastHit& hit, const Vector3& from, f32 radius, const Vector3& dir, f32 len)
{
	return _impl->cast_sphere(hit, from, radius, dir, len);
}

bool PhysicsWorld::cast_box(RaycastHit& hit, const Vector3& from, const Vector3& half_extents, const Vector3& dir, f32 len)
{
	return _impl->cast_box(hit, from, half_extents, dir, len);
}

Vector3 PhysicsWorld::gravity() const
{
	return _impl->gravity();
}

void PhysicsWorld::set_gravity(const Vector3& g)
{
	_impl->set_gravity(g);
}

void PhysicsWorld::update_actor_world_poses(const UnitId* begin, const UnitId* end, const Matrix4x4* begin_world)
{
	_impl->update_actor_world_poses(begin, end, begin_world);
}

void PhysicsWorld::update(f32 dt)
{
	_impl->update(dt);
}

EventStream& PhysicsWorld::events()
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

#endif // CROWN_PHYSICS_NOOP
