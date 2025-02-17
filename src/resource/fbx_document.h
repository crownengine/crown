/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "core/filesystem/types.h"
#   include "core/math/types.h"
#   include "core/memory/types.h"
#   include "core/strings/dynamic_string.h"
#   include "resource/types.h"

struct ufbx_scene; // Avoids #include <ufbx.h>
struct ufbx_node;  // Ditto.

namespace crown
{
struct FBXDocument
{
	ufbx_scene *scene;

	///
	explicit FBXDocument(Allocator &a);

	///
	~FBXDocument();
};

namespace fbx
{
	///
	s32 parse(FBXDocument &fbx, Buffer &buf, CompileOptions &opts);

	///
	s32 parse(FBXDocument &fbx, const char *path, CompileOptions &opts);

} // namespace fbx

} // namespace crown

#endif // if CROWN_CAN_COMPILE
