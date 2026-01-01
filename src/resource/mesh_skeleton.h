/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/types.h"
#   include "resource/mesh_skeleton_resource.h"

namespace crown
{
struct AnimationSkeleton
{
	Array<BoneTransform> local_transforms;
	Array<u32> parents;
	Array<Matrix4x4> binding_matrices;

	///
	explicit AnimationSkeleton(Allocator &a);
};

namespace mesh_skeleton
{
	///
	s32 parse(AnimationSkeleton &s, const char *path, CompileOptions &opts);

	///
	s32 parse(AnimationSkeleton &s, CompileOptions &opts);

	///
	s32 write(AnimationSkeleton &s, CompileOptions &opts);

} // namespace mesh_skeleton

} // namespace crown

#endif // if CROWN_CAN_COMPILE
