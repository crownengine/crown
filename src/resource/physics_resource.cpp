/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/file_buffer.inl"
#include "core/filesystem/filesystem.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/aabb.inl"
#include "core/math/constants.h"
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
#include "world/types.h"

namespace crown
{
namespace physics_config_resource
{
	const PhysicsMaterial *materials_array(const PhysicsConfigResource *pcr)
	{
		return (const PhysicsMaterial *)((char *)pcr + pcr->materials_offset);
	}

	u32 material_index(const PhysicsMaterial *materials, u32 num, StringId32 name)
	{
		for (u32 i = 0; i < num; ++i) {
			if (materials[i].name == name)
				return i;
		}

		CE_FATAL("Material not found");
		return UINT32_MAX;
	}

	const PhysicsActor *actors_array(const PhysicsConfigResource *pcr)
	{
		return (const PhysicsActor *)((char *)pcr + pcr->actors_offset);
	}

	u32 actor_index(const PhysicsActor *actors, u32 num, StringId32 name)
	{
		for (u32 i = 0; i < num; ++i) {
			if (actors[i].name == name)
				return i;
		}

		CE_FATAL("Actor not found");
		return UINT32_MAX;
	}

	const PhysicsCollisionFilter *filters_array(const PhysicsConfigResource *pcr)
	{
		return (const PhysicsCollisionFilter *)((char *)pcr + pcr->filters_offset);
	}

	u32 filter_index(const PhysicsCollisionFilter *filters, u32 num, StringId32 name)
	{
		for (u32 i = 0; i < num; ++i) {
			if (filters[i].name == name)
				return i;
		}

		CE_FATAL("Filter not found");
		return UINT32_MAX;
	}

} // namespace physics_config_resource

#if CROWN_CAN_COMPILE
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

	struct JointInfo
	{
		const char *name;
		JointType::Enum type;
	};

	static const JointInfo s_joint[] =
	{
		{ "fixed",  JointType::FIXED  },
		{ "hinge",  JointType::HINGE  },
		{ "spring", JointType::SPRING }
	};
	CE_STATIC_ASSERT(countof(s_joint) == JointType::COUNT);

	static ColliderType::Enum shape_type_to_enum(const char *type)
	{
		for (u32 i = 0; i < countof(s_collider); ++i) {
			if (strcmp(type, s_collider[i].name) == 0)
				return s_collider[i].type;
		}

		return ColliderType::COUNT;
	}

	static JointType::Enum joint_type_to_enum(const char *type)
	{
		for (u32 i = 0; i < countof(s_joint); ++i) {
			if (strcmp(type, s_joint[i].name) == 0)
				return s_joint[i].type;
		}

		return JointType::COUNT;
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

	s32 compile_collider(Buffer &output, FlatJsonObject &obj, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		DynamicString type(ta);
		RETURN_IF_ERROR(sjson::parse_string(type, flat_json_object::get(obj, "data.shape")), opts);

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
			RETURN_IF_ERROR(sjson::parse_string(source, flat_json_object::get(obj, "data.source")), opts);
		}
		bool explicit_collider = source == "mesh"
			|| (source != "inline" && flat_json_object::has(obj, "data.scene"));

		if (explicit_collider) {
			DynamicString scene(ta);
			DynamicString name(ta);
			RETURN_IF_ERROR(sjson::parse_string(scene, flat_json_object::get(obj, "data.scene")), opts);
			RETURN_IF_ERROR(sjson::parse_string(name, flat_json_object::get(obj, "data.name")), opts);

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
			Quaternion rotation = RETURN_IF_ERROR(sjson::parse_quaternion(flat_json_object::get(obj, "data.collider_data.rotation")), opts);
			Vector3 position = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.collider_data.position")), opts);
			Matrix4x4 matrix_local = from_quaternion_translation(rotation, position);
			cd.local_tm = matrix_local;

			if (cd.type == ColliderType::SPHERE) {
				cd.sphere.radius = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.collider_data.radius")), opts);
			} else if (cd.type == ColliderType::BOX) {
				cd.box.half_size = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.collider_data.half_extents")), opts);
			} else if (cd.type == ColliderType::CAPSULE) {
				cd.capsule.radius = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.collider_data.radius")), opts);
				cd.capsule.height = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.collider_data.height")), opts);
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

	s32 compile_actor(Buffer &output, FlatJsonObject &obj, CompileOptions &opts)
	{
		u32 flags = 0;
		if (flat_json_object::has(obj, "data.lock_translation_x")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_translation_x")), opts);
			flags |= lock ? ActorFlags::LOCK_TRANSLATION_X : 0u;
		}
		if (flat_json_object::has(obj, "data.lock_translation_y")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_translation_y")), opts);
			flags |= lock ? ActorFlags::LOCK_TRANSLATION_Y : 0u;
		}
		if (flat_json_object::has(obj, "data.lock_translation_z")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_translation_z")), opts);
			flags |= lock ? ActorFlags::LOCK_TRANSLATION_Z : 0u;
		}
		if (flat_json_object::has(obj, "data.lock_rotation_x")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_rotation_x")), opts);
			flags |= lock ? ActorFlags::LOCK_ROTATION_X : 0u;
		}
		if (flat_json_object::has(obj, "data.lock_rotation_y")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_rotation_y")), opts);
			flags |= lock ? ActorFlags::LOCK_ROTATION_Y : 0u;
		}
		if (flat_json_object::has(obj, "data.lock_rotation_z")) {
			bool lock = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.lock_rotation_z")), opts);
			flags |= lock ? ActorFlags::LOCK_ROTATION_Z : 0u;
		}

		ActorResource ar;
		ar.actor_class      = RETURN_IF_ERROR(sjson::parse_string_id(flat_json_object::get(obj, "data.class")), opts);
		ar.mass             = RETURN_IF_ERROR(sjson::parse_float    (flat_json_object::get(obj, "data.mass")), opts);
		ar.flags            = flags;
		ar.collision_filter = RETURN_IF_ERROR(sjson::parse_string_id(flat_json_object::get(obj, "data.collision_filter")), opts);
		ar.material         = RETURN_IF_ERROR(sjson::parse_string_id(flat_json_object::get(obj, "data.material")), opts);

		FileBuffer fb(output);
		BinaryWriter bw(fb);
		bw.write(ar.actor_class);
		bw.write(ar.mass);
		bw.write(ar.flags);
		bw.write(ar.collision_filter);
		bw.write(ar.material);
		return 0;
	}

	s32 compile_mover(Buffer &output, FlatJsonObject &obj, CompileOptions &opts)
	{
		MoverDesc md;
		md.capsule.radius   = RETURN_IF_ERROR(sjson::parse_float    (flat_json_object::get(obj, "data.radius")), opts);
		md.capsule.height   = RETURN_IF_ERROR(sjson::parse_float    (flat_json_object::get(obj, "data.height")), opts);
		md.max_slope_angle  = RETURN_IF_ERROR(sjson::parse_float    (flat_json_object::get(obj, "data.max_slope_angle")), opts);
		md.collision_filter = RETURN_IF_ERROR(sjson::parse_string_id(flat_json_object::get(obj, "data.collision_filter")), opts);
		if (flat_json_object::has(obj, "data.center")) {
			md.center = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.center")), opts);
		} else {
			md.center = VECTOR3_ZERO;
		}

		FileBuffer fb(output);
		BinaryWriter bw(fb);
		bw.write(md.capsule.radius);
		bw.write(md.capsule.height);
		bw.write(md.max_slope_angle);
		bw.write(md.collision_filter);
		bw.write(md.center);
		return 0;
	}

	s32 compile_joint(Buffer &output, FlatJsonObject &obj, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		DynamicString type(ta);
		RETURN_IF_ERROR(sjson::parse_string(type, flat_json_object::get(obj, "data.type")), opts);

		JointType::Enum jt = joint_type_to_enum(type.c_str());
		RETURN_IF_FALSE(jt != JointType::COUNT
			, opts
			, "Unknown joint type: '%s'"
			, type.c_str()
			);

		JointDesc jd;
		jd.type     = jt;
		jd.anchor_0 = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.anchor_0")), opts);
		jd.anchor_1 = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.anchor_1")), opts);

		switch (jd.type) {
		case JointType::HINGE:
			jd.hinge.use_motor         = RETURN_IF_ERROR(sjson::parse_bool (flat_json_object::get(obj, "data.use_motor")), opts);
			jd.hinge.target_velocity   = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.target_velocity")), opts);
			jd.hinge.max_motor_impulse = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.max_motor_impulse")), opts);
			jd.hinge.lower_limit       = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.lower_limit")), opts);
			jd.hinge.upper_limit       = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.upper_limit")), opts);
			jd.hinge.bounciness        = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.bounciness")), opts);
			break;
		}

		FileBuffer fb(output);
		BinaryWriter bw(fb);
		bw.write(jd.type);
		bw.write(jd.anchor_0);
		bw.write(jd.anchor_1);
		bw.write(jd.breakable);
		bw.write(jd._pad[0]);
		bw.write(jd._pad[1]);
		bw.write(jd._pad[2]);
		bw.write(jd.break_force);
		bw.write(jd.hinge);
		bw.write(jd.hinge.axis);
		bw.write(jd.hinge.use_motor);
		bw.write(jd.hinge.target_velocity);
		bw.write(jd.hinge.max_motor_impulse);
		bw.write(jd.hinge.use_limits);
		bw.write(jd.hinge.lower_limit);
		bw.write(jd.hinge.upper_limit);
		bw.write(jd.hinge.bounciness);
		return 0;
	}

} // namespace physics_resource_internal

namespace physics_config_resource_internal
{
	s32 parse_materials(const char *json, Array<PhysicsMaterial> &objects, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, json), opts);

		auto cur = json_object::begin(obj);
		auto end = json_object::end(obj);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(obj, cur);

			const StringView key = cur->first;
			const char *value    = cur->second;

			JsonObject material(ta);
			RETURN_IF_ERROR(sjson::parse_object(material, value), opts);

			PhysicsMaterial mat;
			mat.name             = StringId32(key.data(), key.length());
			mat.friction         = RETURN_IF_ERROR(sjson::parse_float(material["friction"]), opts);
			mat.rolling_friction = RETURN_IF_ERROR(sjson::parse_float(material["rolling_friction"]), opts);
			mat.restitution      = RETURN_IF_ERROR(sjson::parse_float(material["restitution"]), opts);
			if (json_object::has(material, "spinning_friction")) {
				mat.spinning_friction = RETURN_IF_ERROR(sjson::parse_float(material["spinning_friction"]), opts);
			} else {
				mat.spinning_friction = 0.5f;
			}

			array::push_back(objects, mat);
		}

		return 0;
	}

	s32 parse_actors(const char *json, Array<PhysicsActor> &objects, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, json), opts);

		auto cur = json_object::begin(obj);
		auto end = json_object::end(obj);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(obj, cur);

			const StringView key = cur->first;
			const char *value    = cur->second;

			JsonObject actor(ta);
			RETURN_IF_ERROR(sjson::parse_object(actor, value), opts);

			PhysicsActor pa;
			pa.name = StringId32(key.data(), key.length());
			pa.linear_damping  = 0.0f;
			pa.angular_damping = 0.0f;

			if (json_object::has(actor, "linear_damping")) {
				pa.linear_damping = RETURN_IF_ERROR(sjson::parse_float(actor["linear_damping"]), opts);
			}
			if (json_object::has(actor, "angular_damping")) {
				pa.angular_damping = RETURN_IF_ERROR(sjson::parse_float(actor["angular_damping"]), opts);
			}

			pa.flags = 0;
			if (json_object::has(actor, "dynamic")) {
				bool val = RETURN_IF_ERROR(sjson::parse_bool(actor["dynamic"]), opts);
				pa.flags |= val ? CROWN_PHYSICS_ACTOR_DYNAMIC : 0u;
			}
			if (json_object::has(actor, "kinematic")) {
				bool val = RETURN_IF_ERROR(sjson::parse_bool(actor["kinematic"]), opts);
				pa.flags |= val ? CROWN_PHYSICS_ACTOR_KINEMATIC : 0u;
			}
			if (json_object::has(actor, "disable_gravity")) {
				bool val = RETURN_IF_ERROR(sjson::parse_bool(actor["disable_gravity"]), opts);
				pa.flags |= val ? CROWN_PHYSICS_ACTOR_DISABLE_GRAVITY : 0u;
			}
			if (json_object::has(actor, "trigger")) {
				bool val = RETURN_IF_ERROR(sjson::parse_bool(actor["trigger"]), opts);
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
			RETURN_IF_ERROR(sjson::parse(obj, json), _opts);

			auto cur = json_object::begin(obj);
			auto end = json_object::end(obj);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(obj, cur);

				const StringView key = cur->first;
				const StringId32 id  = StringId32(key.data(), key.length());

				hash_map::set(_filter_map, id, new_filter_mask());
			}

			cur = json_object::begin(obj);
			end = json_object::end(obj);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(obj, cur);

				const StringView key = cur->first;
				const char *value    = cur->second;
				const StringId32 id  = StringId32(key.data(), key.length());

				TempAllocator4096 ta;
				JsonObject filter(ta);
				RETURN_IF_ERROR(sjson::parse_object(filter, value), _opts);

				JsonArray collides_with(ta);
				RETURN_IF_ERROR(sjson::parse_array(collides_with, filter["collides_with"]), _opts);

				u32 mask = 0;
				for (u32 i = 0; i < array::size(collides_with); ++i) {
					const StringId32 fi = RETURN_IF_ERROR(sjson::parse_string_id(collides_with[i]), _opts);
					mask |= filter_to_mask(fi);
				}

				// Build mask
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
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

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
			pcr.gravity = RETURN_IF_ERROR(sjson::parse_vector3(obj["gravity"]), opts);
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
#endif // if CROWN_CAN_COMPILE

} // namespace crown
