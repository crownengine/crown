/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/memory/types.h"
#include "resource/types.h"
#include "resource/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct TextureResource
{
	const bgfx::Memory *mem;
	bgfx::TextureHandle handle;
};

} // namespace crown
