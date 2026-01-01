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

namespace unit_resource
{
	const u32 *parents(const UnitResource *ur)
	{
		return (u32 *)&ur[1];
	}

	const ComponentData *component_type_data(const UnitResource *ur, const ComponentData *component)
	{
		if (component == NULL)
			return (ComponentData *)(parents(ur) + ur->num_units);
		else
			return (ComponentData *)memory::align_top(component_payload(component) + component->data_size, alignof(ComponentData));
	}

	const u32 *component_unit_index(const ComponentData *component)
	{
		return (u32 *)(&component[1]);
	}

	const char *component_payload(const ComponentData *component)
	{
		const u32 *unit_index = component_unit_index(component);
		return (char *)memory::align_top(unit_index + component->num_instances, 16);
	}

} // namespace unit_resource

} // namespace crown
