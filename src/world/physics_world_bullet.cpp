/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PHYSICS_BULLET
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/hash_set.inl"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/quaternion.inl"
#include "core/math/vector3.inl"
#include "core/memory/proxy_allocator.h"
#include "core/profiler.h"
#include "core/strings/string_id.inl"
#include "device/log.h"
#include "resource/physics_resource.h"
#include "resource/resource_manager.h"
#include "world/debug_line.h"
#include "world/event_stream.inl"
#include "world/physics.h"
#include "world/physics_world.h"
#include "world/scene_graph.h"
#include "world/unit_manager.h"
#include <BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h>
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
#include <LinearMath/btQuickprof.h>

LOG_SYSTEM(PHYSICS, "physics")

namespace crown
{
namespace physics_globals
{
	static ProxyAllocator *_linear_allocator;
	static ProxyAllocator *_heap_allocator;

	inline void *aligned_alloc_func(size_t size, int alignment)
	{
		return _heap_allocator->allocate((u32)size, (u32)alignment);
	}

	inline void aligned_free_func(void *memblock)
	{
		_heap_allocator->deallocate(memblock);
	}

	inline void *alloc_func(size_t size)
	{
		return _heap_allocator->allocate((u32)size);
	}

	inline void free_func(void *memblock)
	{
		_heap_allocator->deallocate(memblock);
	}

	inline void profile_scope_enter_func(const char *name)
	{
		ENTER_PROFILE_SCOPE(name);
	}

	inline void profile_scope_leave_func()
	{
		LEAVE_PROFILE_SCOPE();
	}

	static PhysicsSettings _settings;
	static btDefaultCollisionConfiguration *_bt_configuration;
	static btCollisionDispatcher *_bt_dispatcher;
	static btBroadphaseInterface *_bt_interface;
	static btSequentialImpulseConstraintSolver *_bt_solver;

	void init(Allocator &linear, Allocator &heap, const PhysicsSettings *settings)
	{
		_linear_allocator = CE_NEW(linear, ProxyAllocator)(linear, "physics");
		_heap_allocator = CE_NEW(*_linear_allocator, ProxyAllocator)(heap, "physics");

		btAlignedAllocSetCustom(alloc_func, free_func);
		btAlignedAllocSetCustomAligned(aligned_alloc_func, aligned_free_func);

		btSetCustomEnterProfileZoneFunc(profile_scope_enter_func);
		btSetCustomLeaveProfileZoneFunc(profile_scope_leave_func);

		_settings         = *settings;
		_bt_configuration = CE_NEW(*_linear_allocator, btDefaultCollisionConfiguration);
		_bt_dispatcher    = CE_NEW(*_linear_allocator, btCollisionDispatcher)(_bt_configuration);
		_bt_interface     = CE_NEW(*_linear_allocator, btDbvtBroadphase);
		_bt_solver        = CE_NEW(*_linear_allocator, btSequentialImpulseConstraintSolver);
	}

	void shutdown(Allocator &linear, Allocator &heap)
	{
		CE_UNUSED_2(linear, heap);

		CE_DELETE(*_linear_allocator, _bt_solver);
		CE_DELETE(*_linear_allocator, _bt_interface);
		CE_DELETE(*_linear_allocator, _bt_dispatcher);
		CE_DELETE(*_linear_allocator, _bt_configuration);

		CE_DELETE(*_linear_allocator, _heap_allocator);
		CE_DELETE(linear, _linear_allocator);
	}

} // namespace physics_globals

static inline btVector3 to_btVector3(const Vector3 &v)
{
	return btVector3(v.x, v.y, v.z);
}

static inline btVector3 to_btVector3(const Color4 &v)
{
	return btVector3(v.x, v.y, v.z);
}

static inline btQuaternion to_btQuaternion(const Quaternion &q)
{
	return btQuaternion(q.x, q.y, q.z, q.w);
}

static inline btTransform to_btTransform(const Matrix4x4 &m)
{
	btMatrix3x3 basis(m.x.x, m.y.x, m.z.x
		, m.x.y, m.y.y, m.z.y
		, m.x.z, m.y.z, m.z.z
		);
	btVector3 pos(m.t.x, m.t.y, m.t.z);
	return btTransform(basis, pos);
}

static inline Vector3 to_vector3(const btVector3 &v)
{
	return { v.x, v.y, v.z };
}

static inline Quaternion to_quaternion(const btQuaternion &q)
{
	return { q.x, q.y, q.z, q.w };
}

static inline Matrix4x4 to_matrix4x4(const btTransform &t)
{
	const btVector3 x = t.m_basis.getRow(0);
	const btVector3 y = t.m_basis.getRow(1);
	const btVector3 z = t.m_basis.getRow(2);
	const btVector3 o = t.m_origin;

	Matrix4x4 m;
	m.x.x = x.x;
	m.x.y = y.x;
	m.x.z = z.x;
	m.x.w = 0.0f;

	m.y.x = x.y;
	m.y.y = y.y;
	m.y.z = z.y;
	m.y.w = 0.0f;

	m.z.x = x.z;
	m.z.y = y.z;
	m.z.z = z.z;
	m.z.w = 0.0f;

	m.t.x = o.x;
	m.t.y = o.y;
	m.t.z = o.z;
	m.t.w = 1.0f;

	return m;
}

struct MyDebugDrawer : public btIDebugDraw
{
	DebugLine *_lines;
	DefaultColors _colors;

	explicit MyDebugDrawer(DebugLine &dl)
		: _lines(&dl)
	{
		_colors.m_activeObject = to_btVector3(COLOR4_ORANGE);
		_colors.m_deactivatedObject = to_btVector3(COLOR4_ORANGE);
		_colors.m_wantsDeactivationObject = to_btVector3(COLOR4_ORANGE);
		_colors.m_disabledDeactivationObject = to_btVector3(COLOR4_ORANGE);
		_colors.m_disabledSimulationObject = to_btVector3(COLOR4_ORANGE);
		_colors.m_aabb = to_btVector3(COLOR4_ORANGE);
		_colors.m_contactPoint = to_btVector3(COLOR4_ORANGE);
	}

	void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) override
	{
		const Vector3 start = to_vector3(from);
		const Vector3 end = to_vector3(to);
		_lines->add_line(start, end, { color.x, color.y, color.z, 1.0f });
	}

	void drawContactPoint(const btVector3 &pointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color) override
	{
		CE_UNUSED_3(normalOnB, distance, lifeTime);
		const Vector3 from = to_vector3(pointOnB);
		_lines->add_sphere(from, 0.1f, { color.x, color.y, color.z, 1.0f });
	}

	void reportErrorWarning(const char *warningString) override
	{
		logw(PHYSICS, warningString);
	}

	void draw3dText(const btVector3 & /*location*/, const char * /*textString*/) override
	{
	}

	void setDebugMode(int /*debugMode*/) override
	{
	}

	int getDebugMode() const override
	{
		return DBG_DrawWireframe
			| DBG_DrawConstraints
			| DBG_DrawConstraintLimits
			| DBG_FastWireframe
			;
	}

	DefaultColors getDefaultColors() const override
	{
		return _colors;
	}
};

struct MyFilterCallback : public btOverlapFilterCallback
{
	bool needBroadphaseCollision(btBroadphaseProxy *proxy0, btBroadphaseProxy *proxy1) const override
	{
		bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
		collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);
		return collides;
	}
};

/// @todo Interact with dynamic objects,
/// Ride kinematicly animated platforms properly
/// More realistic (or maybe just a config option) falling
/// -> Should integrate falling velocity manually and use that in stepDown()
/// Support jumping
/// Support ducking
class btKinematicClosestNotMeRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
	btCollisionObject *_me;

	btKinematicClosestNotMeRayResultCallback(btCollisionObject *me)
		: btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
	{
		_me = me;
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &ray_result, bool normal_in_world_space)
	{
		if (ray_result.m_collisionObject == _me)
			return 1.0;

		return ClosestRayResultCallback::addSingleResult(ray_result, normal_in_world_space);
	}
};

class btKinematicClosestNotMeConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
	btCollisionObject *_me;
	const btVector3 _up;
	btScalar _min_slope_dot;

	btKinematicClosestNotMeConvexResultCallback(btCollisionObject *me, const btVector3 &up, btScalar min_slope_dot)
		: btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
		, _me(me)
		, _up(up)
		, _min_slope_dot(min_slope_dot)
	{
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult &convex_result, bool normal_in_world_space)
	{
		if (convex_result.m_hitCollisionObject == _me)
			return btScalar(1.0);

		if (!convex_result.m_hitCollisionObject->hasContactResponse())
			return btScalar(1.0);

		btVector3 hit_normal_world;
		if (normal_in_world_space)
			hit_normal_world = convex_result.m_hitNormalLocal;
		else // Need to transform normal into world-space.
			hit_normal_world = convex_result.m_hitCollisionObject->m_worldTransform.m_basis * convex_result.m_hitNormalLocal;

		if (_up.dot(hit_normal_world) < _min_slope_dot)
			return btScalar(1.0);

		return ClosestConvexResultCallback::addSingleResult(convex_result, normal_in_world_space);
	}
};

struct MoverFlags
{
	enum Enum : u32
	{
		COLLIDES_SIDES = u32(1) << 0,
		COLLIDES_UP    = u32(1) << 1,
		COLLIDES_DOWN  = u32(1) << 2
	};
};

// Mover is a modified version of btKinematicCharacterController.
struct Mover
{
	u32 _flags;                  // MoverFlags::Enum
	btCollisionWorld *_collision_world;
	btPairCachingGhostObject *_ghost;
	btConvexShape *_shape;       // _ghost's shape, cached to avoid upcast.
	btScalar _max_penetration_depth;
	btScalar _vertical_delta;
	btScalar _max_slope_radians;
	btScalar _max_slope_cosine;  // cosf(_max_slope_radians).
	btScalar _step_height;
	btVector3 _move_delta;
	btVector3 _move_delta_normalized;
	btVector3 _current_position;
	btVector3 _target_position;
	btScalar _current_step_offset;
	btManifoldArray _manifold_array;
	bool _was_on_ground;
	btVector3 _up;
	btVector3 _center;

	BT_DECLARE_ALIGNED_ALLOCATOR();

	Mover(Allocator &allocator, btCollisionWorld *collision_world, btPairCachingGhostObject *ghost, float radius, float height, const btVector3 &up)
	{
		_flags = 0;
		_collision_world = collision_world;
		_ghost = ghost;
		_up.setValue(0.0f, 0.0f, 1.0f);
		_move_delta.setValue(0.0, 0.0, 0.0);
		_vertical_delta = 0.0;
		_was_on_ground = false;
		_current_step_offset = 0.0;
		_max_penetration_depth = 0.2;
		_step_height = 0.02; // FIXME: remove since we only use capsule colliders?
		_center.setValue(0.0f, 0.0f, 0.0f);

		_shape = CE_NEW(allocator, btCapsuleShapeZ)(radius, height);
		_ghost->setCollisionShape(_shape);

		set_up_direction(up);
		set_max_slope(btRadians(45.0));
	}

	void destroy_shape(Allocator &allocator)
	{
		CE_DELETE(allocator, _shape);
	}

	static btVector3 normalized(const btVector3 &v)
	{
		btVector3 n(0, 0, 0);

		if (v.length() > SIMD_EPSILON) {
			n = v.normalized();
		}
		return n;
	}

	// Returns the reflection direction of a ray going 'direction' hitting a surface with normal 'normal'.
	// See: http://www-cs-students.stanford.edu/~adityagp/final/node3.html
	btVector3 reflection_direction(const btVector3 &direction, const btVector3 &normal)
	{
		return direction - (btScalar(2.0) * direction.dot(normal)) * normal;
	}

	// Returns the portion of 'direction' that is parallel to 'normal'.
	btVector3 parallel_component(const btVector3 &direction, const btVector3 &normal)
	{
		btScalar magnitude = direction.dot(normal);
		return normal * magnitude;
	}

	// Returns the portion of 'direction' that is perpendicular to 'normal'.
	btVector3 perpendicular_component(const btVector3 &direction, const btVector3 &normal)
	{
		return direction - parallel_component(direction, normal);
	}

	bool recover_from_penetration()
	{
		// Here we must refresh the overlapping paircache as the penetrating movement itself or the
		// previous recovery iteration might have used setWorldTransform and pushed us into an object
		// that is not in the previous cache contents from the last timestep, as will happen if we
		// are pushed into a new AABB overlap. Unhandled this means the next convex sweep gets stuck.
		//
		// Do this by calling the broadphase's setAabb with the moved AABB, this will update the broadphase
		// paircache and the ghost's internal paircache at the same time.    /BW

		btVector3 min_aabb;
		btVector3 max_aabb;
		_shape->getAabb(_ghost->m_worldTransform, min_aabb, max_aabb);
		_collision_world->getBroadphase()->setAabb(_ghost->m_broadphaseHandle, min_aabb, max_aabb, _collision_world->getDispatcher());

		bool penetration = false;

		_collision_world->getDispatcher()->dispatchAllCollisionPairs(_ghost->getOverlappingPairCache(), _collision_world->getDispatchInfo(), _collision_world->getDispatcher());

		_current_position = _ghost->m_worldTransform.m_origin - _center;

		for (int i = 0; i < _ghost->getOverlappingPairCache()->getNumOverlappingPairs(); i++) {
			_manifold_array.resize(0);

			btBroadphasePair *collisionPair = &_ghost->getOverlappingPairCache()->getOverlappingPairArray()[i];

			btCollisionObject *obj0 = static_cast<btCollisionObject *>(collisionPair->m_pProxy0->m_clientObject);
			btCollisionObject *obj1 = static_cast<btCollisionObject *>(collisionPair->m_pProxy1->m_clientObject);

			if ((obj0 && !obj0->hasContactResponse()) || (obj1 && !obj1->hasContactResponse()))
				continue;

			if (!needs_collision(obj0, obj1))
				continue;

			if (collisionPair->m_algorithm)
				collisionPair->m_algorithm->getAllContactManifolds(_manifold_array);

			for (int j = 0; j < _manifold_array.size(); j++) {
				btPersistentManifold *manifold = _manifold_array[j];
				btScalar directionSign = manifold->getBody0() == _ghost ? btScalar(-1.0) : btScalar(1.0);
				for (int p = 0; p < manifold->getNumContacts(); p++) {
					const btManifoldPoint &pt = manifold->getContactPoint(p);

					btScalar dist = pt.getDistance();

					if (dist < -_max_penetration_depth) {
						_current_position += pt.m_normalWorldOnB * directionSign * dist * btScalar(0.2);
						penetration = true;
					}
				}
			}
		}

		btTransform new_trans = _ghost->m_worldTransform;
		new_trans.m_origin = _current_position + _center;
		_ghost->setWorldTransform(new_trans);
		return penetration;
	}

	bool needs_collision(const btCollisionObject *body0, const btCollisionObject *body1)
	{
		bool collides = (body0->m_broadphaseHandle->m_collisionFilterGroup & body1->m_broadphaseHandle->m_collisionFilterMask) != 0;
		collides = collides && (body1->m_broadphaseHandle->m_collisionFilterGroup & body0->m_broadphaseHandle->m_collisionFilterMask);
		return collides;
	}

	void step_up()
	{
		btScalar step_height = 0.0f;
		if (_vertical_delta < 0.0)
			step_height = _step_height;

		btTransform start;
		btTransform end;

		start.setIdentity();
		end.setIdentity();

		// FIXME: Handle penetration properly (?).
		start.m_origin = _current_position + _center;

		_target_position = _current_position + _up * (step_height + _vertical_delta);
		_current_position = _target_position;

		end.m_origin = _target_position + _center;

		btKinematicClosestNotMeConvexResultCallback callback(_ghost, -_up, _max_slope_cosine);
		callback.m_collisionFilterGroup = _ghost->m_broadphaseHandle->m_collisionFilterGroup;
		callback.m_collisionFilterMask = _ghost->m_broadphaseHandle->m_collisionFilterMask;

		_ghost->convexSweepTest(_shape, start, end, callback, _collision_world->getDispatchInfo().m_allowedCcdPenetration);

		if (callback.hasHit() && _ghost->hasContactResponse() && needs_collision(_ghost, callback.m_hitCollisionObject)) {
			_flags |= MoverFlags::COLLIDES_UP;

			// Only modify the position if the hit was a slope and not a wall or ceiling.
			if (callback.m_hitNormalWorld.dot(_up) > 0.0) {
				// We moved up only a fraction of the step height.
				_current_step_offset = step_height * callback.m_closestHitFraction;
				_current_position.setInterpolate3(_current_position, _target_position, callback.m_closestHitFraction);
			}

			btTransform &xform = _ghost->m_worldTransform;
			xform.m_origin = _current_position + _center;
			_ghost->setWorldTransform(xform);

			// Fix penetration if we hit a ceiling for example.
			int num_penetration_loops = 0;
			while (recover_from_penetration()) {
				num_penetration_loops++;
				if (num_penetration_loops > 4) {
					break; // Character could not recover from penetration.
				}
			}
			_target_position = _ghost->m_worldTransform.m_origin - _center;
			_current_position = _target_position;

			if (_vertical_delta > 0) {
				_vertical_delta = 0.0;
				_current_step_offset = _step_height;
			}
		} else {
			_current_step_offset = step_height;
			_current_position = _target_position;
		}
	}

	void step_forward_and_strafe(const btVector3 &delta)
	{
		btTransform start;
		btTransform end;

		_target_position = _current_position + delta;

		start.setIdentity();
		end.setIdentity();

		btScalar fraction = 1.0;
		int maxIter = 10;

		while (fraction > btScalar(0.01) && maxIter-- > 0) {
			start.m_origin = _current_position + _center;
			end.m_origin   = _target_position + _center;
			btVector3 sweepDirNegative(_current_position - _target_position);

			btKinematicClosestNotMeConvexResultCallback callback(_ghost, sweepDirNegative, btScalar(0.0));
			callback.m_collisionFilterGroup = _ghost->m_broadphaseHandle->m_collisionFilterGroup;
			callback.m_collisionFilterMask = _ghost->m_broadphaseHandle->m_collisionFilterMask;

			if (!(start == end))
				_ghost->convexSweepTest(_shape, start, end, callback, _collision_world->getDispatchInfo().m_allowedCcdPenetration);

			fraction -= callback.m_closestHitFraction;

			if (callback.hasHit() && _ghost->hasContactResponse() && needs_collision(_ghost, callback.m_hitCollisionObject)) {
				// We moved only a fraction.
				_flags |= MoverFlags::COLLIDES_SIDES;

				// Update target position based on collision.
				const btVector3 hit_normal = callback.m_hitNormalWorld;
				const btScalar normal_mag = btScalar(1.0);
				btVector3 movement_dir = _target_position - _current_position;
				btScalar movement_length = movement_dir.length();
				if (movement_length > SIMD_EPSILON) {
					movement_dir.normalize();

					btVector3 reflect_dir = reflection_direction(movement_dir, hit_normal);
					reflect_dir.normalize();

					// btVector3 parallel_dir = parallel_component(reflect_dir, hit_normal);
					btVector3 perpendicular_dir = perpendicular_component(reflect_dir, hit_normal);

					_target_position = _current_position;

					if (normal_mag != 0.0) {
						btVector3 perp_component = perpendicular_dir * btScalar(normal_mag * movement_length);
						_target_position += perp_component;
					}
				}

				btVector3 current_dir = _target_position - _current_position;
				btScalar distance2 = current_dir.length2();
				if (distance2 <= SIMD_EPSILON)
					break;

				current_dir.normalize();
				// See Quake2: "If velocity is against original velocity, stop ead to avoid tiny oscilations in sloping corners."
				if (current_dir.dot(_move_delta_normalized) <= btScalar(0.0))
					break;
			} else {
				_current_position = _target_position;
			}
		}
	}

	void step_down()
	{
		btTransform start;
		btTransform end;
		btTransform end_double;
		bool runonce = false;

		btVector3 orig_position = _target_position;
		btScalar down_velocity = (_vertical_delta < 0.f ? -_vertical_delta : 0.f);

		if (_vertical_delta > 0.0)
			return;

		btVector3 step_drop = _up * (_current_step_offset + down_velocity);
		_target_position -= step_drop;

		btKinematicClosestNotMeConvexResultCallback callback(_ghost, _up, _max_slope_cosine);
		callback.m_collisionFilterGroup = _ghost->m_broadphaseHandle->m_collisionFilterGroup;
		callback.m_collisionFilterMask = _ghost->m_broadphaseHandle->m_collisionFilterMask;

		btKinematicClosestNotMeConvexResultCallback callback2(_ghost, _up, _max_slope_cosine);
		callback2.m_collisionFilterGroup = _ghost->m_broadphaseHandle->m_collisionFilterGroup;
		callback2.m_collisionFilterMask = _ghost->m_broadphaseHandle->m_collisionFilterMask;

		while (1) {
			start.setIdentity();
			end.setIdentity();

			end_double.setIdentity();

			start.m_origin = _current_position + _center;
			end.m_origin   = _target_position + _center;

			// Set double test for 2x the step drop, to check for a large drop vs small drop.
			end_double.m_origin = _target_position - step_drop + _center;

			_ghost->convexSweepTest(_shape, start, end, callback, _collision_world->getDispatchInfo().m_allowedCcdPenetration);

			// Test a double fall height, to see if the character should interpolate it's fall (full) or not (partial).
			if (!callback.hasHit() && _ghost->hasContactResponse())
				_ghost->convexSweepTest(_shape, start, end_double, callback2, _collision_world->getDispatchInfo().m_allowedCcdPenetration);

			btScalar down_vel2 = (_vertical_delta < 0.f ? -_vertical_delta : 0.f);
			bool has_hit = callback2.hasHit() && _ghost->hasContactResponse() && needs_collision(_ghost, callback2.m_hitCollisionObject);

			btScalar step_height = 0.0f;
			if (_vertical_delta < 0.0)
				step_height = _step_height;

			if (down_vel2 > 0.0 && down_vel2 < step_height && has_hit == true && runonce == false && _was_on_ground) {
				// redo the velocity calculation when falling a small amount, for fast stairs motion
				// for larger falls, use the smoother/slower interpolated movement by not touching
				// the target position.

				_target_position = orig_position;
				down_velocity = step_height;

				step_drop = _up * (_current_step_offset + down_velocity);
				_target_position -= step_drop;
				runonce = true;
				continue; // Re-run previous tests.
			}
			break;
		}

		if ((_ghost->hasContactResponse() && (callback.hasHit() && needs_collision(_ghost, callback.m_hitCollisionObject))) || runonce == true) {
			// We dropped a fraction of the height -> hit floor.
			// btScalar fraction = (_current_position.y - callback.m_hitPointWorld.y) / 2;

			_current_position.setInterpolate3(_current_position, _target_position, callback.m_closestHitFraction);
			_flags |= MoverFlags::COLLIDES_DOWN;
		} else {
			// We dropped the full height.
			_current_position = _target_position;
		}
	}

	void move(const btVector3 &delta)
	{
		_move_delta = delta;
		_move_delta_normalized = normalized(delta);

		_current_position = _ghost->m_worldTransform.m_origin - _center;
		_target_position = _current_position;

		_was_on_ground = onGround();
		_flags = 0u;

		// Split move delta into vertical and perpendicular components.
		// FIXME: generalize by projecting _move_delta onto _up?
		_vertical_delta = _move_delta.z;
		btVector3 perpendicular_offset = btVector3(_move_delta.x, _move_delta.y, 0.0f);

		btTransform xform;
		xform = _ghost->m_worldTransform;

		step_up();
		step_forward_and_strafe(perpendicular_offset);
		step_down();

		xform.m_origin = _current_position + _center;
		_ghost->setWorldTransform(xform);

		int num_penetration_loops = 0;
		while (recover_from_penetration()) {
			num_penetration_loops++;
			if (num_penetration_loops > 4)
				break; // Character could not recover from penetration.
		}
	}

	void set_up_direction(const btVector3 &up)
	{
		if (_up == up)
			return;

		btVector3 u = _up;

		if (up.length2() > 0)
			_up = up.normalized();
		else
			_up = btVector3(0.0, 0.0, 0.0);

		if (!_ghost) return;
		btQuaternion rot = rotation(_up, u);

		// set orientation with new up
		btTransform xform;
		xform = _ghost->m_worldTransform;
		btQuaternion orn = rot.inverse() * xform.getRotation();
		xform.setRotation(orn);
		_ghost->setWorldTransform(xform);
	}

	btQuaternion rotation(btVector3 &v0, btVector3 &v1) const
	{
		if (v0.length2() == 0.0f || v1.length2() == 0.0f) {
			btQuaternion q;
			return q;
		}

		return shortestArcQuatNormalize2(v0, v1);
	}

	void reset()
	{
		_vertical_delta = 0.0;
		_was_on_ground = false;
		_move_delta.setValue(0, 0, 0);

		// clear pair cache
		btHashedOverlappingPairCache *cache = _ghost->getOverlappingPairCache();
		while (cache->getOverlappingPairArray().size() > 0) {
			cache->removeOverlappingPair(cache->getOverlappingPairArray()[0].m_pProxy0, cache->getOverlappingPairArray()[0].m_pProxy1, _collision_world->getDispatcher());
		}
	}

	void set_position(const btVector3 &origin)
	{
		btTransform xform;
		xform.setIdentity();
		xform.m_origin = (origin + _center);
		_ghost->setWorldTransform(xform);
	}

	/// The max slope determines the maximum angle that the controller can walk up.
	/// The slope angle is measured in radians.
	void set_max_slope(btScalar angle)
	{
		_max_slope_radians = angle;
		_max_slope_cosine = btCos(angle);
	}

	btScalar max_slope() const
	{
		return _max_slope_radians;
	}

	btScalar radius() const
	{
		return ((btCapsuleShapeZ *)_shape)->getRadius();
	}

	bool onGround() const
	{
		return fabs(_vertical_delta) < SIMD_EPSILON;
	}

	void set_height_radius(Allocator &allocator, float radius, float height)
	{
		CE_ENSURE(_shape != NULL);
		btCapsuleShapeZ *new_capsule = CE_NEW(allocator, btCapsuleShapeZ)(radius, height);

		_ghost->setCollisionShape(new_capsule);
		CE_DELETE(allocator, _shape);
		_shape = new_capsule;

		int num_penetration_loops = 0;
		while (recover_from_penetration()) {
			num_penetration_loops++;
			if (num_penetration_loops > 4)
				break; // Character could not recover from penetration.
		}
	}

	void set_height(Allocator &allocator, float height)
	{
		btCapsuleShapeZ *capsule = (btCapsuleShapeZ *)_shape;
		btScalar radius = capsule->getRadius();

		set_height_radius(allocator, radius, height);
	}

	void set_radius(Allocator &allocator, float radius)
	{
		btCapsuleShapeZ *capsule = (btCapsuleShapeZ *)_shape;
		btScalar height = capsule->getHalfHeight() * 2.0f;

		set_height_radius(allocator, radius, height);
	}

	void set_center(const btVector3 &center)
	{
		btVector3 delta = center - _center;
		_center = center;
		btTransform xform = _ghost->m_worldTransform;
		xform.m_origin += delta;
		_ghost->setWorldTransform(xform);
	}
};

struct PhysicsWorldImpl
{
	struct ColliderInstanceData
	{
		UnitId unit;
		btTriangleIndexVertexArray *vertex_array;
		btCollisionShape *shape;
	};

	struct ActorInstanceData
	{
		UnitId unit;
		btRigidBody *body;
		const ActorResource *resource;
	};

	struct MoverInstanceData
	{
		UnitId unit;
		btPairCachingGhostObject *ghost;
		Mover *mover;
	};

	ProxyAllocator _proxy_allocator;
	Allocator *_allocator;
	UnitManager *_unit_manager;

	HashMap<UnitId, u32> _collider_map;
	HashMap<UnitId, u32> _actor_map;
	HashMap<UnitId, u32> _mover_map;
	Array<ColliderInstanceData> _collider;
	Array<ActorInstanceData> _actor;
	Array<MoverInstanceData> _mover;
	Array<btTypedConstraint *> _joints;

	MyFilterCallback _filter_callback;
	btGhostPairCallback _ghost_pair_callback;
	btDiscreteDynamicsWorld *_dynamics_world;
	SceneGraph *_scene_graph;
	MyDebugDrawer _debug_drawer;

	EventStream _events;
	UnitDestroyCallback _unit_destroy_callback;

	const PhysicsConfigResource *_config_resource;
	bool _debug_drawing;

	HashSet<u64> _pairs0;
	HashSet<u64> _pairs1;
	HashSet<u64> *_curr_pairs;
	HashSet<u64> *_prev_pairs;

	PhysicsWorldImpl(Allocator &a, ResourceManager &rm, UnitManager &um, SceneGraph &sg, DebugLine &dl)
		: _proxy_allocator(a, "physics")
		, _allocator(&_proxy_allocator)
		, _unit_manager(&um)
		, _collider_map(*_allocator)
		, _actor_map(*_allocator)
		, _mover_map(*_allocator)
		, _collider(*_allocator)
		, _actor(*_allocator)
		, _mover(*_allocator)
		, _joints(*_allocator)
		, _dynamics_world(NULL)
		, _scene_graph(&sg)
		, _debug_drawer(dl)
		, _events(*_allocator)
		, _debug_drawing(false)
		, _pairs0(*_allocator)
		, _pairs1(*_allocator)
		, _curr_pairs(&_pairs1)
		, _prev_pairs(&_pairs0)
	{
		_dynamics_world = CE_NEW(*_allocator, btDiscreteDynamicsWorld)(physics_globals::_bt_dispatcher
			, physics_globals::_bt_interface
			, physics_globals::_bt_solver
			, physics_globals::_bt_configuration
			);

		_config_resource = (PhysicsConfigResource *)rm.get(RESOURCE_TYPE_PHYSICS_CONFIG, STRING_ID_64("global", 0x0b2f08fe66e395c0));

		_dynamics_world->setGravity(to_btVector3(_config_resource->gravity));
		_dynamics_world->getCollisionWorld()->setDebugDrawer(&_debug_drawer);
		_dynamics_world->setInternalTickCallback(tick_cb, this);
		_dynamics_world->getPairCache()->setInternalGhostPairCallback(&_ghost_pair_callback);
		_dynamics_world->getPairCache()->setOverlapFilterCallback(&_filter_callback);

		_unit_destroy_callback.destroy = PhysicsWorldImpl::unit_destroyed_callback;
		_unit_destroy_callback.user_data = this;
		_unit_destroy_callback.node.next = NULL;
		_unit_destroy_callback.node.prev = NULL;
		um.register_destroy_callback(&_unit_destroy_callback);
	}

	~PhysicsWorldImpl()
	{
		_unit_manager->unregister_destroy_callback(&_unit_destroy_callback);

		for (u32 i = 0; i < array::size(_mover); ++i) {
			MoverInstanceData *inst = &_mover[i];

			inst->mover->destroy_shape(*_allocator);
			CE_DELETE(*_allocator, inst->mover);
			_dynamics_world->removeCollisionObject(inst->ghost);
			CE_DELETE(*_allocator, inst->ghost);
		}

		for (u32 i = 0; i < array::size(_actor); ++i) {
			btRigidBody *body = _actor[i].body;

			_dynamics_world->removeRigidBody(body);
			CE_DELETE(*_allocator, body->m_optionalMotionState);
			CE_DELETE(*_allocator, body);
		}

		for (u32 i = 0; i < array::size(_collider); ++i) {
			CE_DELETE(*_allocator, _collider[i].vertex_array);
			CE_DELETE(*_allocator, _collider[i].shape);
		}

		CE_DELETE(*_allocator, _dynamics_world);
	}

	PhysicsWorldImpl(const PhysicsWorldImpl &) = delete;

	PhysicsWorldImpl &operator=(const PhysicsWorldImpl &) = delete;

	void collider_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
	{
		const ColliderDesc *cd = (ColliderDesc *)components_data;

		for (u32 i = 0; i < num; ++i) {
			UnitId unit = unit_lookup[unit_index[i]];
			const TransformInstance ti = _scene_graph->instance(unit_lookup[unit_index[i]]);
			const Matrix4x4 tm = _scene_graph->world_pose(ti);

			btTriangleIndexVertexArray *vertex_array = NULL;
			btCollisionShape *shape = NULL;

			switch (cd->type) {
			case ColliderType::SPHERE:
				shape = CE_NEW(*_allocator, btSphereShape)(cd->sphere.radius);
				break;

			case ColliderType::CAPSULE:
				shape = CE_NEW(*_allocator, btCapsuleShape)(cd->capsule.radius, cd->capsule.height);
				break;

			case ColliderType::BOX:
				shape = CE_NEW(*_allocator, btBoxShape)(to_btVector3(cd->box.half_size));
				break;

			case ColliderType::CONVEX_HULL: {
				const u8 *data         = (u8 *)&cd[1];
				const u32 num          = *(u32 *)data;
				const btScalar *points = (btScalar *)(data + sizeof(u32));

				shape = CE_NEW(*_allocator, btConvexHullShape)(points, (int)num, sizeof(Vector3));
				break;
			}

			case ColliderType::MESH: {
				const char *data      = (char *)&cd[1];
				const u32 num_points  = *(u32 *)data;
				const char *points    = data + sizeof(u32);
				const u32 num_indices = *(u32 *)(points + num_points*sizeof(Vector3));
				const char *indices   = points + sizeof(u32) + num_points*sizeof(Vector3);

				btIndexedMesh part;
				part.m_vertexBase          = (const unsigned char *)points;
				part.m_vertexStride        = sizeof(Vector3);
				part.m_numVertices         = num_points;
				part.m_triangleIndexBase   = (const unsigned char *)indices;
				part.m_triangleIndexStride = sizeof(u16)*3;
				part.m_numTriangles        = num_indices/3;
				part.m_indexType           = PHY_SHORT;

				vertex_array = CE_NEW(*_allocator, btTriangleIndexVertexArray)();
				vertex_array->addIndexedMesh(part, PHY_SHORT);

				const btVector3 aabb_min(-1000.0f, -1000.0f, -1000.0f);
				const btVector3 aabb_max(1000.0f, 1000.0f, 1000.0f);
				shape = CE_NEW(*_allocator, btBvhTriangleMeshShape)(vertex_array, false, aabb_min, aabb_max);
				break;
			}

			case ColliderType::HEIGHTFIELD:
				CE_FATAL("Not implemented");
				break;

			default:
				CE_FATAL("Unknown shape type");
				break;
			}

			shape->setLocalScaling(to_btVector3(scale(tm)));

			const u32 last = array::size(_collider);

			ColliderInstanceData cid;
			cid.unit         = unit;
			cid.vertex_array = vertex_array;
			cid.shape        = shape;

			array::push_back(_collider, cid);
			hash_map::set(_collider_map, unit, last);

			cd = (ColliderDesc *)((char *)(cd + 1) + cd->size);
		}
	}

	void collider_destroy(ColliderInstance collider)
	{
		CE_ASSERT(collider.i < array::size(_collider), "Index out of bounds");

		const u32 last      = array::size(_collider) - 1;
		const UnitId u      = _collider[collider.i].unit;
		const UnitId last_u = _collider[last].unit;

		CE_DELETE(*_allocator, _collider[collider.i].vertex_array);
		CE_DELETE(*_allocator, _collider[collider.i].shape);

		_collider[collider.i] = _collider[last];

		array::pop_back(_collider);

		hash_map::set(_collider_map, last_u, collider.i);
		hash_map::remove(_collider_map, u);
	}

	ColliderInstance collider_instance(UnitId unit)
	{
		return make_collider_instance(hash_map::get(_collider_map, unit, UINT32_MAX));
	}

	void actor_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
	{
		const PhysicsActor *actor_classes = physics_config_resource::actors_array(_config_resource);
		const PhysicsMaterial *materials = physics_config_resource::materials_array(_config_resource);
		const PhysicsCollisionFilter *filters = physics_config_resource::filters_array(_config_resource);

		const ActorResource *actors = (ActorResource *)components_data;

		for (u32 i = 0; i < num; ++i) {
			UnitId unit = unit_lookup[unit_index[i]];
			CE_ASSERT(!hash_map::has(_actor_map, unit), "Unit already has an actor component");

			TransformInstance ti = _scene_graph->instance(unit);
			Matrix4x4 tm = _scene_graph->world_pose(ti);
			Matrix4x4 tm_noscale = from_quaternion_translation(rotation(tm), translation(tm));

			u32 actor_i = physics_config_resource::actor_index(actor_classes, _config_resource->num_actors, actors[i].actor_class);
			u32 material_i = physics_config_resource::material_index(materials, _config_resource->num_materials, actors[i].material);
			u32 filter_i = physics_config_resource::filter_index(filters, _config_resource->num_filters, actors[i].collision_filter);

			const PhysicsActor *actor_class = &actor_classes[actor_i];
			const PhysicsMaterial *material = &materials[material_i];
			const PhysicsCollisionFilter *filter = &filters[filter_i];

			const bool is_kinematic = (actor_class->flags & CROWN_PHYSICS_ACTOR_KINEMATIC) != 0;
			const bool is_dynamic   = (actor_class->flags & CROWN_PHYSICS_ACTOR_DYNAMIC) != 0;
			const bool is_static    = !is_kinematic && !is_dynamic;
			const bool is_trigger   = (actor_class->flags & CROWN_PHYSICS_ACTOR_TRIGGER) != 0;

			const f32 mass = is_dynamic ? actors[i].mass : 0.0f;

			// Create compound shape
			ColliderInstance ci = collider_instance(unit);
			btCollisionShape *shape = _collider[ci.i].shape;

			// Create motion state
			const btTransform tr = to_btTransform(tm_noscale);
			btDefaultMotionState *ms = is_static
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
			rbinfo.m_spinningFriction         = material->spinning_friction;
			rbinfo.m_linearSleepingThreshold  = 0.5f; // FIXME
			rbinfo.m_angularSleepingThreshold = 0.7f; // FIXME

			// Create rigid body
			btRigidBody *body = CE_NEW(*_allocator, btRigidBody)(rbinfo);

			int cflags = body->m_collisionFlags;
			cflags |= is_kinematic ? btCollisionObject::CF_KINEMATIC_OBJECT    : 0;
			cflags |= is_static    ? btCollisionObject::CF_STATIC_OBJECT       : 0;
			cflags |= is_trigger   ? btCollisionObject::CF_NO_CONTACT_RESPONSE : 0;
			body->m_collisionFlags = (cflags);
			if (is_kinematic)
				body->setActivationState(DISABLE_DEACTIVATION);

			body->setLinearFactor(btVector3((actors[i].flags & ActorFlags::LOCK_TRANSLATION_X) ? 0.0f : 1.0f
				, (actors[i].flags & ActorFlags::LOCK_TRANSLATION_Y) ? 0.0f : 1.0f
				, (actors[i].flags & ActorFlags::LOCK_TRANSLATION_Z) ? 0.0f : 1.0f
				));

			body->setAngularFactor(btVector3((actors[i].flags & ActorFlags::LOCK_ROTATION_X) ? 0.0f : 1.0f
				, (actors[i].flags & ActorFlags::LOCK_ROTATION_Y) ? 0.0f : 1.0f
				, (actors[i].flags & ActorFlags::LOCK_ROTATION_Z) ? 0.0f : 1.0f
				));

			const u32 last = array::size(_actor);
			body->m_userObjectPointer = ((void *)(uintptr_t)last);

			_dynamics_world->addRigidBody(body, filter->me, filter->mask);

			ActorInstanceData aid;
			aid.unit = unit;
			aid.body = body;
			aid.resource = &actors[i];

			array::push_back(_actor, aid);
			hash_map::set(_actor_map, unit, last);
		}
	}

	void actor_destroy(ActorInstance actor)
	{
		const u32 last      = array::size(_actor) - 1;
		const UnitId u      = _actor[actor.i].unit;
		const UnitId last_u = _actor[last].unit;

		_dynamics_world->removeRigidBody(_actor[actor.i].body);
		CE_DELETE(*_allocator, _actor[actor.i].body->m_optionalMotionState);
		CE_DELETE(*_allocator, _actor[actor.i].body);

		_actor[actor.i] = _actor[last];
		_actor[actor.i].body->m_userObjectPointer = ((void *)(uintptr_t)actor.i);

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

	void actor_teleport_world_position(ActorInstance actor, const Vector3 &p)
	{
		btTransform pose = _actor[actor.i].body->getCenterOfMassTransform();
		pose.m_origin = to_btVector3(p);
		_actor[actor.i].body->setCenterOfMassTransform(pose);
	}

	void actor_teleport_world_rotation(ActorInstance actor, const Quaternion &r)
	{
		btTransform pose = _actor[actor.i].body->getCenterOfMassTransform();
		pose.setRotation(to_btQuaternion(r));
		_actor[actor.i].body->setCenterOfMassTransform(pose);
	}

	void actor_teleport_world_pose(ActorInstance actor, const Matrix4x4 &m)
	{
		const Quaternion rot = rotation(m);
		const Vector3 pos = translation(m);

		btTransform pose = _actor[actor.i].body->getCenterOfMassTransform();
		pose.setRotation(to_btQuaternion(rot));
		pose.m_origin = to_btVector3(pos);
		_actor[actor.i].body->setCenterOfMassTransform(pose);
	}

	Vector3 actor_center_of_mass(ActorInstance actor) const
	{
		return to_vector3(_actor[actor.i].body->getCenterOfMassTransform().m_origin);
	}

	void actor_enable_gravity(ActorInstance actor)
	{
		btRigidBody *body = _actor[actor.i].body;
		body->m_rigidbodyFlags = (body->m_rigidbodyFlags & ~BT_DISABLE_WORLD_GRAVITY);
		body->setGravity(_dynamics_world->getGravity());
	}

	void actor_disable_gravity(ActorInstance actor)
	{
		btRigidBody *body = _actor[actor.i].body;
		body->m_rigidbodyFlags = (body->m_rigidbodyFlags | BT_DISABLE_WORLD_GRAVITY);
		body->setGravity(btVector3(0.0f, 0.0f, 0.0f));
	}

	void actor_enable_collision(ActorInstance actor)
	{
		ActorInstanceData &a = _actor[actor.i];
		const PhysicsCollisionFilter *filters = physics_config_resource::filters_array(_config_resource);
		u32 filter_i = physics_config_resource::filter_index(filters, _config_resource->num_filters, a.resource->collision_filter);
		const PhysicsCollisionFilter *f = &filters[filter_i];

		_dynamics_world->removeRigidBody(a.body);
		_dynamics_world->addRigidBody(a.body, f->me, f->mask);
	}

	void actor_disable_collision(ActorInstance actor)
	{
		ActorInstanceData &a = _actor[actor.i];
		const PhysicsCollisionFilter *filters = physics_config_resource::filters_array(_config_resource);
		u32 filter_i = physics_config_resource::filter_index(filters, _config_resource->num_filters, a.resource->collision_filter);
		const PhysicsCollisionFilter *f = &filters[filter_i];

		// Disable collisions by setting collision mask to 0.
		_dynamics_world->removeRigidBody(a.body);
		_dynamics_world->addRigidBody(a.body, f->me, 0);
	}

	void actor_set_collision_filter(ActorInstance actor, StringId32 filter)
	{
		ActorInstanceData &a = _actor[actor.i];
		const PhysicsCollisionFilter *filters = physics_config_resource::filters_array(_config_resource);
		u32 filter_i = physics_config_resource::filter_index(filters, _config_resource->num_filters, filter);
		const PhysicsCollisionFilter *f = &filters[filter_i];

		_dynamics_world->removeRigidBody(a.body);
		_dynamics_world->addRigidBody(a.body, f->me, f->mask);
	}

	void actor_set_kinematic(ActorInstance actor, bool kinematic)
	{
		btRigidBody *body = _actor[actor.i].body;
		int flags = body->m_collisionFlags;

		if (kinematic) {
			body->m_collisionFlags = (flags | btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(DISABLE_DEACTIVATION);
		} else {
			body->m_collisionFlags = (flags & ~btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(ACTIVE_TAG);
		}
	}

	bool actor_is_static(ActorInstance actor) const
	{
		return _actor[actor.i].body->m_collisionFlags & btCollisionObject::CF_STATIC_OBJECT;
	}

	bool actor_is_dynamic(ActorInstance actor) const
	{
		const int flags = _actor[actor.i].body->m_collisionFlags;
		return !(flags & btCollisionObject::CF_STATIC_OBJECT)
			&& !(flags & btCollisionObject::CF_KINEMATIC_OBJECT)
			;
	}

	bool actor_is_kinematic(ActorInstance actor) const
	{
		const int flags = _actor[actor.i].body->m_collisionFlags;
		return (flags & (btCollisionObject::CF_KINEMATIC_OBJECT)) != 0;
	}

	bool actor_is_nonkinematic(ActorInstance actor) const
	{
		return actor_is_dynamic(actor) && !actor_is_kinematic(actor);
	}

	f32 actor_linear_damping(ActorInstance actor) const
	{
		return _actor[actor.i].body->m_linearDamping;
	}

	void actor_set_linear_damping(ActorInstance actor, f32 rate)
	{
		_actor[actor.i].body->setDamping(rate, _actor[actor.i].body->m_angularDamping);
	}

	f32 actor_angular_damping(ActorInstance actor) const
	{
		return _actor[actor.i].body->m_angularDamping;
	}

	void actor_set_angular_damping(ActorInstance actor, f32 rate)
	{
		_actor[actor.i].body->setDamping(_actor[actor.i].body->m_linearDamping, rate);
	}

	Vector3 actor_linear_velocity(ActorInstance actor) const
	{
		btVector3 v = _actor[actor.i].body->m_linearVelocity;
		return to_vector3(v);
	}

	void actor_set_linear_velocity(ActorInstance actor, const Vector3 &vel)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->setLinearVelocity(to_btVector3(vel));
	}

	Vector3 actor_angular_velocity(ActorInstance actor) const
	{
		btVector3 v = _actor[actor.i].body->m_angularVelocity;
		return to_vector3(v);
	}

	void actor_set_angular_velocity(ActorInstance actor, const Vector3 &vel)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->setAngularVelocity(to_btVector3(vel));
	}

	void actor_add_impulse(ActorInstance actor, const Vector3 &impulse)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->applyCentralImpulse(to_btVector3(impulse));
	}

	void actor_add_impulse_at(ActorInstance actor, const Vector3 &impulse, const Vector3 &pos)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->applyImpulse(to_btVector3(impulse), to_btVector3(pos));
	}

	void actor_add_torque_impulse(ActorInstance actor, const Vector3 &imp)
	{
		_actor[actor.i].body->applyTorqueImpulse(to_btVector3(imp));
	}

	void actor_push(ActorInstance actor, const Vector3 &vel, f32 mass)
	{
		const Vector3 f = vel * mass;
		_actor[actor.i].body->applyCentralForce(to_btVector3(f));
	}

	void actor_push_at(ActorInstance actor, const Vector3 &vel, f32 mass, const Vector3 &pos)
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

	void actor_debug_draw(ActorInstance actor, DebugLine *lines, const Color4 &color)
	{
		collision_object_debug_draw(lines, _actor[actor.i].body, color);
	}

	MoverInstance mover_create(UnitId unit, const MoverDesc *desc)
	{
		u32 unit_index = 0;
		mover_create_instances(desc, 1, &unit, &unit_index);
		return mover(unit);
	}

	void mover_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
	{
		const PhysicsCollisionFilter *filters = physics_config_resource::filters_array(_config_resource);

		const MoverDesc *movers = (MoverDesc *)components_data;

		for (u32 i = 0; i < num; ++i) {
			UnitId unit = unit_lookup[unit_index[i]];
			CE_ASSERT(!hash_map::has(_mover_map, unit), "Unit already has a mover component");

			TransformInstance ti = _scene_graph->instance(unit);
			Matrix4x4 tm = _scene_graph->world_pose(ti);

			u32 filter_i = physics_config_resource::filter_index(filters, _config_resource->num_filters, movers[i].collision_filter);
			const PhysicsCollisionFilter *f = &filters[filter_i];

			const btTransform pose(to_btQuaternion(QUATERNION_IDENTITY), to_btVector3(translation(tm)));

			btPairCachingGhostObject *ghost = CE_NEW(*_allocator, btPairCachingGhostObject)();
			ghost->setWorldTransform(pose);
			ghost->m_userObjectPointer = ((void *)(uintptr_t)UINT32_MAX);

			Mover *mover = CE_NEW(*_allocator, Mover)(*_allocator
				, _dynamics_world
				, ghost
				, movers[i].capsule.radius
				, movers[i].capsule.height
				, to_btVector3(VECTOR3_UP)
				);
			mover->set_max_slope(movers[i].max_slope_angle);
			mover->set_center(to_btVector3(movers[i].center));

			_dynamics_world->addCollisionObject(ghost
				, f->me
				, f->mask
				);

			MoverInstanceData mid;
			mid.unit = unit;
			mid.ghost = ghost;
			mid.mover = mover;

			const u32 last = array::size(_mover);

			array::push_back(_mover, mid);
			hash_map::set(_mover_map, unit, last);
		}
	}

	void mover_destroy(MoverInstance mover)
	{
		const u32 last      = array::size(_mover) - 1;
		const UnitId u      = _mover[mover.i].unit;
		const UnitId last_u = _mover[last].unit;

		_mover[mover.i].mover->destroy_shape(*_allocator);
		CE_DELETE(*_allocator, _mover[mover.i].mover);
		_dynamics_world->removeCollisionObject(_mover[mover.i].ghost);
		CE_DELETE(*_allocator, _mover[mover.i].ghost);

		_mover[mover.i] = _mover[last];
		array::pop_back(_mover);

		hash_map::set(_mover_map, last_u, mover.i);
		hash_map::remove(_mover_map, u);
	}

	MoverInstance mover(UnitId unit)
	{
		return make_mover_instance(hash_map::get(_mover_map, unit, UINT32_MAX));
	}

	void mover_set_height(MoverInstance mover, float height)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->set_height(*_allocator, height);
	}

	f32 mover_radius(MoverInstance mover)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->radius();
	}

	void mover_set_radius(MoverInstance mover, float radius)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->set_radius(*_allocator, radius);
	}

	f32 mover_max_slope_angle(MoverInstance mover)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->max_slope();
	}

	void mover_set_max_slope_angle(MoverInstance mover, f32 angle)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->set_max_slope(angle);
	}

	void mover_set_collision_filter(MoverInstance mover, StringId32 filter)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");

		const PhysicsCollisionFilter *filters = physics_config_resource::filters_array(_config_resource);
		u32 filter_i = physics_config_resource::filter_index(filters, _config_resource->num_filters, filter);
		const PhysicsCollisionFilter *f = &filters[filter_i];

		_dynamics_world->removeCollisionObject(_mover[mover.i].ghost);
		_dynamics_world->addCollisionObject(_mover[mover.i].ghost, f->me, f->mask);
	}

	Vector3 mover_position(MoverInstance mover)
	{
		return to_vector3(_mover[mover.i].mover->_current_position);
	}

	void mover_set_position(MoverInstance mover, const Vector3 &position)
	{
		_mover[mover.i].mover->reset();
		_mover[mover.i].mover->set_position(to_btVector3(position));
	}

	Vector3 mover_center(MoverInstance mover)
	{
		return to_vector3(_mover[mover.i].mover->_center);
	}

	void mover_set_center(MoverInstance mover, const Vector3 &center)
	{
		_mover[mover.i].mover->set_center(to_btVector3(center));
	}

	void mover_move(MoverInstance mover, const Vector3 &delta)
	{
		_mover[mover.i].mover->move(to_btVector3(delta));
	}

	bool mover_collides_sides(MoverInstance mover)
	{
		return (_mover[mover.i].mover->_flags & MoverFlags::COLLIDES_SIDES) != 0;
	}

	bool mover_collides_up(MoverInstance mover)
	{
		return (_mover[mover.i].mover->_flags & MoverFlags::COLLIDES_UP) != 0;
	}

	bool mover_collides_down(MoverInstance mover)
	{
		return (_mover[mover.i].mover->_flags & MoverFlags::COLLIDES_DOWN) != 0;
	}

	void mover_debug_draw(MoverInstance mover, DebugLine *lines, const Color4 &color)
	{
		collision_object_debug_draw(lines, _mover[mover.i].mover->_ghost, color);
	}

	JointInstance joint_create(ActorInstance a0, ActorInstance a1, const JointDesc &jd)
	{
		const btVector3 anchor_0 = to_btVector3(jd.anchor_0);
		const btVector3 anchor_1 = to_btVector3(jd.anchor_1);
		btRigidBody *body_0 = _actor[a0.i].body;
		btRigidBody *body_1 = is_valid(a1) ? _actor[a1.i].body : NULL;

		btTypedConstraint *joint = NULL;
		switch (jd.type) {
		case JointType::FIXED: {
			const btTransform frame_0 = btTransform(btQuaternion::getIdentity(), anchor_0);
			const btTransform frame_1 = btTransform(btQuaternion::getIdentity(), anchor_1);
			joint = CE_NEW(*_allocator, btFixedConstraint)(*body_0
				, *body_1
				, frame_0
				, frame_1
				);
			break;
		}

		case JointType::SPRING:
			joint = CE_NEW(*_allocator, btPoint2PointConstraint)(*body_0
				, *body_1
				, anchor_0
				, anchor_1
				);
			break;

		case JointType::HINGE: {
			btHingeConstraint *hinge = CE_NEW(*_allocator, btHingeConstraint)(*body_0
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
			break;
		}

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

	bool cast_ray(RaycastHit &hit, const Vector3 &from, const Vector3 &dir, f32 len)
	{
		const btVector3 aa = to_btVector3(from);
		const btVector3 bb = to_btVector3(from + dir*len);

		btCollisionWorld::ClosestRayResultCallback cb(aa, bb);
		// Collide with everything
		cb.m_collisionFilterGroup = -1;
		cb.m_collisionFilterMask = -1;

		_dynamics_world->rayTest(aa, bb, cb);

		if (cb.hasHit()) {
			const u32 actor_i = (u32)(uintptr_t)btRigidBody::upcast(cb.m_collisionObject)->m_userObjectPointer;
			if (actor_i == UINT32_MAX)
				return false;

			hit.position = to_vector3(cb.m_hitPointWorld);
			hit.normal   = to_vector3(cb.m_hitNormalWorld);
			hit.time     = (f32)cb.m_closestHitFraction;
			hit.unit     = _actor[actor_i].unit;
			hit.actor.i  = actor_i;
			return true;
		}

		return false;
	}

	bool cast_ray_all(Array<RaycastHit> &hits, const Vector3 &from, const Vector3 &dir, f32 len)
	{
		const btVector3 aa = to_btVector3(from);
		const btVector3 bb = to_btVector3(from + dir*len);

		btCollisionWorld::AllHitsRayResultCallback cb(aa, bb);
		// Collide with everything
		cb.m_collisionFilterGroup = -1;
		cb.m_collisionFilterMask = -1;

		_dynamics_world->rayTest(aa, bb, cb);

		if (cb.hasHit()) {
			const int num = cb.m_hitPointWorld.size();
			array::resize(hits, num);

			for (int i = 0; i < num; ++i) {
				const u32 actor_i = (u32)(uintptr_t)btRigidBody::upcast(cb.m_collisionObjects[i])->m_userObjectPointer;
				if (actor_i == UINT32_MAX)
					return false;

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

	bool cast(RaycastHit &hit, const btConvexShape *shape, const Vector3 &from, const Vector3 &dir, f32 len)
	{
		const btTransform aa(btQuaternion::getIdentity(), to_btVector3(from));
		const btTransform bb(btQuaternion::getIdentity(), to_btVector3(from + dir*len));

		btCollisionWorld::ClosestConvexResultCallback cb(btVector3(0, 0, 0), btVector3(0, 0, 0));
		// Collide with everything
		cb.m_collisionFilterGroup = -1;
		cb.m_collisionFilterMask = -1;
		_dynamics_world->convexSweepTest(shape, aa, bb, cb);

		if (cb.hasHit()) {
			const u32 actor_i = (u32)(uintptr_t)btRigidBody::upcast(cb.m_hitCollisionObject)->m_userObjectPointer;
			if (actor_i == UINT32_MAX)
				return false;

			hit.position = to_vector3(cb.m_hitPointWorld);
			hit.normal   = to_vector3(cb.m_hitNormalWorld);
			hit.time     = (f32)cb.m_closestHitFraction;
			hit.unit     = _actor[actor_i].unit;
			hit.actor.i  = actor_i;
			return true;
		}

		return false;
	}

	bool cast_sphere(RaycastHit &hit, const Vector3 &from, f32 radius, const Vector3 &dir, f32 len)
	{
		btSphereShape shape(radius);
		return cast(hit, &shape, from, dir, len);
	}

	bool cast_box(RaycastHit &hit, const Vector3 &from, const Vector3 &half_extents, const Vector3 &dir, f32 len)
	{
		btBoxShape shape(to_btVector3(half_extents));
		return cast(hit, &shape, from, dir, len);
	}

	Vector3 gravity() const
	{
		return to_vector3(_dynamics_world->getGravity());
	}

	void set_gravity(const Vector3 &g)
	{
		_dynamics_world->setGravity(to_btVector3(g));
	}

	void update_actor_world_poses(const UnitId *begin, const UnitId *end, const Matrix4x4 *begin_world)
	{
		for (; begin != end; ++begin, ++begin_world) {
			u32 inst;

			if ((inst = hash_map::get(_actor_map, *begin, UINT32_MAX)) != UINT32_MAX) {
				btRigidBody *body = _actor[inst].body;

				// http://www.bulletphysics.org/mediawiki-1.5.8/index.php/MotionStates
				btMotionState *motion_state = body->m_optionalMotionState;
				if (motion_state) {
					const Quaternion rot = rotation(*begin_world);
					const Vector3 pos = translation(*begin_world);
					const btTransform new_transform(to_btQuaternion(rot), to_btVector3(pos));
					motion_state->setWorldTransform(new_transform);
					body->activate();
				}
			}

			if ((inst = hash_map::get(_mover_map, *begin, UINT32_MAX)) != UINT32_MAX) {
				Mover *mover = _mover[inst].mover;

				mover->set_position(to_btVector3(translation(*begin_world)));
			}
		}
	}

	void update(f32 dt)
	{
		_dynamics_world->stepSimulation(dt
			, physics_globals::_settings.max_substeps
			, 1.0f/physics_globals::_settings.step_frequency
			);

		const int num = _dynamics_world->getNumCollisionObjects();
		const btCollisionObjectArray &collision_array = _dynamics_world->getCollisionObjectArray();
		// Update actors
		for (int i = 0; i < num; ++i) {
			if ((uintptr_t)collision_array[i]->m_userObjectPointer == (uintptr_t)UINT32_MAX)
				continue;

			btRigidBody *body = btRigidBody::upcast(collision_array[i]);
			if (body
				&& body->m_optionalMotionState
				&& body->isActive()
				) {
				u32 actor_i = (u32)(uintptr_t)body->m_userObjectPointer;
				if (actor_i == UINT32_MAX)
					continue;

				const UnitId unit_id = _actor[actor_i].unit;

				btTransform tr;
				body->m_optionalMotionState->getWorldTransform(tr);

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

	EventStream &events()
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

	static inline u64 encode_pair(const UnitId &a, const UnitId &b)
	{
		return a._idx > b._idx
			? (u64(a._idx) << 32) | u64(b._idx)
			: (u64(b._idx) << 32) | u64(a._idx)
			;
	}

	static inline void decode_pair(UnitId &a, UnitId &b, u64 pair)
	{
		a._idx = (pair & 0x00000000ffffffff) >>  0;
		b._idx = (pair & 0xffffffff00000000) >> 32;
	}

	void tick_callback(btDynamicsWorld *world, btScalar /*dt*/)
	{
		// Limit bodies velocity.
		for (u32 i = 0; i < array::size(_actor); ++i) {
			CE_ENSURE(NULL != _actor[i].body);
			const btVector3 velocity = _actor[i].body->m_linearVelocity;
			const btScalar speed = velocity.length();

			if (speed > 100.0f)
				_actor[i].body->setLinearVelocity(velocity * 100.0f / speed);
		}

		// Check collisions.
		hash_set::clear(*_curr_pairs);

		int num_manifolds = world->getDispatcher()->getNumManifolds();
		for (int i = 0; i < num_manifolds; ++i) {
			const btPersistentManifold *manifold = world->getDispatcher()->getManifoldByIndexInternal(i);

			const btCollisionObject *obj_a = manifold->getBody0();
			const btCollisionObject *obj_b = manifold->getBody1();
			const ActorInstance a0 = make_actor_instance((u32)(uintptr_t)obj_a->m_userObjectPointer);
			const ActorInstance a1 = make_actor_instance((u32)(uintptr_t)obj_b->m_userObjectPointer);
			if (!is_valid(a0) || !is_valid(a1))
				continue;

			const UnitId u0 = _actor[a0.i].unit;
			const UnitId u1 = _actor[a1.i].unit;

			int num_contacts = manifold->getNumContacts();
			if (num_contacts == 0) // When exactly?
				continue;

			const u64 pair = encode_pair(u0, u1);
			hash_set::insert(*_curr_pairs, pair);

			if (!hash_set::has(*_prev_pairs, pair)) {
				// If either A or B is a trigger, only generate a trigger event for the trigger unit.
				// Otherwise generate a regular collision event.
				if (obj_a->m_collisionFlags & btCollisionObject::CF_NO_CONTACT_RESPONSE
					|| obj_b->m_collisionFlags & btCollisionObject::CF_NO_CONTACT_RESPONSE) {
					PhysicsTriggerEvent ev;
					ev.trigger_unit = obj_a->m_collisionFlags & btCollisionObject::CF_NO_CONTACT_RESPONSE ? u0 : u1;
					ev.other_unit = ev.trigger_unit == u0 ? u1 : u0;
					ev.type = PhysicsTriggerEvent::ENTER;
					event_stream::write(_events, EventType::PHYSICS_TRIGGER, ev);
				} else {
					const btManifoldPoint &pt = manifold->getContactPoint(0);

					PhysicsCollisionEvent ev;
					ev.units[0] = u0;
					ev.units[1] = u1;
					ev.actors[0] = a0;
					ev.actors[1] = a1;
					ev.position = to_vector3(pt.m_positionWorldOnB);
					ev.normal = to_vector3(pt.m_normalWorldOnB);
					ev.distance = pt.m_distance1;
					ev.type = PhysicsCollisionEvent::TOUCH_BEGIN;
					event_stream::write(_events, EventType::PHYSICS_COLLISION, ev);
				}
			} else {
				// Do not generate stay events for triggers.
				if (!(obj_a->m_collisionFlags & btCollisionObject::CF_NO_CONTACT_RESPONSE)
					&& !(obj_b->m_collisionFlags & btCollisionObject::CF_NO_CONTACT_RESPONSE)) {
					const btManifoldPoint &pt = manifold->getContactPoint(0);

					PhysicsCollisionEvent ev;
					ev.units[0] = u0;
					ev.units[1] = u1;
					ev.actors[0] = a0;
					ev.actors[1] = a1;
					ev.position = to_vector3(pt.m_positionWorldOnB);
					ev.normal = to_vector3(pt.m_normalWorldOnB);
					ev.distance = pt.m_distance1;
					ev.type = PhysicsCollisionEvent::TOUCHING;
					event_stream::write(_events, EventType::PHYSICS_COLLISION, ev);
				}
			}
		}

		auto cur = hash_set::begin(*_prev_pairs);
		auto end = hash_set::end(*_prev_pairs);
		for (; cur != end; ++cur) {
			HASH_SET_SKIP_HOLE(*_prev_pairs, cur);

			if (!hash_set::has(*_curr_pairs, *cur)) {
				UnitId unit_a;
				UnitId unit_b;
				decode_pair(unit_a, unit_b, *cur);
				ActorInstance actor_a = actor(unit_a);
				ActorInstance actor_b = actor(unit_b);

				// If either A or B is a trigger, only generate a trigger event for the trigger unit.
				// Otherwise generate a regular collision event.
				if (_actor[actor_a.i].body->m_collisionFlags & btCollisionObject::CF_NO_CONTACT_RESPONSE
					|| _actor[actor_b.i].body->m_collisionFlags & btCollisionObject::CF_NO_CONTACT_RESPONSE) {
					PhysicsTriggerEvent ev;
					ev.type = PhysicsTriggerEvent::LEAVE;
					ev.trigger_unit = _actor[actor_a.i].body->m_collisionFlags & btCollisionObject::CF_NO_CONTACT_RESPONSE ? unit_a : unit_b;
					ev.other_unit = ev.trigger_unit == unit_a ? unit_b : unit_a;
					event_stream::write(_events, EventType::PHYSICS_TRIGGER, ev);
				} else {
					PhysicsCollisionEvent ev;
					ev.units[0] = unit_a;
					ev.units[1] = unit_b;
					ev.actors[0] = actor_a;
					ev.actors[1] = actor_b;
					ev.type = PhysicsCollisionEvent::TOUCH_END;
					event_stream::write(_events, EventType::PHYSICS_COLLISION, ev);
				}
			}
		}

		exchange(_curr_pairs, _prev_pairs);
	}

	void unit_destroyed_callback(UnitId unit)
	{
		{
			ActorInstance first = actor(unit);
			if (is_valid(first))
				actor_destroy(first);
		}

		{
			ColliderInstance ci = collider_instance(unit);
			if (is_valid(ci))
				collider_destroy(ci);
		}

		{
			MoverInstance mi = mover(unit);
			if (is_valid(mi))
				mover_destroy(mi);
		}
	}

	static void tick_cb(btDynamicsWorld *world, btScalar dt)
	{
		PhysicsWorldImpl *bw = static_cast<PhysicsWorldImpl *>(world->getWorldUserInfo());
		bw->tick_callback(world, dt);
	}

	static void unit_destroyed_callback(UnitId unit, void *user_ptr)
	{
		static_cast<PhysicsWorldImpl *>(user_ptr)->unit_destroyed_callback(unit);
	}

	static ColliderInstance make_collider_instance(u32 i)
	{
		ColliderInstance inst = { i }; return inst;
	}

	static ActorInstance make_actor_instance(u32 i)
	{
		ActorInstance inst = { i }; return inst;
	}

	static MoverInstance make_mover_instance(u32 i)
	{
		MoverInstance inst = { i }; return inst;
	}

	static JointInstance make_joint_instance(u32 i)
	{
		JointInstance inst = { i }; return inst;
	}

	void collision_object_debug_draw(DebugLine *lines, const btCollisionObject *object, const Color4 &color)
	{
		DebugLine *old_lines = _debug_drawer._lines;
		_debug_drawer._lines = lines;
		_dynamics_world->debugDrawObject(object->m_worldTransform
			, object->m_collisionShape
			, to_btVector3(color)
			);
		_debug_drawer._lines = old_lines;
	}
};

PhysicsWorld::PhysicsWorld(Allocator &a, ResourceManager &rm, UnitManager &um, SceneGraph &sg, DebugLine &dl)
	: _marker(PHYSICS_WORLD_MARKER)
	, _allocator(&a)
	, _impl(NULL)
{
	_impl = CE_NEW(*_allocator, PhysicsWorldImpl)(a, rm, um, sg, dl);
}

PhysicsWorld::~PhysicsWorld()
{
	CE_DELETE(*_allocator, _impl);
	_marker = 0;
}

void PhysicsWorld::collider_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
{
	_impl->collider_create_instances(components_data, num, unit_lookup, unit_index);
}

void PhysicsWorld::collider_destroy(ColliderInstance collider)
{
	_impl->collider_destroy(collider);
}

ColliderInstance PhysicsWorld::collider_instance(UnitId unit)
{
	return _impl->collider_instance(unit);
}

void PhysicsWorld::actor_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
{
	_impl->actor_create_instances(components_data, num, unit_lookup, unit_index);
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

void PhysicsWorld::actor_teleport_world_position(ActorInstance actor, const Vector3 &p)
{
	_impl->actor_teleport_world_position(actor, p);
}

void PhysicsWorld::actor_teleport_world_rotation(ActorInstance actor, const Quaternion &r)
{
	_impl->actor_teleport_world_rotation(actor, r);
}

void PhysicsWorld::actor_teleport_world_pose(ActorInstance actor, const Matrix4x4 &m)
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

void PhysicsWorld::actor_set_linear_velocity(ActorInstance actor, const Vector3 &vel)
{
	_impl->actor_set_linear_velocity(actor, vel);
}

Vector3 PhysicsWorld::actor_angular_velocity(ActorInstance actor) const
{
	return _impl->actor_angular_velocity(actor);
}

void PhysicsWorld::actor_set_angular_velocity(ActorInstance actor, const Vector3 &vel)
{
	_impl->actor_set_angular_velocity(actor, vel);
}

void PhysicsWorld::actor_add_impulse(ActorInstance actor, const Vector3 &impulse)
{
	_impl->actor_add_impulse(actor, impulse);
}

void PhysicsWorld::actor_add_impulse_at(ActorInstance actor, const Vector3 &impulse, const Vector3 &pos)
{
	_impl->actor_add_impulse_at(actor, impulse, pos);
}

void PhysicsWorld::actor_add_torque_impulse(ActorInstance actor, const Vector3 &imp)
{
	_impl->actor_add_torque_impulse(actor, imp);
}

void PhysicsWorld::actor_push(ActorInstance actor, const Vector3 &vel, f32 mass)
{
	_impl->actor_push(actor, vel, mass);
}

void PhysicsWorld::actor_push_at(ActorInstance actor, const Vector3 &vel, f32 mass, const Vector3 &pos)
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

void PhysicsWorld::actor_debug_draw(ActorInstance actor, DebugLine *lines, const Color4 &color)
{
	_impl->actor_debug_draw(actor, lines, color);
}

void PhysicsWorld::mover_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
{
	_impl->mover_create_instances(components_data, num, unit_lookup, unit_index);
}

MoverInstance PhysicsWorld::mover_create(UnitId unit, const MoverDesc *desc)
{
	return _impl->mover_create(unit, desc);
}

void PhysicsWorld::mover_destroy(MoverInstance actor)
{
	_impl->mover_destroy(actor);
}

MoverInstance PhysicsWorld::mover(UnitId unit)
{
	return _impl->mover(unit);
}

void PhysicsWorld::mover_set_height(MoverInstance mover, float height)
{
	_impl->mover_set_height(mover, height);
}

f32 PhysicsWorld::mover_radius(MoverInstance mover)
{
	return _impl->mover_radius(mover);
}

void PhysicsWorld::mover_set_radius(MoverInstance mover, float radius)
{
	_impl->mover_set_radius(mover, radius);
}

f32 PhysicsWorld::mover_max_slope_angle(MoverInstance mover)
{
	return _impl->mover_max_slope_angle(mover);
}

void PhysicsWorld::mover_set_max_slope_angle(MoverInstance mover, f32 angle)
{
	_impl->mover_set_max_slope_angle(mover, angle);
}

void PhysicsWorld::mover_set_collision_filter(MoverInstance mover, StringId32 filter)
{
	_impl->mover_set_collision_filter(mover, filter);
}

Vector3 PhysicsWorld::mover_position(MoverInstance mover)
{
	return _impl->mover_position(mover);
}

void PhysicsWorld::mover_set_position(MoverInstance mover, const Vector3 &position)
{
	return _impl->mover_set_position(mover, position);
}

Vector3 PhysicsWorld::mover_center(MoverInstance mover)
{
	return _impl->mover_center(mover);
}

void PhysicsWorld::mover_set_center(MoverInstance mover, const Vector3 &center)
{
	_impl->mover_set_center(mover, center);
}

void PhysicsWorld::mover_move(MoverInstance mover, const Vector3 &delta)
{
	_impl->mover_move(mover, delta);
}

bool PhysicsWorld::mover_collides_sides(MoverInstance mover)
{
	return _impl->mover_collides_sides(mover);
}

bool PhysicsWorld::mover_collides_up(MoverInstance mover)
{
	return _impl->mover_collides_up(mover);
}

bool PhysicsWorld::mover_collides_down(MoverInstance mover)
{
	return _impl->mover_collides_down(mover);
}

void PhysicsWorld::mover_debug_draw(MoverInstance mover, DebugLine *lines, const Color4 &color)
{
	_impl->mover_debug_draw(mover, lines, color);
}

JointInstance PhysicsWorld::joint_create(ActorInstance a0, ActorInstance a1, const JointDesc &jd)
{
	return _impl->joint_create(a0, a1, jd);
}

void PhysicsWorld::joint_destroy(JointInstance i)
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

#endif // if CROWN_PHYSICS_BULLET
