/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

namespace Crown
{
	const string CROWN_VERSION = "0.0.22";

#if CROWN_PLATFORM_LINUX
	const string ENGINE_DIR = "../engine/linux64/bin";
	const string EXE_PREFIX = "./";
	const string EXE_SUFFIX = "";
#elif CROWN_PLATFORM_WINDOWS
	const string ENGINE_DIR = "../engine/win64/bin";
	const string EXE_PREFIX = "";
	const string EXE_SUFFIX = ".exe";
#endif
	const string ENGINE_EXE = EXE_PREFIX + "crown-development-64" + EXE_SUFFIX;

	const uint16 CROWN_DEFAULT_SERVER_PORT = 10618;
}
