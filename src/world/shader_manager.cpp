/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
ShaderManager::ShaderManager(Allocator& a)
	: _shader_map(a)
{
}

void* ShaderManager::load(File& file, Allocator& a)
{
	BinaryReader br(file);
	u32 version;
	br.read(version);
	CE_ASSERT(version == RESOURCE_HEADER(RESOURCE_VERSION_SHADER), "Wrong version");

	u32 num;
	br.read(num);

	ShaderResource* sr = CE_NEW(a, ShaderResource)(a);
	array::resize(sr->_data, num);

	for (u32 i = 0; i < num; ++i)
	{
		u32 shader_name;
		br.read(shader_name);

		u64 render_state;
		br.read(render_state);

		u32 num_samplers;
		br.read(num_samplers);
		CE_ENSURE(num_samplers < countof(sr->_data[i].samplers));

		for (u32 s = 0; s < num_samplers; ++s)
		{
			u32 sampler_name;
			br.read(sampler_name);

			u32 sampler_state;
			br.read(sampler_state);

			sr->_data[i].samplers[s].name = sampler_name;
			sr->_data[i].samplers[s].state = sampler_state;
		}

		u32 vs_code_size;
		br.read(vs_code_size);
		const bgfx::Memory* vsmem = bgfx::alloc(vs_code_size);
		br.read(vsmem->data, vs_code_size);

		u32 fs_code_size;
		br.read(fs_code_size);
		const bgfx::Memory* fsmem = bgfx::alloc(fs_code_size);
		br.read(fsmem->data, fs_code_size);

		sr->_data[i].name._id = shader_name;
		sr->_data[i].state = render_state;
		sr->_data[i].vsmem = vsmem;
		sr->_data[i].fsmem = fsmem;
	}

	return sr;
}

void ShaderManager::online(StringId64 id, ResourceManager& rm)
{
	const ShaderResource* shader = (ShaderResource*)rm.get(RESOURCE_TYPE_SHADER, id);

	for (u32 i = 0; i < array::size(shader->_data); ++i)
	{
		const ShaderResource::Data& data = shader->_data[i];

		bgfx::ShaderHandle vs = bgfx::createShader(data.vsmem);
		CE_ASSERT(bgfx::isValid(vs), "Failed to create vertex shader");
		bgfx::ShaderHandle fs = bgfx::createShader(data.fsmem);
		CE_ASSERT(bgfx::isValid(fs), "Failed to create fragment shader");
		bgfx::ProgramHandle program = bgfx::createProgram(vs, fs, true);
		CE_ASSERT(bgfx::isValid(program), "Failed to create GPU program");

		add_shader(data.name, data.state, data.samplers, program);
	}
}

void ShaderManager::offline(StringId64 id, ResourceManager& rm)
{
	const ShaderResource* shader = (ShaderResource*)rm.get(RESOURCE_TYPE_SHADER, id);

	for (u32 i = 0; i < array::size(shader->_data); ++i)
	{
		const ShaderResource::Data& data = shader->_data[i];

		ShaderData sd;
		sd.state = BGFX_STATE_DEFAULT;
		sd.program = BGFX_INVALID_HANDLE;
		sd = hash_map::get(_shader_map, data.name, sd);

		bgfx::destroy(sd.program);

		hash_map::remove(_shader_map, data.name);
	}
}

void ShaderManager::unload(Allocator& a, void* res)
{
	CE_DELETE(a, (ShaderResource*)res);
}

void ShaderManager::add_shader(StringId32 name, u64 state, const ShaderResource::Sampler samplers[4], bgfx::ProgramHandle program)
{
	ShaderData sd;
	sd.state = state;
	memcpy(sd.samplers, samplers, sizeof(sd.samplers));
	sd.program = program;
	hash_map::set(_shader_map, name, sd);
}

u32 ShaderManager::sampler_state(StringId32 shader_id, StringId32 sampler_name)
{
	CE_ASSERT(hash_map::has(_shader_map, shader_id), "Shader not found");
	ShaderData sd;
	sd.state = BGFX_STATE_DEFAULT;
	sd.program = BGFX_INVALID_HANDLE;
	sd = hash_map::get(_shader_map, shader_id, sd);

	for (u32 i = 0; i < countof(sd.samplers); ++i)
	{
		if (sd.samplers[i].name == sampler_name._id)
			return sd.samplers[i].state;
	}

	CE_FATAL("Sampler not found");
	return UINT32_MAX;
}

void ShaderManager::submit(StringId32 shader_id, u8 view_id, s32 depth, u64 state)
{
	CE_ASSERT(hash_map::has(_shader_map, shader_id), "Shader not found");
	ShaderData sd;
	sd.state = BGFX_STATE_DEFAULT;
	sd.program = BGFX_INVALID_HANDLE;
	sd = hash_map::get(_shader_map, shader_id, sd);

	bgfx::setState(state != UINT64_MAX ? state : sd.state);
	bgfx::submit(view_id, sd.program, depth);
}

} // namespace crown
