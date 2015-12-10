/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
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
#include "temp_allocator.h"
#include "string_stream.h"

#if CROWN_DEBUG
#	define SHADERC_NAME "./shaderc-debug-"
#else
#	define SHADERC_NAME "shaderc-development-"
#endif // CROWN_DEBUG
#if CROWN_ARCH_32BIT
#	define SHADERC_BITS "32"
#elif CROWN_ARCH_64BIT
#	define SHADERC_BITS "64"
#endif // CROWN_ARCH_32BIT
#if CROWN_PLATFORM_LINUX
#	define SHADERC_PATH SHADERC_NAME""SHADERC_BITS
#elif CROWN_PLATFORM_WINDOWS
#	define SHADERC_PATH SHADERC_NAME""SHADERC_BITS".exe"
#else
# 	define SHADERC_PATH ""
#endif // CROWN_PLATFORM_LINUX

namespace crown
{
namespace shader_resource
{
	int run_external_compiler(const char* infile, const char* outfile, const char* varying, const char* type, const char* platform, StringStream& output)
	{
		using namespace string_stream;

		TempAllocator512 ta;
		StringStream args(ta);
		args << " -f " << infile;
		args << " -o " << outfile;
		args << " --varyingdef " << varying;
		args << " --type " << type;
		args << " --platform " << platform;
		args << " --profile ";
#if CROWN_PLATFORM_LINUX
		args <<	"120";
#elif CROWN_PLATFORM_WINDOWS
		args << (strcmp(type, "vertex") == 0) ? "vs_3_0" : "ps_3_0";
#endif

		return os::execute_process(SHADERC_PATH, c_str(args), output);
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);
		JSONParser json(buf);
		JSONElement root = json.root();

		DynamicString vs_code2 = root.key("vs_code").to_string();
		DynamicString fs_code2 = root.key("fs_code").to_string();
		DynamicString varying_def = root.key("varying_def").to_string();
		DynamicString common_code = root.key("common").to_string();
		DynamicString vs_in_out = root.key("vs_in_out").to_string();
		DynamicString fs_in_out = root.key("fs_in_out").to_string();

		DynamicString vs_code(default_allocator());
		DynamicString fs_code(default_allocator());
		vs_code += vs_in_out;
		vs_code += common_code;
		vs_code += vs_code2;
		fs_code += fs_in_out;
		fs_code += common_code;
		fs_code += fs_code2;

		DynamicString vs_code_path(default_allocator());
		DynamicString fs_code_path(default_allocator());
		DynamicString varying_def_path(default_allocator());
		DynamicString tmpvs_path(default_allocator());
		DynamicString tmpfs_path(default_allocator());

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

		TempAllocator4096 ta;
		StringStream output(ta);
		using namespace string_stream;

		int exitcode = run_external_compiler(vs_code_path.c_str()
			, tmpvs_path.c_str()
			, varying_def_path.c_str()
			, "vertex"
			, opts.platform()
			, output
			);
		RESOURCE_COMPILER_ASSERT(exitcode == 0
			, opts
			, "Failed to compile vertex shader:\n%s"
			, c_str(output)
			);

		array::clear(output);
		exitcode = run_external_compiler(fs_code_path.c_str()
			, tmpfs_path.c_str()
			, varying_def_path.c_str()
			, "fragment"
			, opts.platform()
			, output
			);
		if (exitcode)
		{
			opts.delete_file(tmpvs_path.c_str());
			RESOURCE_COMPILER_ASSERT(false
				, opts
				, "Failed to compile fragment shader:\n%s"
				, c_str(output)
				);
		}

		Buffer tmpvs = opts.read(tmpvs_path.c_str());
		Buffer tmpfs = opts.read(tmpfs_path.c_str());

		opts.write(uint32_t(SHADER_VERSION));
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
		CE_ASSERT(version == SHADER_VERSION, "Wrong version");

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
