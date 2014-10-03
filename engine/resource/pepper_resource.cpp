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

#include "pepper_resource.h"
#include "json_parser.h"

namespace crown
{
namespace pepper_resource
{
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
	}

	void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{

	}

	void online(StringId64 id, ResourceManager& rm)
	{

	}

	void offline(StringId64 id, ResourceManager& rm)
	{

	}

	void unload(Allocator& a, void* resource)
	{

	}
} // namespace pepper_resource
} // namespace crown
