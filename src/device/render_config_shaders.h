/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/strings/string_id.h"
#include "resource/types.h"

namespace crown
{
/// Owns one resource reference per shader library required by the active layout.
/// Acquire additions before destroying the old pipeline; release removals before
/// resolving the new pipeline's program caches.
struct RenderConfigShaders
{
	ResourceManager *_resource_manager;
	Array<StringId64> _libraries;

	RenderConfigShaders(Allocator &a, ResourceManager &rm);
	~RenderConfigShaders();
	RenderConfigShaders(const RenderConfigShaders &) = delete;
	RenderConfigShaders &operator=(const RenderConfigShaders &) = delete;

	void acquire(const RenderConfigResource &resource);
	void release_unused(const RenderConfigResource &resource);
	void clear();
};

} // namespace crown
