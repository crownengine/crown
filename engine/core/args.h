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

#include "assert.h"
#include "types.h"
#include "string_utils.h"

namespace crown
{

enum ArgsOptionType
{
	AOT_SHORT,
	AOT_LONG,
	AOT_NOT_OPTION
};

enum ArgsOptionArgument
{
	AOA_NO_ARGUMENT,
	AOA_REQUIRED_ARGUMENT
};

struct ArgsOption
{
	const char* name;
	int32_t has_arg;
	int32_t* flag;
	int32_t val;
};

/// Parses the command line arguments in a way very similar to GNU getopt.
class Args
{
public:

	Args(int argc, char** argv, const char* shortopts, const ArgsOption* longopts)
		: m_argc(argc)
		, m_argv(argv)
		, m_shortopts(shortopts)
		, m_longopts(longopts)
		, m_optind(1)				// Do not consider argv[0]
		, m_scope(argc)
		, m_optarg(NULL)
	{
		CE_ASSERT(argv != NULL, "Argument vector must be != NULL");
		CE_ASSERT(shortopts != NULL, "Short argument list must be != NULL");
		// longopts could be NULL
	}

	/// Finds the next option character and returns it.
	/// If there are no more option characters, it returns -1 and optind()
	/// returns the index in argv[] of the first argv-element that is not
	/// an option.
	/// If it finds an option that was not included in shortopts or longopts,
	/// or if it finds a missing option argument, it returns '?' character.
	int32_t getopt()
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

	/// Returns the index of the next argument to be processed.
	int32_t optind() const
	{
		return m_optind;
	}

	/// Returns the text of the following argv-element in respect
	/// to the current optind().
	const char* optarg() const
	{
		return m_optarg;
	}


	/// Sets the @a index into argv[] from where to start option scanning.
	/// If @a index >= argc nothing will be scanned.
	void set_optind(int32_t index)
	{
		m_optind = index;
	}

private:

	// Returns the @a option type
	// Returns AOT_SHORT if option is of the form "-x" where 'x' is the option.
	// Returns AOT_LONG if option is of the form "--option" where "option" is the option.
	// Returns AOT_NOT_OPTION in all other cases.
	ArgsOptionType option_type(const char* option)
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


	// Parses a long option
	int32_t long_option(const char* option)
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
						// CE_LOGE("%s: option requires an argument -- '%s'", m_argv[0], current_option->name);

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
		// CE_LOGE("%s: invalid option -- '%s'", m_argv[0], &option[2]);
		m_optind++;
		return '?';
	}

	// Parses a short option
	int32_t short_option(const char* option)
	{
		(void)option;

		// CE_LOGE("%s: invalid option -- '%s'", m_argv[0], &option[1]);
		m_optind++;
		return '?';
	}

	void not_option()
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

	// Returns whether the given option is the last one
	bool end_of_longopts(const ArgsOption* option) const
	{
		return (option->name == NULL && option->has_arg == 0 && option->flag == NULL && option->val == 0);
	}

private:

	int m_argc;
	char** m_argv;

	const char* m_shortopts;
	const ArgsOption* m_longopts;

	// Index of the next argument to be processed
	int32_t m_optind;

	// Number of "true" arguments
	int32_t m_scope;

	// The text of the following argv-element to argv[optind]
	char* m_optarg;
};

} // namespace crown
