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
namespace fbx
{
	///
	s32 parse(AnimationSkeleton &s, Buffer &buf, CompileOptions &opts);

} // namespace fbx

} // namespace crown

#endif // if CROWN_CAN_COMPILE
