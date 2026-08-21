/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "resource/mesh_animation.h"
#   include "resource/types.h"

namespace crown
{
namespace gltf
{
	/// Parses and bakes an animation for the skeleton containing @a skin_name.
	s32 parse(MeshAnimation &a, const char *path, const char *skin_name, CompileOptions &opts);

} // namespace gltf

} // namespace crown

#endif // if CROWN_CAN_COMPILE
