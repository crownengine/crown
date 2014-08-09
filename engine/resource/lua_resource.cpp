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

#if CROWN_PLATFORM_WINDOWS
	#define LUAJIT_EXECUTABLE "luajit.exe"
#else
	#define LUAJIT_EXECUTABLE "./luajit"
#endif

#if defined(CROWN_DEBUG)
	#define LUAJIT_FLAGS "-bg" // Keep debug info
#else
	#define LUAJIT_FLAGS "-b"
#endif

namespace crown
{
namespace lua_resource
{

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	TempAllocator1024 alloc;
	DynamicString res_abs_path(alloc);
	TempAllocator1024 alloc2;
	DynamicString bc_abs_path(alloc2);
	fs.get_absolute_path(resource_path, res_abs_path);
	fs.get_absolute_path("bc.tmp", bc_abs_path);

	const char* luajit[] =
	{
		LUAJIT_EXECUTABLE,
		LUAJIT_FLAGS,
		res_abs_path.c_str(),
		bc_abs_path.c_str(),
		NULL
	};

	os::execute_process(luajit);

	size_t program_size = 0;
	char* program = NULL;

	File* bc = fs.open(bc_abs_path.c_str(), FOM_READ);
	if (bc != NULL)
	{
		program_size = bc->size();
		program = (char*) default_allocator().allocate(program_size);
		bc->read(program, program_size);
		fs.close(bc);
		fs.delete_file(bc_abs_path.c_str());
	}
	else
	{
		CE_LOGE("Error while reading luajit bytecode");
		return;
	}

	LuaHeader header;
	header.version = LUA_RESOURCE_VERSION;
	header.size = program_size;

	out_file->write((char*)&header, sizeof(LuaHeader));
	out_file->write((char*)program, program_size);

	default_allocator().deallocate(program);
}

} // namespace lua_resource
} // namespace crown
