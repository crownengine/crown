#pragma once

// Supporting APIs for the real ShaderManager below. Shader blobs contain a
// fixture index, not GPU bytecode. No shader compiler or GPU is being emulated.
#include "test_sdk.h"

#define BGFX_STATE_DEFAULT 0u
#define BGFX_STENCIL_NONE 0u
#define RESOURCE_VERSION_SHADER 1u
#define CE_FATAL(...) crown::error::abort(__VA_ARGS__)

namespace bgfx
{
struct ShaderHandle { uint16_t idx; };
struct RendererType { enum Enum { Direct3D11, OpenGL, OpenGLES, Vulkan, Count }; };
struct UniformInfo { char name[256]; UniformType::Enum type; uint16_t num; };
struct UniformDecl { std::string name; UniformType::Enum type; };
struct ProgramRecord { ShaderHandle vs, fs; };
inline RendererType::Enum renderer = RendererType::OpenGL;
inline std::vector<std::vector<UniformDecl>> shader_fixtures;
inline std::map<uint16_t, std::vector<UniformHandle>> shaders;
inline std::map<uint16_t, ProgramRecord> programs;
inline uint16_t next_shader = 0, next_program = 0;
inline uint32_t reflection_queries = 0;

inline RendererType::Enum getRendererType() { return renderer; }
inline const char *getRendererName(RendererType::Enum) { return "test renderer"; }

inline ShaderHandle createShader(const Memory *memory)
{
	assert(memory->size == sizeof(uint32_t));
	uint32_t fixture;
	memcpy(&fixture, memory->data, sizeof(fixture));
	release_memory(memory);
	const ShaderHandle handle = { next_shader++ };
	auto &result = shaders[handle.idx];
	for (const UniformDecl &decl : shader_fixtures.at(fixture))
		result.push_back(createUniform(decl.name.c_str(), decl.type));
	return handle;
}

inline uint16_t getShaderUniforms(ShaderHandle shader, UniformHandle *out = nullptr, uint16_t capacity = 0)
{
	++reflection_queries;
	const auto &list = shaders.at(shader.idx);
	if (out)
		std::copy_n(list.begin(), std::min<size_t>(capacity, list.size()), out);
	return uint16_t(list.size());
}

inline void getUniformInfo(UniformHandle handle, UniformInfo &info)
{
	++reflection_queries;
	const auto &record = uniforms.at(handle.idx);
	assert(record.name.size() < sizeof(info.name));
	memcpy(info.name, record.name.c_str(), record.name.size() + 1);
	info.type = record.type;
	info.num = record.count;
}

inline ProgramHandle createProgram(ShaderHandle vs, ShaderHandle fs, bool owns)
{
	assert(owns && shaders.count(vs.idx) && shaders.count(fs.idx));
	const ProgramHandle handle = { next_program++ };
	programs[handle.idx] = { vs, fs };
	return handle;
}

inline void destroy(ShaderHandle shader)
{
	for (UniformHandle uniform : shaders.at(shader.idx))
		destroy(uniform);
	shaders.erase(shader.idx);
}

inline void destroy(ProgramHandle program)
{
	const ProgramRecord record = programs.at(program.idx);
	destroy(record.vs);
	destroy(record.fs);
	programs.erase(program.idx);
}
} // namespace bgfx

namespace crown
{
struct FileMemory
{
	const u8 *data;
	u32 offset = 0;
	FileMemory(const void *p, u32) : data((const u8 *)p) {}
	u32 position() const { return offset; }
};

struct BinaryReader
{
	FileMemory &file;
	explicit BinaryReader(FileMemory &f) : file(f) {}
	template<class T> void read(T &value)
	{
		memcpy(&value, file.data + file.offset, sizeof(T));
		file.offset += sizeof(T);
	}
	void skip(u32 size) { file.offset += size; }
};

static const u32 RESOURCE_TYPE_SHADER = 0;
struct ResourceManager
{
	const void *data = nullptr;
	const void *get(u32, StringId64) { return data; }
};
} // namespace crown
