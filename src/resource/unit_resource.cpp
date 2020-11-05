/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/memory/globals.h"
#include "resource/compile_options.inl"
#include "resource/unit_compiler.h"
#include "resource/unit_resource.h"

namespace crown
{
#if CROWN_CAN_COMPILE
namespace unit_resource_internal
{
	s32 compile(CompileOptions& opts)
	{
		Buffer unit_data(default_allocator());

		UnitCompiler uc(opts);
		s32 err = uc.compile_unit(opts.source_path());
		DATA_COMPILER_ENSURE(err == 0, opts);

		opts.write(uc.blob());
		return 0;
	}

} // namespace unit_resource_internal
#endif // CROWN_CAN_COMPILE

namespace unit_resource
{
	const ComponentData* component_data(const UnitResource* ur)
	{
		return (ComponentData*)(&ur[1]);
	}

} // namespace unit_resource

} // namespace crown
