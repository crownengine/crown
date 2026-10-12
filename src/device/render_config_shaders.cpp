/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/strings/string_id.inl"
#include "device/render_config_shaders.h"
#include "resource/render_config_resource.inl"
#include "resource/resource_manager.h"

namespace crown
{
RenderConfigShaders::RenderConfigShaders(Allocator &a, ResourceManager &rm)
	: _resource_manager(&rm)
	, _libraries(a)
{
}

RenderConfigShaders::~RenderConfigShaders()
{
	clear();
}

void RenderConfigShaders::acquire(const RenderConfigResource &resource)
{
	const u32 *libraries = render_config_resource::shader_libraries(&resource);
	for (u32 i = 0; i < resource.num_shader_libraries; ++i) {
		const StringId64 id(render_config_resource::name(&resource, libraries[i]));
		u32 j = 0;
		for (; j < array::size(_libraries); ++j) {
			if (_libraries[j] == id)
				break;
		}
		if (j != array::size(_libraries))
			continue;

		// At boot the package has already loaded these resources, including
		// in bundle mode. Hot reload is supported only for loose resources.
		_resource_manager->load(NULL, RESOURCE_TYPE_SHADER, id, 0);
		do {
			_resource_manager->complete_requests();
		} while (!_resource_manager->can_get(RESOURCE_TYPE_SHADER, id, ResourceManager::CanGetFlags::STRICT));
		array::push_back(_libraries, id);
	}
}

void RenderConfigShaders::release_unused(const RenderConfigResource &resource)
{
	const u32 *libraries = render_config_resource::shader_libraries(&resource);
	for (u32 i = array::size(_libraries); i > 0; --i) {
		const StringId64 id = _libraries[i - 1];
		bool required = false;
		for (u32 j = 0; j < resource.num_shader_libraries; ++j)
			required |= id == StringId64(render_config_resource::name(&resource, libraries[j]));
		if (required)
			continue;
		_resource_manager->unload(RESOURCE_TYPE_SHADER, id);
		_libraries[i - 1] = _libraries[array::size(_libraries) - 1];
		array::pop_back(_libraries);
	}
}

void RenderConfigShaders::clear()
{
	for (u32 i = array::size(_libraries); i > 0; --i)
		_resource_manager->unload(RESOURCE_TYPE_SHADER, _libraries[i - 1]);
	array::clear(_libraries);
}

} // namespace crown
