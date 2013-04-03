/*
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

#include <cassert>
#include "Types.h"
#include "String.h"

namespace crown
{

enum ArgsOptionType
{
	AOT_SHORT				= 1,
	AOT_LONG				= 2,
	AOT_NOT_OPTION			= 3
};

enum ArgsOptionArgument
{
	AOA_NO_ARGUMENT			= 1,
	AOA_REQUIRED_ARGUMENT	= 2
};

struct ArgsOption
{
	const char*				name;
	int32_t					has_arg;
	int32_t*				flag;
	int32_t					val;
};

/// Parses the command line arguments in a way very similar to GNU getopt.
class Args
{
public:

					
public:

						Args(int argc, char** argv, const char* shortopts, const ArgsOption* longopts);
						~Args();

	int32_t				next_option();
	int32_t				option_index() const;
	const char*			option_argument() const;
	

private:

						// Returns the @option type
						// Returns AOT_SHORT if @option starts with "-"
						// Returns AOT_LONG if @option starts with "--"
	ArgsOptionType		option_type(const char* option);
	int32_t				long_option(const char* option);
	int32_t				short_option(const char* option);

private:

	int					m_argc;
	char**				m_argv;
	const char*			m_shortopts;
	const ArgsOption*	m_longopts;

	int					m_current_arg;

	int					m_option_index;
	char*				m_option_argument;
};

//-----------------------------------------------------------------------------
inline Args::Args(int argc, char** argv, const char* shortopts, const ArgsOption* longopts) :
	m_argc(argc),
	m_argv(argv),
	m_shortopts(shortopts),
	m_longopts(longopts),
	m_current_arg(1),			// Do not consider argv[0]
	m_option_index(0),
	m_option_argument(NULL)
{
	assert(argv != NULL);
	assert(shortopts != NULL);
	// longopts could be NULL
}

//-----------------------------------------------------------------------------
inline Args::~Args()
{
}

//-----------------------------------------------------------------------------
inline int32_t Args::next_option()
{
	// End of arguments
	if (m_current_arg >= m_argc)
	{
		return -1;
	}

	const char* current_arg = m_argv[m_current_arg];

	ArgsOptionType type = option_type(current_arg);

	int32_t return_value = -1;

	switch (type)
	{
		case AOT_SHORT:
		{
			return_value = short_option(current_arg);
			break;
		}
		case AOT_LONG:
		{
			return_value = long_option(current_arg);
			break;
		}
		case AOT_NOT_OPTION:
		{
			return_value = '?';
			break;
		}
		default:
		{
			return_value = '?';
			break;
		}
	}

	m_current_arg++;

	return return_value;
}

//-----------------------------------------------------------------------------
inline int32_t Args::option_index() const
{
	return m_option_index;
}

//-----------------------------------------------------------------------------
inline const char* Args::option_argument() const
{
	return m_option_argument;
}

//-----------------------------------------------------------------------------
inline ArgsOptionType Args::option_type(const char* option)
{
	size_t option_len = string::strlen(option);

	if (option_len > 2)
	{
		if (option[0] == '-' && option[1] == '-')
		{
			return AOT_LONG;
		}
	}

	if (option_len > 1)
	{
		if (option[0] == '-')
		{
			return AOT_SHORT;
		}
	}

	return AOT_NOT_OPTION;
}

//-----------------------------------------------------------------------------
int32_t Args::long_option(const char* option)
{
	int32_t option_index = 0;
	const ArgsOption* current_option = m_longopts;

	while (1)
	{
		if (current_option->name == NULL && current_option->has_arg == 0 &&
			current_option->flag == NULL && current_option->val == 0)
		{
			break;
		}

		if (string::strcmp(current_option->name, &option[2]) == 0)
		{
			m_option_index = option_index;

			if (current_option->has_arg == AOA_REQUIRED_ARGUMENT)
			{
				if (m_argc > m_current_arg + 1)
				{
					m_option_argument = m_argv[m_current_arg + 1];
				}
				else
				{
					m_option_argument = NULL;
				}

				m_current_arg += 1;
			}
			else
			{
				m_option_argument = NULL;
			}

			if (current_option->flag == NULL)
			{
				return current_option->val;
			}
			else
			{
				(*current_option->flag) = current_option->val;

				return 0;
			}
		}

		option_index++;
		current_option++;
	}

	return -1;
}

//-----------------------------------------------------------------------------
int32_t Args::short_option(const char* option)
{
	(void)option;
	return 0;
}

} // namespace crown

