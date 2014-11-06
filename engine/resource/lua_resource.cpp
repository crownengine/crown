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

#include "config.h"
#include "dynamic_string.h"
#include "filesystem.h"
#include "log.h"
#include "lua_resource.h"
#include "os.h"
#include "temp_allocator.h"
#include "array.h"
#include "compile_options.h"

#if defined(CROWN_DEBUG)
	#define LUAJIT_FLAGS "-bg" // Keep debug info
#else
	#define LUAJIT_FLAGS "-b"
#endif

namespace crown
{
namespace lua_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		static const uint32_t VERSION = 1;

		TempAllocator1024 alloc;
		DynamicString res_abs_path(alloc);
		TempAllocator1024 alloc2;
		DynamicString bc_abs_path(alloc2);
		opts.get_absolute_path(path, res_abs_path);
		opts.get_absolute_path("bc.tmp", bc_abs_path);

		const char* luajit[] =
		{
#if CROWN_PLATFORM_LINUX
			"./luajit"
#else
			"luajit.exe"
#endif // CROWN_PLATFORM_LINUX
			LUAJIT_FLAGS,
			res_abs_path.c_str(),
			bc_abs_path.c_str(),
			NULL
		};

		int exitcode = os::execute_process(luajit);
		CE_ASSERT(exitcode == 0, "Failed to compile lua");
		
		Buffer blob = opts.read(bc_abs_path.c_str());
		opts.delete_file(bc_abs_path.c_str());

		LuaResource lr;
		lr.version = VERSION;
		lr.size = array::size(blob);

		opts.write(lr.version);
		opts.write(lr.size);
		opts.write(blob);
	}

	void* load(File& file, Allocator& a)
	{
		const size_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		return res;
	}

	void online(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void offline(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	uint32_t size(const LuaResource* lr)
	{
		return lr->size;
	}

	const char* program(const LuaResource* lr)
	{
		return (char*)lr + sizeof(LuaResource);
	}
} // namespace lua_resource
} // namespace crown
