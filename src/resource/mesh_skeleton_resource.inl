/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/mesh_skeleton_resource.h"

namespace crown
{
namespace mesh_skeleton_resource
{
	const BoneTransform *local_transforms(const MeshSkeletonResource *asr)
	{
		return (BoneTransform *)((char *)asr + asr->local_transforms_offset);
	}

	const u32 *parents(const MeshSkeletonResource *asr)
	{
		return (u32 *)((char *)asr + asr->parents_offset);
	}

	const Matrix4x4 *binding_matrices(const MeshSkeletonResource *asr)
	{
		return (Matrix4x4 *)((char *)asr + asr->binding_matrices_offset);
	}

} // namespace mesh_skeleton_resource

} // namespace crown
