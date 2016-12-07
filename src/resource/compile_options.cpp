/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "compile_options.h"
#include "os.h"

namespace crown
{
int CompileOptions::run_external_compiler(const char* const* argv, StringStream& output)
{
	TempAllocator512 ta;
	StringStream ss(ta);

	for (s32 i = 1; argv[i] != NULL; ++i)
	{
		const char* arg = argv[i];
		for (; *arg; ++arg)
		{
			if (*arg == ' ')
				ss << '\\';
			ss << *arg;
		}
		ss << ' ';
	}

	return os::execute_process(argv[0], string_stream::c_str(ss), output);
}

} // namespace crown
