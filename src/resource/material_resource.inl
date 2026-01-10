/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/material_resource.h"

namespace crown
{
namespace material_resource
{
	UniformData *uniform_data_array(const MaterialResource *mr)
	{
		return (UniformData *)((char *)mr + mr->uniform_data_offset);
	}

	u32 uniform_data_index(const MaterialResource *mr, const UniformData *ud, StringId32 name)
	{
		for (u32 i = 0, n = mr->num_uniforms; i < n; ++i) {
			if (ud[i].name == name)
				return i;
		}

		CE_FATAL("Unknown uniform name");
		return UINT32_MAX;
	}

	const char *uniform_name(const MaterialResource *mr, const UniformData *ud, u32 i)
	{
		return (char *)mr + mr->names_data_offset + ud[i].name_offset;
	}

	TextureData *texture_data_array(const MaterialResource *mr)
	{
		return (TextureData *)((char *)mr + mr->texture_data_offset);
	}

	u32 texture_data_index(const MaterialResource *mr, const TextureData *td, StringId32 name)
	{
		for (u32 i = 0, n = mr->num_textures; i < n; ++i) {
			if (td[i].name == name)
				return i;
		}

		CE_FATAL("Unknown texture name");
		return UINT32_MAX;
	}

	const char *texture_name(const MaterialResource *mr, const TextureData *td, u32 i)
	{
		return (char *)mr + mr->names_data_offset + td[i].sampler_name_offset;
	}

	UniformHandle *uniform_handle(const UniformData *ud, u32 i, char *dynamic)
	{
		return (UniformHandle *)(dynamic + ud[i].data_offset);
	}

	TextureHandle *texture_handle(const TextureData *td, u32 i, char *dynamic)
	{
		return (TextureHandle *)(dynamic + td[i].data_offset);
	}

} // namespace material_resource

} // namespace crown
