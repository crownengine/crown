/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/mesh_skeleton.h"

#if CROWN_CAN_COMPILE
#   include "core/json/json_object.inl"
#   include "core/json/sjson.h"
#   include "core/memory/temp_allocator.inl"
#   include "core/strings/dynamic_string.inl"
#   include "resource/mesh_skeleton_fbx.h"
#   include "resource/resource_id.inl"
#   include "device/log.h"
#   include "resource/compile_options.inl"

LOG_SYSTEM(MESH_SKELETON, "mesh_skeleton")

namespace crown
{
namespace mesh_skeleton
{
	static s32 parse_internal(AnimationSkeleton &s, Buffer &buf, const char *path, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf));

		if (json_object::has(obj, "mesh_resource")) {
			RETURN_IF_ERROR(sjson::parse_string(s.mesh_resource_name, obj["mesh_resource"]));
		} else {
			const char *type = resource_type(path);
			s.mesh_resource_name.set(path, resource_name_length(type, path));
		}
		WARN_IF_MISSING(MESH_SKELETON, "mesh", s.mesh_resource_name.c_str(), opts);
		opts.add_requirement("mesh", s.mesh_resource_name.c_str());

		// Mesh import options affect geometry, so derive the source from its mesh.
		DynamicString mesh_path(ta);
		mesh_path = s.mesh_resource_name;
		mesh_path += ".mesh";
		RETURN_IF_FILE_MISSING(MESH_SKELETON, mesh_path.c_str(), opts);
		Buffer mesh_buf = opts.read(mesh_path.c_str());
		JsonObject mesh_obj(ta);
		RETURN_IF_ERROR(sjson::parse(mesh_obj, mesh_buf));
		DynamicString source(ta);
		if (!json_object::has(mesh_obj, "source") && json_object::has(obj, "source")) {
			// Legacy skeletons may provide the source for an inline mesh.
			RETURN_IF_ERROR(sjson::parse_string(source, obj["source"]));
		} else {
			RETURN_IF_FALSE(MESH_SKELETON
				, json_object::has(mesh_obj, "source")
				, opts
				, "Owning mesh '%s' has no source"
				, s.mesh_resource_name.c_str()
				);
			RETURN_IF_ERROR(sjson::parse_string(source, mesh_obj["source"]));
		}

		RETURN_IF_FILE_MISSING(MESH_SKELETON, source.c_str(), opts);
		Buffer fbx_buf = opts.read(source.c_str());
		return fbx::parse(s, fbx_buf, opts);
	}

	s32 parse(AnimationSkeleton &s, const char *path, CompileOptions &opts)
	{
		RETURN_IF_FILE_MISSING(MESH_SKELETON, path, opts);
		Buffer buf = opts.read(path);
		return parse_internal(s, buf, path, opts);
	}

	s32 parse(AnimationSkeleton &s, CompileOptions &opts)
	{
		Buffer buf = opts.read();
		return parse_internal(s, buf, opts.source_path(), opts);
	}

} // namespace mesh_skeleton

AnimationSkeleton::AnimationSkeleton(Allocator &a)
	: local_transforms(a)
	, parents(a)
	, binding_matrices(a)
	, mesh_resource_name(a)
{
}

} // namespace crown
#endif // if CROWN_CAN_COMPILE
