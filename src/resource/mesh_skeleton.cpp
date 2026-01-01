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
#   include "resource/compile_options.inl"

namespace crown
{
namespace mesh_skeleton
{
	static s32 parse_internal(AnimationSkeleton &s, Buffer &buf, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		DynamicString source(ta);
		RETURN_IF_ERROR(sjson::parse_string(source, obj["source"]), opts);

		RETURN_IF_FILE_MISSING(source.c_str(), opts);
		Buffer fbx_buf = opts.read(source.c_str());
		return fbx::parse(s, fbx_buf, opts);
	}

	s32 parse(AnimationSkeleton &s, const char *path, CompileOptions &opts)
	{
		RETURN_IF_FILE_MISSING(path, opts);
		Buffer buf = opts.read(path);
		return parse_internal(s, buf, opts);
	}

	s32 parse(AnimationSkeleton &s, CompileOptions &opts)
	{
		Buffer buf = opts.read();
		return parse_internal(s, buf, opts);
	}

} // namespace mesh_skeleton

AnimationSkeleton::AnimationSkeleton(Allocator &a)
	: local_transforms(a)
	, parents(a)
	, binding_matrices(a)
{
}

} // namespace crown
#endif // if CROWN_CAN_COMPILE
