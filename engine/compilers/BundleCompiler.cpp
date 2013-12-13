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

#include <inttypes.h>
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

namespace mesh_resource { extern void compile(Filesystem&, const char*, File*); }
namespace texture_resource { extern void compile(Filesystem&, const char*, File*); }
namespace package_resource { extern void compile(Filesystem&, const char*, File*); }
namespace lua_resource { extern void compile(Filesystem&, const char*, File*); }
namespace physics_resource { extern void compile(Filesystem&, const char*, File*); }
namespace unit_resource { extern void compile(Filesystem&, const char*, File*); }
namespace sound_resource { extern void compile(Filesystem&, const char*, File*); }
namespace sprite_resource { extern void compile(Filesystem&, const char*, File*); }

//-----------------------------------------------------------------------------
BundleCompiler::BundleCompiler()
{
}

//-----------------------------------------------------------------------------
bool BundleCompiler::compile(const char* bundle_dir, const char* source_dir, const char* resource)
{
	Vector<DynamicString> files(default_allocator());

	if (resource == NULL)
	{
		BundleCompiler::scan(source_dir, "", files);

		// Create bundle dir if does not exist
		DiskFilesystem temp;
		if (!temp.is_directory(bundle_dir) || !temp.is_file(bundle_dir))
		{
			temp.create_directory(bundle_dir);
		}

		// Copy crown.config to bundle dir
		DiskFilesystem src_fs(source_dir);
		DiskFilesystem dst_fs(bundle_dir);

		if (src_fs.is_file("crown.config"))
		{
			File* src = src_fs.open("crown.config", FOM_READ);
			File* dst = dst_fs.open("crown.config", FOM_WRITE);
			src->copy_to(*dst, src->size());
			src_fs.close(src);
			dst_fs.close(dst);
		}
		else
		{
			Log::d("'crown.config' does not exist.");
			return false;
		}
	}
	else
	{
		DynamicString filename(default_allocator());
		filename = resource;
		files.push_back(filename);
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
		snprintf(out_name, 65, "%.16"PRIx64"", filename_hash);

		// Skip crown.config file
		if (resource_type_hash == CONFIG_TYPE)
		{
			continue;
		}

		Log::i("%s <= %s", out_name, filename);

		DiskFilesystem root_fs(source_dir);
		DiskFilesystem dest_fs(bundle_dir);

		// Open destination file
		File* out_file = dest_fs.open(out_name, FOM_WRITE);

		if (out_file)
		{
			if (resource_type_hash == MESH_TYPE)
			{
				mesh_resource::compile(root_fs, filename, out_file);
			}
			else if (resource_type_hash == TEXTURE_TYPE)
			{
				texture_resource::compile(root_fs, filename, out_file);
			}
			else if (resource_type_hash == LUA_TYPE)
			{
				lua_resource::compile(root_fs, filename, out_file);
			}
			else if(resource_type_hash == SOUND_TYPE)
			{
				sound_resource::compile(root_fs, filename, out_file);
			}
			else if(resource_type_hash == SPRITE_TYPE)
			{
				sprite_resource::compile(root_fs, filename, out_file);
			}
			else if (resource_type_hash == PACKAGE_TYPE)
			{
				package_resource::compile(root_fs, filename, out_file);
			}
			else if (resource_type_hash == UNIT_TYPE)
			{
				unit_resource::compile(root_fs, filename, out_file);
			}
			else if (resource_type_hash == PHYSICS_TYPE)
			{
				physics_resource::compile(root_fs, filename, out_file);
			}
			else
			{
				Log::e("Oops, unknown resource type!");
				return false;
			}

			dest_fs.close(out_file);
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
