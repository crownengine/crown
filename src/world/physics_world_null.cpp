/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PHYSICS_NULL

#include "physics_world.h"

namespace crown
{
namespace physics_globals
{
	void init()
	{
	}

	void shutdown()
	{
	}
} // namespace physics_globals

class PhysicsWorldNull : public PhysicsWorld
{
public:

	PhysicsWorldNull(Allocator& a)
		: _events(a)
	{
	}

	virtual ~PhysicsWorldNull()
	{
	}

	virtual ColliderInstance create_collider(UnitId /*id*/, const ColliderDesc* /*sd*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	virtual void collider_destroy(ColliderInstance /*i*/)
	{
	}

	virtual ColliderInstance first_collider(UnitId /*id*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	virtual ColliderInstance next_collider(ColliderInstance /*i*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	virtual ActorInstance actor_create(UnitId /*id*/, const ActorResource* /*ar*/, const Matrix4x4& /*tm*/)
	{
		return make_actor_instance(UINT32_MAX);
	}

	virtual void actor_destroy(ActorInstance /*i*/)
	{
	}

	virtual ActorInstance actor(UnitId /*id*/)
	{
		return make_actor_instance(UINT32_MAX);
	}

	virtual Vector3 actor_world_position(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual Quaternion actor_world_rotation(ActorInstance /*i*/) const
	{
		return QUATERNION_IDENTITY;
	}

	virtual Matrix4x4 actor_world_pose(ActorInstance /*i*/) const
	{
		return MATRIX4X4_IDENTITY;
	}

	virtual void actor_teleport_world_position(ActorInstance /*i*/, const Vector3& /*p*/)
	{
	}

	virtual void actor_teleport_world_rotation(ActorInstance /*i*/, const Quaternion& /*r*/)
	{
	}

	virtual void actor_teleport_world_pose(ActorInstance /*i*/, const Matrix4x4& /*m*/)
	{
	}

	virtual Vector3 actor_center_of_mass(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void actor_enable_gravity(ActorInstance /*i*/)
	{
	}

	virtual void actor_disable_gravity(ActorInstance /*i*/)
	{
	}

	virtual void actor_enable_collision(ActorInstance /*i*/)
	{
	}

	virtual void actor_disable_collision(ActorInstance /*i*/)
	{
	}

	virtual void actor_set_collision_filter(ActorInstance /*i*/, StringId32 /*filter*/)
	{
	}

	virtual void actor_set_kinematic(ActorInstance /*i*/, bool /*kinematic*/)
	{
	}

	virtual void actor_move(ActorInstance /*i*/, const Vector3& /*pos*/)
	{
	}

	virtual bool actor_is_static(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool actor_is_dynamic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool actor_is_kinematic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool actor_is_nonkinematic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual f32 actor_linear_damping(ActorInstance /*i*/) const
	{
		return 0.0f;
	}

	virtual void actor_set_linear_damping(ActorInstance /*i*/, f32 /*rate*/)
	{
	}

	virtual f32 actor_angular_damping(ActorInstance /*i*/) const
	{
		return 0.0f;
	}

	virtual void actor_set_angular_damping(ActorInstance /*i*/, f32 /*rate*/)
	{
	}

	virtual Vector3 actor_linear_velocity(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void actor_set_linear_velocity(ActorInstance /*i*/, const Vector3& /*vel*/)
	{
	}

	virtual Vector3 actor_angular_velocity(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void actor_set_angular_velocity(ActorInstance /*i*/, const Vector3& /*vel*/)
	{
	}

	virtual void actor_add_impulse(ActorInstance /*i*/, const Vector3& /*impulse*/)
	{
	}

	virtual void actor_add_impulse_at(ActorInstance /*i*/, const Vector3& /*impulse*/, const Vector3& /*pos*/)
	{
	}

	virtual void actor_add_actor_torque_impulse(ActorInstance /*i*/, const Vector3& /*imp*/)
	{
	}

	virtual void actor_push(ActorInstance /*i*/, const Vector3& /*vel*/, f32 /*mass*/)
	{
	}

	virtual void actor_push_at(ActorInstance /*i*/, const Vector3& /*vel*/, f32 /*mass*/, const Vector3& /*pos*/)
	{
	}

	virtual bool actor_is_sleeping(ActorInstance /*i*/)
	{
		return false;
	}

	virtual void actor_wake_up(ActorInstance /*i*/)
	{
	}

	virtual ControllerInstance controller_create(UnitId /*id*/, const ControllerDesc& /*cd*/, const Matrix4x4& /*tm*/)
	{
		return make_controller_instance(UINT32_MAX);
	}

	virtual void controller_destroy(ControllerInstance /*id*/)
	{
	}

	virtual ControllerInstance controller(UnitId /*id*/)
	{
		return make_controller_instance(UINT32_MAX);
	}

	virtual Vector3 controller_position(ControllerInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void controller_move(ControllerInstance /*i*/, const Vector3& /*pos*/)
	{
	}

	virtual void controller_set_height(ControllerInstance /*i*/, f32 /*height*/)
	{
	}

	virtual bool controller_collides_up(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual bool controller_collides_down(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual bool controller_collides_sides(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual JointInstance joint_create(ActorInstance /*a0*/, ActorInstance /*a1*/, const JointDesc& /*jd*/)
	{
		return make_joint_instance(UINT32_MAX);
	}

	virtual void joint_destroy(JointInstance /*i*/)
	{
	}

	virtual void raycast(const Vector3& /*from*/, const Vector3& /*dir*/, f32 /*len*/, RaycastMode::Enum /*mode*/, Array<RaycastHit>& /*hits*/)
	{
	}

	virtual Vector3 gravity() const
	{
		return VECTOR3_ZERO;
	}

	virtual void set_gravity(const Vector3& /*g*/)
	{
	}

	virtual void update_actor_world_poses(const UnitId* /*begin*/, const UnitId* /*end*/, const Matrix4x4* /*begin_world*/)
	{
	}

	virtual void update(f32 /*dt*/)
	{
	}

	virtual EventStream& events()
	{
		return _events;
	}

	virtual void debug_draw()
	{
	}

	virtual void enable_debug_drawing(bool /*enable*/)
	{
	}

private:

	ColliderInstance make_collider_instance(u32 i) { ColliderInstance inst = { i }; return inst; }
	ActorInstance make_actor_instance(u32 i) { ActorInstance inst = { i }; return inst; }
	ControllerInstance make_controller_instance(u32 i) { ControllerInstance inst = { i }; return inst; }
	JointInstance make_joint_instance(u32 i) { JointInstance inst = { i }; return inst; }

	EventStream _events;
};

PhysicsWorld* PhysicsWorld::create(Allocator& a, ResourceManager& /*rm*/, UnitManager& /*um*/, DebugLine& /*dl*/)
{
	return CE_NEW(a, PhysicsWorldNull)(a);
}

void PhysicsWorld::destroy(Allocator& a, PhysicsWorld* pw)
{
	CE_DELETE(a, pw);
}

} // namespace crown

#endif // CROWN_PHYSICS_WORLD_NULL
