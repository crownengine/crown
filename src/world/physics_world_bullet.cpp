/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PHYSICS_BULLET
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/hash_set.inl"
#include "core/event_stream.inl"
#include "core/json/sjson.h"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/quaternion.inl"
#include "core/math/vector3.inl"
#include "core/memory/pool_allocator.h"
#include "core/memory/proxy_allocator.h"
#include "core/memory/temp_allocator.inl"
#include "core/profiler.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.h"
#include "core/strings/string_id.inl"
#include "device/console_server.h"
#include "device/log.h"
#include "resource/physics_resource.inl"
#include "resource/resource_manager.h"
#include "world/debug_line.h"
#include "world/physics.h"
#include "world/physics_world.h"
#include "world/scene_graph.h"
#include "world/unit_manager.h"
#if CROWN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4244) // conversion from 'double' to 'float', possible loss of data
#endif
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
#if CROWN_COMPILER_MSVC
#pragma warning(pop)
#endif

LOG_SYSTEM(PHYSICS, "physics")

namespace crown
{
namespace physics_globals
{
	struct DebugFlags
	{
		enum Enum : u32
		{
			COLLISION_SHAPES = 1u << 0,
			CONTACT_POINT    = 1u << 3,
			CONTACT_NORMAL   = 1u << 4,
		};
	};

	static ProxyAllocator *_linear_allocator;
	static ProxyAllocator *_heap_allocator;
	static bool _debug_enabled;
	static u32 _debug_flags;

	static u32 parameter_flag(StringId32 parameter)
	{
		if (parameter == STRING_ID_32("collision_shapes", UINT32_C(0xac134232)))
			return DebugFlags::COLLISION_SHAPES;
		if (parameter == STRING_ID_32("contact_point", UINT32_C(0x5766de26)))
			return DebugFlags::CONTACT_POINT;
		if (parameter == STRING_ID_32("contact_normal", UINT32_C(0x53b44275)))
			return DebugFlags::CONTACT_NORMAL;
		return 0u;
	}

	static int debug_mode()
	{
		int mode = btIDebugDraw::DBG_NoDebug;

		if (_debug_flags & DebugFlags::COLLISION_SHAPES)
			mode |= btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_FastWireframe;
		if (_debug_flags & (DebugFlags::CONTACT_POINT | DebugFlags::CONTACT_NORMAL))
			mode |= btIDebugDraw::DBG_DrawContactPoints;

		return mode;
	}

	static void command_help()
	{
		logi(PHYSICS, "debug <ON/OFF>      Enable or disable physics debug drawing.");
		logi(PHYSICS, "show <PARAMETER>    Show a physics visualization parameter.");
		logi(PHYSICS, "hide <PARAMETER>    Hide a physics visualization parameter.");
	}

	static void console_command_physics(ConsoleServer &cs, u32 client_id, const JsonArray &args, void * /*user_data*/)
	{
		TempAllocator1024 ta;

		if (array::size(args) < 2) {
			cs.error(client_id, "Usage: physics <debug|show|hide> ...");
			return;
		}

		DynamicString subcmd(ta);
		sjson::parse_string(subcmd, args[1]);

		if (subcmd == "help") {
			command_help();
			return;
		}

		if (subcmd == "debug") {
			if (array::size(args) != 3) {
				cs.error(client_id, "Usage: physics debug <ON/OFF>");
				return;
			}

			DynamicString value(ta);
			sjson::parse_string(value, args[2]);
			if (value.length() == 2 && strncasecmp(value.c_str(), "on", 2) == 0) {
				_debug_enabled = true;
			} else if (value.length() == 3 && strncasecmp(value.c_str(), "off", 3) == 0) {
				_debug_enabled = false;
			} else {
				cs.error(client_id, "Usage: physics debug <ON/OFF>");
				return;
			}
			return;
		}

		if (subcmd == "show" || subcmd == "hide") {
			if (array::size(args) != 3) {
				cs.error(client_id, "Usage: physics <show|hide> <PARAMETER>");
				return;
			}

			DynamicString parameter_name(ta);
			sjson::parse_string(parameter_name, args[2]);

			const u32 flag = parameter_flag(parameter_name.to_string_id());
			if (flag == 0u) {
				cs.error(client_id, "Unknown physics visualization parameter");
				return;
			}

			if (subcmd == "show")
				_debug_flags |= flag;
			else
				_debug_flags &= ~flag;
			return;
		}

		cs.error(client_id, "Unknown physics command");
	}

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

	void init(Allocator &linear, Allocator &heap, ConsoleServer &cs, const PhysicsSettings *settings)
	{
		_linear_allocator = CE_NEW(linear, ProxyAllocator)(linear, "physics");
		_heap_allocator = CE_NEW(*_linear_allocator, ProxyAllocator)(heap, "physics");
		_debug_enabled = false;
		_debug_flags = DebugFlags::COLLISION_SHAPES
		;

		btAlignedAllocSetCustom(alloc_func, free_func);
		btAlignedAllocSetCustomAligned(aligned_alloc_func, aligned_free_func);

		btSetCustomEnterProfileZoneFunc(profile_scope_enter_func);
		btSetCustomLeaveProfileZoneFunc(profile_scope_leave_func);

		_settings         = *settings;
		_bt_configuration = CE_NEW(*_linear_allocator, btDefaultCollisionConfiguration);
		_bt_dispatcher    = CE_NEW(*_linear_allocator, btCollisionDispatcher)(_bt_configuration);
		_bt_interface     = CE_NEW(*_linear_allocator, btDbvtBroadphase);
		_bt_solver        = CE_NEW(*_linear_allocator, btSequentialImpulseConstraintSolver);

		cs.register_command_name("physics", "Configure physics debug visualization.", console_command_physics, NULL);
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

static void draw_mover_step_quad(DebugLine *lines, const Vector3 &bottom, const Vector3 &up, f32 step_quad_half, f32 step_height, f32 half_scale, const Color4 &color)
{
	const Vector3 step_center = bottom + up * step_height;
	lines->add_line(bottom, step_center, color);
	const f32 half = step_quad_half * half_scale;
	const Vector3 q0 = step_center + Vector3{ -half, -half, 0.0f };
	const Vector3 q1 = step_center + Vector3{  half, -half, 0.0f };
	const Vector3 q2 = step_center + Vector3{  half,  half, 0.0f };
	const Vector3 q3 = step_center + Vector3{ -half,  half, 0.0f };
	lines->add_line(q0, q1, color);
	lines->add_line(q1, q2, color);
	lines->add_line(q2, q3, color);
	lines->add_line(q3, q0, color);
}

static void draw_mover_slope_lines(DebugLine *lines, const Vector3 &bottom, const Vector3 &up, btScalar capsule_radius, f32 slope_angle, const Color4 &color)
{
	const f32 slope_length = btMax(0.30f, (f32)capsule_radius * 2.0f);
	const Vector3 cardinals[] = { VECTOR3_XAXIS, -VECTOR3_XAXIS, VECTOR3_YAXIS, -VECTOR3_YAXIS };

	for (u32 i = 0; i < countof(cardinals); ++i) {
		Vector3 slope_dir = cardinals[i] * fcos(slope_angle) + up * fsin(slope_angle);
		slope_dir = normalize(slope_dir);
		lines->add_line(bottom, bottom + slope_dir * slope_length, color);
	}
}

struct MyDebugDrawer : public btIDebugDraw
{
	DebugLine *_lines;
	DefaultColors _colors;

	explicit MyDebugDrawer(DebugLine &dl)
		: _lines(&dl)
	{
		_colors.m_activeObject = to_btVector3(COLOR4_ORANGE);
		_colors.m_deactivatedObject = btVector3(0.45f, 0.65f, 1.0f);
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
		const btVector3 visible = color.length2() == btScalar(0.0f)
			? to_btVector3(COLOR4_ORANGE)
			: color;
		_lines->add_line(start, end, { visible.x, visible.y, visible.z, 1.0f });
	}

	void drawContactPoint(const btVector3 &pointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color) override
	{
		CE_UNUSED_2(distance, lifeTime);
		const Vector3 from = to_vector3(pointOnB);
		const Color4 debug_color = { color.x, color.y, color.z, 1.0f };
		if (physics_globals::_debug_flags & physics_globals::DebugFlags::CONTACT_POINT)
			_lines->add_sphere(from, 0.1f, debug_color);
		if (physics_globals::_debug_flags & physics_globals::DebugFlags::CONTACT_NORMAL) {
			const Vector3 normal = to_vector3(normalOnB);
			_lines->add_line(from, from + normal * 0.1f, debug_color);
		}
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
		return physics_globals::debug_mode();
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
		collides = collides || (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);
		return collides;
	}
};

class MoverClosestNotMeConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
	btCollisionObject *_me;
	explicit MoverClosestNotMeConvexResultCallback(btCollisionObject *me)
		: btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
		, _me(me)
	{
	}

	btScalar addSingleResult(btCollisionWorld::LocalConvexResult &convex_result, bool normal_in_world_space) override
	{
		if (convex_result.m_hitCollisionObject == _me)
			return btScalar(1.0);

		if (!convex_result.m_hitCollisionObject->hasContactResponse()
			&& btGhostObject::upcast(convex_result.m_hitCollisionObject) == NULL)
			return btScalar(1.0);

		return ClosestConvexResultCallback::addSingleResult(convex_result, normal_in_world_space);
	}

	bool needsCollision(btBroadphaseProxy *proxy0) const override
	{
		bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
		collides = collides || (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
		return collides;
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

struct Mover
{
	static constexpr btScalar MIN_VECTOR_LENGTH_SQUARED = btScalar(0.00000001f);
	static constexpr btScalar SEPARATION_TOLERANCE = btScalar(0.000001f);
	static constexpr btScalar MIN_TRAVEL_DISTANCE = btScalar(0.000001f);
	static constexpr btScalar PENETRATION_DEPTH_TOLERANCE = btScalar(0.0001f);
	static constexpr btScalar MIN_SWEEP_HIT_FRACTION = btScalar(0.0001f);
	static constexpr btScalar MIN_MOVEMENT_DISTANCE = btScalar(0.0001f);
	static constexpr btScalar SKIN_WIDTH = btScalar(0.001f);
	static constexpr btScalar MIN_STEP_UP_ADVANCE_FRACTION = btScalar(0.001f);

	u32 _flags;
	btCollisionWorld *_collision_world;
	btPairCachingGhostObject *_ghost;
	btConvexShape *_shape;
	btScalar _max_slope_radians;
	btScalar _max_slope_cosine;
	btScalar _step_height;
	btScalar _detected_step_height;
	btScalar _last_detected_step_height;
	btScalar _detected_slope_radians;
	btVector3 _current_position;
	btManifoldArray _manifold_array;
	u32 _down_actor_i;
	const btRigidBody *_down_actor_body;
	btVector3 _up;
	btVector3 _center;

	struct CollisionHit
	{
		const btCollisionObject *hit_object;
		btVector3 mover_position;
		btVector3 normal;
	};
	static const u32 MAX_COLLISION_HITS = 8;
	CollisionHit _collision_hits[MAX_COLLISION_HITS];
	u32 _num_collision_hits;

	BT_DECLARE_ALIGNED_ALLOCATOR();

	Mover(Allocator &allocator, btCollisionWorld *collision_world, btPairCachingGhostObject *ghost, const MoverDesc &desc, const btVector3 &up)
		: _flags(0)
		, _collision_world(collision_world)
		, _ghost(ghost)
		, _shape(CE_NEW(allocator, btCapsuleShapeZ)(desc.capsule.radius, desc.capsule.height))
		, _max_slope_radians(0.0f)
		, _max_slope_cosine(0.0f)
		, _step_height(btScalar(0.50f))
		, _detected_step_height(0.0f)
		, _last_detected_step_height(0.0f)
		, _detected_slope_radians(0.0f)
		, _current_position(ghost->m_worldTransform.m_origin)
		, _down_actor_i(UINT32_MAX)
		, _down_actor_body(NULL)
		, _up(0.0f, 0.0f, 1.0f)
		, _center(0.0f, 0.0f, 0.0f)
		, _num_collision_hits(0)
	{
		_ghost->setCollisionShape(_shape);
		CE_ASSERT(_ghost->m_collisionShape == _shape, "Mover ghost shape out of sync");
		CE_ASSERT(_shape->m_shapeType == CAPSULE_SHAPE_PROXYTYPE, "Mover shape must be a capsule");
		CE_ASSERT(((btCapsuleShape *)_shape)->getUpAxis() == 2, "Mover shape must be a Z capsule");
		set_up_direction(up);
		set_max_slope(desc.max_slope_angle);
		set_step_height(desc.step_height);
		set_center(to_btVector3(desc.center));
	}

	void destroy_shape(Allocator &allocator)
	{
		CE_DELETE(allocator, _shape);
	}

	static btVector3 normalized(const btVector3 &v)
	{
		return v.length() <= SIMD_EPSILON
			? btVector3(0.0f, 0.0f, 0.0f)
			: v.normalized()
			;
	}

	struct ActivePlane
	{
		btVector3 normal;
		bool walkable;
	};

	static btVector3 clip_velocity_to_plane(const btVector3 &velocity, const btVector3 &normal)
	{
		btVector3 clipped = velocity - normal * (velocity.dot(normal) * btScalar(1.001f));
		const btScalar into = clipped.dot(normal);
		if (into < btScalar(0.0f))
			clipped -= normal * into;
		return clipped;
	}

	btVector3 separation_normal_for_plane(const btVector3 &normal, bool walkable, bool moving_up) const
	{
		if (!walkable && !moving_up && normal.dot(_up) > btScalar(0.0f)) {
			btVector3 lateral = normal - _up * normal.dot(_up);
			if (lateral.length2() > MIN_VECTOR_LENGTH_SQUARED)
				return lateral.normalized();
			return lateral;
		}
		return normal;
	}

	void add_active_plane(ActivePlane *planes, int &plane_count, int max_planes, const btVector3 &move_delta_normalized, const btVector3 &normal, bool walkable) const
	{
		for (int i = 0; i < plane_count; ++i) {
			if (planes[i].normal.dot(normal) >= btScalar(0.99f)) {
				planes[i].walkable = planes[i].walkable || walkable;
				if (planes[i].normal.dot(_up) < normal.dot(_up))
					planes[i].normal = normal;
				return;
			}
		}

		if (plane_count < max_planes) {
			planes[plane_count].normal = normal;
			planes[plane_count].walkable = walkable;
			++plane_count;
			return;
		}

		int replace = 0;
		btScalar replace_score = planes[0].normal.dot(move_delta_normalized);
		for (int i = 1; i < plane_count; ++i) {
			const btScalar score = planes[i].normal.dot(move_delta_normalized);
			if (score < replace_score) {
				replace = i;
				replace_score = score;
			}
		}
		planes[replace].normal = normal;
		planes[replace].walkable = walkable;
	}

	btVector3 solve_contact_separation(const ActivePlane *planes, int plane_count, btScalar skin_width, bool moving_up) const
	{
		btVector3 separation(0.0f, 0.0f, 0.0f);
		bool has_walkable_plane = false;
		bool has_nonwalkable_plane = false;
		for (int i = 0; i < plane_count; ++i) {
			has_walkable_plane |= planes[i].walkable;
			has_nonwalkable_plane |= !planes[i].walkable;
		}
		for (int iteration = 0; iteration < plane_count * 2; ++iteration) {
			bool changed = false;
			for (int i = 0; i < plane_count; ++i) {
				const btVector3 separation_normal = separation_normal_for_plane(planes[i].normal, planes[i].walkable, moving_up);
				const btScalar normal_len2 = separation_normal.length2();
				if (normal_len2 <= MIN_VECTOR_LENGTH_SQUARED)
					continue;

				const btScalar distance = separation.dot(separation_normal);
				if (distance + SEPARATION_TOLERANCE < skin_width) {
					separation += separation_normal * ((skin_width - distance) / normal_len2);
					changed = true;
				}
			}
			if (!changed)
				break;
		}
		if (has_walkable_plane && has_nonwalkable_plane && !moving_up) {
			const btScalar separation_down = separation.dot(_up);
			if (separation_down < btScalar(0.0f))
				separation -= _up * separation_down;
		}
		return separation;
	}

	btVector3 slide_velocity(const btVector3 &velocity, const ActivePlane *planes, int plane_count, bool has_lateral_command, bool moving_up)
	{
		if (plane_count <= 0)
			return velocity;

		bool has_walkable_plane = false;
		bool has_nonwalkable_plane = false;
		for (int i = 0; i < plane_count; ++i) {
			has_walkable_plane |= planes[i].walkable;
			has_nonwalkable_plane |= !planes[i].walkable;
		}

		if (!has_lateral_command && has_walkable_plane)
			return btVector3(0.0f, 0.0f, 0.0f);

		btVector3 slide = velocity;
		for (int iteration = 0; iteration < plane_count * 2; ++iteration) {
			bool changed = false;
			for (int i = 0; i < plane_count; ++i) {
				if (slide.dot(planes[i].normal) < btScalar(0.0f)) {
					slide = clip_velocity_to_plane(slide, planes[i].normal);
					changed = true;
				}
			}
			if (!changed)
				break;
		}

		if (has_nonwalkable_plane) {
			const btScalar upward = slide.dot(_up);
			if (upward > btScalar(0.0f) && !moving_up)
				slide -= _up * upward;
		}
		if (has_walkable_plane && has_nonwalkable_plane) {
			const btScalar downward = slide.dot(_up);
			if (!moving_up && downward < btScalar(0.0f))
				slide -= _up * downward;
		}
		return slide;
	}

	bool needs_collision(const btCollisionObject *body0, const btCollisionObject *body1) const
	{
		if (body0 == NULL
			|| body1 == NULL
			|| body0->m_broadphaseHandle == NULL
			|| body1->m_broadphaseHandle == NULL)
			return false;

		return (body0->m_broadphaseHandle->m_collisionFilterGroup & body1->m_broadphaseHandle->m_collisionFilterMask) != 0
			|| (body1->m_broadphaseHandle->m_collisionFilterGroup & body0->m_broadphaseHandle->m_collisionFilterMask) != 0
			;
	}

	void sync_ghost_position(const btVector3 &position)
	{
		btTransform xform = _ghost->m_worldTransform;
		xform.m_origin = position + _center;
		_ghost->setWorldTransform(xform);
	}

	void update_overlapping_pairs()
	{
		CE_ASSERT(_ghost->m_broadphaseHandle != NULL, "Mover ghost must be added to the broadphase");
		btVector3 min_aabb;
		btVector3 max_aabb;
		_shape->getAabb(_ghost->m_worldTransform, min_aabb, max_aabb);
		_collision_world->getBroadphase()->setAabb(_ghost->m_broadphaseHandle, min_aabb, max_aabb, _collision_world->getDispatcher());
	}

	bool separate(btVector3 &separation_delta)
	{
		const btTransform saved_transform = _ghost->m_worldTransform;
		const btVector3 start_position = saved_transform.m_origin - _center;
		btVector3 working_position = start_position;
		separation_delta.setValue(0.0f, 0.0f, 0.0f);
		bool collided = false;

		for (int loop = 0; loop < 4; ++loop) {
			sync_ghost_position(working_position);
			update_overlapping_pairs();
			_collision_world->getDispatcher()->dispatchAllCollisionPairs(_ghost->getOverlappingPairCache(), _collision_world->getDispatchInfo(), _collision_world->getDispatcher());

			btVector3 loop_offset(0.0f, 0.0f, 0.0f);
			bool loop_collided = false;

			for (int i = 0; i < _ghost->getOverlappingPairCache()->getNumOverlappingPairs(); ++i) {
				_manifold_array.resizeNoInitialize(0);
				btBroadphasePair &collision_pair = _ghost->getOverlappingPairCache()->getOverlappingPairArray()[i];
				btCollisionObject *obj0 = (btCollisionObject *)collision_pair.m_pProxy0->m_clientObject;
				btCollisionObject *obj1 = (btCollisionObject *)collision_pair.m_pProxy1->m_clientObject;

				if ((obj0 != NULL && !obj0->hasContactResponse())
					|| (obj1 != NULL && !obj1->hasContactResponse())
					|| !needs_collision(obj0, obj1))
					continue;

				if (collision_pair.m_algorithm)
					collision_pair.m_algorithm->getAllContactManifolds(_manifold_array);

				for (int j = 0; j < _manifold_array.size(); ++j) {
					btPersistentManifold *manifold = _manifold_array[j];
					const btScalar direction_sign = manifold->getBody0() == _ghost ? btScalar(-1.0f) : btScalar(1.0f);
					for (int p = 0; p < manifold->getNumContacts(); ++p) {
						const btManifoldPoint &pt = manifold->getContactPoint(p);
						if (pt.getDistance() < -PENETRATION_DEPTH_TOLERANCE) {
							loop_offset += pt.m_normalWorldOnB * direction_sign * pt.getDistance() * btScalar(0.2f);
							loop_collided = true;
						}
					}
				}
			}

			if (!loop_collided)
				break;

			collided = true;
			working_position += loop_offset;
			if (loop_offset.length2() <= SIMD_EPSILON)
				break;
		}

		if (collided)
			separation_delta = working_position - start_position;

		_ghost->setWorldTransform(saved_transform);
		update_overlapping_pairs();
		return collided;
	}

	bool fits_at(const btVector3 &position, btScalar min_distance = btScalar(0.0f))
	{
		const btTransform saved_transform = _ghost->m_worldTransform;

		sync_ghost_position(position);
		update_overlapping_pairs();
		_collision_world->getDispatcher()->dispatchAllCollisionPairs(_ghost->getOverlappingPairCache(), _collision_world->getDispatchInfo(), _collision_world->getDispatcher());

		bool fits = true;
		for (int i = 0; i < _ghost->getOverlappingPairCache()->getNumOverlappingPairs() && fits; ++i) {
			_manifold_array.resizeNoInitialize(0);

			btBroadphasePair &collision_pair = _ghost->getOverlappingPairCache()->getOverlappingPairArray()[i];
			btCollisionObject *obj0 = (btCollisionObject *)collision_pair.m_pProxy0->m_clientObject;
			btCollisionObject *obj1 = (btCollisionObject *)collision_pair.m_pProxy1->m_clientObject;

			if ((obj0 != NULL && !obj0->hasContactResponse())
				|| (obj1 != NULL && !obj1->hasContactResponse())
				|| !needs_collision(obj0, obj1))
				continue;

			if (collision_pair.m_algorithm)
				collision_pair.m_algorithm->getAllContactManifolds(_manifold_array);

			for (int j = 0; j < _manifold_array.size() && fits; ++j) {
				btPersistentManifold *manifold = _manifold_array[j];
				for (int p = 0; p < manifold->getNumContacts(); ++p) {
					if (manifold->getContactPoint(p).getDistance()
						+ (obj0->m_collisionShape != NULL ? obj0->m_collisionShape->getMargin() : btScalar(0.0f))
						+ (obj1->m_collisionShape != NULL ? obj1->m_collisionShape->getMargin() : btScalar(0.0f)) < min_distance) {
						fits = false;
						break;
					}
				}
			}
		}

		_ghost->setWorldTransform(saved_transform);
		update_overlapping_pairs();
		return fits;
	}

	void set_down_actor(const btCollisionObject *hit_object)
	{
		const btRigidBody *body = btRigidBody::upcast(hit_object);
		if (body == NULL) {
			_down_actor_i = UINT32_MAX;
			_down_actor_body = NULL;
			return;
		}

		_down_actor_i = (u32)(uintptr_t)body->m_userObjectPointer;
		_down_actor_body = body;
	}

	void classify_contact(const btVector3 &normal, const btVector3 &hit_point_world, const btVector3 &position, const btCollisionObject *hit_object)
	{
		const btScalar up_dot = normal.dot(_up);
		const btCapsuleShapeZ *capsule = (const btCapsuleShapeZ *)_shape;
		const btScalar hit_height_from_bottom = (hit_point_world - (position + _center)).dot(_up) + capsule->getHalfHeight() + capsule->getRadius();

		if (hit_height_from_bottom <= btMax(btScalar(0.01f), capsule->getRadius() * btScalar(0.10f)) && up_dot > btScalar(0.0f)) {
			_flags |= MoverFlags::COLLIDES_DOWN;
			set_down_actor(hit_object);
			return;
		}

		if (up_dot >= _max_slope_cosine) {
			_flags |= MoverFlags::COLLIDES_DOWN;
			set_down_actor(hit_object);
		} else if (up_dot <= -_max_slope_cosine) {
			_flags |= MoverFlags::COLLIDES_UP;
		} else {
			_flags |= MoverFlags::COLLIDES_SIDES;
		}
	}

	bool is_step_like_contact(const btVector3 &hit_point_world, const btVector3 &position, btScalar *detected_height = NULL) const
	{
		const btCapsuleShapeZ *capsule = (const btCapsuleShapeZ *)_shape;
		const btScalar radius = capsule->getRadius();
		const btVector3 to_hit = hit_point_world - (position + _center);
		const btScalar axial = to_hit.dot(_up);
		const btScalar bottom = -(capsule->getHalfHeight() + radius);
		if (axial > bottom + _step_height + btScalar(0.005f))
			return false;

		const btScalar hit_height_from_bottom = axial - bottom;
		if (detected_height != NULL)
			*detected_height = hit_height_from_bottom;
		if (hit_height_from_bottom < btScalar(0.03f) || hit_height_from_bottom > _step_height)
			return false;

		return (to_hit - _up * axial).length() >= btMax(btScalar(0.02f), radius * btScalar(0.20f));
	}

	bool try_step_up(btVector3 &current_position, btVector3 &remaining, btScalar skin_width, bool was_on_ground, bool moving_up)
	{
		if (!was_on_ground || _step_height <= btScalar(0.0f) || moving_up)
			return false;

		const btVector3 lateral = remaining - _up * remaining.dot(_up);
		const btScalar lateral_len = lateral.length();
		if (lateral_len <= MIN_TRAVEL_DISTANCE)
			return false;

		btVector3 stepped = current_position;

		{
			MoverClosestNotMeConvexResultCallback cb(_ghost);
			cb.m_collisionFilterGroup = _ghost->m_broadphaseHandle->m_collisionFilterGroup;
			cb.m_collisionFilterMask = _ghost->m_broadphaseHandle->m_collisionFilterMask;
			_collision_world->convexSweepTest(_shape
				, btTransform(btQuaternion::getIdentity(), stepped + _center)
				, btTransform(btQuaternion::getIdentity(), stepped + _up * _step_height + _center)
				, cb, _collision_world->getDispatchInfo().m_allowedCcdPenetration);

			if (cb.hasHit()) {
				CE_ASSERT(cb.m_hitCollisionObject != NULL, "Mover step-up sweep hit missing collision object");
				const btScalar up_hit = btMax(btScalar(0.0f), btMin(cb.m_closestHitFraction, btScalar(1.0f)));
				if (up_hit <= MIN_SWEEP_HIT_FRACTION)
					return false;
				stepped += _up * (_step_height * btMax(btScalar(0.0f), up_hit - skin_width / btMax(_step_height, MIN_TRAVEL_DISTANCE)));
			} else {
				stepped += _up * _step_height;
			}
		}

		btVector3 advanced = stepped;
		{
			MoverClosestNotMeConvexResultCallback cb(_ghost);
			cb.m_collisionFilterGroup = _ghost->m_broadphaseHandle->m_collisionFilterGroup;
			cb.m_collisionFilterMask = _ghost->m_broadphaseHandle->m_collisionFilterMask;
			_collision_world->convexSweepTest(_shape
				, btTransform(btQuaternion::getIdentity(), stepped + _center)
				, btTransform(btQuaternion::getIdentity(), stepped + lateral + _center)
				, cb, _collision_world->getDispatchInfo().m_allowedCcdPenetration);

			if (cb.hasHit()) {
				CE_ASSERT(cb.m_hitCollisionObject != NULL, "Mover step-up lateral hit missing collision object");
				const btScalar lat_safe = btMax(btScalar(0.0f), btMin(cb.m_closestHitFraction, btScalar(1.0f)) - skin_width / btMax(lateral_len, MIN_TRAVEL_DISTANCE));
				if (lat_safe <= MIN_STEP_UP_ADVANCE_FRACTION)
					return false;
				advanced += lateral * lat_safe;
			} else {
				advanced += lateral;
			}
		}

		{
			const btCapsuleShapeZ *capsule_shape = (const btCapsuleShapeZ *)_shape;
			const btVector3 lateral_dir = lateral / lateral_len;
			const btVector3 check_pos = stepped + lateral_dir * capsule_shape->getRadius();

			MoverClosestNotMeConvexResultCallback cb(_ghost);
			cb.m_collisionFilterGroup = _ghost->m_broadphaseHandle->m_collisionFilterGroup;
			cb.m_collisionFilterMask = _ghost->m_broadphaseHandle->m_collisionFilterMask;
			_collision_world->convexSweepTest(_shape
				, btTransform(btQuaternion::getIdentity(), check_pos + _center)
				, btTransform(btQuaternion::getIdentity(), check_pos - _up * (_step_height + btScalar(0.05f)) + _center)
				, cb, _collision_world->getDispatchInfo().m_allowedCcdPenetration);

			if (!cb.hasHit())
				return false;
			CE_ASSERT(cb.m_hitCollisionObject != NULL, "Mover step-up landing hit missing collision object");

			if (cb.m_hitNormalWorld.length2() <= SIMD_EPSILON)
				return false;
			if (cb.m_hitNormalWorld.normalized().dot(_up) < _max_slope_cosine + btScalar(0.01f))
				return false;

			current_position.setInterpolate3(advanced, advanced - _up * (_step_height + btScalar(0.05f)), cb.m_closestHitFraction);
			_flags |= MoverFlags::COLLIDES_DOWN;
			remaining.setValue(0.0f, 0.0f, 0.0f);
			return true;
		}
	}

	void sweep_ground(bool moving_up, bool snap)
	{
		if (moving_up)
			return;

		const btScalar probe_distance = btMax(_step_height, btScalar(0.05f));

		MoverClosestNotMeConvexResultCallback cb(_ghost);
		cb.m_collisionFilterGroup = _ghost->m_broadphaseHandle->m_collisionFilterGroup;
		cb.m_collisionFilterMask = _ghost->m_broadphaseHandle->m_collisionFilterMask;
		_collision_world->convexSweepTest(_shape
			, btTransform(btQuaternion::getIdentity(), _current_position + _center)
			, btTransform(btQuaternion::getIdentity(), _current_position + _center - _up * probe_distance)
			, cb, _collision_world->getDispatchInfo().m_allowedCcdPenetration);
		if (!cb.hasHit())
			return;
		CE_ASSERT(cb.m_hitCollisionObject != NULL, "Mover ground probe hit missing collision object");

		if (cb.m_hitNormalWorld.length2() <= SIMD_EPSILON)
			return;
		if (cb.m_hitNormalWorld.normalized().dot(_up) < _max_slope_cosine)
			return;

		if (snap)
			_current_position.setInterpolate3(_current_position, _current_position - _up * probe_distance, cb.m_closestHitFraction);

		_flags |= MoverFlags::COLLIDES_DOWN;
		set_down_actor(cb.m_hitCollisionObject);
		if (_num_collision_hits < MAX_COLLISION_HITS) {
			_collision_hits[_num_collision_hits].hit_object = cb.m_hitCollisionObject;
			_collision_hits[_num_collision_hits].mover_position = _current_position;
			_collision_hits[_num_collision_hits].normal = cb.m_hitNormalWorld.normalized();
			++_num_collision_hits;
		}
	}

	void move(const btVector3 &delta)
	{
		CE_ASSERT(_ghost->hasContactResponse(), "Mover ghost must have contact response");
		CE_ASSERT(_ghost->m_collisionShape == _shape, "Mover ghost shape out of sync");
		CE_ASSERT(_shape->m_shapeType == CAPSULE_SHAPE_PROXYTYPE, "Mover shape must be a capsule");
		CE_ASSERT(((btCapsuleShape *)_shape)->getUpAxis() == 2, "Mover shape must be a Z capsule");
		_detected_step_height = 0.0f;
		_last_detected_step_height = 0.0f;
		_detected_slope_radians = 0.0f;
		_num_collision_hits = 0;
		_down_actor_i = UINT32_MAX;
		_down_actor_body = NULL;

		const btVector3 move_delta_normalized = normalized(delta);
		const btScalar vertical_delta = delta.dot(_up);
		const bool was_on_ground = (_flags &MoverFlags::COLLIDES_DOWN) != 0;
		const bool moving_up = vertical_delta > btScalar(0.0f);
		const bool has_lateral_command = (delta - _up * vertical_delta).length2() > MIN_VECTOR_LENGTH_SQUARED;

		_current_position = _ghost->m_worldTransform.m_origin - _center;
		_flags = 0u;

		sync_ghost_position(_current_position);
		update_overlapping_pairs();
		const btVector3 start_position = _current_position;

		btVector3 current_position = _current_position;
		btVector3 remaining = delta;
		const btScalar min_movement = MIN_MOVEMENT_DISTANCE;
		const btScalar skin_width = SKIN_WIDTH;
		ActivePlane active_planes[4];
		int active_plane_count = 0;

		for (int i = 0; i < 8; ++i) {
			const btScalar length = remaining.length();
			if (length <= min_movement)
				break;

			MoverClosestNotMeConvexResultCallback callback(_ghost);
			callback.m_collisionFilterGroup = _ghost->m_broadphaseHandle->m_collisionFilterGroup;
			callback.m_collisionFilterMask = _ghost->m_broadphaseHandle->m_collisionFilterMask;
			_collision_world->convexSweepTest(_shape
				, btTransform(btQuaternion::getIdentity(), current_position + _center)
				, btTransform(btQuaternion::getIdentity(), current_position + remaining + _center)
				, callback, _collision_world->getDispatchInfo().m_allowedCcdPenetration);

			if (!callback.hasHit()) {
				current_position += remaining;
				remaining.setValue(0.0f, 0.0f, 0.0f);
				break;
			}

			CE_ASSERT(callback.m_hitCollisionObject != NULL, "Mover sweep hit missing collision object");

			const btScalar hit_fraction = btMax(btScalar(0.0f), btMin(callback.m_closestHitFraction, btScalar(1.0f)));
			const btScalar safe_fraction = btMax(btScalar(0.0f), hit_fraction - skin_width / btMax(length, MIN_TRAVEL_DISTANCE));
			current_position += remaining * safe_fraction;

			btVector3 normal = callback.m_hitNormalWorld;
			if (normal.length2() <= SIMD_EPSILON)
				break;
			normal.normalize();
			const btScalar up_dot = btClamped(normal.dot(_up), btScalar(-1.0f), btScalar(1.0f));
			_detected_slope_radians = btMax(_detected_slope_radians, btScalar(facos((f32)up_dot)));
			if (_num_collision_hits < MAX_COLLISION_HITS) {
				_collision_hits[_num_collision_hits].hit_object = callback.m_hitCollisionObject;
				_collision_hits[_num_collision_hits].mover_position = current_position;
				_collision_hits[_num_collision_hits].normal = normal;
				++_num_collision_hits;
			}

			classify_contact(normal, callback.m_hitPointWorld, current_position, callback.m_hitCollisionObject);

			const bool walkable = up_dot >= _max_slope_cosine;
			if (vertical_delta > btScalar(0.0f) && was_on_ground && hit_fraction <= btScalar(0.02f) && walkable) {
				current_position += remaining * (btScalar(1.0f) - safe_fraction);
				remaining.setValue(0.0f, 0.0f, 0.0f);
				break;
			}
			if (hit_fraction <= MIN_SWEEP_HIT_FRACTION && remaining.dot(normal) >= btScalar(0.0f)) {
				current_position += remaining;
				remaining.setValue(0.0f, 0.0f, 0.0f);
				break;
			}
			if (!has_lateral_command && !moving_up && walkable) {
				remaining.setValue(0.0f, 0.0f, 0.0f);
				break;
			}

			add_active_plane(active_planes, active_plane_count, countof(active_planes), move_delta_normalized, normal, walkable);

			btScalar detected_step_height = 0.0f;
			const bool is_step_contact = is_step_like_contact(callback.m_hitPointWorld, current_position, &detected_step_height);
			if (is_step_contact) {
				_detected_step_height = btMax(_detected_step_height, detected_step_height);
				_last_detected_step_height = detected_step_height;
			}

			if (is_step_contact && try_step_up(current_position, remaining, skin_width, was_on_ground, moving_up))
				break;

			current_position += solve_contact_separation(active_planes, active_plane_count, skin_width, moving_up);
			remaining = slide_velocity(remaining * (btScalar(1.0f) - hit_fraction), active_planes, active_plane_count, has_lateral_command, moving_up);
			if (remaining.length2() <= min_movement * min_movement)
				break;
		}

		_current_position = current_position;
		sync_ghost_position(_current_position);
		sweep_ground(moving_up, true);
		sync_ghost_position(_current_position);
		sweep_ground(moving_up, false);
		if (!fits_at(_current_position, -PENETRATION_DEPTH_TOLERANCE)) {
			_current_position = start_position;
			_flags = 0u;
			_down_actor_i = UINT32_MAX;
			_down_actor_body = NULL;
			_num_collision_hits = 0;
			sync_ghost_position(_current_position);
			update_overlapping_pairs();
			sweep_ground(moving_up, false);
		}
	}

	void set_up_direction(const btVector3 &up)
	{
		if (_up == up)
			return;

		const btVector3 old_up = _up;
		_up = up.length2() > btScalar(0.0f)
			? up.normalized()
			: btVector3(0.0f, 0.0f, 0.0f)
			;

		btTransform xform = _ghost->m_worldTransform;
		xform.setRotation(rotation(_up, old_up).inverse() * xform.getRotation());
		_ghost->setWorldTransform(xform);
	}

	btQuaternion rotation(const btVector3 &v0, const btVector3 &v1) const
	{
		if (v0.length2() == btScalar(0.0f) || v1.length2() == btScalar(0.0f))
			return btQuaternion();
		btVector3 from = v0;
		btVector3 to = v1;
		return shortestArcQuatNormalize2(from, to);
	}

	void reset()
	{
		_detected_step_height = 0.0f;
		_last_detected_step_height = 0.0f;
		_detected_slope_radians = 0.0f;
		_num_collision_hits = 0;
		_down_actor_i = UINT32_MAX;
		_down_actor_body = NULL;

		btHashedOverlappingPairCache *cache = _ghost->getOverlappingPairCache();
		while (cache->getOverlappingPairArray().size() > 0)
			cache->removeOverlappingPair(cache->getOverlappingPairArray()[0].m_pProxy0, cache->getOverlappingPairArray()[0].m_pProxy1, _collision_world->getDispatcher());
	}

	void set_position(const btVector3 &origin)
	{
		reset();
		_ghost->setWorldTransform(btTransform(btQuaternion::getIdentity(), origin + _center));
		_current_position = origin;
	}

	void set_max_slope(btScalar angle)
	{
		_max_slope_radians = angle;
		_max_slope_cosine = btScalar(fcos((f32)angle));
	}

	btScalar max_slope() const
	{
		return _max_slope_radians;
	}

	void set_step_height(btScalar height)
	{
		_step_height = btMax(btScalar(0.0f), height);
	}

	btScalar radius() const
	{
		return ((btCapsuleShapeZ *)_shape)->getRadius();
	}

	void set_height_radius(Allocator &allocator, float radius, float height)
	{
		_ghost->setCollisionShape(CE_NEW(allocator, btCapsuleShapeZ)(radius, height));
		CE_DELETE(allocator, _shape);
		_shape = (btConvexShape *)_ghost->m_collisionShape;
		CE_ASSERT(((btCapsuleShape *)_shape)->getUpAxis() == 2, "Mover shape must be a Z capsule");
	}

	void set_height(Allocator &allocator, float height)
	{
		set_height_radius(allocator, ((btCapsuleShapeZ *)_shape)->getRadius(), height);
	}

	void set_radius(Allocator &allocator, float radius)
	{
		set_height_radius(allocator, radius, ((btCapsuleShapeZ *)_shape)->getHalfHeight() * 2.0f);
	}

	void set_center(const btVector3 &center)
	{
		btTransform xform = _ghost->m_worldTransform;
		xform.m_origin += center - _center;
		_center = center;
		_ghost->setWorldTransform(xform);
	}
};

struct PhysicsWorldImpl
{
	union ColliderShape
	{
		btSphereShape sphere;
		btCapsuleShape capsule;
		btCapsuleShapeZ capsule_z;
		btBoxShape box;

		///
		ColliderShape()
		{
		}

		///
		~ColliderShape()
		{
		}
	};

	struct ColliderInstanceData
	{
		UnitId unit;
		Allocator *allocator;
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

	PoolAllocator _shapes_pool;
	PoolAllocator _bodies_pool;
	PoolAllocator _motion_states_pool;
	PoolAllocator _ghosts_pool;

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
		, _shapes_pool(*_allocator, 128, sizeof(ColliderShape), 16)
		, _bodies_pool(*_allocator, 1024, sizeof(btRigidBody), 16)
		, _motion_states_pool(*_allocator, 1024, sizeof(btDefaultMotionState), 16)
		, _ghosts_pool(*_allocator, 64, sizeof(btPairCachingGhostObject), 16)
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

			inst->mover->destroy_shape(_shapes_pool);
			CE_DELETE(*_allocator, inst->mover);
			_dynamics_world->removeCollisionObject(inst->ghost);
			CE_DELETE(_ghosts_pool, inst->ghost);
		}

		for (u32 i = 0; i < array::size(_actor); ++i) {
			btRigidBody *body = _actor[i].body;

			_dynamics_world->removeRigidBody(body);
			CE_DELETE(_motion_states_pool, body->m_optionalMotionState);
			CE_DELETE(_bodies_pool, body);
		}

		for (u32 i = 0; i < array::size(_collider); ++i) {
			Allocator *allocator = _collider[i].allocator;

			CE_DELETE(*allocator, _collider[i].vertex_array);
			CE_DELETE(*allocator, _collider[i].shape);
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
			const TransformId ti = _scene_graph->instance(unit_lookup[unit_index[i]]);
			const Matrix4x4 tm = _scene_graph->world_pose(ti);

			btTriangleIndexVertexArray *vertex_array = NULL;
			btCollisionShape *shape = NULL;
			Allocator *allocator = &_shapes_pool;

			switch (cd->type) {
			case ColliderType::SPHERE:
				shape = CE_NEW(_shapes_pool, btSphereShape)(cd->sphere.radius);
				break;

			case ColliderType::CAPSULE:
				shape = CE_NEW(_shapes_pool, btCapsuleShape)(cd->capsule.radius, cd->capsule.height);
				break;

			case ColliderType::BOX:
				shape = CE_NEW(_shapes_pool, btBoxShape)(to_btVector3(cd->box.half_size));
				break;

			case ColliderType::CONVEX_HULL: {
				const u8 *data         = (u8 *)&cd[1];
				const u32 num          = *(u32 *)data;
				const btScalar *points = (btScalar *)(data + sizeof(u32));

				allocator = _allocator;
				shape = CE_NEW(*allocator, btConvexHullShape)(points, (int)num, sizeof(Vector3));
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

				allocator = _allocator;
				vertex_array = CE_NEW(*allocator, btTriangleIndexVertexArray)();
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
			cid.allocator    = allocator;
			cid.vertex_array = vertex_array;
			cid.shape        = shape;

			array::push_back(_collider, cid);
			hash_map::set(_collider_map, unit, last);

			cd = (ColliderDesc *)((char *)(cd + 1) + cd->size);
		}
	}

	void collider_destroy(ColliderId collider)
	{
		CE_ASSERT(collider.i < array::size(_collider), "Index out of bounds");

		const u32 last      = array::size(_collider) - 1;
		const UnitId u      = _collider[collider.i].unit;
		const UnitId last_u = _collider[last].unit;

		Allocator *allocator = _collider[collider.i].allocator;
		CE_DELETE(*allocator, _collider[collider.i].vertex_array);
		CE_DELETE(*allocator, _collider[collider.i].shape);

		_collider[collider.i] = _collider[last];

		array::pop_back(_collider);

		hash_map::set(_collider_map, last_u, collider.i);
		hash_map::remove(_collider_map, u);
	}

	ColliderId collider_instance(UnitId unit)
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

			TransformId ti = _scene_graph->instance(unit);
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
			ColliderId ci = collider_instance(unit);
			btCollisionShape *shape = _collider[ci.i].shape;

			// Create motion state
			const btTransform tr = to_btTransform(tm_noscale);
			btDefaultMotionState *ms = is_static
				? NULL
				: CE_NEW(_motion_states_pool, btDefaultMotionState)(tr)
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
			btRigidBody *body = CE_NEW(_bodies_pool, btRigidBody)(rbinfo);

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

	void actor_destroy(ActorId actor)
	{
		const u32 last      = array::size(_actor) - 1;
		const UnitId u      = _actor[actor.i].unit;
		const UnitId last_u = _actor[last].unit;

		_dynamics_world->removeRigidBody(_actor[actor.i].body);
		CE_DELETE(_motion_states_pool, _actor[actor.i].body->m_optionalMotionState);
		CE_DELETE(_bodies_pool, _actor[actor.i].body);

		_actor[actor.i] = _actor[last];
		_actor[actor.i].body->m_userObjectPointer = ((void *)(uintptr_t)actor.i);

		array::pop_back(_actor);

		hash_map::set(_actor_map, last_u, actor.i);
		hash_map::remove(_actor_map, u);
	}

	ActorId actor(UnitId unit)
	{
		return make_actor_instance(hash_map::get(_actor_map, unit, UINT32_MAX));
	}

	Vector3 actor_world_position(ActorId actor) const
	{
		return to_vector3(_actor[actor.i].body->getCenterOfMassPosition());
	}

	Quaternion actor_world_rotation(ActorId actor) const
	{
		return to_quaternion(_actor[actor.i].body->getOrientation());
	}

	Matrix4x4 actor_world_pose(ActorId actor) const
	{
		return to_matrix4x4(_actor[actor.i].body->getCenterOfMassTransform());
	}

	void actor_teleport_world_position(ActorId actor, const Vector3 &p)
	{
		btTransform pose = _actor[actor.i].body->getCenterOfMassTransform();
		pose.m_origin = to_btVector3(p);
		_actor[actor.i].body->setCenterOfMassTransform(pose);
	}

	void actor_teleport_world_rotation(ActorId actor, const Quaternion &r)
	{
		btTransform pose = _actor[actor.i].body->getCenterOfMassTransform();
		pose.setRotation(to_btQuaternion(r));
		_actor[actor.i].body->setCenterOfMassTransform(pose);
	}

	void actor_teleport_world_pose(ActorId actor, const Matrix4x4 &m)
	{
		const Quaternion rot = rotation(m);
		const Vector3 pos = translation(m);

		btTransform pose = _actor[actor.i].body->getCenterOfMassTransform();
		pose.setRotation(to_btQuaternion(rot));
		pose.m_origin = to_btVector3(pos);
		_actor[actor.i].body->setCenterOfMassTransform(pose);
	}

	Vector3 actor_center_of_mass(ActorId actor) const
	{
		return to_vector3(_actor[actor.i].body->getCenterOfMassTransform().m_origin);
	}

	void actor_enable_gravity(ActorId actor)
	{
		btRigidBody *body = _actor[actor.i].body;
		body->m_rigidbodyFlags = (body->m_rigidbodyFlags & ~BT_DISABLE_WORLD_GRAVITY);
		body->setGravity(_dynamics_world->getGravity());
	}

	void actor_disable_gravity(ActorId actor)
	{
		btRigidBody *body = _actor[actor.i].body;
		body->m_rigidbodyFlags = (body->m_rigidbodyFlags | BT_DISABLE_WORLD_GRAVITY);
		body->setGravity(btVector3(0.0f, 0.0f, 0.0f));
	}

	void actor_enable_collision(ActorId actor)
	{
		ActorInstanceData &a = _actor[actor.i];
		const PhysicsCollisionFilter *filters = physics_config_resource::filters_array(_config_resource);
		u32 filter_i = physics_config_resource::filter_index(filters, _config_resource->num_filters, a.resource->collision_filter);
		const PhysicsCollisionFilter *f = &filters[filter_i];

		_dynamics_world->removeRigidBody(a.body);
		_dynamics_world->addRigidBody(a.body, f->me, f->mask);
	}

	void actor_disable_collision(ActorId actor)
	{
		ActorInstanceData &a = _actor[actor.i];
		const PhysicsCollisionFilter *filters = physics_config_resource::filters_array(_config_resource);
		u32 filter_i = physics_config_resource::filter_index(filters, _config_resource->num_filters, a.resource->collision_filter);
		const PhysicsCollisionFilter *f = &filters[filter_i];

		// Disable collisions by setting collision mask to 0.
		_dynamics_world->removeRigidBody(a.body);
		_dynamics_world->addRigidBody(a.body, f->me, 0);
	}

	void actor_set_collision_filter(ActorId actor, StringId32 filter)
	{
		ActorInstanceData &a = _actor[actor.i];
		const PhysicsCollisionFilter *filters = physics_config_resource::filters_array(_config_resource);
		u32 filter_i = physics_config_resource::filter_index(filters, _config_resource->num_filters, filter);
		const PhysicsCollisionFilter *f = &filters[filter_i];

		_dynamics_world->removeRigidBody(a.body);
		_dynamics_world->addRigidBody(a.body, f->me, f->mask);
	}

	void actor_set_kinematic(ActorId actor, bool kinematic)
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

	bool actor_is_static(ActorId actor) const
	{
		return _actor[actor.i].body->m_collisionFlags & btCollisionObject::CF_STATIC_OBJECT;
	}

	bool actor_is_dynamic(ActorId actor) const
	{
		const int flags = _actor[actor.i].body->m_collisionFlags;
		return !(flags & btCollisionObject::CF_STATIC_OBJECT)
			&& !(flags & btCollisionObject::CF_KINEMATIC_OBJECT)
			;
	}

	bool actor_is_kinematic(ActorId actor) const
	{
		const int flags = _actor[actor.i].body->m_collisionFlags;
		return (flags & (btCollisionObject::CF_KINEMATIC_OBJECT)) != 0;
	}

	bool actor_is_nonkinematic(ActorId actor) const
	{
		return actor_is_dynamic(actor) && !actor_is_kinematic(actor);
	}

	f32 actor_linear_damping(ActorId actor) const
	{
		return _actor[actor.i].body->m_linearDamping;
	}

	void actor_set_linear_damping(ActorId actor, f32 rate)
	{
		_actor[actor.i].body->setDamping(rate, _actor[actor.i].body->m_angularDamping);
	}

	f32 actor_angular_damping(ActorId actor) const
	{
		return _actor[actor.i].body->m_angularDamping;
	}

	void actor_set_angular_damping(ActorId actor, f32 rate)
	{
		_actor[actor.i].body->setDamping(_actor[actor.i].body->m_linearDamping, rate);
	}

	Vector3 actor_linear_velocity(ActorId actor) const
	{
		btVector3 v = _actor[actor.i].body->m_linearVelocity;
		return to_vector3(v);
	}

	void actor_set_linear_velocity(ActorId actor, const Vector3 &vel)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->setLinearVelocity(to_btVector3(vel));
	}

	Vector3 actor_angular_velocity(ActorId actor) const
	{
		btVector3 v = _actor[actor.i].body->m_angularVelocity;
		return to_vector3(v);
	}

	void actor_set_angular_velocity(ActorId actor, const Vector3 &vel)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->setAngularVelocity(to_btVector3(vel));
	}

	void actor_add_impulse(ActorId actor, const Vector3 &impulse)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->applyCentralImpulse(to_btVector3(impulse));
	}

	void actor_add_impulse_at(ActorId actor, const Vector3 &impulse, const Vector3 &pos)
	{
		_actor[actor.i].body->activate();
		_actor[actor.i].body->applyImpulse(to_btVector3(impulse), to_btVector3(pos));
	}

	void actor_add_torque_impulse(ActorId actor, const Vector3 &imp)
	{
		_actor[actor.i].body->applyTorqueImpulse(to_btVector3(imp));
	}

	void actor_push(ActorId actor, const Vector3 &vel, f32 mass)
	{
		const Vector3 f = vel * mass;
		_actor[actor.i].body->applyCentralForce(to_btVector3(f));
	}

	void actor_push_at(ActorId actor, const Vector3 &vel, f32 mass, const Vector3 &pos)
	{
		const Vector3 f = vel * mass;
		_actor[actor.i].body->applyForce(to_btVector3(f), to_btVector3(pos));
	}

	bool actor_is_sleeping(ActorId actor)
	{
		return !_actor[actor.i].body->isActive();
	}

	void actor_wake_up(ActorId actor)
	{
		_actor[actor.i].body->activate(true);
	}

	void actor_debug_draw(ActorId actor, DebugLine *lines, const Color4 &color)
	{
		collision_object_debug_draw(lines, _actor[actor.i].body, color);
	}

	MoverId mover_create(UnitId unit, const MoverDesc *desc)
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

			const Matrix4x4 tm = _scene_graph->world_pose(_scene_graph->instance(unit));
			const PhysicsCollisionFilter *f = &filters[physics_config_resource::filter_index(filters, _config_resource->num_filters, movers[i].collision_filter)];

			btPairCachingGhostObject *ghost = CE_NEW(_ghosts_pool, btPairCachingGhostObject)();
			ghost->setWorldTransform(btTransform(to_btQuaternion(QUATERNION_IDENTITY), to_btVector3(translation(tm))));
			ghost->m_userObjectPointer = ((void *)(uintptr_t)UINT32_MAX);

			Mover *mover = CE_NEW(*_allocator, Mover)(_shapes_pool
				, _dynamics_world
				, ghost
				, movers[i]
				, to_btVector3(VECTOR3_UP)
				);

			_dynamics_world->addCollisionObject(ghost, f->me, f->mask);

			const u32 last = array::size(_mover);
			array::push_back(_mover, { unit, ghost, mover });
			hash_map::set(_mover_map, unit, last);
		}
	}

	void mover_destroy(MoverId mover)
	{
		const u32 last      = array::size(_mover) - 1;
		const UnitId u      = _mover[mover.i].unit;
		const UnitId last_u = _mover[last].unit;

		_mover[mover.i].mover->destroy_shape(_shapes_pool);
		CE_DELETE(*_allocator, _mover[mover.i].mover);
		_dynamics_world->removeCollisionObject(_mover[mover.i].ghost);
		CE_DELETE(_ghosts_pool, _mover[mover.i].ghost);

		_mover[mover.i] = _mover[last];
		array::pop_back(_mover);

		hash_map::set(_mover_map, last_u, mover.i);
		hash_map::remove(_mover_map, u);
	}

	MoverId mover(UnitId unit)
	{
		return make_mover_instance(hash_map::get(_mover_map, unit, UINT32_MAX));
	}

	void mover_set_height(MoverId mover, float height)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->set_height(_shapes_pool, height);
	}

	f32 mover_radius(MoverId mover)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->radius();
	}

	void mover_set_radius(MoverId mover, float radius)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->set_radius(_shapes_pool, radius);
	}

	f32 mover_max_slope_angle(MoverId mover)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->max_slope();
	}

	void mover_set_max_slope_angle(MoverId mover, f32 angle)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->set_max_slope(angle);
	}

	f32 mover_step_height(MoverId mover)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->_step_height;
	}

	void mover_set_step_height(MoverId mover, f32 height)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		_mover[mover.i].mover->set_step_height(height);
	}

	void mover_set_collision_filter(MoverId mover, StringId32 filter)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");

		const PhysicsCollisionFilter *filters = physics_config_resource::filters_array(_config_resource);
		u32 filter_i = physics_config_resource::filter_index(filters, _config_resource->num_filters, filter);
		const PhysicsCollisionFilter *f = &filters[filter_i];

		_dynamics_world->removeCollisionObject(_mover[mover.i].ghost);
		_dynamics_world->addCollisionObject(_mover[mover.i].ghost, f->me, f->mask);
	}

	Vector3 mover_position(MoverId mover)
	{
		return to_vector3(_mover[mover.i].mover->_current_position);
	}

	void mover_set_position(MoverId mover, const Vector3 &position)
	{
		_mover[mover.i].mover->set_position(to_btVector3(position));
	}

	Vector3 mover_center(MoverId mover)
	{
		return to_vector3(_mover[mover.i].mover->_center);
	}

	void mover_set_center(MoverId mover, const Vector3 &center)
	{
		_mover[mover.i].mover->set_center(to_btVector3(center));
	}

	void mover_move(MoverId mover, const Vector3 &delta)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");

		MoverInstanceData &moving = _mover[mover.i];
		const btVector3 mover_delta = to_btVector3(delta);
		moving.mover->move(mover_delta);

		const btCollisionObject *emitted_hit_objects[Mover::MAX_COLLISION_HITS];
		u32 num_emitted_hit_objects = 0;
		for (u32 hit_i = 0; hit_i < moving.mover->_num_collision_hits; ++hit_i) {
			const Mover::CollisionHit &hit = moving.mover->_collision_hits[hit_i];
			const btCollisionObject *hit_object = hit.hit_object;
			CE_ASSERT(hit_object != NULL, "Recorded mover hit missing collision object");
			CE_ASSERT(hit_object != moving.mover->_ghost, "Recorded mover hit unexpectedly points to self");

			u32 emitted_i = 0;
			for (; emitted_i < num_emitted_hit_objects; ++emitted_i) {
				if (emitted_hit_objects[emitted_i] == hit_object)
					break;
			}
			if (emitted_i != num_emitted_hit_objects)
				continue;

			if (num_emitted_hit_objects < countof(emitted_hit_objects))
				emitted_hit_objects[num_emitted_hit_objects++] = hit_object;

			const btRigidBody *body = btRigidBody::upcast(hit_object);
			if (body != NULL) {
				const u32 actor_i = (u32)(uintptr_t)body->m_userObjectPointer;
				if (actor_i < array::size(_actor) && _actor[actor_i].body == body) {
					PhysicsMoverActorCollisionEvent ev;
					ev.mover_unit = moving.unit;
					ev.actor_unit = _actor[actor_i].unit;
					ev.actor = make_actor_instance(actor_i);
					ev.normal = to_vector3(hit.normal);
					ev.position = to_vector3(hit.mover_position);
					ev.direction = to_vector3(Mover::normalized(mover_delta));
					ev.direction_length = (f32)mover_delta.length();
					event_stream::write(_events, EventType::PHYSICS_MOVER_ACTOR_COLLISION, ev);
					continue;
				}
			}

			for (u32 other_mover_i = 0; other_mover_i < array::size(_mover); ++other_mover_i) {
				if (other_mover_i == mover.i)
					continue;
				if (_mover[other_mover_i].ghost != hit_object)
					continue;

				PhysicsMoverMoverCollisionEvent ev;
				ev.mover_unit = moving.unit;
				ev.other_mover_unit = _mover[other_mover_i].unit;
				ev.mover = mover;
				ev.other_mover = make_mover_instance(other_mover_i);
				event_stream::write(_events, EventType::PHYSICS_MOVER_MOVER_COLLISION, ev);
				break;
			}
		}
	}

	bool mover_separate(MoverId mover, Vector3 &separation_delta)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		btVector3 separation_bt;
		const bool result = _mover[mover.i].mover->separate(separation_bt);
		separation_delta = to_vector3(separation_bt);
		return result;
	}

	bool mover_fits_at(MoverId mover, const Vector3 &position)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		return _mover[mover.i].mover->fits_at(to_btVector3(position));
	}

	bool mover_collides_sides(MoverId mover)
	{
		return (_mover[mover.i].mover->_flags & MoverFlags::COLLIDES_SIDES) != 0;
	}

	bool mover_collides_up(MoverId mover)
	{
		return (_mover[mover.i].mover->_flags & MoverFlags::COLLIDES_UP) != 0;
	}

	bool mover_collides_down(MoverId mover)
	{
		return (_mover[mover.i].mover->_flags & MoverFlags::COLLIDES_DOWN) != 0;
	}

	ActorId mover_actor_colliding_down(MoverId mover)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		Mover *m = _mover[mover.i].mover;

		const u32 actor_i = m->_down_actor_i;
		if (actor_i >= array::size(_actor))
			return make_actor_instance(UINT32_MAX);
		if (_actor[actor_i].body != m->_down_actor_body)
			return make_actor_instance(UINT32_MAX);
		return make_actor_instance(actor_i);
	}

	void mover_debug_draw(MoverId mover, DebugLine *lines, const Color4 &color)
	{
		CE_ASSERT(mover.i < array::size(_mover), "Index out of bounds");
		Mover *m = _mover[mover.i].mover;
		collision_object_debug_draw(lines, m->_ghost, color);

		const btCapsuleShapeZ *capsule = (btCapsuleShapeZ *)m->_shape;
		const btScalar capsule_radius = capsule->getRadius();
		const Vector3 up = to_vector3(Mover::normalized(m->_up));
		const Vector3 bottom = to_vector3(m->_current_position + m->_center) - up * (capsule->getHalfHeight() + capsule_radius);

		const f32 step_quad_half = btMax(0.05f, (f32)capsule_radius * 0.75f);
		draw_mover_step_quad(lines, bottom, up, step_quad_half, (f32)m->_step_height, 1.0f, COLOR4_YELLOW);
		draw_mover_step_quad(lines, bottom, up, step_quad_half, m->_last_detected_step_height > 0.0f ? (f32)m->_last_detected_step_height : (f32)m->_detected_step_height, 2.0f, COLOR4_BLUE);
		draw_mover_slope_lines(lines, bottom, up, capsule_radius, (f32)m->_max_slope_radians, COLOR4_YELLOW);
		draw_mover_slope_lines(lines, bottom, up, capsule_radius, (f32)m->_detected_slope_radians, COLOR4_BLUE);
	}

	JointId joint_create(ActorId a0, ActorId a1, const JointDesc &jd)
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

	void joint_destroy(JointId /*i*/)
	{
		CE_FATAL("Not implemented yet");
	}

	bool cast_ray(RaycastHit &hit, const Vector3 &from, const Vector3 &dir, f32 len)
	{
		const btVector3 aa = to_btVector3(from);
		const btVector3 bb = to_btVector3(from + dir*len);

		btCollisionWorld::ClosestRayResultCallback cb(aa, bb);
		// Collide with everything.
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
		// Collide with everything.
		cb.m_collisionFilterGroup = -1;
		cb.m_collisionFilterMask = -1;

		_dynamics_world->rayTest(aa, bb, cb);

		if (cb.hasHit()) {
			const int num = cb.m_hitPointWorld.size();
			for (int i = 0; i < num; ++i) {
				const btCollisionObject *obj = cb.m_collisionObjects[i];
				const btRigidBody *body = btRigidBody::upcast(obj);
				if (!body)
					continue;

				const u32 actor_i = (u32)(uintptr_t)body->m_userObjectPointer;
				if (actor_i == UINT32_MAX)
					continue;

				RaycastHit rh;
				rh.position = to_vector3(cb.m_hitPointWorld[i]);
				rh.normal   = to_vector3(cb.m_hitNormalWorld[i]);
				rh.time     = (f32)cb.m_hitFractions[i];
				rh.unit     = _actor[actor_i].unit;
				rh.actor.i  = actor_i;
				array::push_back(hits, rh);
			}

			return array::size(hits) > 0;
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
			const btRigidBody *body = btRigidBody::upcast(cb.m_hitCollisionObject);
			if (!body)
				return false;

			const u32 actor_i = (u32)(uintptr_t)body->m_userObjectPointer;
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
		if (!_debug_drawing && !physics_globals::_debug_enabled)
			return;

		if (_debug_drawer.getDebugMode() == btIDebugDraw::DBG_NoDebug)
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
			const ActorId a0 = make_actor_instance((u32)(uintptr_t)obj_a->m_userObjectPointer);
			const ActorId a1 = make_actor_instance((u32)(uintptr_t)obj_b->m_userObjectPointer);
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
				ActorId actor_a = actor(unit_a);
				ActorId actor_b = actor(unit_b);

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
			ActorId first = actor(unit);
			if (is_valid(first))
				actor_destroy(first);
		}

		{
			ColliderId ci = collider_instance(unit);
			if (is_valid(ci))
				collider_destroy(ci);
		}

		{
			MoverId mi = mover(unit);
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

	static ColliderId make_collider_instance(u32 i)
	{
		ColliderId inst = { i }; return inst;
	}

	static ActorId make_actor_instance(u32 i)
	{
		ActorId inst = { i }; return inst;
	}

	static MoverId make_mover_instance(u32 i)
	{
		MoverId inst = { i }; return inst;
	}

	static JointId make_joint_instance(u32 i)
	{
		JointId inst = { i }; return inst;
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

void PhysicsWorld::collider_destroy(ColliderId collider)
{
	_impl->collider_destroy(collider);
}

ColliderId PhysicsWorld::collider_instance(UnitId unit)
{
	return _impl->collider_instance(unit);
}

void PhysicsWorld::actor_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
{
	_impl->actor_create_instances(components_data, num, unit_lookup, unit_index);
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

void PhysicsWorld::actor_debug_draw(ActorId actor, DebugLine *lines, const Color4 &color)
{
	_impl->actor_debug_draw(actor, lines, color);
}

void PhysicsWorld::mover_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
{
	_impl->mover_create_instances(components_data, num, unit_lookup, unit_index);
}

MoverId PhysicsWorld::mover_create(UnitId unit, const MoverDesc *desc)
{
	return _impl->mover_create(unit, desc);
}

void PhysicsWorld::mover_destroy(MoverId actor)
{
	_impl->mover_destroy(actor);
}

MoverId PhysicsWorld::mover(UnitId unit)
{
	return _impl->mover(unit);
}

void PhysicsWorld::mover_set_height(MoverId mover, float height)
{
	_impl->mover_set_height(mover, height);
}

f32 PhysicsWorld::mover_radius(MoverId mover)
{
	return _impl->mover_radius(mover);
}

void PhysicsWorld::mover_set_radius(MoverId mover, float radius)
{
	_impl->mover_set_radius(mover, radius);
}

f32 PhysicsWorld::mover_max_slope_angle(MoverId mover)
{
	return _impl->mover_max_slope_angle(mover);
}

void PhysicsWorld::mover_set_max_slope_angle(MoverId mover, f32 angle)
{
	_impl->mover_set_max_slope_angle(mover, angle);
}

f32 PhysicsWorld::mover_step_height(MoverId mover)
{
	return _impl->mover_step_height(mover);
}

void PhysicsWorld::mover_set_step_height(MoverId mover, f32 height)
{
	_impl->mover_set_step_height(mover, height);
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

Vector3 PhysicsWorld::mover_center(MoverId mover)
{
	return _impl->mover_center(mover);
}

void PhysicsWorld::mover_set_center(MoverId mover, const Vector3 &center)
{
	_impl->mover_set_center(mover, center);
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

void PhysicsWorld::mover_debug_draw(MoverId mover, DebugLine *lines, const Color4 &color)
{
	_impl->mover_debug_draw(mover, lines, color);
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

#endif // if CROWN_PHYSICS_BULLET
