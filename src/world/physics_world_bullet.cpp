/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PHYSICS_BULLET

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/quaternion.inl"
#include "core/math/vector3.inl"
#include "core/memory/proxy_allocator.h"
#include "device/log.h"
#include "resource/physics_resource.h"
#include "resource/resource_manager.h"
#include "world/debug_line.h"
#include "world/event_stream.inl"
#include "world/physics.h"
#include "world/physics_world.h"
#include "world/unit_manager.h"
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletDynamics/ConstraintSolver/btFixedConstraint.h>
#include <BulletDynamics/ConstraintSolver/btHingeConstraint.h>
#include <BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/ConstraintSolver/btSliderConstraint.h>
#define BT_THREADSAFE 0
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btDefaultMotionState.h>
#include <LinearMath/btIDebugDraw.h>

LOG_SYSTEM(PHYSICS, "physics")

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

static inline btVector3 to_btVector3(const Vector3& v)
{
	return btVector3(v.x, v.y, v.z);
}

static inline btQuaternion to_btQuaternion(const Quaternion& q)
{
	return btQuaternion(q.x, q.y, q.z, q.w);
}

static inline btTransform to_btTransform(const Matrix4x4& m)
{
	btMatrix3x3 basis(m.x.x, m.y.x, m.z.x
		, m.x.y, m.y.y, m.z.y
		, m.x.z, m.y.z, m.z.z
		);
	btVector3 pos(m.t.x, m.t.y, m.t.z);
	return btTransform(basis, pos);
}

static inline Vector3 to_vector3(const btVector3& v)
{
	return vector3(v.x(), v.y(), v.z());
}

static inline Quaternion to_quaternion(const btQuaternion& q)
{
	return from_elements(q.x(), q.y(), q.z(), q.w());
}

static inline Matrix4x4 to_matrix4x4(const btTransform& t)
{
	const btVector3 x = t.getBasis().getRow(0);
	const btVector3 y = t.getBasis().getRow(1);
	const btVector3 z = t.getBasis().getRow(2);
	const btVector3 o = t.getOrigin();

	Matrix4x4 m;
	m.x.x = x.x();
	m.x.y = y.x();
	m.x.z = z.x();
	m.x.w = 0.0f;

	m.y.x = x.y();
	m.y.y = y.y();
	m.y.z = z.y();
	m.y.w = 0.0f;

	m.z.x = x.z();
	m.z.y = y.z();
	m.z.z = z.z();
	m.z.w = 0.0f;

	m.t.x = o.x();
	m.t.y = o.y();
	m.t.z = o.z();
	m.t.w = 1.0f;

	return m;
}

struct MyDebugDrawer : public btIDebugDraw
{
	DebugLine* _lines;

	explicit MyDebugDrawer(DebugLine& dl)
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
		logw(PHYSICS, warningString);
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
};

struct MyFilterCallback : public btOverlapFilterCallback
{
	bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const
	{
		bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
		collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

		return collides;
	}
};

struct PhysicsWorldImpl
{
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
		btRigidBody* body;
	};

	Allocator* _allocator;
	UnitManager* _unit_manager;

	HashMap<UnitId, u32> _collider_map;
	HashMap<UnitId, u32> _actor_map;
	Array<ColliderInstanceData> _collider;
	Array<ActorInstanceData> _actor;
	Array<btTypedConstraint*> _joints;

	MyFilterCallback _filter_callback;
	btDiscreteDynamicsWorld* _dynamics_world;
	MyDebugDrawer _debug_drawer;

	EventStream _events;
	UnitDestroyCallback _unit_destroy_callback;

	const PhysicsConfigResource* _config_resource;
	bool _debug_drawing;

	PhysicsWorldImpl(Allocator& a, ResourceManager& rm, UnitManager& um, DebugLine& dl)
		: _allocator(&a)
		, _unit_manager(&um)
		, _collider_map(a)
		, _actor_map(a)
		, _collider(a)
		, _actor(a)
		, _joints(a)
		, _dynamics_world(NULL)
		, _debug_drawer(dl)
		, _events(a)
		, _debug_drawing(false)
	{
		_dynamics_world = CE_NEW(*_allocator, btDiscreteDynamicsWorld)(physics_globals::_bt_dispatcher
			, physics_globals::_bt_interface
			, physics_globals::_bt_solver
			, physics_globals::_bt_configuration
			);

		_dynamics_world->getCollisionWorld()->setDebugDrawer(&_debug_drawer);
		_dynamics_world->setInternalTickCallback(tick_cb, this);
		_dynamics_world->getPairCache()->setOverlapFilterCallback(&_filter_callback);

		_config_resource = (PhysicsConfigResource*)rm.get(RESOURCE_TYPE_PHYSICS_CONFIG, STRING_ID_64("global", 0x0b2f08fe66e395c0));

		_unit_destroy_callback.destroy = PhysicsWorldImpl::unit_destroyed_callback;
		_unit_destroy_callback.user_data = this;
		_unit_destroy_callback.node.next = NULL;
		_unit_destroy_callback.node.prev = NULL;
		um.register_destroy_callback(&_unit_destroy_callback);
	}

	~PhysicsWorldImpl()
	{
		_unit_manager->unregister_destroy_callback(&_unit_destroy_callback);

		for (u32 i = 0; i < array::size(_actor); ++i)
		{
			btRigidBody* body = _actor[i].body;

			_dynamics_world->removeRigidBody(body);
			CE_DELETE(*_allocator, body->getMotionState());
			CE_DELETE(*_allocator, body->getCollisionShape());
			CE_DELETE(*_allocator, body);
		}

		for (u32 i = 0; i < array::size(_collider); ++i)
		{
			CE_DELETE(*_allocator, _collider[i].vertex_array);
			CE_DELETE(*_allocator, _collider[i].shape);
		}

		CE_DELETE(*_allocator, _dynamics_world);
	}

	ColliderInstance collider_create(UnitId unit, const ColliderDesc* sd, const Vector3& scale)
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
			}
			break;

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
			}
			break;

		case ColliderType::HEIGHTFIELD:
			CE_FATAL("Not implemented");
			break;

		default:
			CE_FATAL("Unknown shape type");
			break;
		}

		child_shape->setLocalScaling(to_btVector3(scale));

		const u32 last = array::size(_collider);

		ColliderInstanceData cid;
		cid.unit         = unit;
		cid.local_tm     = sd->local_tm;
		cid.vertex_array = vertex_array;
		cid.shape        = child_shape;
		cid.next.i       = UINT32_MAX;

		ColliderInstance ci = collider_first(unit);
		while (is_valid(ci) && is_valid(collider_next(ci)))
			ci = collider_next(ci);

		if (is_valid(ci))
			_collider[ci.i].next.i = last;
		else
			hash_map::set(_collider_map, unit, last);

		array::push_back(_collider, cid);
		return make_collider_instance(last);
	}

	void collider_destroy(ColliderInstance collider)
	{
		CE_ASSERT(collider.i < array::size(_collider), "Index out of bounds");

		const u32 last                 = array::size(_collider) - 1;
		const UnitId u                 = _collider[collider.i].unit;
		const ColliderInstance first_i = collider_first(u);
		const ColliderInstance last_i  = make_collider_instance(last);

		collider_swap_node(last_i, collider);
		collider_remove_node(first_i, collider);

		CE_DELETE(*_allocator, _collider[collider.i].vertex_array);
		CE_DELETE(*_allocator, _collider[collider.i].shape);

		_collider[collider.i] = _collider[last];

		array::pop_back(_collider);
	}

	void collider_remove_node(ColliderInstance first, ColliderInstance collider)
	{
		CE_ASSERT(first.i < array::size(_collider), "Index out of bounds");
		CE_ASSERT(collider.i < array::size(_collider), "Index out of bounds");

		const UnitId u = _collider[first.i].unit;

		if (collider.i == first.i)
		{
			if (!is_valid(collider_next(collider)))
				hash_map::remove(_collider_map, u);
			else
				hash_map::set(_collider_map, u, collider_next(collider).i);
		}
		else
		{
			ColliderInstance prev = collider_previous(collider);
			_collider[prev.i].next = collider_next(collider);
		}
	}

	void collider_swap_node(ColliderInstance a, ColliderInstance b)
	{
		CE_ASSERT(a.i < array::size(_collider), "Index out of bounds");
		CE_ASSERT(b.i < array::size(_collider), "Index out of bounds");

		const UnitId u = _collider[a.i].unit;
		const ColliderInstance first_i = collider_first(u);

		if (a.i == first_i.i)
		{
			hash_map::set(_collider_map, u, b.i);
		}
		else
		{
			const ColliderInstance prev_a = collider_previous(a);
			CE_ENSURE(prev_a.i != a.i);
			_collider[prev_a.i].next = b;
		}
	}

	ColliderInstance collider_first(UnitId unit)
	{
		return make_collider_instance(hash_map::get(_collider_map, unit, UINT32_MAX));
	}

	ColliderInstance collider_next(ColliderInstance collider)
	{
		return _collider[collider.i].next;
	}

	ColliderInstance collider_previous(ColliderInstance collider)
	{
		CE_ASSERT(collider.i < array::size(_collider), "Index out of bounds");

		const UnitId u = _collider[collider.i].unit;

		ColliderInstance curr = collider_first(u);
		ColliderInstance prev = { UINT32_MAX };

		while (curr.i != collider.i)
		{
			prev = curr;
			curr = collider_next(curr);
		}

		return prev;
	}

	ActorInstance actor_create(UnitId unit, const ActorResource* ar, const Matrix4x4& tm)
	{
		CE_ASSERT(!hash_map::has(_actor_map, unit), "Unit already has an actor component");

		const PhysicsActor* actor_class = physics_config_resource::actor(_config_resource, ar->actor_class);
		const PhysicsMaterial* material = physics_config_resource::material(_config_resource, ar->material);

		const bool is_kinematic = (actor_class->flags & PhysicsActor::KINEMATIC) != 0;
		const bool is_dynamic   = (actor_class->flags & PhysicsActor::DYNAMIC) != 0;
		const bool is_static    = !is_kinematic && !is_dynamic;
		const bool is_trigger   = (actor_class->flags & PhysicsActor::TRIGGER) != 0;
		const f32  mass         = is_dynamic ? ar->mass : 0.0f;

		// Create compound shape
		btCompoundShape* shape = CE_NEW(*_allocator, btCompoundShape)(true);
		ColliderInstance ci = collider_first(unit);
		while (is_valid(ci))
		{
			shape->addChildShape(to_btTransform(_collider[ci.i].local_tm), _collider[ci.i].shape);
			ci = collider_next(ci);
		}

		// Create motion state
		const btTransform tr = to_btTransform(tm);
		btDefaultMotionState* ms = is_static
			? NULL
			: CE_NEW(*_allocator, btDefaultMotionState)(tr)
			;

		// If dynamic, calculate inertia
		btVector3 inertia;
		if (mass != 0.0f) // Actor is dynamic iff mass != 0
			shape->calculateLocalInertia(mass, inertia);

		btRigidBody::btRigidBodyConstructionInfo rbinfo(mass, ms, shape, inertia);
		rbinfo.m_startWorldTransform      = tr;
		rbinfo.m_linearDamping            = actor_class->linear_damping;
		rbinfo.m_angularDamping           = actor_class->angular_damping;
		rbinfo.m_restitution              = material->restitution;
		rbinfo.m_friction                 = material->friction;
		rbinfo.m_rollingFriction          = material->rolling_friction;
		rbinfo.m_linearSleepingThreshold  = 0.5f; // FIXME
		rbinfo.m_angularSleepingThreshold = 0.7f; // FIXME

		// Create rigid body
		btRigidBody* body = CE_NEW(*_allocator, btRigidBody)(rbinfo);

		int cflags = body->getCollisionFlags();
		cflags |= is_kinematic ? btCollisionObject::CF_KINEMATIC_OBJECT    : 0;
		cflags |= is_static    ? btCollisionObject::CF_STATIC_OBJECT       : 0;
		cflags |= is_trigger   ? btCollisionObject::CF_NO_CONTACT_RESPONSE : 0;
		body->setCollisionFlags(cflags);
		if (is_kinematic)
			body->setActivationState(DISABLE_DEACTIVATION);

		body->setLinearFactor(btVector3(
			(ar->flags & ActorFlags::LOCK_TRANSLATION_X) ? 0.0f : 1.0f,
			(ar->flags & ActorFlags::LOCK_TRANSLATION_Y) ? 0.0f : 1.0f,
			(ar->flags & ActorFlags::LOCK_TRANSLATION_Z) ? 0.0f : 1.0f)
		);
		body->setAngularFactor(btVector3(
			(ar->flags & ActorFlags::LOCK_ROTATION_X) ? 0.0f : 1.0f,
			(ar->flags & ActorFlags::LOCK_ROTATION_Y) ? 0.0f : 1.0f,
			(ar->flags & ActorFlags::LOCK_ROTATION_Z) ? 0.0f : 1.0f)
		);

		const u32 last = array::size(_actor);
		body->setUserPointer((void*)(uintptr_t)last);

		// Set collision filters
		const u32 me   = physics_config_resource::filter(_config_resource, ar->collision_filter)->me;
		const u32 mask = physics_config_resource::filter(_config_resource, ar->collision_filter)->mask;

		_dynamics_world->addRigidBody(body, me, mask);

		ActorInstanceData aid;
		aid.unit = unit;
		aid.body = body;

		array::push_back(_actor, aid);
		hash_map::set(_actor_map, unit, last);

		return make_actor_instance(last);
	}

	void actor_destroy(ActorInstance actor)
	{
		const u32 last      = array::size(_actor) - 1;
		const UnitId u      = _actor[actor.i].unit;
		const UnitId last_u = _actor[last].unit;

		_dynamics_world->removeRigidBody(_actor[actor.i].body);
		CE_DELETE(*_allocator, _actor[actor.i].body->getMotionState());
		CE_DELETE(*_allocator, _actor[actor.i].body->getCollisionShape());
		CE_DELETE(*_allocator, _actor[actor.i].body);

		_actor[actor.i] = _actor[last];
		_actor[actor.i].body->setUserPointer((void*)(uintptr_t)actor.i);

		array::pop_back(_actor);

		hash_map::set(_actor_map, last_u, actor.i);
		hash_map::remove(_actor_map, u);
	}

	ActorInstance actor(UnitId unit)
	{
		return make_actor_instance(hash_map::get(_actor_map, unit, UINT32_MAX));
	}

	Vector3 actor_world_position(ActorInstance actor) const
	{
		return to_vector3(_actor[actor.i].body->getCenterOfMassPosition());
	}

	Quaternion actor_world_rotation(ActorInstance actor) const
	{
		return to_quaternion(_actor[actor.i].body->getOrientation());
	}

	Matrix4x4 actor_world_pose(ActorInstance actor) const
	{
		return to_matrix4x4(_actor[actor.i].body->getCenterOfMassTransform());
	}

	void actor_teleport_world_position(ActorInstance actor, const Vector3& p)
	{
		btTransform pose = _actor[actor.i].body->getCenterOfMassTransform();
		pose.setOrigin(to_btVector3(p));
		_actor[actor.i].body->setCenterOfMassTransform(pose);
	}

	void actor_teleport_world_rotation(ActorInstance actor, const Quaternion& r)
	{
		btTransform pose = _actor[actor.i].body->getCenterOfMassTransform();
		pose.setRotation(to_btQuaternion(r));
		_actor[actor.i].body->setCenterOfMassTransform(pose);
	}

	void actor_teleport_world_pose(ActorInstance actor, const Matrix4x4& m)
	{
		const Quaternion rot = rotation(m);
		const Vector3 pos = translation(m);

		btTransform pose = _actor[actor.i].body->getCenterOfMassTransform();
		pose.setRotation(to_btQuaternion(rot));
		pose.setOrigin(to_btVector3(pos));
		_actor[actor.i].body->setCenterOfMassTransform(pose);
	}

	Vector3 actor_center_of_mass(ActorInstance actor) const
	{
		return to_vector3(_actor[actor.i].body->getCenterOfMassTransform().getOrigin());
	}

	void actor_enable_gravity(ActorInstance actor)
	{
		btRigidBody* body = _actor[actor.i].body;
		body->setFlags(body->getFlags() & ~BT_DISABLE_WORLD_GRAVITY);
		body->setGravity(_dynamics_world->getGravity());
	}

	void actor_disable_gravity(ActorInstance actor)
	{
		btRigidBody* body = _actor[actor.i].body;
		body->setFlags(body->getFlags() | BT_DISABLE_WORLD_GRAVITY);
		body->setGravity(btVector3(0.0f, 0.0f, 0.0f));
	}

	void actor_enable_collision(ActorInstance /*i*/)
	{
		CE_FATAL("Not implemented yet");
	}

	void actor_disable_collision(ActorInstance /*i*/)
	{
		CE_FATAL("Not implemented yet");
	}

	void actor_set_collision_filter(ActorInstance /*i*/, StringId32 /*filter*/)
	{
		CE_FATAL("Not implemented yet");
	}

	void actor_set_kinematic(ActorInstance actor, bool kinematic)
	{
		btRigidBody* body = _actor[actor.i].body;
		int flags = body->getCollisionFlags();

		if (kinematic)
		{
			body->setCollisionFlags(flags | btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(DISABLE_DEACTIVATION);
		}
		else
		{
			body->setCollisionFlags(flags & ~btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(ACTIVE_TAG);
		}
	}

	bool actor_is_static(ActorInstance actor) const
	{
		return _actor[actor.i].body->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT;
	}

	bool actor_is_dynamic(ActorInstance actor) const
	{
		const int flags = _actor[actor.i].body->getCollisionFlags();
		return !(flags & btCollisionObject::CF_STATIC_OBJECT)
			&& !(flags & btCollisionObject::CF_KINEMATIC_OBJECT)
			;
	}

	bool actor_is_kinematic(ActorInstance actor) const
	{
		const int flags = _actor[actor.i].body->getCollisionFlags();
		return (flags & btCollisionObject::CF_KINEMATIC_OBJECT) != 0;
	}

	bool actor_is_nonkinematic(ActorInstance actor) const
	{
		return actor_is_dynamic(actor) && !actor_is_kinematic(actor);
	}

	f32 actor_linear_damping(ActorInstance actor) const
	{
		return _actor[actor.i].body->getLinearDamping();
	}

	void actor_set_linear_damping(ActorInstance actor, f32 rate)
	{
		_actor[actor.i].body->setDamping(rate, _actor[actor.i].body->getAngularDamping());
	}

	f32 actor_angular_damping(ActorInstance actor) const
	{
		return _actor[actor.i].body->getAngularDamping();
	}

	void actor_set_angular_damping(ActorInstance actor, f32 rate)
	{
		_actor[actor.i].body->setDamping(_actor[actor.i].body->getLinearDamping(), rate);
	}

	Vector3 actor_linear_velocity(ActorInstance actor) const
	{
		btVector3 v = _actor[actor.i].body->getLinearVelocity();
		return to_vector3(v);
	}

	void actor_set_linear_velocity(ActorInstance actor, const Vector3& vel)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->setLinearVelocity(to_btVector3(vel));
	}

	Vector3 actor_angular_velocity(ActorInstance actor) const
	{
		btVector3 v = _actor[actor.i].body->getAngularVelocity();
		return to_vector3(v);
	}

	void actor_set_angular_velocity(ActorInstance actor, const Vector3& vel)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->setAngularVelocity(to_btVector3(vel));
	}

	void actor_add_impulse(ActorInstance actor, const Vector3& impulse)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->applyCentralImpulse(to_btVector3(impulse));
	}

	void actor_add_impulse_at(ActorInstance actor, const Vector3& impulse, const Vector3& pos)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->applyImpulse(to_btVector3(impulse), to_btVector3(pos));
	}

	void actor_add_torque_impulse(ActorInstance actor, const Vector3& imp)
	{
		_actor[actor.i].body->applyTorqueImpulse(to_btVector3(imp));
	}

	void actor_push(ActorInstance actor, const Vector3& vel, f32 mass)
	{
		const Vector3 f = vel * mass;
		_actor[actor.i].body->applyCentralForce(to_btVector3(f));
	}

	void actor_push_at(ActorInstance actor, const Vector3& vel, f32 mass, const Vector3& pos)
	{
		const Vector3 f = vel * mass;
		_actor[actor.i].body->applyForce(to_btVector3(f), to_btVector3(pos));
	}

	bool actor_is_sleeping(ActorInstance actor)
	{
		return !_actor[actor.i].body->isActive();
	}

	void actor_wake_up(ActorInstance actor)
	{
		_actor[actor.i].body->activate(true);
	}

	JointInstance joint_create(ActorInstance a0, ActorInstance a1, const JointDesc& jd)
	{
		const btVector3 anchor_0 = to_btVector3(jd.anchor_0);
		const btVector3 anchor_1 = to_btVector3(jd.anchor_1);
		btRigidBody* body_0 = _actor[a0.i].body;
		btRigidBody* body_1 = is_valid(a1) ? _actor[a1.i].body : NULL;

		btTypedConstraint* joint = NULL;
		switch(jd.type)
		{
		case JointType::FIXED:
			{
				const btTransform frame_0 = btTransform(btQuaternion::getIdentity(), anchor_0);
				const btTransform frame_1 = btTransform(btQuaternion::getIdentity(), anchor_1);
				joint = CE_NEW(*_allocator, btFixedConstraint)(*body_0
					, *body_1
					, frame_0
					, frame_1
					);
			}
			break;

		case JointType::SPRING:
			joint = CE_NEW(*_allocator, btPoint2PointConstraint)(*body_0
				, *body_1
				, anchor_0
				, anchor_1
				);
			break;

		case JointType::HINGE:
			{
				btHingeConstraint* hinge = CE_NEW(*_allocator, btHingeConstraint)(*body_0
					, *body_1
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
			}
			break;

		default:
			CE_FATAL("Unknown joint type");
			break;
		}

		joint->setBreakingImpulseThreshold(jd.break_force);
		_dynamics_world->addConstraint(joint);

		return make_joint_instance(UINT32_MAX);
	}

	void joint_destroy(JointInstance /*i*/)
	{
		CE_FATAL("Not implemented yet");
	}

	bool cast_ray(RaycastHit& hit, const Vector3& from, const Vector3& dir, f32 len)
	{
		const btVector3 aa = to_btVector3(from);
		const btVector3 bb = to_btVector3(from + dir*len);

		btCollisionWorld::ClosestRayResultCallback cb(aa, bb);
		// Collide with everything
		cb.m_collisionFilterGroup = -1;
		cb.m_collisionFilterMask = -1;

		_dynamics_world->rayTest(aa, bb, cb);

		if (cb.hasHit())
		{
			const u32 actor_i = (u32)(uintptr_t)btRigidBody::upcast(cb.m_collisionObject)->getUserPointer();

			hit.position = to_vector3(cb.m_hitPointWorld);
			hit.normal   = to_vector3(cb.m_hitNormalWorld);
			hit.time     = (f32)cb.m_closestHitFraction;
			hit.unit     = _actor[actor_i].unit;
			hit.actor.i  = actor_i;
			return true;
		}

		return false;
	}

	bool cast_ray_all(Array<RaycastHit>& hits, const Vector3& from, const Vector3& dir, f32 len)
	{
		const btVector3 aa = to_btVector3(from);
		const btVector3 bb = to_btVector3(from + dir*len);

		btCollisionWorld::AllHitsRayResultCallback cb(aa, bb);
		// Collide with everything
		cb.m_collisionFilterGroup = -1;
		cb.m_collisionFilterMask = -1;

		_dynamics_world->rayTest(aa, bb, cb);

		if (cb.hasHit())
		{
			const int num = cb.m_hitPointWorld.size();
			array::resize(hits, num);

			for (int i = 0; i < num; ++i)
			{
				const u32 actor_i = (u32)(uintptr_t)btRigidBody::upcast(cb.m_collisionObjects[i])->getUserPointer();

				hits[i].position = to_vector3(cb.m_hitPointWorld[i]);
				hits[i].normal   = to_vector3(cb.m_hitNormalWorld[i]);
				hits[i].time     = (f32)cb.m_closestHitFraction;
				hits[i].unit     = _actor[actor_i].unit;
				hits[i].actor.i  = actor_i;
			}

			return true;
		}

		return false;
	}

	bool cast(RaycastHit& hit, const btConvexShape* shape, const Vector3& from, const Vector3& dir, f32 len)
	{
		const btTransform aa(btQuaternion::getIdentity(), to_btVector3(from));
		const btTransform bb(btQuaternion::getIdentity(), to_btVector3(from + dir*len));

		btCollisionWorld::ClosestConvexResultCallback cb(btVector3(0, 0, 0), btVector3(0, 0, 0));
		// Collide with everything
		cb.m_collisionFilterGroup = -1;
		cb.m_collisionFilterMask = -1;
		_dynamics_world->convexSweepTest(shape, aa, bb, cb);

		if (cb.hasHit())
		{
			const u32 actor_i = (u32)(uintptr_t)btRigidBody::upcast(cb.m_hitCollisionObject)->getUserPointer();

			hit.position = to_vector3(cb.m_hitPointWorld);
			hit.normal   = to_vector3(cb.m_hitNormalWorld);
			hit.time     = (f32)cb.m_closestHitFraction;
			hit.unit     = _actor[actor_i].unit;
			hit.actor.i  = actor_i;
			return true;
		}

		return false;
	}

	bool cast_sphere(RaycastHit& hit, const Vector3& from, f32 radius, const Vector3& dir, f32 len)
	{
		btSphereShape shape(radius);
		return cast(hit, &shape, from, dir, len);
	}

	bool cast_box(RaycastHit& hit, const Vector3& from, const Vector3& half_extents, const Vector3& dir, f32 len)
	{
		btBoxShape shape(to_btVector3(half_extents));
		return cast(hit, &shape, from, dir, len);
	}

	Vector3 gravity() const
	{
		return to_vector3(_dynamics_world->getGravity());
	}

	void set_gravity(const Vector3& g)
	{
		_dynamics_world->setGravity(to_btVector3(g));
	}

	void update_actor_world_poses(const UnitId* begin, const UnitId* end, const Matrix4x4* begin_world)
	{
		for (; begin != end; ++begin, ++begin_world)
		{
			const u32 ai = hash_map::get(_actor_map, *begin, UINT32_MAX);
			if (ai == UINT32_MAX)
				continue;

			const Quaternion rot = rotation(*begin_world);
			const Vector3 pos = translation(*begin_world);
			// http://www.bulletphysics.org/mediawiki-1.5.8/index.php/MotionStates
			btMotionState* ms = _actor[ai].body->getMotionState();
			if (ms)
				ms->setWorldTransform(btTransform(to_btQuaternion(rot), to_btVector3(pos)));
		}
	}

	void update(f32 dt)
	{
		// 12Hz to 120Hz
		_dynamics_world->stepSimulation(dt, 7, 1.0f/60.0f);

		const int num = _dynamics_world->getNumCollisionObjects();
		const btCollisionObjectArray& collision_array = _dynamics_world->getCollisionObjectArray();
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
				const UnitId unit_id = _actor[(u32)(uintptr_t)body->getUserPointer()].unit;

				btTransform tr;
				body->getMotionState()->getWorldTransform(tr);

				// Post transform event
				{
					PhysicsTransformEvent ev;
					ev.unit_id = unit_id;
					ev.world = to_matrix4x4(tr);
					event_stream::write(_events, EventType::PHYSICS_TRANSFORM, ev);
				}
			}
		}
	}

	EventStream& events()
	{
		return _events;
	}

	void debug_draw()
	{
		if (!_debug_drawing)
			return;

		_dynamics_world->debugDrawWorld();
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
			CE_ENSURE(NULL != _actor[i].body);
			const btVector3 velocity = _actor[i].body->getLinearVelocity();
			const btScalar speed = velocity.length();

			if (speed > 100.0f)
				_actor[i].body->setLinearVelocity(velocity * 100.0f / speed);
		}

		// Check collisions
		int num_manifolds = world->getDispatcher()->getNumManifolds();
		for (int i = 0; i < num_manifolds; ++i)
		{
			const btPersistentManifold* manifold = world->getDispatcher()->getManifoldByIndexInternal(i);

			const btCollisionObject* obj_a = manifold->getBody0();
			const btCollisionObject* obj_b = manifold->getBody1();
			const ActorInstance a0 = make_actor_instance((u32)(uintptr_t)obj_a->getUserPointer());
			const ActorInstance a1 = make_actor_instance((u32)(uintptr_t)obj_b->getUserPointer());
			const UnitId u0 = _actor[a0.i].unit;
			const UnitId u1 = _actor[a1.i].unit;

			int num_contacts = manifold->getNumContacts();
			for (int j = 0; j < num_contacts; ++j)
			{
				const btManifoldPoint& pt = manifold->getContactPoint(j);
				if (pt.m_distance1 < 0.0f)
				{
					// Post collision event
					PhysicsCollisionEvent ev;
					ev.type = pt.m_lifeTime == 1 ? PhysicsCollisionEvent::TOUCH_BEGIN : PhysicsCollisionEvent::TOUCHING;
					ev.units[0] = u0;
					ev.units[1] = u1;
					ev.actors[0] = a0;
					ev.actors[1] = a1;
					ev.position = to_vector3(pt.m_positionWorldOnB);
					ev.normal = to_vector3(pt.m_normalWorldOnB);
					ev.distance = pt.m_distance1;
					event_stream::write(_events, EventType::PHYSICS_COLLISION, ev);
				}
			}
		}
	}

	void unit_destroyed_callback(UnitId unit)
	{
		{
			ActorInstance first = actor(unit);
			if (is_valid(first))
				actor_destroy(first);
		}

		{
			ColliderInstance curr = collider_first(unit);
			ColliderInstance next;

			while (is_valid(curr))
			{
				next = collider_next(curr);
				collider_destroy(curr);
				curr = next;
			}
		}
	}

	static void tick_cb(btDynamicsWorld* world, btScalar dt)
	{
		PhysicsWorldImpl* bw = static_cast<PhysicsWorldImpl*>(world->getWorldUserInfo());
		bw->tick_callback(world, dt);
	}

	static void unit_destroyed_callback(UnitId unit, void* user_ptr)
	{
		static_cast<PhysicsWorldImpl*>(user_ptr)->unit_destroyed_callback(unit);
	}

	static ColliderInstance make_collider_instance(u32 i) { ColliderInstance inst = { i }; return inst; }
	static ActorInstance make_actor_instance(u32 i) { ActorInstance inst = { i }; return inst; }
	static JointInstance make_joint_instance(u32 i) { JointInstance inst = { i }; return inst; }
};

PhysicsWorld::PhysicsWorld(Allocator& a, ResourceManager& rm, UnitManager& um, DebugLine& dl)
	: _marker(PHYSICS_WORLD_MARKER)
	, _allocator(&a)
	, _impl(NULL)
{
	_impl = CE_NEW(*_allocator, PhysicsWorldImpl)(a, rm, um, dl);
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

void PhysicsWorld::collider_destroy(ColliderInstance collider)
{
	_impl->collider_destroy(collider);
}

ColliderInstance PhysicsWorld::collider_first(UnitId unit)
{
	return _impl->collider_first(unit);
}

ColliderInstance PhysicsWorld::collider_next(ColliderInstance collider)
{
	return _impl->collider_next(collider);
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

#endif // CROWN_PHYSICS_BULLET
