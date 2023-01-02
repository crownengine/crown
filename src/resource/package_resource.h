/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "core/strings/types.h"
#include "resource/types.h"
#include "resource/types.h"

namespace crown
{
struct PackageResource
{
	struct Resource
	{
		StringId64 type;
		StringId64 name;

		///
		Resource();

		///
		Resource(StringId64 t, StringId64 n);
	};

	Array<Resource> resources;

	///
	explicit PackageResource(Allocator &a);
};

namespace package_resource_internal
{
	s32 compile(CompileOptions &opts);
	void *load(File &file, Allocator &a);
	void unload(Allocator &allocator, void *resource);

} // namespace package_resource_internal

} // namespace crown
