/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "allocator.h"
#include "file.h"
#include "filesystem.h"
#include "resource_types.h"
#include "unit_compiler.h"

namespace crown
{
namespace unit_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		Buffer unit_data(default_allocator());

		UnitCompiler uc(opts);
		uc.compile_unit(path);
		unit_data = uc.get();

		opts.write(unit_data);
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t size = file.size();
		void* res = a.allocate(size);
		file.read(res, size);
		CE_ASSERT(*(uint32_t*)res == UNIT_VERSION, "Wrong version");
		return res;
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}

} // namespace unit_resource
} // namespace crown
