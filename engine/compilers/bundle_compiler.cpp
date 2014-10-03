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

#include "config.h"
#include "bundle_compiler.h"
#include "vector.h"
#include "dynamic_string.h"
#include "allocator.h"
#include "os.h"
#include "log.h"
#include "resource.h"
#include "path.h"
#include "disk_filesystem.h"
#include "compile_options.h"
#include "resource_registry.h"
#include <inttypes.h>

namespace crown
{

//-----------------------------------------------------------------------------
BundleCompiler::BundleCompiler(const char* source_dir, const char* bundle_dir)
	: _source_fs(source_dir)
	, _bundle_fs(bundle_dir)
{
	DiskFilesystem temp;
	temp.create_directory(bundle_dir);
}

//-----------------------------------------------------------------------------
bool BundleCompiler::compile(const char* type, const char* name, const char* platform)
{
	const ResourceId id(type, name);
	char out_name[512];
	snprintf(out_name, 512, "%.16"PRIx64"-%.16"PRIx64, id.type, id.name);
	char path[512];
	snprintf(path, 512, "%s.%s", name, type);

	CE_LOGI("%s <= %s.%s", out_name, name, type);

	File* outf = _bundle_fs.open(out_name, FOM_WRITE);
	CompileOptions opts(_source_fs, outf, platform);
	resource_on_compile(id.type, path, opts);
	_bundle_fs.close(outf);
}

//-----------------------------------------------------------------------------
bool BundleCompiler::compile_all(const char* platform)
{
	Vector<DynamicString> files(default_allocator());
	BundleCompiler::scan("", files);

	if (!_source_fs.is_file("crown.config"))
	{
		CE_LOGD("'crown.config' does not exist.");
		return false;
	}

	File* src = _source_fs.open("crown.config", FOM_READ);
	File* dst = _bundle_fs.open("crown.config", FOM_WRITE);
	src->copy_to(*dst, src->size());
	_source_fs.close(src);
	_bundle_fs.close(dst);

	// Compile all resources
	for (uint32_t i = 0; i < vector::size(files); i++)
	{
		if (files[i].ends_with(".tga")
			|| files[i].ends_with(".dds")
			|| files[i].ends_with(".sh")
			|| files[i].ends_with(".sc")
			|| files[i].starts_with(".")
			|| files[i].ends_with(".config"))
		continue;

		const char* filename = files[i].c_str();
		char type[256];
		char name[256];
		path::extension(filename, type, 256);
		path::filename_without_extension(filename, name, 256);

		compile(type, name, platform);
	}

	return true;
}

//-----------------------------------------------------------------------------
void BundleCompiler::scan(const char* cur_dir, Vector<DynamicString>& files)
{
	Vector<DynamicString> my_files(default_allocator());

	_source_fs.list_files(cur_dir, my_files);

	for (uint32_t i = 0; i < vector::size(my_files); i++)
	{
		DynamicString file_i(default_allocator());

		if (string::strcmp(cur_dir, "") != 0)
		{
			file_i += cur_dir;
			file_i += '/';
		}
		file_i += my_files[i];

		if (_source_fs.is_directory(file_i.c_str()))
		{
			BundleCompiler::scan(file_i.c_str(), files);
		}
		else // Assume a regular file
		{
			vector::push_back(files, file_i);
		}
	}
}

namespace bundle_compiler
{
	bool main(const CommandLineSettings& cls)
	{
		if (cls.do_compile)
		{
			bool ok = bundle_compiler_globals::compiler()->compile_all(cls.platform);
			if (!ok || !cls.do_continue)
			{
				return false;
			}
		}

		return true;
	}
} // namespace bundle_compiler

namespace bundle_compiler_globals
{
	BundleCompiler* _compiler = NULL;

	void init(const char* source_dir, const char* bundle_dir)
	{
#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS
		_compiler = CE_NEW(default_allocator(), BundleCompiler)(source_dir, bundle_dir);
#endif
	}

	void shutdown()
	{
		CE_DELETE(default_allocator(), _compiler);
	}

	BundleCompiler* compiler()
	{
		return _compiler;
	}
} // namespace bundle_compiler_globals
} // namespace crown
