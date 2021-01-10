/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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
	const char* type = resource_type(path);
	const u32 name_len = resource_name_length(type, path);
	return resource_id(type, strlen32(type), path, name_len);
}

void destination_path(DynamicString& path, ResourceId id)
{
	TempAllocator128 ta;
	DynamicString id_hex(ta);
	id_hex.from_string_id(id);
	path::join(path, CROWN_DATA_DIRECTORY, id_hex.c_str());
}

} // namespace crown
