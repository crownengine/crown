/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/containers/hash_set.inl"
#include "core/filesystem/file.h"
#include "core/memory/temp_allocator.inl"
#include "core/process.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_stream.inl"
#include "core/strings/string_view.inl"
#include "resource/compile_options.inl"
#include "resource/lua_resource.h"

namespace crown
{
namespace lua_resource
{
	const char *program(const LuaResource *lr)
	{
		return (char *)&lr[1];
	}

} // namespace lua_resource

#if CROWN_CAN_COMPILE
namespace lua_resource_internal
{
	static const char *skip_blanks(const char *lua)
	{
		lua = skip_spaces(lua);

		if (*lua == '-') {
			++lua;
			if (*lua++ == '-' && *lua++ == '[' && *lua++ == '[') {
				// Multi-line comment.
				const char *mlc_end = strstr(lua, "--]]");
				if (mlc_end != NULL)
					lua = mlc_end + 4;
				else
					lua += strlen(lua);
			} else {
				// Single-line comment.
				while (*lua && *lua != '\n')
					++lua;
			}
		}

		return skip_spaces(lua);
	}

	void find_requirements(HashSet<StringView> &requirements, const char *lua)
	{
		while (*lua) {
			lua = skip_blanks(lua);

			if (*lua == 'r') { // Find require()s.
				const char *require = strstr(lua, "require");
				if (!require) {
					++lua;
					continue;
				}

				lua = skip_blanks(require + 7);

				if (*lua == '(')
					++lua;

				lua = skip_blanks(lua);

				if (*lua == '\'' || *lua == '"') {
					const char *param_begin = lua + 1;
					const char *param_end   = strchr(param_begin, *lua);
					if (param_end != NULL) {
						hash_set::insert(requirements
							, StringView(param_begin, param_end - param_begin)
							);
						lua = param_end + 1;
					}
				}
			} else if (*lua == '-' || isspace(*lua)) {
				lua = skip_blanks(lua);
			} else if (*lua) {
				++lua;
			}
		}
	}

	s32 compile(CompileOptions &opts)
	{
		TempAllocator1024 ta;
		DynamicString lua_src(ta);
		DynamicString lua_out(ta);
		opts.absolute_path(lua_src, opts.source_path());
		opts.temporary_path(lua_out, "lua");

	#if CROWN_PLATFORM_LINUX
		#define CROWN_32BIT_BIN_DIR "linux32"
	#elif CROWN_PLATFORM_WINDOWS
		#define CROWN_32BIT_BIN_DIR "windows32"
	#else
		#error "Unsupported platform"
	#endif

	#if CROWN_DEBUG
		#define LUAJIT_FLAGS "-bg" // Keep debug info
	#else
		#define LUAJIT_FLAGS "-b"
	#endif

		const char *argv[16];

		if (opts._platform == Platform::HTML5) {
			argv[0] = EXE_PATH("../../" CROWN_32BIT_BIN_DIR "/bin/luac");
			argv[1] = "-o";
			argv[2] = lua_out.c_str();
			argv[3] = lua_src.c_str();
			argv[4] = NULL;
		} else {
			const char *luajit = EXE_PATH("luajit");
			if (opts._platform == Platform::ANDROID)
				luajit = EXE_PATH("../../" CROWN_32BIT_BIN_DIR "/bin/luajit");

			argv[0] = luajit;
			argv[1] = LUAJIT_FLAGS;
			argv[2] = lua_src.c_str();
			argv[3] = lua_out.c_str();
			argv[4] = NULL;
		}

		Process pr;
		s32 sc;
		sc = pr.spawn(argv, CROWN_PROCESS_STDOUT_PIPE | CROWN_PROCESS_STDERR_MERGE);
		RETURN_IF_FALSE(sc == 0
			, opts
			, "Failed to spawn `%s`"
			, argv[0]
			);

		// Scan the .lua code for requirements.
		Buffer lua_code = opts.read();
		HashSet<StringView> requirements(default_allocator());
		lua_resource_internal::find_requirements(requirements, array::begin(lua_code));

		auto cur = hash_set::begin(requirements);
		auto end = hash_set::end(requirements);
		for (; cur != end; ++cur) {
			HASH_SET_SKIP_HOLE(requirements, cur);

			TempAllocator256 ta;
			DynamicString name(ta);
			name = *cur;
			opts.add_requirement("lua", name.c_str());
		}

		StringStream output(ta);
		opts.read_output(output, pr);
		s32 ec = pr.wait();
		RETURN_IF_FALSE(ec == 0
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
#endif // if CROWN_CAN_COMPILE

} // namespace crown
