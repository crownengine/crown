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

#include "VSCompiler.h"
#include "DiskFile.h"
#include "Resource.h"

namespace crown
{

//-----------------------------------------------------------------------------
VSCompiler::VSCompiler(const char* root_path, const char* dest_path) :
	Compiler(root_path, dest_path, VERTEX_SHADER_TYPE),
	m_file_size(0),
	m_file_data(NULL)
{
}

//-----------------------------------------------------------------------------
VSCompiler::~VSCompiler()
{
	cleanup_impl();
}

//-----------------------------------------------------------------------------
size_t VSCompiler::read_header_impl(DiskFile* in_file)
{
	(void) in_file;
	return 0;
}

//-----------------------------------------------------------------------------
size_t VSCompiler::read_resource_impl(DiskFile* in_file)
{
	m_file_size = in_file->size();

	m_file_data = (char*)default_allocator().allocate(m_file_size * sizeof(char));
	
	// Copy the entire file into the buffer
	in_file->read(m_file_data, m_file_size);

	// Return total resource size
	return m_file_size + sizeof(uint32_t);
}

//-----------------------------------------------------------------------------
void VSCompiler::write_header_impl(DiskFile* out_file)
{
	out_file->write(&m_file_size, sizeof(uint32_t));
}

//-----------------------------------------------------------------------------
void VSCompiler::write_resource_impl(DiskFile* out_file)
{
	out_file->write(m_file_data, m_file_size);
}

//-----------------------------------------------------------------------------
void VSCompiler::cleanup_impl()
{
	if (m_file_data)
	{
		default_allocator().deallocate(m_file_data);
		m_file_data = NULL;
	}
}

} // namespace crown

