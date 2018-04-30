/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/array.h"
#include "core/filesystem/file.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_stream.h"
#include "resource/compile_options.h"
#include "resource/lua_resource.h"

#define LUAJIT_NAME "./luajit"

#if CROWN_PLATFORM_WINDOWS
	#define EXE ".exe"
#else
 	#define EXE ""
#endif // CROWN_PLATFORM_WINDOWS

#define LUAJIT_EXE LUAJIT_NAME EXE

#if CROWN_DEBUG
	#define LUAJIT_FLAGS "-bg" // Keep debug info
#else
	#define LUAJIT_FLAGS "-b"
#endif // CROWN_DEBUG

namespace crown
{
namespace lua_resource_internal
{
	void compile(CompileOptions& opts)
	{
		TempAllocator1024 ta;
		DynamicString luasrc(ta);
		DynamicString luabin(ta);
		opts.get_absolute_path(opts.source_path(), luasrc);
		opts.get_temporary_path("lua", luabin);

		StringStream output(ta);
		const char* argv[] =
		{
			LUAJIT_EXE,
			LUAJIT_FLAGS,
			luasrc.c_str(),
			luabin.c_str(),
			NULL
		};
		int ec = opts.run_external_compiler(argv, output);
		DATA_COMPILER_ASSERT(ec == 0
			, opts
			, "Failed to compile lua:\n%s"
			, string_stream::c_str(output)
			);

		Buffer blob = opts.read_temporary(luabin.c_str());
		opts.delete_file(luabin.c_str());

		LuaResource lr;
		lr.version = RESOURCE_VERSION_SCRIPT;
		lr.size = array::size(blob);

		opts.write(lr.version);
		opts.write(lr.size);
		opts.write(blob);
	}

} // namespace lua_resource_internal

namespace lua_resource
{
	const char* program(const LuaResource* lr)
	{
		return (char*)&lr[1];
	}

} // namespace lua_resource

} // namespace crown
