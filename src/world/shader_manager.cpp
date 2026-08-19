/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/hash_map.inl"
#include "core/filesystem/reader_writer.inl"
#include "core/filesystem/file_memory.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "resource/resource_manager.h"
#include "world/shader_manager.h"

namespace crown
{
static ShaderData SHADER_DATA_INVALID =
{
	BGFX_STATE_DEFAULT,
	BGFX_STENCIL_NONE,
	BGFX_STENCIL_NONE,
	0u,
	{
		{
			0u, 0u, 0u
		}
	},
	BGFX_INVALID_HANDLE,
#if CROWN_CAN_RELOAD
	NULL
#endif
};

static ShaderBackend::Enum renderer_type_to_shader_backend(bgfx::RendererType::Enum renderer_type)
{
	switch (renderer_type) {
	case bgfx::RendererType::Direct3D11: return ShaderBackend::HLSL;
	case bgfx::RendererType::OpenGL:     return ShaderBackend::GLSL;
	case bgfx::RendererType::OpenGLES:   return ShaderBackend::ESSL;
	case bgfx::RendererType::Vulkan:     return ShaderBackend::SPIRV;
	default:                             return ShaderBackend::COUNT;
	}
}

ShaderManager::ShaderManager(Allocator &a)
	: _shader_map(a)
{
}

void ShaderManager::create_shaders(const void *shader_resource)
{
	FileMemory fm(shader_resource, UINT32_MAX);
	BinaryReader br(fm);

	const bgfx::RendererType::Enum renderer_type = bgfx::getRendererType();
	const ShaderBackend::Enum backend = renderer_type_to_shader_backend(renderer_type);
	if (backend == ShaderBackend::COUNT) {
		CE_FATAL("Unsupported renderer type: %s", bgfx::getRendererName(renderer_type));
		return;
	}

	u32 version;
	br.read(version);
	CE_ASSERT(version == RESOURCE_HEADER(RESOURCE_VERSION_SHADER), "Wrong version");

	u32 num;
	br.read(num);

	for (u32 i = 0; i < num; ++i) {
		StringId32 name;
		br.read(name._id);

		ShaderData sd;
		br.read(sd.state);
		br.read(sd.stencil_front);
		br.read(sd.stencil_back);

		br.read(sd.num_samplers);
		CE_ENSURE(sd.num_samplers <= countof(sd.samplers));
		for (u32 s = 0; s < sd.num_samplers; ++s) {
			br.read(sd.samplers[s].name);
			br.read(sd.samplers[s].state);
			br.read(sd.samplers[s].stage);
		}

		u32 num_codes;
		br.read(num_codes);
		CE_ENSURE(num_codes > 0 && num_codes <= ShaderBackend::COUNT);

		const u8 *vs_data = NULL;
		u32 vs_size = 0;
		const u8 *fs_data = NULL;
		u32 fs_size = 0;

		for (u32 c = 0; c < num_codes; ++c) {
			u32 code_backend;
			br.read(code_backend);
			CE_ENSURE(code_backend < ShaderBackend::COUNT);

			u32 code_vs_size;
			br.read(code_vs_size);
			CE_ENSURE(code_vs_size > 0);
			const u8 *code_vs_data = (const u8 *)shader_resource + fm.position();
			br.skip(code_vs_size);

			u32 code_fs_size;
			br.read(code_fs_size);
			CE_ENSURE(code_fs_size > 0);
			const u8 *code_fs_data = (const u8 *)shader_resource + fm.position();
			br.skip(code_fs_size);

			if (code_backend == backend) {
				vs_data = code_vs_data;
				vs_size = code_vs_size;
				fs_data = code_fs_data;
				fs_size = code_fs_size;
			}
		}

		if (vs_data == NULL || fs_data == NULL) {
			CE_FATAL("Missing shader code for renderer type: %s", bgfx::getRendererName(renderer_type));
			continue;
		}

		char buf[STRING_ID32_BUF_LEN];
		CE_ASSERT(!hash_map::has(_shader_map, name)
			, "Duplicate shader variant: #ID(%s)"
			, name.to_string(buf, sizeof(buf))
			);
		CE_UNUSED(buf);

		bgfx::ShaderHandle vs = bgfx::createShader(bgfx::makeRef(vs_data, vs_size));
		CE_ASSERT(bgfx::isValid(vs), "Failed to create vertex shader");
		bgfx::ShaderHandle fs = bgfx::createShader(bgfx::makeRef(fs_data, fs_size));
		CE_ASSERT(bgfx::isValid(fs), "Failed to create fragment shader");
		sd.program = bgfx::createProgram(vs, fs, true);
		CE_ASSERT(bgfx::isValid(sd.program), "Failed to create GPU program");
#if CROWN_CAN_RELOAD
		sd.resource = shader_resource;
#endif
		hash_map::set(_shader_map, name, sd);
	}
}

void ShaderManager::destroy_shaders(const void *shader_resource)
{
	FileMemory fm(shader_resource, UINT32_MAX);
	BinaryReader br(fm);

	u32 version;
	br.read(version);
	CE_ASSERT(version == RESOURCE_HEADER(RESOURCE_VERSION_SHADER), "Wrong version");

	u32 num;
	br.read(num);

	for (u32 i = 0; i < num; ++i) {
		StringId32 name;
		br.read(name._id);

		u64 state;
		u32 stencil_front;
		u32 stencil_back;
		br.read(state);
		br.read(stencil_front);
		br.read(stencil_back);

		u32 num_samplers;
		br.read(num_samplers);
		CE_ENSURE(num_samplers <= countof(SHADER_DATA_INVALID.samplers));
		for (u32 s = 0; s < num_samplers; ++s) {
			u32 sampler_name;
			u32 sampler_state;
			u32 sampler_stage;
			br.read(sampler_name);
			br.read(sampler_state);
			br.read(sampler_stage);
		}

		u32 num_codes;
		br.read(num_codes);
		CE_ENSURE(num_codes > 0 && num_codes <= ShaderBackend::COUNT);
		for (u32 c = 0; c < num_codes; ++c) {
			u32 backend;
			br.read(backend);
			CE_ENSURE(backend < ShaderBackend::COUNT);

			u32 vs_size;
			br.read(vs_size);
			CE_ENSURE(vs_size > 0);
			br.skip(vs_size);

			u32 fs_size;
			br.read(fs_size);
			CE_ENSURE(fs_size > 0);
			br.skip(fs_size);
		}

		ShaderData sd = hash_map::get(_shader_map, name, SHADER_DATA_INVALID);
		bgfx::destroy(sd.program);
		hash_map::remove(_shader_map, name);
	}

	bgfx::frame();
	bgfx::frame();
}

void ShaderManager::online(StringId64 id, ResourceManager &rm)
{
	const void *shader_resource = rm.get(RESOURCE_TYPE_SHADER, id);
	create_shaders(shader_resource);
}

void ShaderManager::offline(StringId64 id, ResourceManager &rm)
{
	const void *shader_resource = rm.get(RESOURCE_TYPE_SHADER, id);
	destroy_shaders(shader_resource);
}

ShaderData ShaderManager::shader(StringId32 name)
{
	const ShaderData fallback = hash_map::get(_shader_map
		, STRING_ID_32("fallback", UINT32_C(0x251fc0cd))
		, SHADER_DATA_INVALID
		);
	return hash_map::get(_shader_map, name, fallback);
}

} // namespace crown
