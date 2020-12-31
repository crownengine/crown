/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/types.h"
#include "resource/types.h"

namespace crown
{
namespace config_resource_internal
{
	s32 compile(CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);

} // namespace config_resource_internal

} // namespace crown
