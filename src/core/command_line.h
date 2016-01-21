/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "string_utils.h"

namespace crown
{

/// Helper for parsing command line.
class CommandLine
{
	int _argc;
	char** _argv;

public:

	CommandLine(int argc, char** argv)
		: _argc(argc)
		, _argv(argv)
	{
	}

	int find_argument(const char* longopt, char shortopt)
	{
		for (int i = 0; i < _argc; ++i)
		{
			if (is_longopt(_argv[i], longopt) || is_shortopt(_argv[i], shortopt))
			{
				return i;
			}
		}

		return _argc;
	}

	bool is_shortopt(const char* arg, char shortopt)
	{
		return shortopt != '\0'
			&& strlen32(arg) > 1
			&& arg[0] == '-'
			&& arg[1] == shortopt
			;
	}

	bool is_longopt(const char* arg, const char* longopt)
	{
		return strlen32(arg) > 2
			&& arg[0] == '-'
			&& arg[1] == '-'
			&& strcmp(&arg[2], longopt) == 0
			;
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
};

} // namespace crown
