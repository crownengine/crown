/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"
#include "core/math/types.h"
#include "core/types.h"
#include "resource/types.h"

namespace crown
{
#define MESH_SKELETON_MAX_BONES 1024

struct BoneTransform
{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
};

struct MeshSkeletonResource
{
	u32 version;
	u32 num_bones;               ///< Number of bones in the skeleton.
	u32 local_transforms_offset; ///< Offset to first local transform.
	u32 parents_offset;          ///< Offset to first parent of first transform.
	u32 binding_matrices_offset; ///< Offset to first binding matrix.
	// BoneTransform local_transforms[num_bones];
	// u32 parents[num_bones];
	// Matrix4x4 binding_matrices[num_bones];
};

#if CROWN_CAN_COMPILE
namespace mesh_skeleton_resource_internal
{
	///
	s32 compile(CompileOptions &opts);

} // namespace mesh_skeleton_resource_internal
#endif

namespace mesh_skeleton_resource
{
	///
	const BoneTransform *local_transforms(const MeshSkeletonResource *asr);

	///
	const u32 *parents(const MeshSkeletonResource *asr);

	///
	const Matrix4x4 *binding_matrices(const MeshSkeletonResource *asr);

} // namespace mesh_skeleton_resource

} // namespace crown
