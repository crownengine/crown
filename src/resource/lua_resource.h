/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "resource/types.h"
#include "resource/types.h"

namespace crown
{
struct LuaResource
{
	u32 version;
	u32 size;
//	char program[size]
};

namespace lua_resource_internal
{
	///
	void find_requirements(HashSet<StringView> &requirements, const char *lua);

} // namespace lua_resource_internal

namespace lua_resource
{
	/// Returns the lua program.
	const char *program(const LuaResource *lr);

} // namespace lua_resource

} // namespace crown
