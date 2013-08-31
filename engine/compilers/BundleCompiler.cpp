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
#include "DiskFilesystem.h"
#include "TempAllocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
BundleCompiler::BundleCompiler()
{
}

//-----------------------------------------------------------------------------
bool BundleCompiler::compile(const char* bundle_dir, const char* source_dir)
{
	Vector<DynamicString> files(default_allocator());
	BundleCompiler::scan(source_dir, "", files);

	// Create bundle dir if does not exist
	DiskFilesystem temp;
	if (!temp.is_directory(bundle_dir) || !temp.is_file(bundle_dir))
	{
		temp.create_directory(bundle_dir);
	}

	// Compile all resources
	for (uint32_t i = 0; i < files.size(); i++)
	{
		const char* filename = files[i].c_str();
		uint64_t filename_hash = hash::murmur2_64(filename, string::strlen(filename), 0);

		char filename_extension[32];
		path::extension(filename, filename_extension, 32);
		uint32_t resource_type_hash = hash::murmur2_32(filename_extension, string::strlen(filename_extension), 0);

		char out_name[65];
		snprintf(out_name, 65, "%.16llx", filename_hash);

		Log::i("%s <= %s", out_name, filename);

		bool result = false;
		if (resource_type_hash == TEXTURE_TYPE)
		{
			result = m_tga.compile(source_dir, bundle_dir, filename, out_name);
		}
		else if (resource_type_hash == LUA_TYPE)
		{
			result = m_lua.compile(source_dir, bundle_dir, filename, out_name);
		}
		else
		{
			Log::e("Oops, unknown resource type!");
			return false;
		}

		if (!result)
		{
			return false;
		}
	}

	return true;
}

void BundleCompiler::scan(const char* source_dir, const char* cur_dir, Vector<DynamicString>& files)
{
	Vector<DynamicString> my_files(default_allocator());

	DiskFilesystem fs(source_dir);
	fs.list_files(cur_dir, my_files);

	for (uint32_t i = 0; i < my_files.size(); i++)
	{
		DynamicString file_i(default_allocator());

		if (string::strcmp(cur_dir, "") != 0)
		{
			file_i += cur_dir;
			file_i += '/';
		}
		file_i += my_files[i];

		if (fs.is_directory(file_i.c_str()))
		{
			BundleCompiler::scan(source_dir, file_i.c_str(), files);
		}
		else // Assume a regular file
		{
			files.push_back(file_i);
		}
	}
}

} // namespace crown
