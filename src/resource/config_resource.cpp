/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config_resource.h"
#include "allocator.h"
#include "compile_options.h"

namespace crown
{

namespace config_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		opts.write(opts.read(path));
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t size = file.size();
		char* res = (char*)a.allocate(size + 1);
		file.read(res, size + 1);
		res[size] = '\0';
		return res;
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}
} // namespace config_resource

} // namespace crown
