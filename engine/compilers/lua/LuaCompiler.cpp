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

#include "LuaCompiler.h"
#include "LuaResource.h"
#include "TempAllocator.h"
#include "DynamicString.h"
#include "Filesystem.h"
#include "OS.h"

namespace crown
{

//-----------------------------------------------------------------------------
LuaCompiler::LuaCompiler()
	: m_luajit_blob_size(0), m_luajit_blob(NULL)
{
}

//-----------------------------------------------------------------------------
LuaCompiler::~LuaCompiler()
{
}

//-----------------------------------------------------------------------------
size_t LuaCompiler::compile_impl(Filesystem& fs, const char* resource_path)
{
	TempAllocator1024 alloc;
	DynamicString res_abs_path(alloc);
	TempAllocator1024 alloc2;
	DynamicString bc_abs_path(alloc2);
	fs.get_absolute_path(resource_path, res_abs_path);
	fs.get_absolute_path("bc.tmp", bc_abs_path);

	const char* luajit[] =
	{
		"./luajit",
		"-b",
		res_abs_path.c_str(),
		bc_abs_path.c_str(),
		NULL
	};

	os::execute_process(luajit);

	File* bc = fs.open(bc_abs_path.c_str(), FOM_READ);
	if (bc != NULL)
	{
		m_luajit_blob_size = bc->size();
		m_luajit_blob = (char*) default_allocator().allocate(m_luajit_blob_size);
		bc->read(m_luajit_blob, m_luajit_blob_size);
		fs.close(bc);
		fs.delete_file(bc_abs_path.c_str());
	}
	else
	{
		Log::e("Error while reading luajit bytecode");
		return 0;
	}

	return sizeof(LuaHeader) + m_luajit_blob_size;
}

//-----------------------------------------------------------------------------
void LuaCompiler::write_impl(File* out_file)
{
	LuaHeader header;
	header.version = LUA_RESOURCE_VERSION;
	header.size = m_luajit_blob_size;

	out_file->write((char*)&header, sizeof(LuaHeader));
	out_file->write((char*)m_luajit_blob, m_luajit_blob_size);

	default_allocator().deallocate(m_luajit_blob);
	m_luajit_blob_size = 0;
	m_luajit_blob = NULL;
}

} // namespace crown
