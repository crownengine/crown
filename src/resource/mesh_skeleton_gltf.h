/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "resource/mesh_skeleton.h"
#   include "resource/types.h"

namespace crown
{
namespace gltf
{
	/// Parses the skeleton containing @a skin_name from the glTF document at @a path.
	s32 parse(AnimationSkeleton &s, const char *path, const char *skin_name, CompileOptions &opts);

} // namespace gltf

} // namespace crown

#endif // if CROWN_CAN_COMPILE
