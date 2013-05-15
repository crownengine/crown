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

#include "FileStream.h"
#include "Types.h"
#include "Log.h"
#include "MathUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
FileStream::FileStream(StreamOpenMode mode, const char* filename) :
	Stream(mode),
	m_last_was_read(true)
{
	m_file.open(filename, mode);
}

//-----------------------------------------------------------------------------
FileStream::~FileStream()
{
	//m_file.close();
}

//-----------------------------------------------------------------------------
void FileStream::seek(size_t position)
{
	check_valid();

	m_file.seek(position);
}

//-----------------------------------------------------------------------------
void FileStream::seek_to_end()
{
	check_valid();

	m_file.seek_to_end();
}

//-----------------------------------------------------------------------------
void FileStream::skip(size_t bytes)
{
	check_valid();

	m_file.skip(bytes);
}

//-----------------------------------------------------------------------------
uint8_t FileStream::read_byte()
{
	check_valid();

	if (!m_last_was_read)
	{
		m_last_was_read = true;
		m_file.seek(0);
	}

	uint8_t buffer;

	assert(m_file.read(&buffer, 1));

	return buffer;
}

//-----------------------------------------------------------------------------
void FileStream::read(void* buffer, size_t size)
{
	check_valid();

	if (!m_last_was_read)
	{
		m_last_was_read = true;
		m_file.seek(0);
	}

	assert(m_file.read(buffer, size));
}

//-----------------------------------------------------------------------------
bool FileStream::copy_to(Stream* stream, size_t size)
{
	check_valid();

	if (stream == 0)
		return false;
	if (size == 0)
		return true;

	const size_t chunksize = 1024*1024;

	char* buff = new char[chunksize];

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
					stream->write(buff, read_bytes);
				}
			}

			delete[] buff;
			//Either the file gave an error, or ended before size bytes could be copied
			return false;
		}

		stream->write(buff, read_bytes);
		tot_read_bytes += read_bytes;
	}

	delete [] buff;
	return true;
}

//-----------------------------------------------------------------------------
bool FileStream::end_of_stream() const
{
	return position() == size();
}

//-----------------------------------------------------------------------------
bool FileStream::is_valid() const
{
	return m_file.is_open();
}

//-----------------------------------------------------------------------------
void FileStream::write_byte(uint8_t val)
{
	check_valid();

	if (m_last_was_read)
	{
		m_last_was_read = false;
		m_file.seek(0);
	}

	assert(m_file.write(&val, 1) == 1);
}

//-----------------------------------------------------------------------------
void FileStream::write(const void* buffer, size_t size)
{
	check_valid();

	if (m_last_was_read)
	{
		m_last_was_read = false;
		m_file.seek(0);
	}

	assert(m_file.write(buffer, size) == size);
}

//-----------------------------------------------------------------------------
void FileStream::flush()
{
	check_valid();
	
	// FIXME implement flush in File
}

//-----------------------------------------------------------------------------
size_t FileStream::position() const
{
	check_valid();

	return m_file.position();
}

//-----------------------------------------------------------------------------
size_t FileStream::size() const
{
	check_valid();
	
	return m_file.size();
}

//-----------------------------------------------------------------------------
bool FileStream::can_read() const
{
	check_valid();

	return true;
}

//-----------------------------------------------------------------------------
bool FileStream::can_write() const
{
	check_valid();

	return true;
}

//-----------------------------------------------------------------------------
bool FileStream::can_seek() const
{
	return true;
}

} // namespace crown

