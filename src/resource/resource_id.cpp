/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/filesystem/path.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "resource/resource_id.inl"

namespace crown
{
ResourceId resource_id(const StringView &path)
{
	const StringView type = resource_type(path);
	const u32 name_len = resource_name_length(type, path);
	return resource_id(type, StringView(path.data(), name_len));
}

ResourceId resource_id(const char *path)
{
	return resource_id(StringView(path));
}

void destination_path(DynamicString &path, ResourceId id)
{
	TempAllocator128 ta;
	DynamicString id_hex(ta);
	id_hex.from_string_id(id);
	path::join(path, CROWN_DATA_DIRECTORY, id_hex.c_str());
}

void stream_destination_path(DynamicString &path, ResourceId id)
{
	TempAllocator128 ta;
	DynamicString id_hex(ta);
	id_hex.from_string_id(id);
	id_hex += ".stream";
	path::join(path, CROWN_DATA_DIRECTORY, id_hex.c_str());
}

} // namespace crown
