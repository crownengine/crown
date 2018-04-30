/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
	void compile(CompileOptions& opts)
	{
		Buffer unit_data(default_allocator());

		UnitCompiler uc(opts);
		uc.compile_unit(opts.source_path());

		opts.write(uc.blob());
	}

} // namespace unit_resource_internal

} // namespace crown
