/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#include "resource/sound.h"
#include "resource/types.h"

namespace crown
{
namespace wav
{
	///
	s32 parse(Sound &s, Buffer &buf, CompileOptions &opts);

} // namespace wav

} // namespace crown

#endif
