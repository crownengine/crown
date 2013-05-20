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

#include "LuaCompiler.h"
#include "FileStream.h"
#include "Resource.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
LuaCompiler::LuaCompiler(const char* root_path, const char* dest_path) :
	Compiler(root_path, dest_path, SCRIPT_TYPE),
	m_file_size(0),
	m_file_data(NULL)
{
}

//-----------------------------------------------------------------------------
LuaCompiler::~LuaCompiler()
{
	cleanup_impl();
}

//-----------------------------------------------------------------------------
size_t LuaCompiler::read_header_impl(FileStream* in_file)
{
	(void) in_file;
	return 0;
}

//-----------------------------------------------------------------------------
size_t LuaCompiler::read_resource_impl(FileStream* in_file)
{
	Filesystem fs(root_path());

	char tmp_resource[os::MAX_PATH_LENGTH];

	string::strcpy(tmp_resource, resource_name());
	string::strcat(tmp_resource, ".script");

	if (!fs.exists(tmp_resource))
	{
		Log::e("'%s': resource cannot be found.");
		return 0;
	}

	FileStream* tmp_file = (FileStream*)fs.open(tmp_resource, SOM_READ);

	m_file_size = tmp_file->size();

	if (m_file_size == 0)
	{
		Log::e("'%s': resource is empty.");
		return 0;
	}

	m_file_data = new char[m_file_size];

	// Copy the entire file into the buffer
	tmp_file->read(m_file_data, m_file_size);

	// Returns the total size of the resource
	return m_file_size;
}

//-----------------------------------------------------------------------------
void LuaCompiler::write_header_impl(FileStream* out_file)
{
	(void) out_file;
}

//-----------------------------------------------------------------------------
void LuaCompiler::write_resource_impl(FileStream* out_file)
{
	out_file->write(m_file_data, m_file_size);
}

//-----------------------------------------------------------------------------
void LuaCompiler::cleanup_impl()
{
	if (m_file_data)
	{
		delete[] m_file_data;
		m_file_data = NULL;
	}
}

} // namespace crown
