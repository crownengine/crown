/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
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
			if ((shortopt != '\0' && strlen(_argv[i]) > 1 && _argv[i][0] == '-' && _argv[i][1] == shortopt) ||
				(strlen(_argv[i]) > 2 && _argv[i][0] == '-' && _argv[i][1] == '-' && strcmp(&_argv[i][2], longopt) == 0))
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
