/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "shader.h"
#include "config.h"
#include "filesystem.h"
#include "json_parser.h"
#include "os.h"
#include "reader_writer.h"
#include "resource_manager.h"
#include "compile_options.h"

#if CROWN_DEBUG
		#define SHADERC_NAME "shadercDebug"
#else
		#define SHADERC_NAME "shadercRelease"
#endif // CROWN_DEBUG
		#define SHADERC_PATH SHADERC_NAME
#if CROWN_PLATFORM_WINDOWS
		#define SHADERC_PATH SHADERC_NAME".exe"
#endif // CROWN_PLATFORM_WINDOWS

namespace crown
{
namespace shader_resource
{
	static const char* _scplatform[Platform::COUNT] =
	{
		"linux",
		"windows",
		"android"
	};

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);
		JSONParser json(array::begin(buf));
		JSONElement root = json.root();

		DynamicString vs_code2;
		DynamicString fs_code2;
		DynamicString varying_def;
		DynamicString common_code;
		DynamicString vs_in_out;
		DynamicString fs_in_out;

		root.key("vs_code").to_string(vs_code2);
		root.key("fs_code").to_string(fs_code2);
		root.key("varying_def").to_string(varying_def);
		root.key("common").to_string(common_code);
		root.key("vs_in_out").to_string(vs_in_out);
		root.key("fs_in_out").to_string(fs_in_out);

		DynamicString vs_code;
		DynamicString fs_code;
		vs_code += vs_in_out;
		vs_code += common_code;
		vs_code += vs_code2;
		fs_code += fs_in_out;
		fs_code += common_code;
		fs_code += fs_code2;

		DynamicString vs_code_path;
		DynamicString fs_code_path;
		DynamicString varying_def_path;
		DynamicString tmpvs_path;
		DynamicString tmpfs_path;

		opts.get_absolute_path("vs_code.tmp", vs_code_path);
		opts.get_absolute_path("fs_code.tmp", fs_code_path);
		opts.get_absolute_path("varying.tmp", varying_def_path);
		opts.get_absolute_path("tmpvs", tmpvs_path);
		opts.get_absolute_path("tmpfs", tmpfs_path);

		File* vs_file = opts._fs.open(vs_code_path.c_str(), FOM_WRITE);
		vs_file->write(vs_code.c_str(), vs_code.length());
		opts._fs.close(vs_file);

		File* fs_file = opts._fs.open(fs_code_path.c_str(), FOM_WRITE);
		fs_file->write(fs_code.c_str(), fs_code.length());
		opts._fs.close(fs_file);

		File* varying_file = opts._fs.open(varying_def_path.c_str(), FOM_WRITE);
		varying_file->write(varying_def.c_str(), varying_def.length());
		opts._fs.close(varying_file);

		const char* compile_vs[] =
		{
			SHADERC_PATH,
			"-f", vs_code_path.c_str(),
			"-o", tmpvs_path.c_str(),
			"--varyingdef", varying_def_path.c_str(),
			"--type", "vertex",
			"--platform", _scplatform[opts.platform()],
#if CROWN_PLATFORM_WINDOWS
			"--profile", "vs_3_0",
#endif
			NULL
		};
		int exitcode = os::execute_process(compile_vs);
		CE_ASSERT(exitcode == 0, "Failed to compile vertex shader");

		const char* compile_fs[] =
		{
			SHADERC_PATH,
			"-f", fs_code_path.c_str(),
			"-o", tmpfs_path.c_str(),
			"--varyingdef", varying_def_path.c_str(),
			"--type", "fragment",
			"--platform", _scplatform[opts.platform()],
#if CROWN_PLATFORM_WINDOWS
			"--profile", "ps_3_0",
#endif
			NULL
		};
		exitcode = os::execute_process(compile_fs);
		if (exitcode)
		{
			opts.delete_file(tmpvs_path.c_str());
			CE_ASSERT(exitcode == 0, "Failed to compile fragment shader");
		}

		Buffer tmpvs = opts.read(tmpvs_path.c_str());
		Buffer tmpfs = opts.read(tmpfs_path.c_str());

		opts.write(uint32_t(1)); // version
		opts.write(uint32_t(array::size(tmpvs)));
		opts.write(array::begin(tmpvs), array::size(tmpvs));
		opts.write(uint32_t(array::size(tmpfs)));
		opts.write(array::begin(tmpfs), array::size(tmpfs));

		opts.delete_file(vs_code_path.c_str());
		opts.delete_file(fs_code_path.c_str());
		opts.delete_file(varying_def_path.c_str());
		opts.delete_file(tmpvs_path.c_str());
		opts.delete_file(tmpfs_path.c_str());
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader br(file);
		uint32_t version;
		br.read(version);

		uint32_t vs_code_size;
		br.read(vs_code_size);
		const bgfx::Memory* vsmem = bgfx::alloc(vs_code_size);
		br.read(vsmem->data, vs_code_size);

		uint32_t fs_code_size;
		br.read(fs_code_size);
		const bgfx::Memory* fsmem = bgfx::alloc(fs_code_size);
		br.read(fsmem->data, fs_code_size);

		Shader* shader = (Shader*) a.allocate(sizeof(Shader));
		shader->vs = vsmem;
		shader->fs = fsmem;
		shader->program.idx = bgfx::invalidHandle;

		return shader;
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		Shader* shader = (Shader*) rm.get(SHADER_TYPE, id);
		bgfx::ShaderHandle vs = bgfx::createShader(shader->vs);
		CE_ASSERT(bgfx::isValid(vs), "Failed to create vertex shader");
		bgfx::ShaderHandle fs = bgfx::createShader(shader->fs);
		CE_ASSERT(bgfx::isValid(fs), "Failed to create fragment shader");
		shader->program = bgfx::createProgram(vs, fs, true);
		CE_ASSERT(bgfx::isValid(shader->program), "Failed to create GPU program");
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		Shader* shader = (Shader*) rm.get(SHADER_TYPE, id);
		bgfx::destroyProgram(shader->program);
	}

	void unload(Allocator& a, void* res)
	{
		a.deallocate(res);
	}
} // namespace shader_resource
} // namespace crown
