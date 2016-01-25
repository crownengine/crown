/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "allocator.h"
#include "bundle_compiler.h"
#include "compile_options.h"
#include "config.h"
#include "config_resource.h"
#include "disk_filesystem.h"
#include "dynamic_string.h"
#include "font_resource.h"
#include "level_resource.h"
#include "log.h"
#include "lua_resource.h"
#include "material_resource.h"
#include "mesh_resource.h"
#include "os.h"
#include "package_resource.h"
#include "path.h"
#include "physics_resource.h"
#include "shader_resource.h"
#include "sort_map.h"
#include "sound_resource.h"
#include "sprite_resource.h"
#include "temp_allocator.h"
#include "texture_resource.h"
#include "unit_resource.h"
#include "vector.h"
#include <setjmp.h>

namespace crown
{
BundleCompiler::BundleCompiler(const char* source_dir, const char* bundle_dir)
	: _source_fs(source_dir)
	, _bundle_fs(bundle_dir)
	, _compilers(default_allocator())
	, _files(default_allocator())
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
	namespace cor = config_resource;

	register_resource_compiler(SCRIPT_TYPE,           SCRIPT_VERSION,           lur::compile);
	register_resource_compiler(TEXTURE_TYPE,          TEXTURE_VERSION,          txr::compile);
	register_resource_compiler(MESH_TYPE,             MESH_VERSION,             mhr::compile);
	register_resource_compiler(SOUND_TYPE,            SOUND_VERSION,            sdr::compile);
	register_resource_compiler(UNIT_TYPE,             UNIT_VERSION,             utr::compile);
	register_resource_compiler(SPRITE_TYPE,           SPRITE_VERSION,           spr::compile);
	register_resource_compiler(PACKAGE_TYPE,          PACKAGE_VERSION,          pkr::compile);
	register_resource_compiler(PHYSICS_TYPE,          PHYSICS_VERSION,          phr::compile);
	register_resource_compiler(MATERIAL_TYPE,         MATERIAL_VERSION,         mtr::compile);
	register_resource_compiler(PHYSICS_CONFIG_TYPE,   PHYSICS_CONFIG_VERSION,   pcr::compile);
	register_resource_compiler(FONT_TYPE,             FONT_VERSION,             ftr::compile);
	register_resource_compiler(LEVEL_TYPE,            LEVEL_VERSION,            lvr::compile);
	register_resource_compiler(SHADER_TYPE,           SHADER_VERSION,           shr::compile);
	register_resource_compiler(SPRITE_ANIMATION_TYPE, SPRITE_ANIMATION_VERSION, sar::compile);
	register_resource_compiler(CONFIG_TYPE,           CONFIG_VERSION,           cor::compile);

	DiskFilesystem temp;
	temp.create_directory(bundle_dir);

	scan_source_dir("");

	if (!_bundle_fs.exists(CROWN_DATA_DIRECTORY))
		_bundle_fs.create_directory(CROWN_DATA_DIRECTORY);
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

	bool success = true;
	jmp_buf buf;

	Buffer output(default_allocator());
	array::reserve(output, 4*1024*1024);

	if (!setjmp(buf))
	{
		CompileOptions opts(_source_fs, output, platform, &buf);
		compile(_type, src_path.c_str(), opts);
		File* outf = _bundle_fs.open(path.c_str(), FileOpenMode::WRITE);
		uint32_t size = array::size(output);
		uint32_t written = outf->write(array::begin(output), size);
		_bundle_fs.close(*outf);
		success = size == written;
	}
	else
	{
		success = false;
	}

	return success;
}

bool BundleCompiler::compile_all(const char* platform)
{
	for (uint32_t i = 0; i < vector::size(_files); ++i)
	{
		if (_files[i].ends_with(".tga")
			|| _files[i].ends_with(".dds")
			|| _files[i].ends_with(".sh")
			|| _files[i].ends_with(".sc")
			|| _files[i].starts_with(".")
			|| _files[i].ends_with(".tmp")
			|| _files[i].ends_with(".wav"))
		continue;

		const char* filename = _files[i].c_str();
		const char* type = path::extension(filename);

		char name[256];
		const uint32_t size = uint32_t(type - filename - 1);
		strncpy(name, filename, size);
		name[size] = '\0';

		if (!compile(type, name, platform))
			return false;
	}

	return true;
}

void BundleCompiler::register_resource_compiler(StringId64 type, uint32_t version, CompileFunction compiler)
{
	CE_ASSERT(!sort_map::has(_compilers, type), "Type already registered");
	CE_ASSERT_NOT_NULL(compiler);

	ResourceTypeData rtd;
	rtd.version = version;
	rtd.compiler = compiler;

	sort_map::set(_compilers, type, rtd);
	sort_map::sort(_compilers);
}

void BundleCompiler::compile(StringId64 type, const char* path, CompileOptions& opts)
{
	CE_ASSERT(sort_map::has(_compilers, type), "Compiler not found");

	sort_map::get(_compilers, type, ResourceTypeData()).compiler(path, opts);
}

uint32_t BundleCompiler::version(StringId64 type)
{
	CE_ASSERT(sort_map::has(_compilers, type), "Compiler not found");

	return sort_map::get(_compilers, type, ResourceTypeData()).version;
}

void BundleCompiler::scan_source_dir(const char* cur_dir)
{
	Vector<DynamicString> my_files(default_allocator());
	_source_fs.list_files(cur_dir, my_files);

	for (uint32_t i = 0; i < vector::size(my_files); ++i)
	{
		TempAllocator512 ta;
		DynamicString file_i(ta);

		if (strcmp(cur_dir, "") != 0)
		{
			file_i += cur_dir;
			file_i += '/';
		}
		file_i += my_files[i];

		if (_source_fs.is_directory(file_i.c_str()))
		{
			BundleCompiler::scan_source_dir(file_i.c_str());
		}
		else // Assume a regular file
		{
			vector::push_back(_files, file_i);
		}
	}
}

namespace bundle_compiler
{
	bool main(bool do_compile, bool do_continue, const char* platform)
	{
		bool can_proceed = true;

		if (do_compile)
		{
			bool success = bundle_compiler_globals::compiler()->compile_all(platform);
			can_proceed = !(!success || !do_continue);
		}

		return can_proceed;
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
