/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/memory/globals.h"
#include "core/memory/memory.inl"
#include "resource/compile_options.inl"
#include "resource/unit_compiler.h"
#include "resource/unit_resource.h"

namespace crown
{
#if CROWN_CAN_COMPILE
namespace unit_resource_internal
{
	s32 compile(CompileOptions &opts)
	{
		UnitCompiler uc(default_allocator());
		s32 err = unit_compiler::parse_unit(uc, opts.source_path(), opts);
		ENSURE_OR_RETURN(err == 0, opts);
		Buffer blob(default_allocator());
		err = unit_compiler::blob(blob, uc);
		ENSURE_OR_RETURN(err == 0, opts);
		opts.write(blob);
		return 0;
	}

} // namespace unit_resource_internal
#endif

} // namespace crown
