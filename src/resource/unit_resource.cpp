/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "allocator.h"
#include "array.h"
#include "file.h"
#include "filesystem.h"
#include "resource_types.h"
#include "sort_map.h"
#include "unit_compiler.h"

namespace crown
{
namespace unit_resource_internal
{
	void compile(const char* path, CompileOptions& opts)
	{
		Buffer unit_data(default_allocator());

		UnitCompiler uc(opts);
		uc.compile_unit(path);

		opts.write(uc.blob());
	}
} // namespace unit_resource_internal

} // namespace crown
