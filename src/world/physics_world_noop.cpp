/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PHYSICS_NOOP

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

	PhysicsWorldImpl(Allocator& a)
		: _events(a)
	{
	}

	~PhysicsWorldImpl()
	{
	}

	ColliderInstance collider_create(UnitId /*id*/, const ColliderDesc* /*sd*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	void collider_destroy(ColliderInstance /*i*/)
	{
	}

	ColliderInstance collider_first(UnitId /*id*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	ColliderInstance collider_next(ColliderInstance /*i*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	ActorInstance actor_create(UnitId /*id*/, const ActorResource* /*ar*/, const Matrix4x4& /*tm*/)
	{
		return make_actor_instance(UINT32_MAX);
	}

	void actor_destroy(ActorInstance /*i*/)
	{
	}

	ActorInstance actor(UnitId /*id*/)
	{
		return make_actor_instance(UINT32_MAX);
	}

	Vector3 actor_world_position(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	Quaternion actor_world_rotation(ActorInstance /*i*/) const
	{
		return QUATERNION_IDENTITY;
	}

	Matrix4x4 actor_world_pose(ActorInstance /*i*/) const
	{
		return MATRIX4X4_IDENTITY;
	}

	void actor_teleport_world_position(ActorInstance /*i*/, const Vector3& /*p*/)
	{
	}

	void actor_teleport_world_rotation(ActorInstance /*i*/, const Quaternion& /*r*/)
	{
	}

	void actor_teleport_world_pose(ActorInstance /*i*/, const Matrix4x4& /*m*/)
	{
	}

	Vector3 actor_center_of_mass(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	void actor_enable_gravity(ActorInstance /*i*/)
	{
	}

	void actor_disable_gravity(ActorInstance /*i*/)
	{
	}

	void actor_enable_collision(ActorInstance /*i*/)
	{
	}

	void actor_disable_collision(ActorInstance /*i*/)
	{
	}

	void actor_set_collision_filter(ActorInstance /*i*/, StringId32 /*filter*/)
	{
	}

	void actor_set_kinematic(ActorInstance /*i*/, bool /*kinematic*/)
	{
	}

	bool actor_is_static(ActorInstance /*i*/) const
	{
		return false;
	}

	bool actor_is_dynamic(ActorInstance /*i*/) const
	{
		return false;
	}

	bool actor_is_kinematic(ActorInstance /*i*/) const
	{
		return false;
	}

	bool actor_is_nonkinematic(ActorInstance /*i*/) const
	{
		return false;
	}

	f32 actor_linear_damping(ActorInstance /*i*/) const
	{
		return 0.0f;
	}

	void actor_set_linear_damping(ActorInstance /*i*/, f32 /*rate*/)
	{
	}

	f32 actor_angular_damping(ActorInstance /*i*/) const
	{
		return 0.0f;
	}

	void actor_set_angular_damping(ActorInstance /*i*/, f32 /*rate*/)
	{
	}

	Vector3 actor_linear_velocity(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	void actor_set_linear_velocity(ActorInstance /*i*/, const Vector3& /*vel*/)
	{
	}

	Vector3 actor_angular_velocity(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	void actor_set_angular_velocity(ActorInstance /*i*/, const Vector3& /*vel*/)
	{
	}

	void actor_add_impulse(ActorInstance /*i*/, const Vector3& /*impulse*/)
	{
	}

	void actor_add_impulse_at(ActorInstance /*i*/, const Vector3& /*impulse*/, const Vector3& /*pos*/)
	{
	}

	void actor_add_torque_impulse(ActorInstance /*i*/, const Vector3& /*imp*/)
	{
	}

	void actor_push(ActorInstance /*i*/, const Vector3& /*vel*/, f32 /*mass*/)
	{
	}

	void actor_push_at(ActorInstance /*i*/, const Vector3& /*vel*/, f32 /*mass*/, const Vector3& /*pos*/)
	{
	}

	bool actor_is_sleeping(ActorInstance /*i*/)
	{
		return false;
	}

	void actor_wake_up(ActorInstance /*i*/)
	{
	}

	JointInstance joint_create(ActorInstance /*a0*/, ActorInstance /*a1*/, const JointDesc& /*jd*/)
	{
		return make_joint_instance(UINT32_MAX);
	}

	void joint_destroy(JointInstance /*i*/)
	{
	}

	void raycast(const Vector3& /*from*/, const Vector3& /*dir*/, f32 /*len*/, RaycastMode::Enum /*mode*/, Array<RaycastHit>& /*hits*/)
	{
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

ColliderInstance PhysicsWorld::collider_create(UnitId id, const ColliderDesc* sd)
{
	return _impl->collider_create(id, sd);
}

void PhysicsWorld::collider_destroy(ColliderInstance i)
{
	_impl->collider_destroy(i);
}

ColliderInstance PhysicsWorld::collider_first(UnitId id)
{
	return _impl->collider_first(id);
}

ColliderInstance PhysicsWorld::collider_next(ColliderInstance i)
{
	return _impl->collider_next(i);
}

ActorInstance PhysicsWorld::actor_create(UnitId id, const ActorResource* ar, const Matrix4x4& tm)
{
	return _impl->actor_create(id, ar, tm);
}

void PhysicsWorld::actor_destroy(ActorInstance i)
{
	_impl->actor_destroy(i);
}

ActorInstance PhysicsWorld::actor(UnitId id)
{
	return _impl->actor(id);
}

Vector3 PhysicsWorld::actor_world_position(ActorInstance i) const
{
	return _impl->actor_world_position(i);
}

Quaternion PhysicsWorld::actor_world_rotation(ActorInstance i) const
{
	return _impl->actor_world_rotation(i);
}

Matrix4x4 PhysicsWorld::actor_world_pose(ActorInstance i) const
{
	return _impl->actor_world_pose(i);
}

void PhysicsWorld::actor_teleport_world_position(ActorInstance i, const Vector3& p)
{
	_impl->actor_teleport_world_position(i, p);
}

void PhysicsWorld::actor_teleport_world_rotation(ActorInstance i, const Quaternion& r)
{
	_impl->actor_teleport_world_rotation(i, r);
}

void PhysicsWorld::actor_teleport_world_pose(ActorInstance i, const Matrix4x4& m)
{
	_impl->actor_teleport_world_pose(i, m);
}

Vector3 PhysicsWorld::actor_center_of_mass(ActorInstance i) const
{
	return _impl->actor_center_of_mass(i);
}

void PhysicsWorld::actor_enable_gravity(ActorInstance i)
{
	_impl->actor_enable_gravity(i);
}

void PhysicsWorld::actor_disable_gravity(ActorInstance i)
{
	_impl->actor_disable_gravity(i);
}

void PhysicsWorld::actor_enable_collision(ActorInstance i)
{
	_impl->actor_enable_collision(i);
}

void PhysicsWorld::actor_disable_collision(ActorInstance i)
{
	_impl->actor_disable_collision(i);
}

void PhysicsWorld::actor_set_collision_filter(ActorInstance i, StringId32 filter)
{
	_impl->actor_set_collision_filter(i, filter);
}

void PhysicsWorld::actor_set_kinematic(ActorInstance i, bool kinematic)
{
	_impl->actor_set_kinematic(i, kinematic);
}

bool PhysicsWorld::actor_is_static(ActorInstance i) const
{
	return _impl->actor_is_static(i);
}

bool PhysicsWorld::actor_is_dynamic(ActorInstance i) const
{
	return _impl->actor_is_dynamic(i);
}

bool PhysicsWorld::actor_is_kinematic(ActorInstance i) const
{
	return _impl->actor_is_kinematic(i);
}

bool PhysicsWorld::actor_is_nonkinematic(ActorInstance i) const
{
	return _impl->actor_is_nonkinematic(i);
}

f32 PhysicsWorld::actor_linear_damping(ActorInstance i) const
{
	return _impl->actor_linear_damping(i);
}

void PhysicsWorld::actor_set_linear_damping(ActorInstance i, f32 rate)
{
	_impl->actor_set_linear_damping(i, rate);
}

f32 PhysicsWorld::actor_angular_damping(ActorInstance i) const
{
	return _impl->actor_angular_damping(i);
}

void PhysicsWorld::actor_set_angular_damping(ActorInstance i, f32 rate)
{
	_impl->actor_set_angular_damping(i, rate);
}

Vector3 PhysicsWorld::actor_linear_velocity(ActorInstance i) const
{
	return _impl->actor_linear_velocity(i);
}

void PhysicsWorld::actor_set_linear_velocity(ActorInstance i, const Vector3& vel)
{
	_impl->actor_set_linear_velocity(i, vel);
}

Vector3 PhysicsWorld::actor_angular_velocity(ActorInstance i) const
{
	return _impl->actor_angular_velocity(i);
}

void PhysicsWorld::actor_set_angular_velocity(ActorInstance i, const Vector3& vel)
{
	_impl->actor_set_angular_velocity(i, vel);
}

void PhysicsWorld::actor_add_impulse(ActorInstance i, const Vector3& impulse)
{
	_impl->actor_add_impulse(i, impulse);
}

void PhysicsWorld::actor_add_impulse_at(ActorInstance i, const Vector3& impulse, const Vector3& pos)
{
	_impl->actor_add_impulse_at(i, impulse, pos);
}

void PhysicsWorld::actor_add_torque_impulse(ActorInstance i, const Vector3& imp)
{
	_impl->actor_add_torque_impulse(i, imp);
}

void PhysicsWorld::actor_push(ActorInstance i, const Vector3& vel, f32 mass)
{
	_impl->actor_push(i, vel, mass);
}

void PhysicsWorld::actor_push_at(ActorInstance i, const Vector3& vel, f32 mass, const Vector3& pos)
{
	_impl->actor_push_at(i, vel, mass, pos);
}

bool PhysicsWorld::actor_is_sleeping(ActorInstance i)
{
	return _impl->actor_is_sleeping(i);
}

void PhysicsWorld::actor_wake_up(ActorInstance i)
{
	_impl->actor_wake_up(i);
}

JointInstance PhysicsWorld::joint_create(ActorInstance a0, ActorInstance a1, const JointDesc& jd)
{
	return _impl->joint_create(a0, a1, jd);
}

void PhysicsWorld::joint_destroy(JointInstance i)
{
	_impl->joint_destroy(i);
}

void PhysicsWorld::raycast(const Vector3& from, const Vector3& dir, f32 len, RaycastMode::Enum mode, Array<RaycastHit>& hits)
{
	_impl->raycast(from, dir, len, mode, hits);
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
