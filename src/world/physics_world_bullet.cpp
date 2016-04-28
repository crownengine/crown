/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PHYSICS_BULLET

#include "array.h"
#include "color4.h"
#include "debug_line.h"
#include "hash.h"
#include "log.h"
#include "matrix4x4.h"
#include "physics.h"
#include "physics_resource.h"
#include "physics_world.h"
#include "proxy_allocator.h"
#include "quaternion.h"
#include "resource_manager.h"
#include "unit_manager.h"
#include "vector3.h"
#include <btBoxShape.h>
#include <btBvhTriangleMeshShape.h>
#include <btCapsuleShape.h>
#include <btCollisionObject.h>
#include <btCompoundShape.h>
#include <btConvexHullShape.h>
#include <btConvexTriangleMeshShape.h>
#include <btDbvtBroadphase.h>
#include <btDefaultCollisionConfiguration.h>
#include <btDefaultMotionState.h>
#include <btDiscreteDynamicsWorld.h>
#include <btFixedConstraint.h>
#include <btGhostObject.h>
#include <btHeightfieldTerrainShape.h>
#include <btHingeConstraint.h>
#include <btIDebugDraw.h>
#include <btKinematicCharacterController.h>
#include <btPoint2PointConstraint.h>
#include <btRigidBody.h>
#include <btSequentialImpulseConstraintSolver.h>
#include <btSliderConstraint.h>
#include <btSphereShape.h>
#include <btStaticPlaneShape.h>
#include <btTriangleMesh.h>

namespace crown
{
namespace physics_globals
{
	static btDefaultCollisionConfiguration* _bt_configuration;
	static btCollisionDispatcher* _bt_dispatcher;
	static btBroadphaseInterface* _bt_interface;
	static btSequentialImpulseConstraintSolver* _bt_solver;

	void init(Allocator& a)
	{
		_bt_configuration = CE_NEW(a, btDefaultCollisionConfiguration);
		_bt_dispatcher    = CE_NEW(a, btCollisionDispatcher)(_bt_configuration);
		_bt_interface     = CE_NEW(a, btDbvtBroadphase);
		_bt_solver        = CE_NEW(a, btSequentialImpulseConstraintSolver);
	}

	void shutdown(Allocator& a)
	{
		CE_DELETE(a, _bt_solver);
		CE_DELETE(a, _bt_interface);
		CE_DELETE(a, _bt_dispatcher);
		CE_DELETE(a, _bt_configuration);
	}
} // namespace physics_globals

static btVector3 to_btVector3(const Vector3& v)
{
	return btVector3(v.x, v.y, v.z);
}

static btQuaternion to_btQuaternion(const Quaternion& q)
{
	return btQuaternion(q.x, q.y, q.z, q.w);
}

static btTransform to_btTransform(const Matrix4x4& m)
{
	btMatrix3x3 basis(m.x.x, m.y.x, m.z.x
		, m.x.y, m.y.y, m.z.y
		, m.x.z, m.y.z, m.z.z
		);
	btVector3 pos(m.t.x, m.t.y, m.t.z);
	return btTransform(basis, pos);
}

static Vector3 to_vector3(const btVector3& v)
{
	return vector3(v.x(), v.y(), v.z());
}

static Quaternion to_quaternion(const btQuaternion& q)
{
	return quaternion(q.x(), q.y(), q.z(), q.w());
}

class MyDebugDrawer : public btIDebugDraw
{
public:

	MyDebugDrawer(DebugLine& dl)
		: _lines(&dl)
	{
	}

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& /*color*/)
	{
		const Vector3 start = to_vector3(from);
		const Vector3 end = to_vector3(to);
		_lines->add_line(start, end, COLOR4_ORANGE);
	}

	void drawContactPoint(const btVector3& pointOnB, const btVector3& /*normalOnB*/, btScalar /*distance*/, int /*lifeTime*/, const btVector3& /*color*/)
	{
		const Vector3 from = to_vector3(pointOnB);
		_lines->add_sphere(from, 0.1f, COLOR4_WHITE);
	}

	void reportErrorWarning(const char* warningString)
	{
		CE_LOGW(warningString);
	}

	void draw3dText(const btVector3& /*location*/, const char* /*textString*/)
	{
	}

	void setDebugMode(int /*debugMode*/)
	{
	}

	int getDebugMode() const
	{
		return DBG_DrawWireframe
			| DBG_DrawConstraints
			| DBG_DrawConstraintLimits
			| DBG_FastWireframe
			;
	}

public:

	DebugLine* _lines;
};

class MyFilterCallback : public btOverlapFilterCallback
{
	bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const
	{
		bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
		collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

		return collides;
	}
};

class BulletWorld : public PhysicsWorld
{
public:

	BulletWorld(Allocator& a, ResourceManager& rm, UnitManager& um, DebugLine& dl)
		: _allocator(&a)
		, _unit_manager(&um)
		, _collider_map(a)
		, _actor_map(a)
		, _controller_map(a)
		, _collider(a)
		, _actor(a)
		, _controller(a)
		, _joints(a)
		, _scene(NULL)
		, _debug_drawer(dl)
		, _events(a)
		, _debug_drawing(false)
	{
		_scene = CE_NEW(*_allocator, btDiscreteDynamicsWorld)(physics_globals::_bt_dispatcher
			, physics_globals::_bt_interface
			, physics_globals::_bt_solver
			, physics_globals::_bt_configuration
			);

		_scene->getCollisionWorld()->setDebugDrawer(&_debug_drawer);
		_scene->setInternalTickCallback(tick_cb, this);
		_scene->getPairCache()->setOverlapFilterCallback(&_filter_cb);

		_config_resource = (const PhysicsConfigResource*)rm.get(RESOURCE_TYPE_PHYSICS_CONFIG, StringId64("global"));

		um.register_destroy_function(BulletWorld::unit_destroyed_callback, this);
	}

	~BulletWorld()
	{
		_unit_manager->unregister_destroy_function(this);

		for (u32 i = 0; i < array::size(_actor); ++i)
		{
			btRigidBody* rb = _actor[i].actor;

			_scene->removeRigidBody(rb);
			CE_DELETE(*_allocator, rb->getMotionState());
			CE_DELETE(*_allocator, rb->getCollisionShape());
			CE_DELETE(*_allocator, rb);
		}

		for (u32 i = 0; i < array::size(_collider); ++i)
		{
			CE_DELETE(*_allocator, _collider[i].vertex_array);
			CE_DELETE(*_allocator, _collider[i].shape);
		}

		CE_DELETE(*_allocator, _scene);
	}

	ColliderInstance create_collider(UnitId id, const ColliderDesc* sd)
	{
		btTriangleIndexVertexArray* vertex_array = NULL;
		btCollisionShape* child_shape = NULL;

		switch(sd->type)
		{
			case ColliderType::SPHERE:
				child_shape = CE_NEW(*_allocator, btSphereShape)(sd->sphere.radius);
				break;
			case ColliderType::CAPSULE:
				child_shape = CE_NEW(*_allocator, btCapsuleShape)(sd->capsule.radius, sd->capsule.height);
				break;
			case ColliderType::BOX:
				child_shape = CE_NEW(*_allocator, btBoxShape)(to_btVector3(sd->box.half_size));
				break;
			case ColliderType::CONVEX_HULL:
			{
				const char* data       = (char*)&sd[1];
				const u32 num          = *(u32*)data;
				const btScalar* points = (btScalar*)(data + sizeof(u32));

				child_shape = CE_NEW(*_allocator, btConvexHullShape)(points, (int)num, sizeof(Vector3));
				break;
			}
			case ColliderType::MESH:
			{
				const char* data      = (char*)&sd[1];
				const u32 num_points  = *(u32*)data;
				const char* points    = data + sizeof(u32);
				const u32 num_indices = *(u32*)(points + num_points*sizeof(Vector3));
				const char* indices   = points + sizeof(u32) + num_points*sizeof(Vector3);

				btIndexedMesh part;
				part.m_vertexBase          = (const unsigned char*)points;
				part.m_vertexStride        = sizeof(Vector3);
				part.m_numVertices         = num_points;
				part.m_triangleIndexBase   = (const unsigned char*)indices;
				part.m_triangleIndexStride = sizeof(u16)*3;
				part.m_numTriangles        = num_indices/3;
				part.m_indexType           = PHY_SHORT;

				vertex_array = CE_NEW(*_allocator, btTriangleIndexVertexArray)();
				vertex_array->addIndexedMesh(part, PHY_SHORT);

				const btVector3 aabb_min(-1000.0f,-1000.0f,-1000.0f);
				const btVector3 aabb_max(1000.0f,1000.0f,1000.0f);
				child_shape = CE_NEW(*_allocator, btBvhTriangleMeshShape)(vertex_array, false, aabb_min, aabb_max);
				break;
			}
			case ColliderType::HEIGHTFIELD:
			{
				CE_FATAL("Not implemented yet");
				break;
			}
			default:
			{
				CE_FATAL("Bad shape");
				break;
			}
		}

		const u32 last = array::size(_collider);

		ColliderInstanceData cid;
		cid.unit         = id;
		cid.local_tm     = sd->local_tm;
		cid.vertex_array = vertex_array;
		cid.shape        = child_shape;
		cid.next.i       = UINT32_MAX;

		ColliderInstance ci = first_collider(id);
		while (is_valid(ci) && is_valid(next_collider(ci)))
			ci = next_collider(ci);

		if (is_valid(ci))
			_collider[ci.i].next.i = last;
		else
			hash::set(_collider_map, id.encode(), last);

		array::push_back(_collider, cid);
		return make_collider_instance(last);
	}

	ColliderInstance first_collider(UnitId id)
	{
		return make_collider_instance(hash::get(_collider_map, id.encode(), UINT32_MAX));
	}

	ColliderInstance next_collider(ColliderInstance i)
	{
		return _collider[i.i].next;
	}

	ActorInstance create_actor(UnitId id, const ActorResource* ar, const Matrix4x4& tm)
	{
		const PhysicsConfigActor* actor_class = physics_config_resource::actor(_config_resource, ar->actor_class);

		const bool is_kinematic = (actor_class->flags & PhysicsConfigActor::KINEMATIC) != 0;
		const bool is_dynamic   = (actor_class->flags & PhysicsConfigActor::DYNAMIC) != 0;
		const bool is_static    = !is_kinematic && !is_dynamic;
		const f32  mass         = is_dynamic ? ar->mass : 0.0f;

		// Create compound shape
		btCompoundShape* shape = CE_NEW(*_allocator, btCompoundShape)(true);

		ColliderInstance ci = first_collider(id);
		while (is_valid(ci))
		{
			shape->addChildShape(btTransform::getIdentity(), _collider[ci.i].shape);
			ci = next_collider(ci);
		}

		// Create motion state
		btDefaultMotionState* ms = CE_NEW(*_allocator, btDefaultMotionState)(to_btTransform(tm));

		// If dynamic, calculate inertia
		btVector3 inertia;
		if (mass != 0.0f) // Actor is dynamic iff mass != 0
			shape->calculateLocalInertia(mass, inertia);

		btRigidBody::btRigidBodyConstructionInfo rbinfo(mass, ms, shape, inertia);
		rbinfo.m_linearDamping = actor_class->linear_damping;
		rbinfo.m_angularDamping = actor_class->angular_damping;
		rbinfo.m_restitution = 0.81f; // FIXME
		rbinfo.m_friction = 0.8f; // FIXME
		rbinfo.m_rollingFriction = 0.5f; // FIXME
		rbinfo.m_linearSleepingThreshold = 0.01f; // FIXME
		rbinfo.m_angularSleepingThreshold = 0.01f; // FIXME

		// Create rigid body
		btRigidBody* actor = CE_NEW(*_allocator, btRigidBody)(rbinfo);

		int cflags = actor->getCollisionFlags();
		cflags |= is_kinematic ? btCollisionObject::CF_KINEMATIC_OBJECT : 0;
		cflags |= is_static ? btCollisionObject::CF_STATIC_OBJECT : 0;
		actor->setCollisionFlags(cflags);

		actor->setLinearFactor(btVector3(
			(ar->flags & ActorFlags::LOCK_TRANSLATION_X) ? 0.0f : 1.0f,
			(ar->flags & ActorFlags::LOCK_TRANSLATION_Y) ? 0.0f : 1.0f,
			(ar->flags & ActorFlags::LOCK_TRANSLATION_Z) ? 0.0f : 1.0f)
		);
		actor->setAngularFactor(btVector3(
			(ar->flags & ActorFlags::LOCK_ROTATION_X) ? 0.0f : 1.0f,
			(ar->flags & ActorFlags::LOCK_ROTATION_Y) ? 0.0f : 1.0f,
			(ar->flags & ActorFlags::LOCK_ROTATION_Z) ? 0.0f : 1.0f)
		);

		const u32 last = array::size(_actor);

		actor->setUserPointer((void*)(uintptr_t)last);

		// Set collision filters
		const u32 me = physics_config_resource::filter(_config_resource, ar->collision_filter)->me;
		const u32 mask = physics_config_resource::filter(_config_resource, ar->collision_filter)->mask;

		_scene->addRigidBody(actor, me, mask);

		ActorInstanceData aid;
		aid.unit  = id;
		aid.actor = actor;

		array::push_back(_actor, aid);
		hash::set(_actor_map, id.encode(), last);

		return make_actor_instance(last);
	}

	void destroy_actor(ActorInstance i)
	{
		const u32 last      = array::size(_actor) - 1;
		const UnitId u      = _actor[i.i].unit;
		const UnitId last_u = _actor[last].unit;

		_scene->removeRigidBody(_actor[i.i].actor);
		CE_DELETE(*_allocator, _actor[i.i].actor->getMotionState());
		CE_DELETE(*_allocator, _actor[i.i].actor->getCollisionShape());
		CE_DELETE(*_allocator, _actor[i.i].actor);

		_actor[i.i] = _actor[last];
		_actor[i.i].actor->setUserPointer((void*)(uintptr_t)i.i);

		array::pop_back(_actor);

		hash::set(_actor_map, last_u.encode(), i.i);
		hash::remove(_actor_map, u.encode());
	}

	ActorInstance actor(UnitId id)
	{
		return make_actor_instance(hash::get(_actor_map, id.encode(), UINT32_MAX));
	}

	Vector3 actor_world_position(ActorInstance i) const
	{
		btTransform pose;
		_actor[i.i].actor->getMotionState()->getWorldTransform(pose);
		const btVector3 p = pose.getOrigin();
		return to_vector3(p);
	}

	Quaternion actor_world_rotation(ActorInstance i) const
	{
		btTransform pose;
		_actor[i.i].actor->getMotionState()->getWorldTransform(pose);
		return to_quaternion(pose.getRotation());
	}

	Matrix4x4 actor_world_pose(ActorInstance i) const
	{
		btTransform pose;
		_actor[i.i].actor->getMotionState()->getWorldTransform(pose);
		const btQuaternion r = pose.getRotation();
		const btVector3 p = pose.getOrigin();
		return matrix4x4(to_quaternion(r), to_vector3(p));
	}

	void teleport_actor_world_position(ActorInstance i, const Vector3& p)
	{
		btTransform pose = _actor[i.i].actor->getCenterOfMassTransform();
		pose.setOrigin(to_btVector3(p));
		_actor[i.i].actor->setCenterOfMassTransform(pose);
	}

	void teleport_actor_world_rotation(ActorInstance i, const Quaternion& r)
	{
		btTransform pose = _actor[i.i].actor->getCenterOfMassTransform();
		pose.setRotation(to_btQuaternion(r));
		_actor[i.i].actor->setCenterOfMassTransform(pose);
	}

	void teleport_actor_world_pose(ActorInstance i, const Matrix4x4& m)
	{
		const Quaternion rot = rotation(m);
		const Vector3 pos = translation(m);

		btTransform pose = _actor[i.i].actor->getCenterOfMassTransform();
		pose.setRotation(to_btQuaternion(rot));
		pose.setOrigin(to_btVector3(pos));
		_actor[i.i].actor->setCenterOfMassTransform(pose);
	}

	Vector3 actor_center_of_mass(ActorInstance i) const
	{
		if (is_static(i))
			return VECTOR3_ZERO;

		const btVector3 c = _actor[i.i].actor->getCenterOfMassTransform().getOrigin();
		return to_vector3(c);
	}

	void enable_actor_gravity(ActorInstance i)
	{
		_actor[i.i].actor->setGravity(_scene->getGravity());
	}

	void disable_actor_gravity(ActorInstance i)
	{
		_actor[i.i].actor->setGravity(btVector3(0.0f, 0.0f, 0.0f));
	}

	void enable_actor_collision(ActorInstance /*i*/)
	{
		CE_FATAL("Not implemented yet");
	}

	void disable_actor_collision(ActorInstance /*i*/)
	{
		CE_FATAL("Not implemented yet");
	}

	void set_actor_collision_filter(ActorInstance /*i*/, StringId32 /*filter*/)
	{
		CE_FATAL("Not implemented yet");
	}

	void set_actor_kinematic(ActorInstance i, bool kinematic)
	{
		if (kinematic)
			_actor[i.i].actor->setCollisionFlags(_actor[i.i].actor->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	}

	void move_actor(ActorInstance i, const Vector3& pos)
	{
		if (!is_kinematic(i))
			return;

		_actor[i.i].actor->setLinearVelocity(to_btVector3(pos));
	}

	bool is_static(ActorInstance i) const
	{
		return _actor[i.i].actor->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT;
	}

	bool is_dynamic(ActorInstance i) const
	{
		const int flags = _actor[i.i].actor->getCollisionFlags();
		return !(flags & btCollisionObject::CF_STATIC_OBJECT)
			&& !(flags & btCollisionObject::CF_KINEMATIC_OBJECT)
			;
	}

	bool is_kinematic(ActorInstance i) const
	{
		const int flags = _actor[i.i].actor->getCollisionFlags();
		return (flags & btCollisionObject::CF_KINEMATIC_OBJECT) != 0;
	}

	bool is_nonkinematic(ActorInstance i) const
	{
		return is_dynamic(i) && !is_kinematic(i);
	}

	f32 actor_linear_damping(ActorInstance i) const
	{
		return _actor[i.i].actor->getLinearDamping();
	}

	void set_actor_linear_damping(ActorInstance i, f32 rate)
	{
		_actor[i.i].actor->setDamping(rate, _actor[i.i].actor->getAngularDamping());
	}

	f32 actor_angular_damping(ActorInstance i) const
	{
		return _actor[i.i].actor->getAngularDamping();
	}

	void set_actor_angular_damping(ActorInstance i, f32 rate)
	{
		_actor[i.i].actor->setDamping(_actor[i.i].actor->getLinearDamping(), rate);
	}

	Vector3 actor_linear_velocity(ActorInstance i) const
	{
		btVector3 v = _actor[i.i].actor->getLinearVelocity();
		return to_vector3(v);
	}

	void set_actor_linear_velocity(ActorInstance i, const Vector3& vel)
	{
		_actor[i.i].actor->setLinearVelocity(to_btVector3(vel));
	}

	Vector3 actor_angular_velocity(ActorInstance i) const
	{
		btVector3 v = _actor[i.i].actor->getAngularVelocity();
		return to_vector3(v);
	}

	void set_actor_angular_velocity(ActorInstance i, const Vector3& vel)
	{
		_actor[i.i].actor->setAngularVelocity(to_btVector3(vel));
	}

	void add_actor_impulse(ActorInstance i, const Vector3& impulse)
	{
		_actor[i.i].actor->activate();
		_actor[i.i].actor->applyCentralImpulse(to_btVector3(impulse));
	}

	void add_actor_impulse_at(ActorInstance i, const Vector3& impulse, const Vector3& pos)
	{
		_actor[i.i].actor->activate();
		_actor[i.i].actor->applyImpulse(to_btVector3(impulse), to_btVector3(pos));
	}

	void add_actor_torque_impulse(ActorInstance i, const Vector3& imp)
	{
		_actor[i.i].actor->applyTorqueImpulse(to_btVector3(imp));
	}

	void push_actor(ActorInstance i, const Vector3& vel, f32 mass)
	{
		const Vector3 f = vel * mass;
		_actor[i.i].actor->applyCentralForce(to_btVector3(f));
	}

	void push_actor_at(ActorInstance i, const Vector3& vel, f32 mass, const Vector3& pos)
	{
		const Vector3 f = vel * mass;
		_actor[i.i].actor->applyForce(to_btVector3(f), to_btVector3(pos));
	}

	bool is_sleeping(ActorInstance i)
	{
		return !_actor[i.i].actor->isActive();
	}

	void wake_up(ActorInstance i)
	{
		_actor[i.i].actor->activate(true);
	}

	ControllerInstance create_controller(UnitId /*id*/,	const ControllerDesc& /*cd*/, const Matrix4x4& /*tm*/)
	{
		CE_FATAL("Not implemented yet");
		return make_controller_instance(UINT32_MAX);
	}

	void destroy_controller(ControllerInstance /*i*/)
	{
		CE_FATAL("Not implemented yet");
	}

	ControllerInstance controller(UnitId id)
	{
		return make_controller_instance(hash::get(_controller_map, id.encode(), UINT32_MAX));
	}

	void move_controller(ControllerInstance i, const Vector3& dir)
	{
		_controller[i.i].contr->setWalkDirection(to_btVector3(dir));
	}

	void set_height(ControllerInstance /*i*/, f32 /*height*/)
	{
		CE_FATAL("Not implemented yet");
	}

	Vector3 position(ControllerInstance /*i*/) const
	{
		CE_FATAL("Not implemented yet");
		return VECTOR3_ZERO;
	}

	bool collides_up(ControllerInstance /*i*/) const
	{
		CE_FATAL("Not implemented yet");
		return false;
	}

	bool collides_down(ControllerInstance /*i*/) const
	{
		CE_FATAL("Not implemented yet");
		return false;
	}

	bool collides_sides(ControllerInstance /*i*/) const
	{
		CE_FATAL("Not implemented yet");
		return false;
	}

	JointInstance create_joint(ActorInstance a0, ActorInstance a1, const JointDesc& jd)
	{
		const btVector3 anchor_0 = to_btVector3(jd.anchor_0);
		const btVector3 anchor_1 = to_btVector3(jd.anchor_1);
		btRigidBody* actor_0 = _actor[a0.i].actor;
		btRigidBody* actor_1 = is_valid(a1) ? _actor[a1.i].actor : NULL;

		btTypedConstraint* joint = NULL;
		switch(jd.type)
		{
			case JointType::FIXED:
			{
				const btTransform frame_0 = btTransform(btQuaternion::getIdentity(), anchor_0);
				const btTransform frame_1 = btTransform(btQuaternion::getIdentity(), anchor_1);
	 			joint = CE_NEW(*_allocator, btFixedConstraint)(*actor_0
	 				, *actor_1
	 				, frame_0
	 				, frame_1
	 				);
				break;
			}
			case JointType::SPRING:
			{
				joint = CE_NEW(*_allocator, btPoint2PointConstraint)(*actor_0
					, *actor_1
					, anchor_0
					, anchor_1
					);
				break;
			}
			case JointType::HINGE:
			{
				btHingeConstraint* hinge = CE_NEW(*_allocator, btHingeConstraint)(*actor_0
					, *actor_1
					, anchor_0
					, anchor_1
					, to_btVector3(jd.hinge.axis)
					, to_btVector3(jd.hinge.axis)
					);

				hinge->enableAngularMotor(jd.hinge.use_motor
					, jd.hinge.target_velocity
					, jd.hinge.max_motor_impulse
					);

				hinge->setLimit(jd.hinge.lower_limit
					, jd.hinge.upper_limit
					, jd.hinge.bounciness
					);

				joint = hinge;
				break;
			}
			default:
			{
				CE_FATAL("Bad joint type");
				break;
			}
		}

		joint->setBreakingImpulseThreshold(jd.break_force);
		_scene->addConstraint(joint);

		return make_joint_instance(UINT32_MAX);
	}

	void destroy_joint(JointInstance /*i*/)
	{
		CE_FATAL("Not implemented yet");
	}

	void raycast(const Vector3& from, const Vector3& dir, f32 len, RaycastMode::Enum mode, Array<RaycastHit>& hits)
	{
		const btVector3 start = to_btVector3(from);
		const btVector3 end = to_btVector3(from + dir*len);

		switch (mode)
		{
			case RaycastMode::CLOSEST:
			{
				btCollisionWorld::ClosestRayResultCallback cb(start, end);
				_scene->rayTest(start, end, cb);

				if (cb.hasHit())
				{
					RaycastHit hit;
					hit.position = to_vector3(cb.m_hitPointWorld);
					hit.normal = to_vector3(cb.m_hitNormalWorld);
					hit.actor.i = (u32)(uintptr_t)btRigidBody::upcast(cb.m_collisionObject)->getUserPointer();
					array::push_back(hits, hit);
				}

				break;
			}
			case RaycastMode::ALL:
			{
				btCollisionWorld::AllHitsRayResultCallback cb(start, end);
				_scene->rayTest(start, end, cb);

				if (cb.hasHit())
				{
					const int num = cb.m_hitPointWorld.size();
					array::resize(hits, num);

					for (int i = 0; i < num; ++i)
					{
						RaycastHit hit;
						hit.position = to_vector3(cb.m_hitPointWorld[i]);
						hit.normal = to_vector3(cb.m_hitNormalWorld[i]);
						hit.actor.i = (u32)(uintptr_t)btRigidBody::upcast(cb.m_collisionObjects[i])->getUserPointer();
						hits[i] = hit;
					}
				}

				break;
			}
			default:
			{
				CE_FATAL("Bad raycast mode");
				break;
			}
		}
	}

	Vector3 gravity() const
	{
		return to_vector3(_scene->getGravity());
	}

	void set_gravity(const Vector3& g)
	{
		_scene->setGravity(to_btVector3(g));
	}

	void update_actor_world_poses(const UnitId* begin, const UnitId* end, const Matrix4x4* begin_world)
	{
		for (; begin != end; ++begin, ++begin_world)
		{
			const u32 ai = hash::get(_actor_map, begin->encode(), UINT32_MAX);
			if (ai == UINT32_MAX)
				continue;

			const Quaternion rot = rotation(*begin_world);
			const Vector3 pos = translation(*begin_world);
			// http://www.bulletphysics.org/mediawiki-1.5.8/index.php/MotionStates
			_actor[ai].actor->getMotionState()->setWorldTransform(btTransform(to_btQuaternion(rot), to_btVector3(pos)));
		}
	}

	void update(f32 dt)
	{
		_scene->stepSimulation(dt);

		const int num = _scene->getNumCollisionObjects();
		const btCollisionObjectArray& collision_array = _scene->getCollisionObjectArray();
	    // Update actors
		for (int i = 0; i < num; ++i)
		{
			if ((uintptr_t)collision_array[i]->getUserPointer() == (uintptr_t)UINT32_MAX)
				continue;

			btRigidBody* body = btRigidBody::upcast(collision_array[i]);
			if (body
				&& body->getMotionState()
				&& body->isActive()
				)
			{
				btTransform tr;
				body->getMotionState()->getWorldTransform(tr);
				const btQuaternion rot_bt = tr.getRotation();
				const btVector3 pos_bt    = tr.getOrigin();
				const Quaternion rot      = to_quaternion(rot_bt);
				const Vector3 pos         = to_vector3(pos_bt);

				const u32 a_idx = (u32)(uintptr_t)body->getUserPointer();
				const UnitId unit_id = _actor[a_idx].unit;

				post_transform_event(unit_id, pos, rot);
			}
		}
	}

	EventStream& events()
	{
		return _events;
	}

	void draw_debug()
	{
		if (!_debug_drawing)
			return;

		_scene->debugDrawWorld();
		_debug_drawer._lines->submit();
		_debug_drawer._lines->reset();
	}

	void enable_debug_drawing(bool enable)
	{
		_debug_drawing = enable;
	}

	void tick_callback(btDynamicsWorld* world, btScalar /*dt*/)
	{
		// Limit bodies velocity
		for (u32 i = 0; i < array::size(_actor); ++i)
		{
			CE_ASSERT_NOT_NULL(_actor[i].actor);
			const btVector3 velocity = _actor[i].actor->getLinearVelocity();
			const btScalar speed = velocity.length();

			if (speed > 100.0f)
				_actor[i].actor->setLinearVelocity(velocity * 100.0f / speed);
		}

		// Check collisions
		int num_manifolds = world->getDispatcher()->getNumManifolds();
		for (int i = 0; i < num_manifolds; ++i)
		{
			const btPersistentManifold* contact_manifold = world->getDispatcher()->getManifoldByIndexInternal(i);

			const btCollisionObject* actor_a = contact_manifold->getBody0();
			const btCollisionObject* actor_b = contact_manifold->getBody1();
			const ActorInstance a0 = make_actor_instance((u32)(uintptr_t)actor_a->getUserPointer());
			const ActorInstance a1 = make_actor_instance((u32)(uintptr_t)actor_b->getUserPointer());

			int num_contacts = contact_manifold->getNumContacts();
			for (int j = 0; j < num_contacts; ++j)
			{
				const btManifoldPoint& point = contact_manifold->getContactPoint(j);
				if (point.getDistance() < 0.0f)
				{
					const btVector3& where_a = point.getPositionWorldOnA();
					const btVector3& where_b = point.getPositionWorldOnB();
					const btVector3& normal = point.m_normalWorldOnB;

					post_collision_event(a0
						, a1
						, to_vector3(where_a)
						, to_vector3(normal)
						, point.getLifeTime() > 0
							? PhysicsCollisionEvent::BEGIN_TOUCH
							: PhysicsCollisionEvent::END_TOUCH   // FIXME
						);
				}
			}
		}
	}

	void unit_destroyed_callback(UnitId id)
	{
		ActorInstance first = actor(id);
		if (is_valid(first))
			destroy_actor(first);
	}

	static void tick_cb(btDynamicsWorld* world, btScalar dt)
	{
		BulletWorld* bw = static_cast<BulletWorld*>(world->getWorldUserInfo());
		bw->tick_callback(world, dt);
	}

	static void unit_destroyed_callback(UnitId id, void* user_ptr)
	{
		((BulletWorld*)user_ptr)->unit_destroyed_callback(id);
	}

private:

	bool is_valid(ColliderInstance i) { return i.i != UINT32_MAX; }
	bool is_valid(ActorInstance i) { return i.i != UINT32_MAX; }
	bool is_valid(ControllerInstance i) { return i.i != UINT32_MAX; }
	bool is_valid(JointInstance i) { return i.i != UINT32_MAX; }

	ColliderInstance make_collider_instance(u32 i) { ColliderInstance inst = { i }; return inst; }
	ActorInstance make_actor_instance(u32 i) { ActorInstance inst = { i }; return inst; }
	ControllerInstance make_controller_instance(u32 i) { ControllerInstance inst = { i }; return inst; }
	JointInstance make_joint_instance(u32 i) { JointInstance inst = { i }; return inst; }

	void post_collision_event(ActorInstance a0, ActorInstance a1, const Vector3& where, const Vector3& normal, PhysicsCollisionEvent::Type type)
	{
		PhysicsCollisionEvent ev;
		ev.type = type;
		ev.actors[0] = a0;
		ev.actors[1] = a1;
		ev.where = where;
		ev.normal = normal;

		event_stream::write(_events, EventType::PHYSICS_COLLISION, ev);
	}

	void post_trigger_event(ActorInstance trigger, ActorInstance other, PhysicsTriggerEvent::Type type)
	{
		PhysicsTriggerEvent ev;
		ev.type = type;
		ev.trigger = trigger;
		ev.other = other;

		event_stream::write(_events, EventType::PHYSICS_TRIGGER, ev);
	}

	void post_transform_event(UnitId id, const Vector3& pos, const Quaternion& rot)
	{
		PhysicsTransformEvent ev;
		ev.unit_id = id;
		ev.position = pos;
		ev.rotation = rot;

		event_stream::write(_events, EventType::PHYSICS_TRANSFORM, ev);
	}

	struct ColliderInstanceData
	{
		UnitId unit;
		Matrix4x4 local_tm;
		btTriangleIndexVertexArray* vertex_array;
		btCollisionShape* shape;
		ColliderInstance next;
	};

	struct ActorInstanceData
	{
		UnitId unit;
		btRigidBody* actor;
	};

	struct ControllerInstanceData
	{
		UnitId unit;
		btKinematicCharacterController* contr;
	};

	Allocator* _allocator;
	UnitManager* _unit_manager;

	Hash<u32> _collider_map;
	Hash<u32> _actor_map;
	Hash<u32> _controller_map;
	Array<ColliderInstanceData> _collider;
	Array<ActorInstanceData> _actor;
	Array<ControllerInstanceData> _controller;
	Array<btTypedConstraint*> _joints;

	MyFilterCallback _filter_cb;
	btDiscreteDynamicsWorld* _scene;
	MyDebugDrawer _debug_drawer;

	EventStream _events;

	const PhysicsConfigResource* _config_resource;
	bool _debug_drawing;
};

namespace physics_world
{
	PhysicsWorld* create(Allocator& a, ResourceManager& rm, UnitManager& um, DebugLine& dl)
	{
		return CE_NEW(a, BulletWorld)(a, rm, um, dl);
	}

	void destroy(Allocator& a, PhysicsWorld* pw)
	{
		CE_DELETE(a, pw);
	}
} // namespace physics_world

} // namespace crown

#endif // CROWN_PHYSICS_BULLET
