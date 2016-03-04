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
#include "file.h"
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
class LineReader
{
	const char* _str;
	const u32 _len;
	u32 _pos;

public:

	LineReader(const char* str)
		: _str(str)
		, _len(strlen32(str))
		, _pos(0)
	{
	}

	void read_line(DynamicString& line)
	{
		const char* s  = &_str[_pos];
		const char* nl = strnl(s);
		_pos += u32(nl - s);
		line.set(s, nl - s);
	}

	bool eof()
	{
		return _str[_pos] == '\0';
	}
};

BundleCompiler::BundleCompiler(const char* source_dir, const char* bundle_dir)
	: _source_fs(default_allocator(), source_dir)
	, _bundle_fs(default_allocator(), bundle_dir)
	, _compilers(default_allocator())
	, _files(default_allocator())
	, _globs(default_allocator())
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

	register_resource_compiler(RESOURCE_TYPE_SCRIPT,           RESOURCE_VERSION_SCRIPT,           lur::compile);
	register_resource_compiler(RESOURCE_TYPE_TEXTURE,          RESOURCE_VERSION_TEXTURE,          txr::compile);
	register_resource_compiler(RESOURCE_TYPE_MESH,             RESOURCE_VERSION_MESH,             mhr::compile);
	register_resource_compiler(RESOURCE_TYPE_SOUND,            RESOURCE_VERSION_SOUND,            sdr::compile);
	register_resource_compiler(RESOURCE_TYPE_UNIT,             RESOURCE_VERSION_UNIT,             utr::compile);
	register_resource_compiler(RESOURCE_TYPE_SPRITE,           RESOURCE_VERSION_SPRITE,           spr::compile);
	register_resource_compiler(RESOURCE_TYPE_PACKAGE,          RESOURCE_VERSION_PACKAGE,          pkr::compile);
	register_resource_compiler(RESOURCE_TYPE_PHYSICS,          RESOURCE_VERSION_PHYSICS,          phr::compile);
	register_resource_compiler(RESOURCE_TYPE_MATERIAL,         RESOURCE_VERSION_MATERIAL,         mtr::compile);
	register_resource_compiler(RESOURCE_TYPE_PHYSICS_CONFIG,   RESOURCE_VERSION_PHYSICS_CONFIG,   pcr::compile);
	register_resource_compiler(RESOURCE_TYPE_FONT,             RESOURCE_VERSION_FONT,             ftr::compile);
	register_resource_compiler(RESOURCE_TYPE_LEVEL,            RESOURCE_VERSION_LEVEL,            lvr::compile);
	register_resource_compiler(RESOURCE_TYPE_SHADER,           RESOURCE_VERSION_SHADER,           shr::compile);
	register_resource_compiler(RESOURCE_TYPE_SPRITE_ANIMATION, RESOURCE_VERSION_SPRITE_ANIMATION, sar::compile);
	register_resource_compiler(RESOURCE_TYPE_CONFIG,           RESOURCE_VERSION_CONFIG,           cor::compile);

	_bundle_fs.create_directory(bundle_dir);

	if (!_bundle_fs.exists(CROWN_DATA_DIRECTORY))
		_bundle_fs.create_directory(CROWN_DATA_DIRECTORY);

	scan_source_dir("");

	TempAllocator512 ta;
	vector::push_back(_globs, DynamicString("*.tmp", ta));
	vector::push_back(_globs, DynamicString("*.wav", ta));
	vector::push_back(_globs, DynamicString("*.ogg", ta));
	vector::push_back(_globs, DynamicString("*.png", ta));
	vector::push_back(_globs, DynamicString("*.tga", ta));
	vector::push_back(_globs, DynamicString("*.dds", ta));
	vector::push_back(_globs, DynamicString("*.ktx", ta));
	vector::push_back(_globs, DynamicString("*.pvr", ta));
	vector::push_back(_globs, DynamicString("*.swn", ta)); // VIM swap file.
	vector::push_back(_globs, DynamicString("*.swo", ta)); // VIM swap file.
	vector::push_back(_globs, DynamicString("*.swp", ta)); // VIM swap file.
	vector::push_back(_globs, DynamicString("*~", ta));
	vector::push_back(_globs, DynamicString(".*", ta));

	if (_source_fs.exists(CROWN_BUNDLEIGNORE))
	{
		File& file = *_source_fs.open(CROWN_BUNDLEIGNORE, FileOpenMode::READ);
		const u32 size = file.size();
		char* data = (char*)default_allocator().allocate(size + 1);
		file.read(data, size);
		data[size] = '\0';
		_source_fs.close(file);

		LineReader lr(data);

		while (!lr.eof())
		{
			TempAllocator512 ta;
			DynamicString line(ta);
			lr.read_line(line);

			line.trim();

			if (line.empty() || line.starts_with("#"))
				continue;

			vector::push_back(_globs, line);
		}

		default_allocator().deallocate(data);
	}
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
		u32 size = array::size(output);
		u32 written = outf->write(array::begin(output), size);
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
	for (u32 i = 0; i < vector::size(_files); ++i)
	{
		const char* filename = _files[i].c_str();

		bool ignore = false;
		for (u32 gg = 0; gg < vector::size(_globs); ++gg)
		{
			const char* glob = _globs[gg].c_str();

			if (wildcmp(glob, filename))
			{
				ignore = true;
				break;
			}
		}

		const char* type = path::extension(filename);

		if (ignore || type == NULL)
			continue;

		char name[256];
		const u32 size = u32(type - filename - 1);
		strncpy(name, filename, size);
		name[size] = '\0';

		if (!compile(type, name, platform))
			return false;
	}

	return true;
}

void BundleCompiler::register_resource_compiler(StringId64 type, u32 version, CompileFunction compiler)
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

u32 BundleCompiler::version(StringId64 type)
{
	CE_ASSERT(sort_map::has(_compilers, type), "Compiler not found");

	return sort_map::get(_compilers, type, ResourceTypeData()).version;
}

void BundleCompiler::scan_source_dir(const char* cur_dir)
{
	Vector<DynamicString> my_files(default_allocator());
	_source_fs.list_files(cur_dir, my_files);

	for (u32 i = 0; i < vector::size(my_files); ++i)
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

} // namespace crown
