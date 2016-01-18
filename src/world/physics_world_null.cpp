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

	virtual ColliderInstance create_collider(UnitId /*id*/, const ShapeDesc* /*sd*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	virtual ColliderInstance first_collider(UnitId /*id*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	virtual ColliderInstance next_collider(ColliderInstance /*i*/)
	{
		return make_collider_instance(UINT32_MAX);
	}

	virtual ActorInstance create_actor(UnitId /*id*/, const ActorResource* /*ar*/, const Matrix4x4& /*tm*/)
	{
		return make_actor_instance(UINT32_MAX);
	}

	virtual void destroy_actor(ActorInstance /*i*/)
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

	virtual void teleport_actor_world_position(ActorInstance /*i*/, const Vector3& /*p*/)
	{
	}

	virtual void teleport_actor_world_rotation(ActorInstance /*i*/, const Quaternion& /*r*/)
	{
	}

	virtual void teleport_actor_world_pose(ActorInstance /*i*/, const Matrix4x4& /*m*/)
	{
	}

	virtual Vector3 actor_center_of_mass(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void enable_actor_gravity(ActorInstance /*i*/)
	{
	}

	virtual void disable_actor_gravity(ActorInstance /*i*/)
	{
	}

	virtual void enable_actor_collision(ActorInstance /*i*/)
	{
	}

	virtual void disable_actor_collision(ActorInstance /*i*/)
	{
	}

	virtual void set_actor_collision_filter(ActorInstance /*i*/, StringId32 /*filter*/)
	{
	}

	virtual void set_actor_kinematic(ActorInstance /*i*/, bool /*kinematic*/)
	{
	}

	virtual void move_actor(ActorInstance /*i*/, const Vector3& /*pos*/)
	{
	}

	virtual bool is_static(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool is_dynamic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool is_kinematic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual bool is_nonkinematic(ActorInstance /*i*/) const
	{
		return false;
	}

	virtual float actor_linear_damping(ActorInstance /*i*/) const
	{
		return 0.0f;
	}

	virtual void set_actor_linear_damping(ActorInstance /*i*/, float /*rate*/)
	{
	}

	virtual float actor_angular_damping(ActorInstance /*i*/) const
	{
		return 0.0f;
	}

	virtual void set_actor_angular_damping(ActorInstance /*i*/, float /*rate*/)
	{
	}

	virtual Vector3 actor_linear_velocity(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void set_actor_linear_velocity(ActorInstance /*i*/, const Vector3& /*vel*/)
	{
	}

	virtual Vector3 actor_angular_velocity(ActorInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void set_actor_angular_velocity(ActorInstance /*i*/, const Vector3& /*vel*/)
	{
	}

	virtual void add_actor_impulse(ActorInstance /*i*/, const Vector3& /*impulse*/)
	{
	}

	virtual void add_actor_impulse_at(ActorInstance /*i*/, const Vector3& /*impulse*/, const Vector3& /*pos*/)
	{
	}

	virtual void add_actor_torque_impulse(ActorInstance /*i*/, const Vector3& /*imp*/)
	{
	}

	virtual void push_actor(ActorInstance /*i*/, const Vector3& /*vel*/, float /*mass*/)
	{
	}

	virtual void push_actor_at(ActorInstance /*i*/, const Vector3& /*vel*/, float /*mass*/, const Vector3& /*pos*/)
	{
	}

	virtual bool is_sleeping(ActorInstance /*i*/)
	{
		return false;
	}

	virtual void wake_up(ActorInstance /*i*/)
	{
	}

	virtual ControllerInstance create_controller(UnitId /*id*/, const ControllerDesc& /*cd*/, const Matrix4x4& /*tm*/)
	{
		return make_controller_instance(UINT32_MAX);
	}

	virtual void destroy_controller(ControllerInstance /*id*/)
	{
	}

	virtual ControllerInstance controller(UnitId /*id*/)
	{
		return make_controller_instance(UINT32_MAX);
	}

	virtual Vector3 position(ControllerInstance /*i*/) const
	{
		return VECTOR3_ZERO;
	}

	virtual void move_controller(ControllerInstance /*i*/, const Vector3& /*pos*/)
	{
	}

	virtual void set_height(ControllerInstance /*i*/, float /*height*/)
	{
	}

	virtual bool collides_up(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual bool collides_down(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual bool collides_sides(ControllerInstance /*i*/) const
	{
		return false;
	}

	virtual JointInstance create_joint(ActorInstance /*a0*/, ActorInstance /*a1*/, const JointDesc& /*jd*/)
	{
		return make_joint_instance(UINT32_MAX);
	}

	virtual void destroy_joint(JointInstance /*i*/)
	{
	}

	virtual void raycast(const Vector3& /*from*/, const Vector3& /*dir*/, float /*len*/, RaycastMode::Enum /*mode*/, Array<RaycastHit>& /*hits*/)
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

	virtual void update(float /*dt*/)
	{
	}

	virtual EventStream& events()
	{
		return _events;
	}

	virtual void draw_debug()
	{
	}

	virtual void enable_debug_drawing(bool /*enable*/)
	{
	}

private:

	ColliderInstance make_collider_instance(uint32_t i) { ColliderInstance inst = { i }; return inst; }
	ActorInstance make_actor_instance(uint32_t i) { ActorInstance inst = { i }; return inst; }
	ControllerInstance make_controller_instance(uint32_t i) { ControllerInstance inst = { i }; return inst; }
	JointInstance make_joint_instance(uint32_t i) { JointInstance inst = { i }; return inst; }

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
