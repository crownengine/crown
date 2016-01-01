/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "shader_manager.h"
#include "sort_map.h"
#include "resource_manager.h"
#include "reader_writer.h"
#include "shader.h"
#include "string_utils.h"
#include "array.h"

namespace crown
{

ShaderManager::ShaderManager(Allocator& a)
	: _shader_map(a)
{
}

void* ShaderManager::load(File& file, Allocator& a)
{
	BinaryReader br(file);
	uint32_t version;
	br.read(version);
	CE_ASSERT(version == SHADER_VERSION, "Wrong version");

	uint32_t num;
	br.read(num);

	ShaderResource* sr = CE_NEW(a, ShaderResource)(a);
	array::resize(sr->_data, num);

	for (uint32_t i = 0; i < num; ++i)
	{
		uint32_t shader_name;
		br.read(shader_name);

		uint32_t vs_code_size;
		br.read(vs_code_size);
		const bgfx::Memory* vsmem = bgfx::alloc(vs_code_size);
		br.read(vsmem->data, vs_code_size);

		uint32_t fs_code_size;
		br.read(fs_code_size);
		const bgfx::Memory* fsmem = bgfx::alloc(fs_code_size);
		br.read(fsmem->data, fs_code_size);

		sr->_data[i].name._id = shader_name;
		sr->_data[i].vsmem = vsmem;
		sr->_data[i].fsmem = fsmem;
	}

	return sr;
}

void ShaderManager::online(StringId64 id, ResourceManager& rm)
{
	const ShaderResource* shader = (ShaderResource*)rm.get(SHADER_TYPE, id);

	for (uint32_t i = 0; i < array::size(shader->_data); ++i)
	{
		const ShaderResource::Data& data = shader->_data[i];

		bgfx::ShaderHandle vs = bgfx::createShader(data.vsmem);
		CE_ASSERT(bgfx::isValid(vs), "Failed to create vertex shader");
		bgfx::ShaderHandle fs = bgfx::createShader(data.fsmem);
		CE_ASSERT(bgfx::isValid(fs), "Failed to create fragment shader");
		bgfx::ProgramHandle program = bgfx::createProgram(vs, fs, true);
		CE_ASSERT(bgfx::isValid(program), "Failed to create GPU program");

		add_shader(data.name, program);
	}
}

void ShaderManager::offline(StringId64 id, ResourceManager& rm)
{
	const ShaderResource* shader = (ShaderResource*)rm.get(SHADER_TYPE, id);

	for (uint32_t i = 0; i < array::size(shader->_data); ++i)
	{
		const ShaderResource::Data& data = shader->_data[i];

		bgfx::destroyProgram(get(data.name));

		sort_map::remove(_shader_map, data.name);
		sort_map::sort(_shader_map);
	}
}

void ShaderManager::unload(Allocator& a, void* res)
{
	CE_DELETE(a, (ShaderResource*)res);
}

void ShaderManager::add_shader(StringId32 name, bgfx::ProgramHandle program)
{
	ShaderData sd;
	sd.program = program;
	sort_map::set(_shader_map, name, sd);
	sort_map::sort(_shader_map);
}

bgfx::ProgramHandle ShaderManager::get(StringId32 shader)
{
	CE_ASSERT(sort_map::has(_shader_map, shader), "Shader not found");
	ShaderData deffault;
	deffault.program = BGFX_INVALID_HANDLE;
	return sort_map::get(_shader_map, shader, deffault).program;
}

} // namespace crown
