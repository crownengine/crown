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

#include "TXTCompiler.h"
#include "FileStream.h"

namespace crown
{

//-----------------------------------------------------------------------------
TXTCompiler::TXTCompiler(const char* root_path, const char* dest_path, const char* resource, uint32_t seed) :
	Compiler(root_path, dest_path, resource, seed),
	m_file_size(0),
	m_file_data(NULL)
{
}

//-----------------------------------------------------------------------------
TXTCompiler::~TXTCompiler()
{
	if (m_file_data)
	{
		delete[] m_file_data;
	}
}

//-----------------------------------------------------------------------------
bool TXTCompiler::compile()
{
	FileStream* file = Compiler::source_file();

	m_file_size = file->size();

	if (m_file_size == 0)
	{
		return false;
	}

	m_file_data = new char[m_file_size];
	
	// Copy the entire file into the buffer
	file->read(m_file_data, m_file_size);

	// Prepare for writing
	Compiler::prepare_header(m_file_size + sizeof(uint32_t));

	return true;
}

//-----------------------------------------------------------------------------
void TXTCompiler::write()
{
	Compiler::write_header();

	FileStream* file = Compiler::destination_file();

	file->write(&m_file_size, sizeof(uint32_t));
	file->write(m_file_data, m_file_size);
}

} // namespace crown

