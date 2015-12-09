/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"
#include "bundle_compiler.h"
#include "vector.h"
#include "dynamic_string.h"
#include "allocator.h"
#include "os.h"
#include "log.h"
#include "path.h"
#include "disk_filesystem.h"
#include "compile_options.h"
#include "temp_allocator.h"
#include "sort_map.h"
#include "lua_resource.h"
#include "texture_resource.h"
#include "mesh_resource.h"
#include "sound_resource.h"
#include "sprite_resource.h"
#include "package_resource.h"
#include "unit_resource.h"
#include "physics_resource.h"
#include "material_resource.h"
#include "font_resource.h"
#include "level_resource.h"
#include "shader.h"

namespace crown
{

BundleCompiler::BundleCompiler(const char* source_dir, const char* bundle_dir)
	: _source_fs(source_dir)
	, _bundle_fs(bundle_dir)
	, _compilers(default_allocator())
{
	namespace pcr = physics_config_resource;
	namespace phr = physics_resource;
	namespace pkr = package_resource;
	namespace sdr = sound_resource;
	namespace mhr = mesh_resource;
	namespace utr = unit_resource;
	namespace txr = texture_resource;
	namespace mtr = material_resource;
	namespace lur = lua_resource;
	namespace ftr = font_resource;
	namespace lvr = level_resource;
	namespace spr = sprite_resource;
	namespace shr = shader_resource;
	namespace sar = sprite_animation_resource;

	register_resource_compiler(SCRIPT_TYPE,           lur::compile);
	register_resource_compiler(TEXTURE_TYPE,          txr::compile);
	register_resource_compiler(MESH_TYPE,             mhr::compile);
	register_resource_compiler(SOUND_TYPE,            sdr::compile);
	register_resource_compiler(UNIT_TYPE,             utr::compile);
	register_resource_compiler(SPRITE_TYPE,           spr::compile);
	register_resource_compiler(PACKAGE_TYPE,          pkr::compile);
	register_resource_compiler(PHYSICS_TYPE,          phr::compile);
	register_resource_compiler(MATERIAL_TYPE,         mtr::compile);
	register_resource_compiler(PHYSICS_CONFIG_TYPE,   pcr::compile);
	register_resource_compiler(FONT_TYPE,             ftr::compile);
	register_resource_compiler(LEVEL_TYPE,            lvr::compile);
	register_resource_compiler(SHADER_TYPE,           shr::compile);
	register_resource_compiler(SPRITE_ANIMATION_TYPE, sar::compile);

	DiskFilesystem temp;
	temp.create_directory(bundle_dir);
}

bool BundleCompiler::compile(const char* type, const char* name, const char* platform)
{
	StringId64 _type(type);
	StringId64 _name(name);

	TempAllocator512 alloc;
	DynamicString path(alloc);
	TempAllocator512 alloc2;
	DynamicString src_path(alloc2);

	src_path += name;
	src_path += ".";
	src_path += type;

	char res_name[1 + 2*StringId64::STRING_LENGTH];
	_type.to_string(res_name);
	res_name[16] = '-';
	_name.to_string(res_name + 17);

	path::join(CROWN_DATA_DIRECTORY, res_name, path);

	CE_LOGI("%s <= %s.%s", res_name, name, type);

	File* outf = _bundle_fs.open(path.c_str(), FOM_WRITE);

	CompileOptions opts(_source_fs, outf, platform);
	compile(_type, src_path.c_str(), opts);

	_bundle_fs.close(outf);
	return true;
}

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

	if (!_bundle_fs.exists(CROWN_DATA_DIRECTORY))
		_bundle_fs.create_directory(CROWN_DATA_DIRECTORY);

	// Compile all resources
	for (uint32_t i = 0; i < vector::size(files); i++)
	{
		if (files[i].ends_with(".tga")
			|| files[i].ends_with(".dds")
			|| files[i].ends_with(".sh")
			|| files[i].ends_with(".sc")
			|| files[i].starts_with(".")
			|| files[i].ends_with(".config")
			|| files[i].ends_with(".tmp")
			|| files[i].ends_with(".wav"))
		continue;

		const char* filename = files[i].c_str();
		const char* type = path::extension(filename);

		char name[256];
		const uint32_t size = type - filename - 1;
		strncpy(name, filename, size);
		name[size] = '\0';

		compile(type, name, platform);
	}

	return true;
}

void BundleCompiler::register_resource_compiler(StringId64 type, CompileFunction compiler)
{
	CE_ASSERT_NOT_NULL(compiler);
	sort_map::set(_compilers, type, compiler);
	sort_map::sort(_compilers);
}

void BundleCompiler::compile(StringId64 type, const char* path, CompileOptions& opts)
{
	CE_ASSERT(sort_map::has(_compilers, type), "Compiler not found");
	sort_map::get(_compilers, type, (CompileFunction)NULL)(path, opts);
}

void BundleCompiler::scan(const char* cur_dir, Vector<DynamicString>& files)
{
	Vector<DynamicString> my_files(default_allocator());

	_source_fs.list_files(cur_dir, my_files);

	for (uint32_t i = 0; i < vector::size(my_files); i++)
	{
		DynamicString file_i(default_allocator());

		if (strcmp(cur_dir, "") != 0)
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
	bool main(bool do_compile, bool do_continue, const char* platform)
	{
		if (do_compile)
		{
			bool ok = bundle_compiler_globals::compiler()->compile_all(platform);
			if (!ok || !do_continue)
			{
				return false;
			}
		}

		return true;
	}
} // namespace bundle_compiler

namespace bundle_compiler_globals
{
	char _buffer[sizeof(BundleCompiler)];
	BundleCompiler* _compiler = NULL;

	void init(const char* source_dir, const char* bundle_dir)
	{
#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS
		_compiler = new (_buffer) BundleCompiler(source_dir, bundle_dir);
#endif
	}

	void shutdown()
	{
		_compiler->~BundleCompiler();
		_compiler = NULL;
	}

	BundleCompiler* compiler()
	{
		return _compiler;
	}
} // namespace bundle_compiler_globals
} // namespace crown
