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

#include "Assert.h"
#include "Types.h"
#include "String.h"

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
	const char*				name;
	int32_t					has_arg;
	int32_t*				flag;
	int32_t					val;
};

/// Parses the command line arguments in a way very similar to GNU getopt.
class Args
{
public:

						Args(int argc, char** argv, const char* shortopts, const ArgsOption* longopts);
						~Args();

	/// Finds the next option character and returns it.
	/// If there are no more option characters, it returns -1 and optind()
	/// returns the index in argv[] of the first argv-element that is not
	/// an option.
	/// If it finds an option that was not included in shortopts or longopts,
	/// or if it finds a missing option argument, it returns '?' character.
	int32_t				getopt();

	/// Returns the index of the next argument to be processed.
	int32_t				optind() const;

	/// Returns the text of the following argv-element in respect
	/// to the current optind().
	const char*			optarg() const;


	/// Sets the @a index into argv[] from where to start option scanning.
	/// If @a index >= argc nothing will be scanned.
	void				set_optind(int32_t index);

private:

	// Returns the @a option type
	// Returns AOT_SHORT if option is of the form "-x" where 'x' is the option.
	// Returns AOT_LONG if option is of the form "--option" where "option" is the option.
	// Returns AOT_NOT_OPTION in all other cases.
	ArgsOptionType		option_type(const char* option);

	// Parses a long option
	int32_t				long_option(const char* option);

	// Parses a short option
	int32_t				short_option(const char* option);

	void				not_option();

	// Returns whether the given option is the last one
	bool				end_of_longopts(const ArgsOption* option) const;

private:

	int					m_argc;
	char**				m_argv;

	const char*			m_shortopts;
	const ArgsOption*	m_longopts;

	// Index of the next argument to be processed
	int32_t				m_optind;

	// Number of "true" arguments
	int32_t				m_scope;

	// The text of the following argv-element to argv[optind]
	char*				m_optarg;
};

} // namespace crown

