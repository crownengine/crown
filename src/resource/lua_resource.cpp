/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"
#include "dynamic_string.h"
#include "lua_resource.h"
#include "os.h"
#include "temp_allocator.h"
#include "array.h"
#include "compile_options.h"

#if CROWN_DEBUG
	#define LUAJIT_FLAGS "-bg" // Keep debug info
#else
	#define LUAJIT_FLAGS "-b"
#endif // CROWN_DEBUG

namespace crown
{
namespace lua_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		TempAllocator1024 alloc;
		DynamicString res_abs_path(alloc);
		TempAllocator1024 alloc2;
		DynamicString bc_abs_path(alloc2);
		opts.get_absolute_path(path, res_abs_path);
		opts.get_absolute_path("bc.tmp", bc_abs_path);

		const char* luajit[] =
		{
#if CROWN_PLATFORM_LINUX
			"./luajit",
#else
			"luajit.exe",
#endif // CROWN_PLATFORM_LINUX
			LUAJIT_FLAGS,
			res_abs_path.c_str(),
			bc_abs_path.c_str(),
			NULL
		};

		int exitcode = os::execute_process(luajit);
		CE_ASSERT(exitcode == 0, "Failed to compile lua");

		Buffer blob = opts.read(bc_abs_path.c_str());
		opts.delete_file(bc_abs_path.c_str());

		LuaResource lr;
		lr.version = SCRIPT_VERSION;
		lr.size = array::size(blob);

		opts.write(lr.version);
		opts.write(lr.size);
		opts.write(blob);
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		CE_ASSERT(*(uint32_t*)res == SCRIPT_VERSION, "Wrong version");
		return res;
	}

	void online(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void offline(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	uint32_t size(const LuaResource* lr)
	{
		return lr->size;
	}

	const char* program(const LuaResource* lr)
	{
		return (char*)lr + sizeof(LuaResource);
	}
} // namespace lua_resource
} // namespace crown
