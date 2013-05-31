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

#include <stdlib.h>

#include "MemoryFile.h"
#include "MathUtils.h"
#include "Log.h"
#include "Types.h"
#include "Allocator.h"
#include "OS.h"

namespace crown
{

//-----------------------------------------------------------------------------
MemoryBuffer::MemoryBuffer()
{
}

//-----------------------------------------------------------------------------
MemoryBuffer::~MemoryBuffer()
{
}

//-----------------------------------------------------------------------------
DynamicMemoryBuffer::DynamicMemoryBuffer(Allocator& allocator, size_t initial_capacity) :
	m_allocator(allocator),
	m_buffer(NULL)
{
	m_buffer = (uint8_t*)m_allocator.allocate(initial_capacity);
}

//-----------------------------------------------------------------------------
DynamicMemoryBuffer::~DynamicMemoryBuffer()
{
	if (m_buffer)
	{
		m_allocator.deallocate(m_buffer);
	}
}

//-----------------------------------------------------------------------------
void DynamicMemoryBuffer::check_space(size_t offset, size_t size)
{
	if (offset + size > m_capacity)
	{
		m_capacity = (size_t) ((offset + size) * 1.2f);
		// FIXME FIXME FIXME
		m_buffer = (uint8_t*) realloc(m_buffer, m_capacity);
	}
}

//-----------------------------------------------------------------------------
void DynamicMemoryBuffer::write(uint8_t* src, size_t offset, size_t size)
{
	check_space(offset, size);

	for (size_t i = 0; i < size; i++)
	{
		m_buffer[offset + i] = src[i];
	}

	//If the writing goes beyond the end of buffer
	if (offset + size > this->m_size)
	{
		this->m_size = offset + size;
	}
}

//-----------------------------------------------------------------------------
void DynamicMemoryBuffer::release()
{
	// FIXME
}

//-----------------------------------------------------------------------------
void DynamicMemoryBuffer::allocate(size_t capacity)
{
	// FIXME
}

//-----------------------------------------------------------------------------
MemoryFile::MemoryFile(MemoryBuffer* buffer, FileOpenMode mode) :
	File(mode),
	m_memory(buffer),
	m_memory_offset(0)
{
}

//-----------------------------------------------------------------------------
MemoryFile::~MemoryFile()
{
}

//-----------------------------------------------------------------------------
void MemoryFile::seek(size_t position)
{
	check_valid();
	
	m_memory_offset = position;

	// Allow seek to m_memory->size() position, that means end of file,
	// reading not allowed but you can write if it's dynamic
	CE_ASSERT(m_memory_offset <= m_memory->size(), "Trying to seek beyond end of file");
}

//-----------------------------------------------------------------------------
void MemoryFile::seek_to_end()
{
	check_valid();

	m_memory_offset = m_memory->size() - 1;
}

//-----------------------------------------------------------------------------
void MemoryFile::skip(size_t bytes)
{
	check_valid();

	m_memory_offset += bytes;

	//Allow seek to m_memory->getSize() position, that means end of file, reading not allowed but you can write if it's dynamic
	CE_ASSERT(m_memory_offset <= m_memory->size(), "Trying to skip beyond end of file");
}

//-----------------------------------------------------------------------------
void MemoryFile::read(void* buffer, size_t size)
{
	check_valid();
	uint8_t* src = m_memory->data();
	uint8_t* dest = (uint8_t*) buffer;

	if (m_memory_offset + size > m_memory->size())
	{
		Log::e("Trying to read beyond the end of file.");
	}

	for (size_t i = 0; i < size; i++)
	{
		dest[i] = src[m_memory_offset+i];
	}

	m_memory_offset += size;
}

//-----------------------------------------------------------------------------
bool MemoryFile::copy_to(File& file, size_t size)
{
	check_valid();

	file.write(&(m_memory->data()[m_memory_offset]), math::min(m_memory->size()-m_memory_offset, size));

	return true;
}

//-----------------------------------------------------------------------------
void MemoryFile::write(const void* buffer, size_t size)
{
	check_valid();
	m_memory->write((uint8_t*)buffer, m_memory_offset, size);
	m_memory_offset += size;
}

//-----------------------------------------------------------------------------
void MemoryFile::flush()
{
	return;
}

//-----------------------------------------------------------------------------
void MemoryFile::dump()
{
	uint8_t* buff = m_memory->data();

	for (size_t i = 0; i < m_memory->size(); i++)
	{
		os::printf("%3i ", buff[i]);
	}
}

} // namespace crown

