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

#include "MaterialResource.h"
#include "DynamicString.h"
#include "StringUtils.h"
#include "StringUtils.h"
#include "JSONParser.h"
#include "Filesystem.h"
#include "ReaderWriter.h"

namespace crown
{
namespace material_resource
{

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	JSONParser json(*file);
	fs.close(file);

	JSONElement root = json.root();

	DynamicString vs_src;
	DynamicString fs_src;
	DynamicString varying_src;

	root.key("vs_source").to_string(vs_src);
	root.key("fs_source").to_string(fs_src);
	root.key("varying_source").to_string(varying_src);

	vs_src += ".sc";
	fs_src += ".sc";
	varying_src += ".def.sc";

	DynamicString vs_src_abs;
	DynamicString fs_src_abs;
	DynamicString varying_src_abs;
	DynamicString vs_bin_abs;
	DynamicString fs_bin_abs;

	fs.get_absolute_path(vs_src.c_str(), vs_src_abs);
	fs.get_absolute_path(fs_src.c_str(), fs_src_abs);
	fs.get_absolute_path(varying_src.c_str(), varying_src_abs);
	fs.get_absolute_path("vs_bin.tmp", vs_bin_abs);
	fs.get_absolute_path("fs_bin.tmp", fs_bin_abs);

	const char* shaderc_vs[] =
	{
		"./shaderc",
		"-f", vs_src_abs.c_str(),
		"-o", vs_bin_abs.c_str(),
		"--platform", "linux",
		"--varyingdef", varying_src_abs.c_str(),
		"--type", "vertex",
		NULL
	};

	const char* shaderc_fs[] =
	{
		"shaderc",
		"-f", fs_src_abs.c_str(),
		"-o", fs_bin_abs.c_str(),
		"--platform", "linux",
		"--varyingdef", varying_src_abs.c_str(),
		"--type", "fragment",
		NULL
	};

	os::execute_process(shaderc_vs);
	os::execute_process(shaderc_fs);

	File* vs_code = fs.open(vs_bin_abs.c_str(), FOM_READ);
	File* fs_code = fs.open(fs_bin_abs.c_str(), FOM_READ);

	const uint32_t vs_code_size = vs_code->size();
	const uint32_t fs_code_size = fs_code->size();

	// Write material
	BinaryWriter bw(*out_file);

	bw.write(vs_code_size);
	bw.write(uint32_t(16 + 0));
	bw.write(fs_code_size);
	bw.write(uint32_t(16 + vs_code_size));

	char* shaders = (char*) default_allocator().allocate(vs_code_size + fs_code_size);
	vs_code->read(shaders + 0, vs_code_size);
	fs_code->read(shaders + vs_code_size, fs_code_size);

	bw.write(shaders, vs_code_size + fs_code_size);

	default_allocator().deallocate(shaders);

	fs.close(vs_code);
	fs.close(fs_code);
	fs.delete_file(vs_bin_abs.c_str());
	fs.delete_file(fs_bin_abs.c_str());
}

} // namespace material_resource
} // namespace crown
