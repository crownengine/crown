/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "compiler_types.h"
#include "filesystem_types.h"

namespace crown
{

void resource_on_compile(StringId64 type, const char* path, CompileOptions& opts);
void* resource_on_load(StringId64 type, File& file, Allocator& a);
void resource_on_online(StringId64 type, StringId64 id, ResourceManager& rm);
void resource_on_offline(StringId64 type, StringId64 id, ResourceManager& rm);
void resource_on_unload(StringId64 type, Allocator& allocator, void* resource);

} // namespace crown
