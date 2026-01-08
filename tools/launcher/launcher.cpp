/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/filesystem/path.h"
#include "core/memory/allocator.h"
#include "core/memory/globals.h"
#include "core/os.h"
#include "core/process.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_view.inl"
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED
#define STB_SPRINTF_NOFLOAT
#include <stb_sprintf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if CROWN_PLATFORM_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#elif CROWN_PLATFORM_LINUX
#	include <unistd.h> // readlink
#	include <errno.h>
#endif

using namespace crown;

int main_internal(int argc, char **argv)
{
	CE_UNUSED(argc);
	char launcher_path[4*4096];
	StringView launcher_dir;
	StringView editor_dir;
	DynamicString editor_cwd(default_allocator());
	DynamicString editor_exe(default_allocator());

#if CROWN_PLATFORM_LINUX
	ssize_t len = readlink("/proc/self/exe", launcher_path, sizeof(launcher_path) - 1);
	if (len == -1) {
		printf("readlink: errno %d\n", errno);
		return EXIT_FAILURE;
	}
	launcher_path[len] = '\0';

	editor_dir = StringView("platforms/linux64/bin");
	os::setenv("UBUNTU_MENUPROXY", "");
#elif CROWN_PLATFORM_WINDOWS
	wchar_t buf[MAX_PATH];
	DWORD buf_size = (DWORD)sizeof(buf);
	DWORD ret = GetModuleFileNameW(NULL, buf, buf_size);
	if (ret == 0 || ret >= buf_size) {
		printf("GetModuleFileNameW: error\n");
		return EXIT_FAILURE;
	}

	if (WideCharToMultiByte(CP_UTF8, 0, buf, -1, launcher_path, (int)sizeof(launcher_path), NULL, NULL) == 0) {
		printf("WideCharToMultiByte: error\n");
		return EXIT_FAILURE;
	}

	editor_dir = StringView("platforms\\windows64\\bin");
#else
	#error "Unsupported platform."
#endif

	launcher_dir = path::parent_dir(launcher_path);
	path::join(editor_cwd, launcher_dir, editor_dir);
	path::join(editor_exe, editor_cwd.c_str(), "crown-editor-release" EXE_SUFFIX);
	os::setcwd(editor_cwd.c_str());

	Process pr;
	argv[0] = (char *)editor_exe.c_str();

	if (pr.spawn(argv, CROWN_PROCESS_STDOUT_PIPE | CROWN_PROCESS_STDERR_MERGE) != 0) {
		printf("Cannot spawn %s\n", argv[0]);
		return EXIT_FAILURE;
	}

	return pr.wait();
}

int main(int argc, char **argv)
{
	CE_UNUSED(argc);
	memory_globals::init();
	int exit_code = main_internal(argc, argv);
	memory_globals::shutdown();
	return exit_code;
}
