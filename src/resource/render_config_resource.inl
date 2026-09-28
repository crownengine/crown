/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "resource/render_config_resource.h"
#include "core/strings/string_id.inl"

namespace crown
{
namespace render_config_resource
{
	inline const RenderConditionName *conditions(const RenderConfigResource *r)
	{
		return (const RenderConditionName *)((const char *)r + r->conditions_offset);
	}

	inline const RenderResourceData *resources(const RenderConfigResource *r)
	{
		return (const RenderResourceData *)((const char *)r + r->resources_offset);
	}

	inline const RenderLayerData *layers(const RenderConfigResource *r)
	{
		return (const RenderLayerData *)((const char *)r + r->layers_offset);
	}

	inline const RenderGeneratorData *generators(const RenderConfigResource *r)
	{
		return (const RenderGeneratorData *)((const char *)r + r->generators_offset);
	}

	inline const RenderModifierData *modifiers(const RenderConfigResource *r)
	{
		return (const RenderModifierData *)((const char *)r + r->modifiers_offset);
	}

	inline const RenderInputData *inputs(const RenderConfigResource *r)
	{
		return (const RenderInputData *)((const char *)r + r->inputs_offset);
	}

	inline const RenderUniformData *uniforms(const RenderConfigResource *r)
	{
		return (const RenderUniformData *)((const char *)r + r->uniforms_offset);
	}

	inline const RenderShaderLayerData *shader_layers(const RenderConfigResource *r)
	{
		return (const RenderShaderLayerData *)((const char *)r + r->shader_layers_offset);
	}

	inline const char *name(const RenderConfigResource *r, u32 offset)
	{
		CE_ASSERT(offset < r->names_size, "Invalid render_config name offset");
		return (const char *)r + r->names_offset + offset;
	}

	inline bool enabled(const RenderCondition &condition, u32 flags)
	{
		return (flags & condition.required) == condition.required
			&& (flags & condition.excluded) == 0;
	}


} // namespace render_config_resource
} // namespace crown
