/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/hash_map.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/math/aabb.h"
#include "core/math/quaternion.h"
#include "core/math/sphere.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string.h"
#include "resource/compile_options.h"
#include "resource/physics_resource.h"
#include "world/types.h"

namespace crown
{
namespace physics_resource_internal
{
	struct ColliderInfo
	{
		const char* name;
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
		const char* name;
		JointType::Enum type;
	};

	static const JointInfo s_joint[] =
	{
		{ "fixed",  JointType::FIXED  },
		{ "hinge",  JointType::HINGE  },
		{ "spring", JointType::SPRING }
	};
	CE_STATIC_ASSERT(countof(s_joint) == JointType::COUNT);

	static ColliderType::Enum shape_type_to_enum(const char* type)
	{
		for (u32 i = 0; i < countof(s_collider); ++i)
		{
			if (strcmp(type, s_collider[i].name) == 0)
				return s_collider[i].type;
		}

		return ColliderType::COUNT;
	}

	static JointType::Enum joint_type_to_enum(const char* type)
	{
		for (u32 i = 0; i < countof(s_joint); ++i)
		{
			if (strcmp(type, s_joint[i].name) == 0)
				return s_joint[i].type;
		}

		return JointType::COUNT;
	}

	void compile_sphere(const Array<Vector3>& points, ColliderDesc& sd)
	{
		Sphere sphere;
		sphere::reset(sphere);
		sphere::add_points(sphere, array::size(points), array::begin(points));

		sd.sphere.radius = sphere.r;
	}

	void compile_capsule(const Array<Vector3>& points, ColliderDesc& sd)
	{
		AABB aabb;
		aabb::from_points(aabb, array::size(points), array::begin(points));

		const Vector3 origin = aabb::center(aabb) * sd.local_tm;
		sd.local_tm.t = vector4(origin.x, origin.y, origin.z, 1.0f);
		sd.capsule.radius = aabb::radius(aabb) / 2.0f;
		sd.capsule.height = (aabb.max.y - aabb.min.y) / 2.0f;
	}

	void compile_box(const Array<Vector3>& points, ColliderDesc& sd)
	{
		AABB aabb;
		aabb::from_points(aabb, array::size(points), array::begin(points));

		const Vector3 origin = aabb::center(aabb) * sd.local_tm;
		sd.local_tm.t = vector4(origin.x, origin.y, origin.z, 1.0f);
		sd.box.half_size = (aabb.max - aabb.min) * 0.5f;
	}

	s32 compile_collider(Buffer& output, const char* json, CompileOptions& opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		DynamicString type(ta);
		sjson::parse_string(obj["shape"], type);

		ColliderType::Enum st = shape_type_to_enum(type.c_str());
		DATA_COMPILER_ASSERT(st != ColliderType::COUNT
			, opts
			, "Unknown shape type: '%s'"
			, type.c_str()
			);

		ColliderDesc cd;
		memset((void*)&cd, 0, sizeof(cd));
		cd.type     = st;
		cd.local_tm = MATRIX4X4_IDENTITY;
		cd.size     = 0;
		DynamicString source(ta);
		if (json_object::has(obj, "source"))
			sjson::parse_string(obj["source"], source);
		bool explicit_collider = source == "mesh" || json_object::has(obj, "scene");

		if (explicit_collider) {
			// Parse .mesh
			DynamicString scene(ta);
			DynamicString name(ta);
			sjson::parse_string(obj["scene"], scene);
			sjson::parse_string(obj["name"], name);
			DATA_COMPILER_ASSERT_RESOURCE_EXISTS("mesh", scene.c_str(), opts);
			scene += ".mesh";

			Buffer file = opts.read(scene.c_str());
			JsonObject json_mesh(ta);
			JsonObject geometries(ta);
			JsonObject geometry(ta);
			JsonObject nodes(ta);
			JsonObject node(ta);
			sjson::parse(file, json_mesh);
			sjson::parse(json_mesh["geometries"], geometries);
			DATA_COMPILER_ASSERT(json_object::has(geometries, name.c_str())
				, opts
				, "Geometry '%s' does not exist"
				, name.c_str()
				);
			sjson::parse(geometries[name.c_str()], geometry);
			sjson::parse(json_mesh["nodes"], nodes);
			DATA_COMPILER_ASSERT(json_object::has(nodes, name.c_str())
				, opts
				, "Node '%s' does not exist"
				, name.c_str()
				);
			sjson::parse(nodes[name.c_str()], node);

			Matrix4x4 matrix_local = sjson::parse_matrix4x4(node["matrix_local"]);
			cd.local_tm = matrix_local;

			JsonArray positions(ta);
			sjson::parse_array(geometry["position"], positions);

			JsonObject indices(ta);
			JsonArray indices_data(ta);
			JsonArray position_indices(ta);
			sjson::parse_object(geometry["indices"], indices);
			sjson::parse_array(indices["data"], indices_data);
			sjson::parse_array(indices_data[0], position_indices);

			Array<Vector3> points(default_allocator());
			for (u32 i = 0; i < array::size(positions); i += 3)
			{
				Vector3 p;
				p.x = sjson::parse_float(positions[i + 0]);
				p.y = sjson::parse_float(positions[i + 1]);
				p.z = sjson::parse_float(positions[i + 2]);
				array::push_back(points, p);
			}

			Array<u16> point_indices(default_allocator());
			for (u32 i = 0; i < array::size(position_indices); ++i)
			{
				array::push_back(point_indices, (u16)sjson::parse_int(position_indices[i]));
			}

			switch (cd.type)
			{
			case ColliderType::SPHERE:      compile_sphere(points, cd); break;
			case ColliderType::CAPSULE:     compile_capsule(points, cd); break;
			case ColliderType::BOX:         compile_box(points, cd); break;
			case ColliderType::CONVEX_HULL: break;
			case ColliderType::MESH:        break;
			case ColliderType::HEIGHTFIELD:
				DATA_COMPILER_ASSERT(false, opts, "Not implemented yet");
				break;
			}

			const u32 num_points  = array::size(points);
			const u32 num_indices = array::size(point_indices);

			const bool needs_points = cd.type == ColliderType::CONVEX_HULL
				|| cd.type == ColliderType::MESH;

			cd.size += (needs_points ? sizeof(u32) + sizeof(Vector3)*array::size(points) : 0);
			cd.size += (cd.type == ColliderType::MESH ? sizeof(u32) + sizeof(u16)*array::size(point_indices) : 0);

			array::push(output, (char*)&cd, sizeof(cd));

			if (needs_points)
			{
				array::push(output, (char*)&num_points, sizeof(num_points));
				array::push(output, (char*)array::begin(points), sizeof(Vector3)*array::size(points));
			}
			if (cd.type == ColliderType::MESH)
			{
				array::push(output, (char*)&num_indices, sizeof(num_indices));
				array::push(output, (char*)array::begin(point_indices), sizeof(u16)*array::size(point_indices));
			}

		} else {
			JsonObject collider_data(ta);
			JsonArray org(ta);
			DATA_COMPILER_ASSERT(json_object::has(obj, "collider_data")
				, opts
				, "No collider_data found"
				);
			sjson::parse_object(obj["collider_data"], collider_data);
			Quaternion rotation = sjson::parse_quaternion(collider_data["rotation"]);
			Vector3 position = sjson::parse_vector3(collider_data["position"]);
			Matrix4x4 matrix_local = matrix4x4(rotation, position);
			cd.local_tm = matrix_local;

			if (cd.type == ColliderType::SPHERE) {
				cd.sphere.radius = sjson::parse_float(collider_data["radius"]);
			} else if (cd.type == ColliderType::BOX) {
				JsonArray ext(ta);
				sjson::parse_array(collider_data["half_extents"], ext);
				cd.box.half_size = sjson::parse_vector3(collider_data["half_extents"]);;
			} else if (cd.type == ColliderType::CAPSULE) {
				cd.capsule.radius = sjson::parse_float(collider_data["radius"]);
				cd.capsule.height = sjson::parse_float(collider_data["height"]);
			}

			array::push(output, (char*)&cd, sizeof(cd));
		}

		return 0;
	}

	s32 compile_actor(Buffer& output, const char* json, CompileOptions& /*opts*/)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		ActorResource ar;
		ar.actor_class      = sjson::parse_string_id(obj["class"]);
		ar.mass             = sjson::parse_float    (obj["mass"]);
		ar.collision_filter = sjson::parse_string_id(obj["collision_filter"]);
		ar.material         = sjson::parse_string_id(obj["material"]);

		ar.flags = 0;
		ar.flags |= (json_object::has(obj, "lock_translation_x") && sjson::parse_bool(obj["lock_translation_x"])) ? ActorFlags::LOCK_TRANSLATION_X : 0;
		ar.flags |= (json_object::has(obj, "lock_translation_y") && sjson::parse_bool(obj["lock_translation_y"])) ? ActorFlags::LOCK_TRANSLATION_Y : 0;
		ar.flags |= (json_object::has(obj, "lock_translation_z") && sjson::parse_bool(obj["lock_translation_z"])) ? ActorFlags::LOCK_TRANSLATION_Z : 0;
		ar.flags |= (json_object::has(obj, "lock_rotation_x") && sjson::parse_bool(obj["lock_rotation_x"])) ? ActorFlags::LOCK_ROTATION_X : 0;
		ar.flags |= (json_object::has(obj, "lock_rotation_y") && sjson::parse_bool(obj["lock_rotation_y"])) ? ActorFlags::LOCK_ROTATION_Y : 0;
		ar.flags |= (json_object::has(obj, "lock_rotation_z") && sjson::parse_bool(obj["lock_rotation_z"])) ? ActorFlags::LOCK_ROTATION_Z : 0;

		array::push(output, (char*)&ar, sizeof(ar));

		return 0;
	}

	s32 compile_joint(Buffer& output, const char* json, CompileOptions& opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		DynamicString type(ta);
		sjson::parse_string(obj["type"], type);

		JointType::Enum jt = joint_type_to_enum(type.c_str());
		DATA_COMPILER_ASSERT(jt != JointType::COUNT
			, opts
			, "Unknown joint type: '%s'"
			, type.c_str()
			);

		JointDesc jd;
		jd.type     = jt;
		jd.anchor_0 = sjson::parse_vector3(obj["anchor_0"]);
		jd.anchor_1 = sjson::parse_vector3(obj["anchor_1"]);

		switch (jd.type)
		{
		case JointType::HINGE:
			jd.hinge.use_motor         = sjson::parse_bool (obj["use_motor"]);
			jd.hinge.target_velocity   = sjson::parse_float(obj["target_velocity"]);
			jd.hinge.max_motor_impulse = sjson::parse_float(obj["max_motor_impulse"]);
			jd.hinge.lower_limit       = sjson::parse_float(obj["lower_limit"]);
			jd.hinge.upper_limit       = sjson::parse_float(obj["upper_limit"]);
			jd.hinge.bounciness        = sjson::parse_float(obj["bounciness"]);
			break;
		}

		array::push(output, (char*)&jd, sizeof(jd));

		return 0;
	}

} // namespace physics_resource_internal

namespace physics_config_resource_internal
{
	void parse_materials(const char* json, Array<PhysicsMaterial>& objects)
	{
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(json, object);

		auto cur = json_object::begin(object);
		auto end = json_object::end(object);
		for (; cur != end; ++cur)
		{
			JSON_OBJECT_SKIP_HOLE(object, cur);

			const StringView key = cur->first;
			const char* value    = cur->second;

			JsonObject material(ta);
			sjson::parse_object(value, material);

			PhysicsMaterial mat;
			mat.name             = StringId32(key.data(), key.length());
			mat.friction         = sjson::parse_float(material["friction"]);
			mat.rolling_friction = sjson::parse_float(material["rolling_friction"]);
			mat.restitution      = sjson::parse_float(material["restitution"]);

			array::push_back(objects, mat);
		}
	}

	void parse_actors(const char* json, Array<PhysicsActor>& objects)
	{
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(json, object);

		auto cur = json_object::begin(object);
		auto end = json_object::end(object);
		for (; cur != end; ++cur)
		{
			JSON_OBJECT_SKIP_HOLE(object, cur);

			const StringView key = cur->first;
			const char* value    = cur->second;

			JsonObject actor(ta);
			sjson::parse_object(value, actor);

			PhysicsActor pa;
			pa.name = StringId32(key.data(), key.length());
			pa.linear_damping  = 0.0f;
			pa.angular_damping = 0.0f;

			if (json_object::has(actor, "linear_damping"))
				pa.linear_damping = sjson::parse_float(actor["linear_damping"]);
			if (json_object::has(actor, "angular_damping"))
				pa.angular_damping = sjson::parse_float(actor["angular_damping"]);

			pa.flags = 0;
			pa.flags |= (json_object::has(actor, "dynamic")         && sjson::parse_bool(actor["dynamic"])        ) ? PhysicsActor::DYNAMIC         : 0;
			pa.flags |= (json_object::has(actor, "kinematic")       && sjson::parse_bool(actor["kinematic"])      ) ? PhysicsActor::KINEMATIC       : 0;
			pa.flags |= (json_object::has(actor, "disable_gravity") && sjson::parse_bool(actor["disable_gravity"])) ? PhysicsActor::DISABLE_GRAVITY : 0;
			pa.flags |= (json_object::has(actor, "trigger")         && sjson::parse_bool(actor["trigger"])        ) ? PhysicsActor::TRIGGER         : 0;

			array::push_back(objects, pa);
		}
	}

	struct CollisionFilterCompiler
	{
		CompileOptions& _opts;
		HashMap<StringId32, u32> _filter_map;
		Array<PhysicsCollisionFilter> _filters;
		u32 _filter;

		CollisionFilterCompiler(CompileOptions& opts)
			: _opts(opts)
			, _filter_map(default_allocator())
			, _filters(default_allocator())
			, _filter(1)
		{
		}

		void parse(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject object(ta);
			sjson::parse(json, object);

			auto cur = json_object::begin(object);
			auto end = json_object::end(object);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(object, cur);

				const StringView key = cur->first;
				const StringId32 id  = StringId32(key.data(), key.length());

				hash_map::set(_filter_map, id, new_filter_mask());
			}

			cur = json_object::begin(object);
			end = json_object::end(object);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(object, cur);

				const StringView key = cur->first;
				const char* value    = cur->second;
				const StringId32 id  = StringId32(key.data(), key.length());

				TempAllocator4096 ta;
				JsonObject filter(ta);
				sjson::parse_object(value, filter);

				JsonArray collides_with(ta);
				sjson::parse_array(filter["collides_with"], collides_with);

				u32 mask = 0;
				for (u32 i = 0; i < array::size(collides_with); ++i)
				{
					const StringId32 fi = sjson::parse_string_id(collides_with[i]);
					mask |= filter_to_mask(fi);
				}

				// Build mask
				PhysicsCollisionFilter pcf;
				pcf.name = id;
				pcf.me   = filter_to_mask(id);
				pcf.mask = mask;

				array::push_back(_filters, pcf);
			}
		}

		u32 new_filter_mask()
		{
			DATA_COMPILER_ASSERT(_filter != 0x80000000u
				, _opts
				, "Too many collision filters"
				);

			const u32 f = _filter;
			_filter = _filter << 1;
			return f;
		}

		u32 filter_to_mask(StringId32 filter)
		{
			DATA_COMPILER_ASSERT(hash_map::has(_filter_map, filter)
				, _opts
				, "Filter not found"
				);

			return hash_map::get(_filter_map, filter, 0u);
		}
	};

	s32 compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		Array<PhysicsMaterial> materials(default_allocator());
		Array<PhysicsActor> actors(default_allocator());
		CollisionFilterCompiler cfc(opts);

		// Parse materials
		if (json_object::has(object, "collision_filters"))
			cfc.parse(object["collision_filters"]);
		if (json_object::has(object, "materials"))
			parse_materials(object["materials"], materials);
		if (json_object::has(object, "actors"))
			parse_actors(object["actors"], actors);

		// Setup struct for writing
		PhysicsConfigResource pcr;
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
		offt += sizeof(PhysicsCollisionFilter) * pcr.num_filters;

		// Write all
		opts.write(pcr.version);
		opts.write(pcr.num_materials);
		opts.write(pcr.materials_offset);
		opts.write(pcr.num_actors);
		opts.write(pcr.actors_offset);
		opts.write(pcr.num_filters);
		opts.write(pcr.filters_offset);

		// Write materials
		for (u32 i = 0; i < pcr.num_materials; ++i)
		{
			opts.write(materials[i].name._id);
			opts.write(materials[i].friction);
			opts.write(materials[i].rolling_friction);
			opts.write(materials[i].restitution);
		}

		// Write actors
		for (u32 i = 0; i < pcr.num_actors; ++i)
		{
			opts.write(actors[i].name._id);
			opts.write(actors[i].linear_damping);
			opts.write(actors[i].angular_damping);
			opts.write(actors[i].flags);
		}

		// Write collision filters
		for (u32 i = 0; i < array::size(cfc._filters); ++i)
		{
			opts.write(cfc._filters[i].name._id);
			opts.write(cfc._filters[i].me);
			opts.write(cfc._filters[i].mask);
		}

		return 0;
	}

} // namespace physics_config_resource_internal

namespace physics_config_resource
{
	const PhysicsMaterial* material(const PhysicsConfigResource* pcr, StringId32 name)
	{
		const PhysicsMaterial* begin = (PhysicsMaterial*)((const char*)pcr + pcr->materials_offset);
		for (u32 i = 0; i < pcr->num_materials; ++i)
		{
			if (begin[i].name == name)
				return &begin[i];
		}

		CE_FATAL("Material not found");
		return NULL;
	}

	const PhysicsActor* actor(const PhysicsConfigResource* pcr, StringId32 name)
	{
		const PhysicsActor* begin = (PhysicsActor*)((const char*)pcr + pcr->actors_offset);
		for (u32 i = 0; i < pcr->num_actors; ++i)
		{
			if (begin[i].name == name)
				return &begin[i];
		}

		CE_FATAL("Actor not found");
		return NULL;
	}

	const PhysicsCollisionFilter* filter(const PhysicsConfigResource* pcr, StringId32 name)
	{
		const PhysicsCollisionFilter* begin = (PhysicsCollisionFilter*)((const char*)pcr + pcr->filters_offset);
		for (u32 i = 0; i < pcr->num_filters; ++i)
		{
			if (begin[i].name == name)
				return &begin[i];
		}

		CE_FATAL("Filter not found");
		return NULL;
	}

} // namespace physics_config_resource

} // namespace crown
