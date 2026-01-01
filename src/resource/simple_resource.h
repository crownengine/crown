/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"

namespace crown
{
namespace simple_resource
{
	///
	void *load(File &file, Allocator &a);

	///
	void unload(Allocator &a, void *data);

	///
	void *load_from_bundle(File &file, Allocator &a);

	///
	void unload_from_bundle(Allocator &a, void *data);

} // namespace simple_resource

} // namespace crown
