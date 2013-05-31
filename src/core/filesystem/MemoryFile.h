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

#include "Types.h"
#include "File.h"
#include "Allocator.h"
#include "Assert.h"

namespace crown
{

class MemoryBuffer
{
public:

						MemoryBuffer();
	virtual				~MemoryBuffer();

	virtual void		release() = 0;
	virtual void		allocate(size_t size) = 0;

	virtual bool		is_valid() = 0;
	virtual size_t		size() = 0;
	virtual uint8_t*	data() = 0;

	virtual void		write(uint8_t* src, size_t offset, size_t size) = 0;
};

class DynamicMemoryBuffer: public MemoryBuffer
{
public:

						DynamicMemoryBuffer(Allocator& allocator, size_t initial_capacity);
	virtual				~DynamicMemoryBuffer();

	void				release();
	void				allocate(size_t capacity);

	inline bool			is_valid() { return m_buffer != 0; }

	void				check_space(size_t offset, size_t space);
	void				write(uint8_t* src, size_t offset, size_t size);

	inline size_t		size() { return m_size; }
	inline size_t		capacity() { return m_capacity; }

	inline uint8_t*		data() { return m_buffer; }

protected:

	Allocator&			m_allocator;
	uint8_t*			m_buffer;
	size_t				m_capacity;
	size_t				m_size;
};

/// Memory file.
/// Access memory buffers.
class MemoryFile: public File
{
public:

	/// @copydoc File::File()
						MemoryFile(MemoryBuffer* buffer, FileOpenMode mode);

	/// @copydoc File::~File()
	virtual				~MemoryFile();

	/// @copydoc File::seek()
	void				seek(size_t position);

	/// @copydoc File::seek_to_end()
	void				seek_to_end();

	/// @copydoc File::skip()
	void				skip(size_t bytes);

	/// @copydoc File::read()
	void				read(void* buffer, size_t size);

	/// @copydoc File::write()
	void				write(const void* buffer, size_t size);

	/// @copydoc File::copy_to()
	bool				copy_to(File& file, size_t size = 0);

	/// @copydoc File::flush()
	void				flush();

	/// @copydoc File::end_of_file()
	bool				end_of_file() const { return size() == m_memory_offset; }

	/// @copydoc File::is_valid()
	bool				is_valid() const { CE_ASSERT(m_memory != NULL, "Memory is NULL"); return m_memory->is_valid(); }

	/// @copydoc File::size()
	size_t				size() const { CE_ASSERT(m_memory != NULL, "Memory is NULL"); return m_memory->size(); }

	/// @copydoc File::position()
	size_t				position() const { return m_memory_offset; }

	/// @copydoc File::can_read()
	bool				can_read() const { return true; }

	/// @copydoc File::can_write()
	bool				can_write() const { return true; }

	/// @copydoc File::can_seek()
	bool				can_seek() const { return true; }

	/// Dumps the data to the console.
	void				dump();

protected:

	inline void			check_valid() { CE_ASSERT(m_memory != NULL, "Memory is NULL"); }

	MemoryBuffer*		m_memory;
	size_t				m_memory_offset;
};

}
