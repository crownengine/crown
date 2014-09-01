/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "shader.h"
#include "filesystem.h"
#include "json_parser.h"
#include "os.h"
#include "reader_writer.h"
#include "config.h"

namespace crown
{
namespace shader_resource
{
	//-----------------------------------------------------------------------------
	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);
		JSONParser json(array::begin(buf));
		JSONElement root = json.root();

		DynamicString vs_code;
		DynamicString fs_code;
		DynamicString varying_def;

		root.key("vs_code").to_string(vs_code);
		root.key("fs_code").to_string(fs_code);
		root.key("varying_def").to_string(varying_def);

		DynamicString vs_code_path;
		DynamicString fs_code_path;
		DynamicString varying_def_path;
		DynamicString tmpvs_path;
		DynamicString tmpfs_path;

		opts.get_absolute_path(vs_code.c_str(), vs_code_path);
		opts.get_absolute_path(fs_code.c_str(), fs_code_path);
		opts.get_absolute_path(varying_def.c_str(), varying_def_path);
		opts.get_absolute_path("tmpvs", tmpvs_path);
		opts.get_absolute_path("tmpfs", tmpfs_path);

		const char* compile_vs[] =
		{
			"shaderc",
			"-f", vs_code_path.c_str(),
			"-o", tmpvs_path.c_str(),
			"--varyingdef", varying_def_path.c_str(),
			"--type", "vertex",
			"--platform", opts.platform(),
#if CROWN_PLATFORM_WINDOWS
			"--profile", "vs_3_0",
#endif
			NULL
		};
		os::execute_process(compile_vs);

		const char* compile_fs[] =
		{
			"shaderc",
			"-f", fs_code_path.c_str(),
			"-o", tmpfs_path.c_str(),
			"--varyingdef", varying_def_path.c_str(),
			"--type", "fragment",
			"--platform", opts.platform(),
#if CROWN_PLATFORM_WINDOWS
			"--profile", "ps_3_0",
#endif
			NULL
		};
		os::execute_process(compile_fs);

		Buffer tmpvs = opts.read(tmpvs_path.c_str());
		Buffer tmpfs = opts.read(tmpfs_path.c_str());

		opts.write(uint32_t(1)); // version
		opts.write(uint32_t(array::size(tmpvs)));
		opts.write(array::begin(tmpvs), array::size(tmpvs));
		opts.write(uint32_t(array::size(tmpfs)));
		opts.write(array::begin(tmpfs), array::size(tmpfs));

		opts.delete_file(tmpvs_path.c_str());
		opts.delete_file(tmpfs_path.c_str());
	}

	void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);

		BinaryReader br(*file);
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

		bundle.close(file);

		Shader* shader = (Shader*) default_allocator().allocate(sizeof(Shader));
		shader->vs = vsmem;
		shader->fs = fsmem;
		shader->program.idx = bgfx::invalidHandle;

		return shader;
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		ResourceId res_id;
		res_id.type = SHADER_TYPE;
		res_id.name = id;

		Shader* shader = (Shader*) rm.get(res_id);
		bgfx::ShaderHandle vs = bgfx::createShader(shader->vs);
		bgfx::ShaderHandle fs = bgfx::createShader(shader->fs);
		shader->program = bgfx::createProgram(vs, fs, true);
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		ResourceId res_id;
		res_id.type = SHADER_TYPE;
		res_id.name = id;

		Shader* shader = (Shader*) rm.get(res_id);
		bgfx::destroyProgram(shader->program);
	}

	void unload(Allocator& a, void* res)
	{
		a.deallocate(res);
	}
} // namespace shader_resource
} // namespace crown
