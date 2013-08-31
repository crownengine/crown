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

#include "BundleCompiler.h"
#include "Vector.h"
#include "DynamicString.h"
#include "Allocator.h"
#include "OS.h"
#include "Log.h"
#include "Resource.h"
#include "Path.h"

namespace crown
{

//-----------------------------------------------------------------------------
BundleCompiler::BundleCompiler()
{
}

//-----------------------------------------------------------------------------
bool BundleCompiler::compile(const char* bundle_dir, const char* source_dir)
{
	// Get list of all files from source dir
	Vector<DynamicString> files(default_allocator());
	os::list_files(source_dir, files);

	// Compile all resources
	for (uint32_t i = 0; i < files.size(); i++)
	{
		DynamicString& filename = files[i];

		char resource_name[1024];
		char resource_type[1024];
		path::filename_without_extension(filename.c_str(), resource_name, 1024);
		path::extension(filename.c_str(), resource_type, 1024);

		uint32_t resource_name_hash = hash::murmur2_32(resource_name, string::strlen(resource_name), 0);
		uint32_t resource_type_hash = hash::murmur2_32(resource_type, string::strlen(resource_type), 0);

		char out_name[1024];
		snprintf(out_name, 1024, "%.8X%.8X", resource_name_hash, resource_type_hash);
		Log::i("%s <= %s\n", out_name, filename.c_str());

		bool result = false;
		if (resource_type_hash == TEXTURE_TYPE)
		{
			result = m_tga.compile(source_dir, bundle_dir, filename.c_str(), out_name);
		}
		else if (resource_type_hash == LUA_TYPE)
		{
			result = m_lua.compile(source_dir, bundle_dir, filename.c_str(), out_name);
		}
		else
		{
			continue;
		}

		if (!result)
		{
			return false;
		}
	}

	return true;
}

} // namespace crown
