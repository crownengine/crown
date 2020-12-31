/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/filesystem/file.h"
#include "core/memory/temp_allocator.inl"
#include "core/process.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_stream.inl"
#include "resource/compile_options.inl"
#include "resource/lua_resource.h"

#if CROWN_DEBUG
	#define LUAJIT_FLAGS "-bg" // Keep debug info
#else
	#define LUAJIT_FLAGS "-b"
#endif // CROWN_DEBUG

namespace crown
{
namespace lua_resource
{
	const char* program(const LuaResource* lr)
	{
		return (char*)&lr[1];
	}

} // namespace lua_resource

#if CROWN_CAN_COMPILE
namespace lua_resource_internal
{
	s32 compile(CompileOptions& opts)
	{
		TempAllocator1024 ta;
		DynamicString lua_src(ta);
		DynamicString lua_out(ta);
		opts.absolute_path(lua_src, opts.source_path());
		opts.temporary_path(lua_out, "lua");

		const char* argv[] =
		{
			EXE_PATH("luajit"),
			LUAJIT_FLAGS,
			lua_src.c_str(),
			lua_out.c_str(),
			NULL
		};
		Process pr;
		s32 sc = pr.spawn(argv, ProcessFlags::STDOUT_PIPE | ProcessFlags::STDERR_MERGE);
		DATA_COMPILER_ASSERT(sc == 0
			, opts
			, "Failed to spawn `%s`"
			, argv[0]
			);
		StringStream output(ta);
		opts.read_output(output, pr);
		s32 ec = pr.wait();
		DATA_COMPILER_ASSERT(ec == 0
			, opts
			, "Failed to compile lua:\n%s"
			, string_stream::c_str(output)
			);

		Buffer blob = opts.read_temporary(lua_out.c_str());
		opts.delete_file(lua_out.c_str());

		LuaResource lr;
		lr.version = RESOURCE_HEADER(RESOURCE_VERSION_SCRIPT);
		lr.size = array::size(blob);

		opts.write(lr.version);
		opts.write(lr.size);
		opts.write(blob);

		return 0;
	}

} // namespace lua_resource_internal
#endif // CROWN_CAN_COMPILE

} // namespace crown
