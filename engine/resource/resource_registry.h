/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
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

void resource_on_compile(uint64_t type, const char* path, CompileOptions& opts);
void* resource_on_load(uint64_t type, File& file, Allocator& a);
void resource_on_online(uint64_t type, StringId64 id, ResourceManager& rm);
void resource_on_offline(uint64_t type, StringId64 id, ResourceManager& rm);
void resource_on_unload(uint64_t type, Allocator& allocator, void* resource);

} // namespace crown
