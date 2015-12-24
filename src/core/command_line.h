/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "string_utils.h"

namespace crown
{

/// Helper for parsing command line.
struct CommandLine
{
	CommandLine(int argc, char** argv)
		: _argc(argc)
		, _argv(argv)
	{
	}

	int find_argument(const char* longopt, char shortopt)
	{
		for (int i = 0; i < _argc; i++)
		{
			if ((shortopt != '\0' && strlen32(_argv[i]) > 1 && _argv[i][0] == '-' && _argv[i][1] == shortopt) ||
				(strlen32(_argv[i]) > 2 && _argv[i][0] == '-' && _argv[i][1] == '-' && strcmp(&_argv[i][2], longopt) == 0))
			{
				return i;
			}
		}

		return _argc;
	}

	const char* get_parameter(const char* longopt, char shortopt = '\0')
	{
		int argc = find_argument(longopt, shortopt);
		return argc < _argc ? _argv[argc + 1] : NULL;
	}

	bool has_argument(const char* longopt, char shortopt = '\0')
	{
		return find_argument(longopt, shortopt) < _argc;
	}

private:

	int _argc;
	char** _argv;
};

} // namespace crown
