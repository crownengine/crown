/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/file_buffer.inl"
#include "core/filesystem/filesystem.h"
#include "core/guid.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/aabb.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/quaternion.inl"
#include "core/math/sphere.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "resource/compile_options.inl"
#include "resource/mesh.h"
#include "resource/mesh_resource.h"
#include "resource/physics_resource.h"
#include "resource/unit_compiler.h"
#include "world/types.h"

namespace crown
{
namespace physics_resource_internal
{
	struct ColliderInfo
	{
		const char *name;
		ColliderType::Enum type;
	};

	static const ColliderInfo s_collider[] =
	{
		{ "sphere",      ColliderType::SPHERE      },
		{ "capsule",     ColliderType::CAPSULE     },
		{ "box",         ColliderType::BOX         },
		{ "convex_hull", ColliderType::CONVEX_HULL },
		{ "mesh",        ColliderType::MESH        },
		{ "heightfield", ColliderType::HEIGHTFIELD }
	};
	CE_STATIC_ASSERT(countof(s_collider) == ColliderType::COUNT);

	struct D6MotionInfo
	{
		const char *name;
		D6Motion::Enum mode;
	};

	static const D6MotionInfo s_d6_motion[] =
	{
		{ "locked",  D6Motion::LOCKED  },
		{ "limited", D6Motion::LIMITED },
		{ "free",    D6Motion::FREE    }
	};
	CE_STATIC_ASSERT(countof(s_d6_motion) == D6Motion::COUNT);

	struct D6MotorModeInfo
	{
		const char *name;
		D6MotorMode::Enum mode;
	};

	static const D6MotorModeInfo s_d6_motor_mode[] =
	{
		{ "off",      D6MotorMode::OFF      },
		{ "velocity", D6MotorMode::VELOCITY },
		{ "position", D6MotorMode::POSITION }
	};
	CE_STATIC_ASSERT(countof(s_d6_motor_mode) == D6MotorMode::COUNT);

	static ColliderType::Enum shape_type_to_enum(const char *type)
	{
		for (u32 i = 0; i < countof(s_collider); ++i) {
			if (strcmp(type, s_collider[i].name) == 0)
				return s_collider[i].type;
		}

		return ColliderType::COUNT;
	}

	static D6Motion::Enum d6_motion_to_enum(const char *name)
	{
		for (u32 i = 0; i < countof(s_d6_motion); ++i) {
			if (strcmp(name, s_d6_motion[i].name) == 0)
				return s_d6_motion[i].mode;
		}

		return D6Motion::COUNT;
	}

	static D6MotorMode::Enum d6_motor_mode_to_enum(const char *name)
	{
		for (u32 i = 0; i < countof(s_d6_motor_mode); ++i) {
			if (strcmp(name, s_d6_motor_mode[i].name) == 0)
				return s_d6_motor_mode[i].mode;
		}

		return D6MotorMode::COUNT;
	}

	void compile_sphere(ColliderDesc &sd, const Array<Vector3> &points)
	{
		AABB aabb;
		aabb::from_points(aabb, array::size(points), array::begin(points));

		const Vector3 origin = aabb::center(aabb);
		sd.local_tm.t = { origin.x, origin.y, origin.z, 1.0f };

		sd.sphere.radius = max(0.0f, aabb.max.x - aabb.min.x);
		sd.sphere.radius = max(sd.sphere.radius, aabb.max.y - aabb.min.y);
		sd.sphere.radius = max(sd.sphere.radius, aabb.max.z - aabb.min.z);
		sd.sphere.radius *= 0.5f;
	}

	void compile_capsule(ColliderDesc &sd, const Array<Vector3> &points)
	{
		AABB aabb;
		aabb::from_points(aabb, array::size(points), array::begin(points));

		const Vector3 origin = aabb::center(aabb);
		sd.local_tm.t = { origin.x, origin.y, origin.z, 1.0f };
		sd.capsule.radius = aabb::radius(aabb) / 2.0f;
		sd.capsule.height = (aabb.max.y - aabb.min.y) / 2.0f;
	}

	void compile_box(ColliderDesc &sd, const Array<Vector3> &points)
	{
		AABB aabb;
		aabb::from_points(aabb, array::size(points), array::begin(points));

		const Vector3 origin = aabb::center(aabb);
		sd.local_tm.t = { origin.x, origin.y, origin.z, 1.0f };
		sd.box.half_size = (aabb.max - aabb.min) * 0.5f;
	}

	s32 compile_collider(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
	{
		CE_UNUSED(compiler);

		TempAllocator4096 ta;
		DynamicString type(ta);
		RETURN_IF_ERROR(sjson::parse_string(type, flat_json_object::get(obj, "data.shape")));

		ColliderType::Enum st = shape_type_to_enum(type.c_str());
		RETURN_IF_FALSE(st != ColliderType::COUNT
			, opts
			, "Unknown shape type: '%s'"
			, type.c_str()
			);

		ColliderDesc cd;
		memset((void *)&cd, 0, sizeof(cd));
		cd.type     = st;
		cd.local_tm = MATRIX4X4_IDENTITY;
		cd.size     = 0;

		Array<Vector3> points(default_allocator());
		Array<u32> point_indices(default_allocator());

		DynamicString source(ta);
		if (flat_json_object::has(obj, "data.source")) {
			RETURN_IF_ERROR(sjson::parse_string(source, flat_json_object::get(obj, "data.source")));
		}
		bool explicit_collider = source == "mesh"
			|| (source != "inline" && flat_json_object::has(obj, "data.scene"));

		if (explicit_collider) {
			DynamicString scene(ta);
			DynamicString name(ta);
			RETURN_IF_ERROR(sjson::parse_string(scene, flat_json_object::get(obj, "data.scene")));
			RETURN_IF_ERROR(sjson::parse_string(name, flat_json_object::get(obj, "data.name")));

			// Parse mesh resource.
			RETURN_IF_RESOURCE_MISSING("mesh", scene.c_str(), opts);
			scene += ".mesh";
			Mesh mesh(default_allocator());
			s32 err = mesh::parse(mesh, scene.c_str(), opts);
			ENSURE_OR_RETURN(err == 0, opts);

			Node deffault_node(default_allocator());
			Node &node = hash_map::get(mesh._nodes, name, deffault_node);
			RETURN_IF_FALSE(&node != &deffault_node
				, opts
				, "Node '%s' does not exist"
				, name.c_str()
				);

			Geometry deffault_geometry(default_allocator());
			Geometry &geometry = hash_map::get(mesh._geometries, node._geometry, deffault_geometry);
			RETURN_IF_FALSE(&geometry != &deffault_geometry
				, opts
				, "Geometry '%s' does not exist"
				, node._geometry.c_str()
				);

			for (u32 i = 0; i < array::size(geometry._positions); i += 3) {
				Vector3 p;
				p.x = geometry._positions[i + 0];
				p.y = geometry._positions[i + 1];
				p.z = geometry._positions[i + 2];
				array::push_back(points, p);
			}
			RETURN_IF_FALSE(array::size(points) > 0
				, opts
				, "Collider is empty '%s'"
				, name.c_str()
				);

			point_indices = geometry._position_indices;

			switch (cd.type) {
			case ColliderType::SPHERE:      compile_sphere(cd, points); break;
			case ColliderType::CAPSULE:     compile_capsule(cd, points); break;
			case ColliderType::BOX:         compile_box(cd, points); break;
			case ColliderType::CONVEX_HULL: break;
			case ColliderType::MESH:        break;
			case ColliderType::HEIGHTFIELD:
				RETURN_IF_FALSE(false, opts, "Not implemented yet");
				break;
			default:
				RETURN_IF_FALSE(false, opts, "Invalid collider type");
				break;
			}
		} else {
			RETURN_IF_FALSE(flat_json_object::has(obj, "data.collider_data")
				, opts
				, "No collider_data found"
				);
			Quaternion rotation = RETURN_IF_ERROR(sjson::parse_quaternion(flat_json_object::get(obj, "data.collider_data.rotation")));
			Vector3 position = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.collider_data.position")));
			Matrix4x4 matrix_local = from_quaternion_translation(rotation, position);
			cd.local_tm = matrix_local;

			if (cd.type == ColliderType::SPHERE) {
				cd.sphere.radius = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.collider_data.radius")));
			} else if (cd.type == ColliderType::BOX) {
				cd.box.half_size = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.collider_data.half_extents")));
			} else if (cd.type == ColliderType::CAPSULE) {
				cd.capsule.radius = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.collider_data.radius")));
				cd.capsule.height = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.collider_data.height")));
			} else {
				RETURN_IF_FALSE(false, opts, "Invalid collider type");
			}
		}

		const bool needs_points = cd.type == ColliderType::CONVEX_HULL
			|| cd.type == ColliderType::MESH;
		if (needs_points) {
			cd.size += sizeof(u32) + sizeof(Vector3)*array::size(points);
			if (cd.type == ColliderType::MESH)
				cd.size += sizeof(u32) + sizeof(u16)*array::size(point_indices);
		}

		FileBuffer fb(output);
		BinaryWriter bw(fb);
		bw.write(cd.type);
		bw.write(cd.local_tm);
		bw.write(cd.sphere.radius);
		bw.write(cd.capsule.radius);
		bw.write(cd.capsule.height);
		bw.write(cd.box.half_size);
		bw.write(cd.heightfield.width);
		bw.write(cd.heightfield.length);
		bw.write(cd.heightfield.height_scale);
		bw.write(cd.heightfield.height_min);
		bw.write(cd.heightfield.height_max);
		bw.write(cd.size);

		if (needs_points) {
			bw.write(array::size(points));
			for (u32 ii = 0; ii < array::size(points); ++ii)
				bw.write(points[ii]);

			if (cd.type == ColliderType::MESH) {
				bw.write(array::size(point_indices));
				for (u32 ii = 0; ii < array::size(point_indices); ++ii)
					bw.write((u16)point_indices[ii]);
			}
		}
		return 0;
	}

	s32 compile_actor(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
	{
		CE_UNUSED_2(compiler, opts);

		u32 flags = 0;
		if (flat_json_object::has(obj, "data.lock_translation_x")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_translation_x")));
			flags |= lock ? ActorFlags::LOCK_TRANSLATION_X : 0u;
		}
		if (flat_json_object::has(obj, "data.lock_translation_y")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_translation_y")));
			flags |= lock ? ActorFlags::LOCK_TRANSLATION_Y : 0u;
		}
		if (flat_json_object::has(obj, "data.lock_translation_z")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_translation_z")));
			flags |= lock ? ActorFlags::LOCK_TRANSLATION_Z : 0u;
		}
		if (flat_json_object::has(obj, "data.lock_rotation_x")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_rotation_x")));
			flags |= lock ? ActorFlags::LOCK_ROTATION_X : 0u;
		}
		if (flat_json_object::has(obj, "data.lock_rotation_y")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_rotation_y")));
			flags |= lock ? ActorFlags::LOCK_ROTATION_Y : 0u;
		}
		if (flat_json_object::has(obj, "data.lock_rotation_z")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_rotation_z")));
			flags |= lock ? ActorFlags::LOCK_ROTATION_Z : 0u;
		}

		ActorResource ar;
		ar.actor_class      = RETURN_IF_ERROR(sjson::parse_string_id(flat_json_object::get(obj, "data.class")));
		ar.mass             = RETURN_IF_ERROR(sjson::parse_float    (flat_json_object::get(obj, "data.mass")));
		ar.flags            = flags;
		ar.collision_filter = RETURN_IF_ERROR(sjson::parse_string_id(flat_json_object::get(obj, "data.collision_filter")));
		ar.material         = RETURN_IF_ERROR(sjson::parse_string_id(flat_json_object::get(obj, "data.material")));

		FileBuffer fb(output);
		BinaryWriter bw(fb);
		bw.write(ar.actor_class);
		bw.write(ar.mass);
		bw.write(ar.flags);
		bw.write(ar.collision_filter);
		bw.write(ar.material);
		return 0;
	}

	s32 compile_mover(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
	{
		CE_UNUSED_2(compiler, opts);

		MoverDesc md;
		md.capsule.radius   = RETURN_IF_ERROR(sjson::parse_float    (flat_json_object::get(obj, "data.radius")));
		md.capsule.height   = RETURN_IF_ERROR(sjson::parse_float    (flat_json_object::get(obj, "data.height")));
		md.max_slope_angle  = RETURN_IF_ERROR(sjson::parse_float    (flat_json_object::get(obj, "data.max_slope_angle")));
		if (flat_json_object::has(obj, "data.step_height")) {
			md.step_height = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.step_height")));
		} else {
			md.step_height = 0.5f;
		}
		md.collision_filter = RETURN_IF_ERROR(sjson::parse_string_id(flat_json_object::get(obj, "data.collision_filter")));
		if (flat_json_object::has(obj, "data.center")) {
			md.center = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.center")));
		} else {
			md.center = VECTOR3_ZERO;
		}

		FileBuffer fb(output);
		BinaryWriter bw(fb);
		bw.write(md.capsule.radius);
		bw.write(md.capsule.height);
		bw.write(md.max_slope_angle);
		bw.write(md.step_height);
		bw.write(md.collision_filter);
		bw.write(md.center);
		return 0;
	}

	s32 joint_common_parse(JointDesc &jd, JointType::Enum jt, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
	{
		const Guid other_unit_id = RETURN_IF_ERROR(sjson::parse_guid(flat_json_object::get(obj, "data.other_actor")));

		jd = {};
		jd.type_and_flags = u32(jt) << JOINT_TYPE_AND_FLAGS_TYPE_SHIFT;
		jd.other_actor_unit_index = UINT32_MAX;
		jd.pose = MATRIX4X4_IDENTITY;
		jd.other_pose = MATRIX4X4_IDENTITY;
		jd.break_force = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.break_force")));

		if (jt != JointType::FIXED) {
			Vector3 position = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.position")));
			Quaternion rotation = QUATERNION_IDENTITY;
			Vector3 other_position = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.other_position")));
			Quaternion other_rotation = QUATERNION_IDENTITY;

			if (jt != JointType::SPHERICAL) {
				rotation = RETURN_IF_ERROR(sjson::parse_quaternion(flat_json_object::get(obj, "data.rotation")));
				other_rotation = RETURN_IF_ERROR(sjson::parse_quaternion(flat_json_object::get(obj, "data.other_rotation")));
			}

			jd.pose = from_quaternion_translation(rotation, position);
			jd.other_pose = from_quaternion_translation(other_rotation, other_position);
		}

		if (other_unit_id != GUID_ZERO) {
			const u32 root_unit_index = compiler._unit_roots[compiler._current_unit_index];
			jd.other_actor_unit_index = unit_compiler::find_unit_index(compiler, other_unit_id, root_unit_index);
			RETURN_IF_FALSE(jd.other_actor_unit_index != UINT32_MAX
				, opts
				, "Joint references a unit outside this unit resource"
				);

			Unit *other_unit = unit_compiler::find_unit(compiler, jd.other_actor_unit_index);
			bool has_actor = false;
			RETURN_IF_ERROR(unit_compiler::unit_has_component_type(has_actor
				, other_unit
				, STRING_ID_32("actor", UINT32_C(0x374cf583))
				));
			RETURN_IF_FALSE(has_actor
				, opts
				, "Joint references a unit without an actor component"
				);
		}

		return 0;
	}

	s32 joint_common_write(FileBuffer &fb, JointDesc &jd)
	{
		BinaryWriter bw(fb);
		bw.write(jd.type_and_flags);
		bw.write(jd.other_actor_unit_index);
		bw.write(jd.pose);
		bw.write(jd.other_pose);
		bw.write(jd.break_force);
		return 0;
	}

	s32 compile_fixed_joint(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
	{
		JointDesc jd;
		s32 err = joint_common_parse(jd, JointType::FIXED, compiler, obj, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		FileBuffer fb(output);
		return joint_common_write(fb, jd);
	}

	s32 compile_hinge_joint(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
	{
		HingeJoint hinge = {};
		hinge.axis = VECTOR3_ZAXIS;
		hinge.lower_limit = -PI_FOURTH;
		hinge.upper_limit = PI_FOURTH;
		JointDesc jd;
		s32 err = joint_common_parse(jd, JointType::HINGE, compiler, obj, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		hinge.axis = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.axis")));

		const bool use_motor = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.use_motor")));
		jd.type_and_flags |= use_motor ? (u32)JointFlags::HINGE_USE_MOTOR : 0u;
		hinge.target_velocity = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.target_velocity")));
		hinge.max_motor_impulse = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.max_motor_impulse")));

		const bool use_limits = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.use_limits")));
		jd.type_and_flags |= use_limits ? (u32)JointFlags::HINGE_USE_LIMITS : 0u;
		hinge.lower_limit = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.lower_limit")));
		hinge.upper_limit = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.upper_limit")));
		hinge.bounciness = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.bounciness")));

		FileBuffer fb(output);
		err = joint_common_write(fb, jd);
		ENSURE_OR_RETURN(err == 0, opts);

		BinaryWriter bw(fb);
		bw.write(hinge.axis);
		bw.write(hinge.target_velocity);
		bw.write(hinge.max_motor_impulse);
		bw.write(hinge.lower_limit);
		bw.write(hinge.upper_limit);
		bw.write(hinge.bounciness);
		return 0;
	}

	s32 compile_spherical_joint(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
	{
		JointDesc jd;
		s32 err = joint_common_parse(jd, JointType::SPHERICAL, compiler, obj, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		FileBuffer fb(output);
		return joint_common_write(fb, jd);
	}

	s32 compile_limb_joint(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
	{
		LimbJoint limb = {};
		JointDesc jd;
		s32 err = joint_common_parse(jd, JointType::LIMB, compiler, obj, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		const char *limb_motion[] = { "data.twist_motion", "data.swing_y_motion", "data.swing_z_motion" };
		const u32 limb_motion_shift[] = { LIMB_JOINT_TWIST_MOTION_SHIFT, LIMB_JOINT_SWING_Y_MOTION_SHIFT, LIMB_JOINT_SWING_Z_MOTION_SHIFT };

		for (u32 i = 0; i < countof(limb_motion); ++i) {
			TempAllocator64 ta;
			DynamicString motion(ta);
			RETURN_IF_ERROR(sjson::parse_string(motion, flat_json_object::get(obj, limb_motion[i])));
			const D6Motion::Enum limb_motion = d6_motion_to_enum(motion.c_str());
			RETURN_IF_FALSE(limb_motion != D6Motion::COUNT
				, opts
				, "Unknown limb motion: '%s'"
				, motion.c_str()
				);
			limb.motion |= u32(limb_motion) << limb_motion_shift[i];
		}

		limb.twist_lower_limit = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.twist_lower_limit")));
		limb.twist_upper_limit = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.twist_upper_limit")));
		limb.swing_y_limit = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.swing_y_limit")));
		limb.swing_z_limit = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.swing_z_limit")));

		FileBuffer fb(output);
		err = joint_common_write(fb, jd);
		ENSURE_OR_RETURN(err == 0, opts);

		BinaryWriter bw(fb);
		bw.write(limb.motion);
		bw.write(limb.twist_lower_limit);
		bw.write(limb.twist_upper_limit);
		bw.write(limb.swing_y_limit);
		bw.write(limb.swing_z_limit);
		return 0;
	}

	s32 compile_spring_joint(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
	{
		SpringJoint spring = {};
		JointDesc jd;
		s32 err = joint_common_parse(jd, JointType::SPRING, compiler, obj, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		spring.stiffness = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.stiffness")));
		spring.damping = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.damping")));

		FileBuffer fb(output);
		err = joint_common_write(fb, jd);
		ENSURE_OR_RETURN(err == 0, opts);

		BinaryWriter bw(fb);
		bw.write(spring.stiffness);
		bw.write(spring.damping);
		return 0;
	}

	s32 compile_d6_joint(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
	{
		D6Joint d6 = {};
		JointDesc jd;
		s32 err = joint_common_parse(jd, JointType::D6, compiler, obj, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		for (u32 axis = 0; axis < 3; ++axis) {
			{
				const char *linear_motion[] = { "data.linear_motion_x", "data.linear_motion_y", "data.linear_motion_z" };
				const u32 linear_motion_shift[] = { D6_JOINT_LINEAR_X_MOTION_SHIFT, D6_JOINT_LINEAR_Y_MOTION_SHIFT, D6_JOINT_LINEAR_Z_MOTION_SHIFT };

				TempAllocator64 ta;
				DynamicString motion(ta);
				RETURN_IF_ERROR(sjson::parse_string(motion, flat_json_object::get(obj, linear_motion[axis])));
				const D6Motion::Enum mot = d6_motion_to_enum(motion.c_str());
				RETURN_IF_FALSE(mot != D6Motion::COUNT
					, opts
					, "Unknown D6 linear motion: '%s'"
					, motion.c_str()
					);
				d6.motion_and_motor |= u32(mot) << linear_motion_shift[axis];
			}

			{
				const char *angular_motion[] = { "data.angular_motion_x", "data.angular_motion_y", "data.angular_motion_z" };
				const u32 angular_motion_shift[] = { D6_JOINT_ANGULAR_X_MOTION_SHIFT, D6_JOINT_ANGULAR_Y_MOTION_SHIFT, D6_JOINT_ANGULAR_Z_MOTION_SHIFT };

				TempAllocator64 ta;
				DynamicString motion(ta);
				RETURN_IF_ERROR(sjson::parse_string(motion, flat_json_object::get(obj, angular_motion[axis])));
				const D6Motion::Enum mot = d6_motion_to_enum(motion.c_str());
				RETURN_IF_FALSE(mot != D6Motion::COUNT
					, opts
					, "Unknown D6 angular motion: '%s'"
					, motion.c_str()
					);
				d6.motion_and_motor |= u32(mot) << angular_motion_shift[axis];
			}

			{
				const char *linear_motor[] = { "data.linear_motor_x", "data.linear_motor_y", "data.linear_motor_z" };
				const u32 linear_motor_shift[] = { D6_JOINT_LINEAR_X_MOTOR_SHIFT, D6_JOINT_LINEAR_Y_MOTOR_SHIFT, D6_JOINT_LINEAR_Z_MOTOR_SHIFT };

				TempAllocator64 ta;
				DynamicString motor_mode(ta);
				RETURN_IF_ERROR(sjson::parse_string(motor_mode, flat_json_object::get(obj, linear_motor[axis])));
				const D6MotorMode::Enum mode = d6_motor_mode_to_enum(motor_mode.c_str());
				RETURN_IF_FALSE(mode != D6MotorMode::COUNT
					, opts
					, "Unknown D6 linear motor mode: '%s'"
					, motor_mode.c_str()
					);
				d6.motion_and_motor |= u32(mode) << linear_motor_shift[axis];
			}

			{
				const char *angular_motor[] = { "data.angular_motor_x", "data.angular_motor_y", "data.angular_motor_z" };
				const u32 angular_motor_shift[] = { D6_JOINT_ANGULAR_X_MOTOR_SHIFT, D6_JOINT_ANGULAR_Y_MOTOR_SHIFT, D6_JOINT_ANGULAR_Z_MOTOR_SHIFT };

				TempAllocator64 ta;
				DynamicString motor_mode(ta);
				RETURN_IF_ERROR(sjson::parse_string(motor_mode, flat_json_object::get(obj, angular_motor[axis])));
				const D6MotorMode::Enum mode = d6_motor_mode_to_enum(motor_mode.c_str());
				RETURN_IF_FALSE(mode != D6MotorMode::COUNT
					, opts
					, "Unknown D6 angular motor mode: '%s'"
					, motor_mode.c_str()
					);
				d6.motion_and_motor |= u32(mode) << angular_motor_shift[axis];
			}
		}

		d6.linear_lower_limit = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.linear_lower_limit")));
		d6.linear_upper_limit = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.linear_upper_limit")));
		d6.angular_lower_limit = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.angular_lower_limit")));
		d6.angular_upper_limit = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.angular_upper_limit")));
		d6.linear_target_velocity = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.linear_target_velocity")));
		d6.linear_target_position = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.linear_target_position")));
		d6.linear_max_motor_force = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.linear_max_motor_force")));
		d6.angular_target_velocity = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.angular_target_velocity")));
		d6.angular_target_position = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.angular_target_position")));
		d6.angular_max_motor_force = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.angular_max_motor_force")));

		FileBuffer fb(output);
		err = joint_common_write(fb, jd);
		ENSURE_OR_RETURN(err == 0, opts);

		BinaryWriter bw(fb);
		bw.write(d6.motion_and_motor);
		bw.write(d6.linear_lower_limit);
		bw.write(d6.linear_upper_limit);
		bw.write(d6.angular_lower_limit);
		bw.write(d6.angular_upper_limit);
		bw.write(d6.linear_target_velocity);
		bw.write(d6.linear_target_position);
		bw.write(d6.linear_max_motor_force);
		bw.write(d6.angular_target_velocity);
		bw.write(d6.angular_target_position);
		bw.write(d6.angular_max_motor_force);
		return 0;
	}

} // namespace physics_resource_internal

namespace physics_config_resource_internal
{
	s32 parse_materials(const char *json, Array<PhysicsMaterial> &objects, CompileOptions &opts)
	{
		CE_UNUSED(opts);

		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, json));

		auto cur = json_object::begin(obj);
		auto end = json_object::end(obj);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(obj, cur);

			const StringView key = cur->first;
			const char *value    = cur->second;

			JsonObject material(ta);
			RETURN_IF_ERROR(sjson::parse_object(material, value));

			PhysicsMaterial mat;
			mat.name             = StringId32(key.data(), key.length());
			mat.friction         = RETURN_IF_ERROR(sjson::parse_float(material["friction"]));
			mat.rolling_friction = RETURN_IF_ERROR(sjson::parse_float(material["rolling_friction"]));
			mat.restitution      = RETURN_IF_ERROR(sjson::parse_float(material["restitution"]));
			if (json_object::has(material, "spinning_friction")) {
				mat.spinning_friction = RETURN_IF_ERROR(sjson::parse_float(material["spinning_friction"]));
			} else {
				mat.spinning_friction = 0.5f;
			}

			array::push_back(objects, mat);
		}

		return 0;
	}

	s32 parse_actors(const char *json, Array<PhysicsActor> &objects, CompileOptions &opts)
	{
		CE_UNUSED(opts);

		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, json));

		auto cur = json_object::begin(obj);
		auto end = json_object::end(obj);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(obj, cur);

			const StringView key = cur->first;
			const char *value    = cur->second;

			JsonObject actor(ta);
			RETURN_IF_ERROR(sjson::parse_object(actor, value));

			PhysicsActor pa;
			pa.name = StringId32(key.data(), key.length());
			pa.linear_damping  = 0.0f;
			pa.angular_damping = 0.0f;

			if (json_object::has(actor, "linear_damping")) {
				pa.linear_damping = RETURN_IF_ERROR(sjson::parse_float(actor["linear_damping"]));
			}
			if (json_object::has(actor, "angular_damping")) {
				pa.angular_damping = RETURN_IF_ERROR(sjson::parse_float(actor["angular_damping"]));
			}

			pa.flags = 0;
			if (json_object::has(actor, "dynamic")) {
				bool val = RETURN_IF_ERROR(sjson::parse_bool(actor["dynamic"]));
				pa.flags |= val ? CROWN_PHYSICS_ACTOR_DYNAMIC : 0u;
			}
			if (json_object::has(actor, "kinematic")) {
				bool val = RETURN_IF_ERROR(sjson::parse_bool(actor["kinematic"]));
				pa.flags |= val ? CROWN_PHYSICS_ACTOR_KINEMATIC : 0u;
			}
			if (json_object::has(actor, "disable_gravity")) {
				bool val = RETURN_IF_ERROR(sjson::parse_bool(actor["disable_gravity"]));
				pa.flags |= val ? CROWN_PHYSICS_ACTOR_DISABLE_GRAVITY : 0u;
			}
			if (json_object::has(actor, "trigger")) {
				bool val = RETURN_IF_ERROR(sjson::parse_bool(actor["trigger"]));
				pa.flags |= val ? CROWN_PHYSICS_ACTOR_TRIGGER : 0u;
			}

			array::push_back(objects, pa);
		}

		return 0;
	}

	struct CollisionFilterCompiler
	{
		CompileOptions &_opts;
		HashMap<StringId32, u32> _filter_map;
		Array<PhysicsCollisionFilter> _filters;
		u32 _filter;

		explicit CollisionFilterCompiler(CompileOptions &opts)
			: _opts(opts)
			, _filter_map(default_allocator())
			, _filters(default_allocator())
			, _filter(1)
		{
		}

		s32 parse(const char *json)
		{
			TempAllocator4096 ta;
			JsonObject obj(ta);
			RETURN_IF_ERROR(sjson::parse(obj, json));

			// Assign group to filter name.
			auto cur = json_object::begin(obj);
			auto end = json_object::end(obj);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(obj, cur);

				const StringView key = cur->first;
				const StringId32 id  = StringId32(key.data(), key.length());

				hash_map::set(_filter_map, id, new_filter_mask());
			}

			// Build masks.
			cur = json_object::begin(obj);
			end = json_object::end(obj);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(obj, cur);

				const StringView key = cur->first;
				const char *value    = cur->second;
				const StringId32 id  = StringId32(key.data(), key.length());

				TempAllocator4096 ta;
				JsonObject filter(ta);
				RETURN_IF_ERROR(sjson::parse_object(filter, value));

				u32 mask = 0;

				if (json_object::has(filter, "collides_with")) {
					JsonArray include(ta);
					RETURN_IF_ERROR(sjson::parse_array(include, filter["collides_with"]));

					for (u32 i = 0; i < array::size(include); ++i) {
						const StringId32 fi = RETURN_IF_ERROR(sjson::parse_string_id(include[i]));
						mask |= filter_to_mask(fi);
					}
				}

				if (json_object::has(filter, "collides_with_all_except")) {
					mask = UINT32_MAX;

					JsonArray exclude(ta);
					RETURN_IF_ERROR(sjson::parse_array(exclude, filter["collides_with_all_except"]));

					for (u32 i = 0; i < array::size(exclude); ++i) {
						const StringId32 fi = RETURN_IF_ERROR(sjson::parse_string_id(exclude[i]));
						mask &= ~filter_to_mask(fi);
					}
				}

				PhysicsCollisionFilter pcf;
				pcf.name = id;
				pcf.me   = filter_to_mask(id);
				pcf.mask = mask;

				array::push_back(_filters, pcf);
			}

			return 0;
		}

		u32 new_filter_mask()
		{
			RETURN_IF_FALSE(_filter != 0x80000000u
				, _opts
				, "Too many collision filters"
				);

			const u32 f = _filter;
			_filter = _filter << 1;
			return f;
		}

		u32 filter_to_mask(StringId32 filter)
		{
			RETURN_IF_FALSE(hash_map::has(_filter_map, filter)
				, _opts
				, "Filter not found"
				);

			return hash_map::get(_filter_map, filter, 0u);
		}
	};

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf));

		Array<PhysicsMaterial> materials(default_allocator());
		Array<PhysicsActor> actors(default_allocator());
		CollisionFilterCompiler cfc(opts);
		PhysicsConfigResource pcr;

		// Parse materials
		s32 err = 0;
		if (json_object::has(obj, "collision_filters")) {
			err = cfc.parse(obj["collision_filters"]);
			ENSURE_OR_RETURN(err == 0, opts);
		}
		if (json_object::has(obj, "materials")) {
			err = parse_materials(obj["materials"], materials, opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}
		if (json_object::has(obj, "actors")) {
			err = parse_actors(obj["actors"], actors, opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}
		if (json_object::has(obj, "gravity")) {
			pcr.gravity = RETURN_IF_ERROR(sjson::parse_vector3(obj["gravity"]));
		} else {
			pcr.gravity = { 0.0f, 0.0f, -10.0f };
		}

		// Setup struct for writing
		pcr.version       = RESOURCE_HEADER(RESOURCE_VERSION_PHYSICS_CONFIG);
		pcr.num_materials = array::size(materials);
		pcr.num_actors    = array::size(actors);
		pcr.num_filters   = array::size(cfc._filters);

		u32 offt = sizeof(PhysicsConfigResource);
		pcr.materials_offset = offt;
		offt += sizeof(PhysicsMaterial) * pcr.num_materials;

		pcr.actors_offset = offt;
		offt += sizeof(PhysicsActor) * pcr.num_actors;

		pcr.filters_offset = offt;

		// Write all
		opts.write(pcr.version);
		opts.write(pcr.num_materials);
		opts.write(pcr.materials_offset);
		opts.write(pcr.num_actors);
		opts.write(pcr.actors_offset);
		opts.write(pcr.num_filters);
		opts.write(pcr.filters_offset);
		opts.write(pcr.gravity);

		// Write materials
		for (u32 i = 0; i < pcr.num_materials; ++i) {
			opts.write(materials[i].name._id);
			opts.write(materials[i].friction);
			opts.write(materials[i].rolling_friction);
			opts.write(materials[i].spinning_friction);
			opts.write(materials[i].restitution);
		}

		// Write actors
		for (u32 i = 0; i < pcr.num_actors; ++i) {
			opts.write(actors[i].name._id);
			opts.write(actors[i].linear_damping);
			opts.write(actors[i].angular_damping);
			opts.write(actors[i].flags);
		}

		// Write collision filters
		for (u32 i = 0; i < array::size(cfc._filters); ++i) {
			opts.write(cfc._filters[i].name._id);
			opts.write(cfc._filters[i].me);
			opts.write(cfc._filters[i].mask);
		}

		return 0;
	}

} // namespace physics_config_resource_internal

} // namespace crown
#endif // if CROWN_CAN_COMPILE
