/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/memory/allocator.h"
#include "core/memory/globals.h"
#include "core/os.h"
#include "core/process.h"
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED
#define STB_SPRINTF_NOFLOAT
#include <stb_sprintf.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	using namespace crown;
	memory_globals::init();
	CE_UNUSED(argc);

#if CROWN_PLATFORM_LINUX
	os::setcwd("platforms/linux64/bin");
	os::setenv("UBUNTU_MENUPROXY", "");
#elif CROWN_PLATFORM_WINDOWS
	os::setcwd("platforms/windows64/bin");
#else
	#error "Unsupported platform."
#endif

	char editor_exe[] = EXE_PATH("crown-editor-release");

	Process pr;
	argv[0] = editor_exe;

	if (pr.spawn(argv, CROWN_PROCESS_STDOUT_PIPE | CROWN_PROCESS_STDERR_MERGE) != 0) {
		printf("Cannot spawn %s\n", argv[0]);
		memory_globals::shutdown();
		return EXIT_FAILURE;
	}

	int exit_code = pr.wait();
	memory_globals::shutdown();
	return exit_code;
}
