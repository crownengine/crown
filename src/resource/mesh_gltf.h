/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "resource/mesh.h"
#   include "resource/types.h"

namespace crown
{
namespace gltf
{
	struct MeshImportOptions
	{
		bool rigid_skinning;
	};

	/// Parses the glTF mesh at @a path.
	s32 parse(Mesh &m, const char *path, const MeshImportOptions &import_options, CompileOptions &opts);

} // namespace gltf

} // namespace crown

#endif // if CROWN_CAN_COMPILE
