/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/filesystem/reader_writer.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "resource/resource_manager.h"
#include "resource/shader_resource.h"
#include "world/shader_manager.h"

namespace crown
{
static ShaderData SHADER_DATA_INVALID =
{
	BGFX_STATE_DEFAULT,
	BGFX_STENCIL_NONE,
	BGFX_STENCIL_NONE,
	{
		{
			0u, 0u
		},
		{
			0u, 0u
		},
		{
			0u, 0u
		},
		{
			0u, 0u
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

static const ShaderResource::Code *shader_code_for_backend(const ShaderResource::Data &data, ShaderBackend::Enum backend)
{
	for (u32 i = 0; i < data.num_codes; ++i) {
		if (data.codes[i].backend == backend)
			return &data.codes[i];
	}

	return NULL;
}

ShaderManager::ShaderManager(Allocator &a)
	: _shader_map(a)
{
}

void *ShaderManager::load(File &file, Allocator &a)
{
	BinaryReader br(file);
	u32 version;
	br.read(version);
	CE_ASSERT(version == RESOURCE_HEADER(RESOURCE_VERSION_SHADER), "Wrong version");

	u32 num;
	br.read(num);

	ShaderResource *sr = CE_NEW(a, ShaderResource)(a);
	array::resize(sr->_data, num);

	for (u32 i = 0; i < num; ++i) {
		br.read(sr->_data[i].name._id);
		br.read(sr->_data[i].state);
		br.read(sr->_data[i].stencil_front);
		br.read(sr->_data[i].stencil_back);

		u32 num_samplers;
		br.read(num_samplers);
		CE_ENSURE(num_samplers < countof(sr->_data[i].samplers));

		for (u32 s = 0; s < num_samplers; ++s) {
			u32 sampler_name;
			br.read(sampler_name);

			u32 sampler_state;
			br.read(sampler_state);

			sr->_data[i].samplers[s].name = sampler_name;
			sr->_data[i].samplers[s].state = sampler_state;
		}

		u32 num_codes;
		br.read(num_codes);
		CE_ENSURE(num_codes > 0 && num_codes <= countof(sr->_data[i].codes));
		sr->_data[i].num_codes = num_codes;

		for (u32 c = 0; c < num_codes; ++c) {
			ShaderResource::Code &code = sr->_data[i].codes[c];

			br.read(code.backend);
			CE_ENSURE(code.backend < ShaderBackend::COUNT);

			br.read(code.vs_size);
			CE_ENSURE(code.vs_size > 0);
			code.vs_data = (u8 *)a.allocate(code.vs_size);
			br.read(code.vs_data, code.vs_size);

			br.read(code.fs_size);
			CE_ENSURE(code.fs_size > 0);
			code.fs_data = (u8 *)a.allocate(code.fs_size);
			br.read(code.fs_data, code.fs_size);
		}
	}

	return sr;
}

void ShaderManager::online(StringId64 id, ResourceManager &rm)
{
	const ShaderResource *shader = (ShaderResource *)rm.get(RESOURCE_TYPE_SHADER, id);
	const bgfx::RendererType::Enum renderer_type = bgfx::getRendererType();
	const ShaderBackend::Enum backend = renderer_type_to_shader_backend(renderer_type);
	if (backend == ShaderBackend::COUNT) {
		CE_FATAL("Unsupported renderer type: %s", bgfx::getRendererName(renderer_type));
		return;
	}

	for (u32 i = 0; i < array::size(shader->_data); ++i) {
		const ShaderResource::Data &data = shader->_data[i];
		const ShaderResource::Code *code = shader_code_for_backend(data, backend);
		if (code == NULL) {
			CE_FATAL("Missing shader code for renderer type: %s", bgfx::getRendererName(renderer_type));
			continue;
		}

		bgfx::ShaderHandle vs = bgfx::createShader(bgfx::makeRef(code->vs_data, code->vs_size));
		CE_ASSERT(bgfx::isValid(vs), "Failed to create vertex shader");
		bgfx::ShaderHandle fs = bgfx::createShader(bgfx::makeRef(code->fs_data, code->fs_size));
		CE_ASSERT(bgfx::isValid(fs), "Failed to create fragment shader");
		bgfx::ProgramHandle program = bgfx::createProgram(vs, fs, true);
		CE_ASSERT(bgfx::isValid(program), "Failed to create GPU program");

		ShaderData sd;
		sd.state = data.state;
		sd.stencil_front = data.stencil_front;
		sd.stencil_back = data.stencil_back;
		memcpy(sd.samplers, data.samplers, sizeof(sd.samplers));
		sd.program = program;
#if CROWN_CAN_RELOAD
		sd.resource = shader;
#endif
		hash_map::set(_shader_map, data.name, sd);
	}
}

void ShaderManager::offline(StringId64 id, ResourceManager &rm)
{
	const ShaderResource *shader = (ShaderResource *)rm.get(RESOURCE_TYPE_SHADER, id);

	for (u32 i = 0; i < array::size(shader->_data); ++i) {
		const ShaderResource::Data &data = shader->_data[i];

		ShaderData sd;
		sd.state = BGFX_STATE_DEFAULT;
		sd.program = BGFX_INVALID_HANDLE;
		sd = hash_map::get(_shader_map, data.name, sd);

		bgfx::destroy(sd.program);

		hash_map::remove(_shader_map, data.name);
	}
}

void ShaderManager::unload(Allocator &a, void *res)
{
	CE_DELETE(a, (ShaderResource *)res);
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
