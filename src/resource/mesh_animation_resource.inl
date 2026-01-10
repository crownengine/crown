/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/mesh_animation_resource.h"

namespace crown
{
namespace mesh_animation_resource
{
	const AnimationKey *animation_keys(const MeshAnimationResource *mar)
	{
		return (AnimationKey *)((char *)mar + mar->keys_offset);
	}

	const u16 *bone_ids(const MeshAnimationResource *mar)
	{
		return (u16 *)((char *)mar + mar->bone_ids_offset);
	}

} // namespace mesh_animation_resource

} // namespace crown
