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

#include "Args.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
Args::Args(int argc, char** argv, const char* shortopts, const ArgsOption* longopts) :
	m_argc(argc),
	m_argv(argv),
	m_shortopts(shortopts),
	m_longopts(longopts),
	m_optind(1),				// Do not consider argv[0]
	m_scope(argc),
	m_optarg(NULL)
{
	CE_ASSERT(argv != NULL, "Argument vector must be != NULL");
	CE_ASSERT(shortopts != NULL, "Short argument list must be != NULL");
	// longopts could be NULL
}

//-----------------------------------------------------------------------------
Args::~Args()
{
}

//-----------------------------------------------------------------------------
int32_t Args::getopt()
{
	// Always reset optarg
	m_optarg = NULL;

	// End of arguments
	if (m_optind >= m_scope)
	{
		return -1;
	}

	switch (option_type(m_argv[m_optind]))
	{
		case AOT_SHORT:
		{
			return short_option(m_argv[m_optind]);
		}
		case AOT_LONG:
		{
			return long_option(m_argv[m_optind]);
		}
		case AOT_NOT_OPTION:
		{
			not_option();
			return getopt();
		}
		default:
		{
			return '?';
		}
	}
}

//-----------------------------------------------------------------------------
int32_t Args::optind() const
{
	return m_optind;
}

//-----------------------------------------------------------------------------
const char* Args::optarg() const
{
	return m_optarg;
}

//-----------------------------------------------------------------------------
void Args::set_optind(int32_t index)
{
	m_optind = index;
}

//-----------------------------------------------------------------------------
ArgsOptionType Args::option_type(const char* option)
{
	const size_t option_len = string::strlen(option);

	if (option_len == 2 && option[0] == '-' && option[1] != '-')
	{
		return AOT_SHORT;
	}
	else if (option_len > 2 && option[0] == '-' && option[1] == '-')
	{
		return AOT_LONG;
	}

	return AOT_NOT_OPTION;
}

//-----------------------------------------------------------------------------
int32_t Args::long_option(const char* option)
{
	const ArgsOption* current_option = m_longopts;

	// Loop through all the long options
	while (!end_of_longopts(current_option))
	{
		if (string::strcmp(current_option->name, &option[2]) == 0)
		{
			// If the option requires an argument
			if (current_option->has_arg == AOA_REQUIRED_ARGUMENT)
			{
				// Read the argument if it exists
				if ((m_optind + 1) < m_scope)
				{
					// Read the argument and skip the following parameter
					m_optarg = m_argv[m_optind + 1];
					m_optind += 2;
				}
				else
				{
					Log::e("%s: option requires an argument -- '%s'", m_argv[0], current_option->name);

					// Missing option
					m_optind += 1;
					return '?';
				}
			}
			// If the option does not require an argument
			else
			{
				m_optind++;
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

		current_option++;
	}

	// Found a long option but was not included in longopts
	Log::e("%s: invalid option -- '%s'", m_argv[0], &option[2]);
	m_optind++;
	return '?';
}

//-----------------------------------------------------------------------------
int32_t Args::short_option(const char* option)
{
	(void)option;

	Log::e("%s: invalid option -- '%s'", m_argv[0], &option[1]);
	m_optind++;
	return '?';
}

//-----------------------------------------------------------------------------
void Args::not_option()
{
	char* current_option = m_argv[m_optind];

	for (int32_t i = m_optind; i < (m_argc - 1); i++)
	{
		m_argv[i] = m_argv[i + 1];
	}

	m_argv[m_argc - 1] = current_option;

	// Reduce the number of true arguments
	m_scope--;
}

//-----------------------------------------------------------------------------
bool Args::end_of_longopts(const ArgsOption* option) const
{
	return (option->name == NULL && option->has_arg == 0 && option->flag == NULL && option->val == 0);
}


} // namespace crown
