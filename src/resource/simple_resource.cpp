/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/filesystem/file_memory.inl"
#include "core/memory/allocator.h"
#include "resource/simple_resource.h"

namespace crown
{
namespace simple_resource
{
	void *load(File &file, Allocator &a)
	{
		const u32 file_size = file.size();
		void *data = a.allocate(file_size, 16);
		file.read(data, file_size);
		return data;
	}

	void unload(Allocator &a, void *data)
	{
		a.deallocate(data);
	}

	void *load_from_bundle(File &file, Allocator &a)
	{
		CE_UNUSED(a);
		return (void *)((FileMemory &)file)._memory;
	}

	void unload_from_bundle(Allocator &a, void *data)
	{
		CE_UNUSED_2(a, data);
		return;
	}

} // namespace simple_resource

} // namespace crown
