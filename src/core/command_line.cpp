/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/command_line.h"
#include "core/strings/string.inl"

namespace crown
{
static bool is_shortopt(const char* arg, char shortopt)
{
	return shortopt != '\0'
		&& strlen32(arg) > 1
		&& arg[0] == '-'
		&& arg[1] == shortopt
		;
}

static bool is_longopt(const char* arg, const char* longopt)
{
	return longopt != NULL
		&& strlen32(arg) > 2
		&& arg[0] == '-'
		&& arg[1] == '-'
		&& strcmp(&arg[2], longopt) == 0
		;
}

static int find_option(int argc, const char** argv, const char* longopt, char shortopt)
{
	for (int i = 0; i < argc; ++i)
	{
		if (is_longopt(argv[i], longopt) || is_shortopt(argv[i], shortopt))
			return i;
	}

	return argc;
}

CommandLine::CommandLine(int argc, const char** argv)
	: _argc(argc)
	, _argv(argv)
{
}

const char* CommandLine::get_parameter(int i, const char* longopt, char shortopt)
{
	int argc = find_option(_argc, _argv, longopt, shortopt);
	return argc + i < _argc ? _argv[argc + i + 1] : NULL;
}

bool CommandLine::has_option(const char* longopt, char shortopt)
{
	return find_option(_argc, _argv, longopt, shortopt) < _argc;
}

} // namespace crown
