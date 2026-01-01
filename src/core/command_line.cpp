/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/command_line.h"
#include "core/strings/string.inl"

namespace crown
{
static bool is_shortopt(const char *arg, char shortopt)
{
	return shortopt != '\0'
		&& strlen32(arg) > 1
		&& arg[0] == '-'
		&& arg[1] == shortopt
		;
}

static bool is_longopt(const char *arg, const char *longopt)
{
	return longopt != NULL
		&& strlen32(arg) > 2
		&& arg[0] == '-'
		&& arg[1] == '-'
		&& strcmp(&arg[2], longopt) == 0
		;
}

static bool is_end_options_marker(const char *arg)
{
	return strlen32(arg) == 2
		&& arg[0] == '-'
		&& arg[1] == '-'
		;
}

static int find_option(int argc, const char **argv, const char *longopt, char shortopt)
{
	for (int i = 0; i < argc; ++i) {
		if (is_end_options_marker(argv[i]))
			break;

		if (is_longopt(argv[i], longopt) || is_shortopt(argv[i], shortopt))
			return i;
	}

	return argc;
}

CommandLine::CommandLine(int argc, const char **argv)
	: _argc(argc)
	, _argv(argv)
{
}

const char *CommandLine::get_parameter(int i, const char *longopt, char shortopt)
{
	const int opt_in = find_option(_argc, _argv, longopt, shortopt);
	const int arg_in = opt_in + 1 + i;

	if (arg_in >= _argc)
		return NULL;

	for (int j = opt_in; j < arg_in; ++j) {
		if (is_end_options_marker(_argv[j + 1]))
			return NULL;
	}

	return _argv[arg_in];
}

bool CommandLine::has_option(const char *longopt, char shortopt)
{
	return find_option(_argc, _argv, longopt, shortopt) < _argc;
}

} // namespace crown
