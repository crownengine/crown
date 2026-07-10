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
#	include <fcntl.h> // _O_TEXT
#	include <io.h>    // _open_osfhandle
#	include <windows.h>
#elif CROWN_PLATFORM_LINUX
#	include <unistd.h> // readlink
#	include <errno.h>
#endif

using namespace crown;

int main_internal(int argc, char **argv)
{
	char launcher_path[4*4096];
	StringView launcher_dir;
	StringView program_dir;
	DynamicString program_cwd(default_allocator());
	DynamicString program_exe(default_allocator());
	const char *program_name = "crown-editor-release" EXE_SUFFIX;
	char **program_argv = argv;

	if (argc > 1 && strcmp(argv[1], "editor") == 0) {
		program_argv = argv + 1;
	} else if (argc > 1 && strcmp(argv[1], "runtime") == 0) {
		program_name = "crown-release" EXE_SUFFIX;
		program_argv = argv + 1;
	}

#if CROWN_PLATFORM_LINUX
	ssize_t len = readlink("/proc/self/exe", launcher_path, sizeof(launcher_path) - 1);
	if (len == -1) {
		printf("readlink: errno %d\n", errno);
		return EXIT_FAILURE;
	}
	launcher_path[len] = '\0';

	program_dir = StringView("platforms/linux64/bin");
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

	program_dir = StringView("platforms\\windows64\\bin");
#else
	#error "Unsupported platform."
#endif
	os::setenv("GTK_THEME", "none"); // Disable foreign theming (hopefully).

	launcher_dir = path::parent_dir(launcher_path);
	path::join(program_cwd, launcher_dir, program_dir);
	path::join(program_exe, program_cwd.c_str(), program_name);
	os::setcwd(program_cwd.c_str());

	Process pr;
	program_argv[0] = (char *)program_exe.c_str();

	if (pr.spawn(program_argv) != 0) {
		printf("Cannot spawn %s\n", program_argv[0]);
		return EXIT_FAILURE;
	}

	return pr.wait();
}

int main(int argc, char **argv)
{
#if CROWN_PLATFORM_WINDOWS
	if (AttachConsole(ATTACH_PARENT_PROCESS) != 0) {
		const DWORD handles[] = { STD_OUTPUT_HANDLE, STD_ERROR_HANDLE, STD_INPUT_HANDLE };
		const char *modes[] = { "w", "w", "r" };
		FILE *stdfds[] = { stdout, stderr, stdin };

		for (u32 i = 0; i < countof(handles); ++i) {
			HANDLE out = GetStdHandle(handles[i]);
			int fd = _open_osfhandle((intptr_t)out, _O_TEXT);
			if (fd != -1) {
				*stdfds[i] = *_fdopen(fd, modes[i]);
				setvbuf(stdfds[i], NULL, _IONBF, 0); // No buffering.
			}
		}
	}
#endif

	memory_globals::init();
	int exit_code = main_internal(argc, argv);
	memory_globals::shutdown();
	return exit_code;
}
