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

#include "DiskFile.h"
#include "Types.h"
#include "Log.h"
#include "MathUtils.h"
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
DiskFile::DiskFile(FileOpenMode mode, const char* filename) :
	File(mode),
	m_file(filename, mode),
	m_last_was_read(true)
{
}

//-----------------------------------------------------------------------------
DiskFile::~DiskFile()
{
	//m_file.close();
}

//-----------------------------------------------------------------------------
void DiskFile::seek(size_t position)
{
	check_valid();

	m_file.seek(position);
}

//-----------------------------------------------------------------------------
void DiskFile::seek_to_end()
{
	check_valid();

	m_file.seek_to_end();
}

//-----------------------------------------------------------------------------
void DiskFile::skip(size_t bytes)
{
	check_valid();

	m_file.skip(bytes);
}

//-----------------------------------------------------------------------------
void DiskFile::read(void* buffer, size_t size)
{
	check_valid();

	if (!m_last_was_read)
	{
		m_last_was_read = true;
		m_file.seek(0);
	}

	size_t bytes_read = m_file.read(buffer, size);
	CE_ASSERT(bytes_read == size, "Failed to read from file");
}

//-----------------------------------------------------------------------------
void DiskFile::write(const void* buffer, size_t size)
{
	check_valid();

	if (m_last_was_read)
	{
		m_last_was_read = false;
		m_file.seek(0);
	}

	size_t bytes_written = m_file.write(buffer, size);
	CE_ASSERT(bytes_written == size, "Failed to write to file");
}

//-----------------------------------------------------------------------------
bool DiskFile::copy_to(File& file, size_t size)
{
	check_valid();

	const size_t chunksize = 1024*1024;

	char* buff = (char*) default_allocator().allocate(chunksize * sizeof(char));

	size_t tot_read_bytes = 0;

	while (tot_read_bytes < size)
	{
		size_t read_bytes;
		size_t expected_read_bytes = math::min(size - tot_read_bytes, chunksize);

		read_bytes = m_file.read(buff, expected_read_bytes);

		if (read_bytes < expected_read_bytes)
		{
			if (m_file.eof())
			{
				if (read_bytes != 0)
				{
					file.write(buff, read_bytes);
				}
			}

			default_allocator().deallocate(buff);
			//Either the file gave an error, or ended before size bytes could be copied
			return false;
		}

		file.write(buff, read_bytes);
		tot_read_bytes += read_bytes;
	}

	default_allocator().deallocate(buff);
	return true;
}

//-----------------------------------------------------------------------------
bool DiskFile::end_of_file() const
{
	return position() == size();
}

//-----------------------------------------------------------------------------
bool DiskFile::is_valid() const
{
	return m_file.is_open();
}

//-----------------------------------------------------------------------------
void DiskFile::flush()
{
	check_valid();
	
	// FIXME implement flush in File
}

//-----------------------------------------------------------------------------
size_t DiskFile::position() const
{
	check_valid();

	return m_file.position();
}

//-----------------------------------------------------------------------------
size_t DiskFile::size() const
{
	check_valid();
	
	return m_file.size();
}

//-----------------------------------------------------------------------------
bool DiskFile::can_read() const
{
	check_valid();

	return true;
}

//-----------------------------------------------------------------------------
bool DiskFile::can_write() const
{
	check_valid();

	return true;
}

//-----------------------------------------------------------------------------
bool DiskFile::can_seek() const
{
	return true;
}

} // namespace crown

