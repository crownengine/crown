/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/filesystem/path.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "resource/resource_id.inl"

namespace crown
{
ResourceId resource_id(const char* path)
{
	CE_ASSERT(path::extension(path) != NULL);
	const char* type = path::extension(path);
	const u32 len = u32(type - path - 1);
	return resource_id(type, strlen32(type), path, len);
}

void destination_path(DynamicString& path, ResourceId id)
{
	TempAllocator128 ta;
	DynamicString id_hex(ta);
	id_hex.from_string_id(id);
	path::join(path, CROWN_DATA_DIRECTORY, id_hex.c_str());
}

} // namespace crown
