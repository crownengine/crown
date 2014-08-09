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

#include "bundle_compiler.h"
#include "vector.h"
#include "dynamic_string.h"
#include "allocator.h"
#include "os.h"
#include "log.h"
#include "resource.h"
#include "path.h"
#include "disk_filesystem.h"
#include "temp_allocator.h"
#include <inttypes.h>

namespace crown
{

namespace mesh_resource { extern void compile(Filesystem&, const char*, File*); }
namespace texture_resource { extern void compile(Filesystem&, const char*, File*); }
namespace package_resource { extern void compile(Filesystem&, const char*, File*); }
namespace lua_resource { extern void compile(Filesystem&, const char*, File*); }
namespace physics_resource { extern void compile(Filesystem&, const char*, File*); }
namespace physics_config_resource { extern void compile(Filesystem&, const char*, File*); }
namespace unit_resource { extern void compile(Filesystem&, const char*, File*); }
namespace sound_resource { extern void compile(Filesystem&, const char*, File*); }
namespace sprite_resource { extern void compile(Filesystem&, const char*, File*); }
namespace material_resource { extern void compile(Filesystem&, const char*, File*); }
namespace font_resource { extern void compile(Filesystem&, const char*, File*); }
namespace level_resource { extern void compile(Filesystem&, const char*, File*); }

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

		DiskFilesystem temp;
		temp.create_directory(bundle_dir);

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
			CE_LOGD("'crown.config' does not exist.");
			return false;
		}
	}
	else
	{
		DynamicString filename(default_allocator());
		filename = resource;
		vector::push_back(files, filename);
	}

	// Compile all resources
	for (uint32_t i = 0; i < vector::size(files); i++)
	{
		if (files[i].ends_with(".tga"))
			continue;
		if (files[i].ends_with(".dds"))
			continue;
		if (files[i].ends_with(".sh"))
			continue;
		if (files[i].ends_with(".sc"))
			continue;
		if (files[i].starts_with("."))
			continue;
		if (files[i].ends_with(".config"))
			continue;

		const char* filename = files[i].c_str();

		char filename_extension[512];
		char filename_without_extension[512];
		path::extension(filename, filename_extension, 512);
		path::filename_without_extension(filename, filename_without_extension, 512);

		const ResourceId name(filename_extension, filename_without_extension);

		char out_name[512];
		snprintf(out_name, 512, "%.16"PRIx64"-%.16"PRIx64, name.type, name.name);
		CE_LOGI("%s <= %s", out_name, filename);

		DiskFilesystem root_fs(source_dir);
		DiskFilesystem dest_fs(bundle_dir);

		// Open destination file
		File* out_file = dest_fs.open(out_name, FOM_WRITE);

		if (out_file)
		{
			if (name.type == MESH_TYPE)
			{
				mesh_resource::compile(root_fs, filename, out_file);
			}
			else if (name.type == TEXTURE_TYPE)
			{
				texture_resource::compile(root_fs, filename, out_file);
			}
			else if (name.type == LUA_TYPE)
			{
				lua_resource::compile(root_fs, filename, out_file);
			}
			else if(name.type == SOUND_TYPE)
			{
				sound_resource::compile(root_fs, filename, out_file);
			}
			else if(name.type == SPRITE_TYPE)
			{
				sprite_resource::compile(root_fs, filename, out_file);
			}
			else if (name.type == PACKAGE_TYPE)
			{
				package_resource::compile(root_fs, filename, out_file);
			}
			else if (name.type == UNIT_TYPE)
			{
				unit_resource::compile(root_fs, filename, out_file);
			}
			else if (name.type == PHYSICS_TYPE)
			{
				physics_resource::compile(root_fs, filename, out_file);
			}
			else if (name.type == MATERIAL_TYPE)
			{
				material_resource::compile(root_fs, filename, out_file);
			}
			else if (name.type == PHYSICS_CONFIG_TYPE)
			{
				physics_config_resource::compile(root_fs, filename, out_file);
			}
			else if (name.type == FONT_TYPE)
			{
				font_resource::compile(root_fs, filename, out_file);
			}
			else if (name.type == LEVEL_TYPE)
			{
				level_resource::compile(root_fs, filename, out_file);
			}
			else
			{
				CE_LOGE("Oops, unknown resource type!");
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

	for (uint32_t i = 0; i < vector::size(my_files); i++)
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
			vector::push_back(files, file_i);
		}
	}
}

} // namespace crown
