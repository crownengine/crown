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
		Buffer config = opts.read(path);
		array::push_back(config, '\0');
		opts.write(config);
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		return res;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}
} // namespace config_resource

} // namespace crown
