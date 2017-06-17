/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "core/containers/array.h"
#include "core/containers/sort_map.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/memory/allocator.h"
#include "resource/types.h"
#include "resource/unit_compiler.h"

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
