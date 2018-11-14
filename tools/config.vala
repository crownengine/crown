/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

namespace Crown
{
	const string CROWN_VERSION = "0.1.1";

#if CROWN_PLATFORM_LINUX
	const string ENGINE_DIR = ".";
	const string EXE_PREFIX = "./";
	const string EXE_SUFFIX = "";
#elif CROWN_PLATFORM_WINDOWS
	const string ENGINE_DIR = ".";
	const string EXE_PREFIX = "";
	const string EXE_SUFFIX = ".exe";
#endif
	const string ENGINE_EXE = EXE_PREFIX + "crown-development" + EXE_SUFFIX;

	const uint16 CROWN_DEFAULT_SERVER_PORT = 10618;

	const string LEVEL_EDITOR_BOOT_DIR = "core/editors/level_editor";
	const string UNIT_PREVIEW_BOOT_DIR = "core/editors/unit_preview";
}
