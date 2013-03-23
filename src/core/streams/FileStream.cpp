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
	Stream(mode), m_file(NULL),
	m_last_was_read(true)
{
	//Takes ownership
	FileOpenMode file_open_mode = (FileOpenMode)0;

	if (math::test_bitmask(mode, SOM_READ))
		file_open_mode = (FileOpenMode)(file_open_mode | FOM_READ);
	if (math::test_bitmask(mode, SOM_WRITE))
		file_open_mode = (FileOpenMode)(file_open_mode | FOM_WRITE);

	m_file = File::open(filename, file_open_mode);
}

//-----------------------------------------------------------------------------
FileStream::~FileStream()
{
	delete m_file;

	m_file = 0;
}

//-----------------------------------------------------------------------------
void FileStream::seek(size_t position)
{
	check_valid();

	m_file->seek(position, SEEK_SET);
}

//-----------------------------------------------------------------------------
void FileStream::seek_to_end()
{
	check_valid();

	m_file->seek(0, SEEK_END);
}

//-----------------------------------------------------------------------------
void FileStream::skip(size_t bytes)
{
	check_valid();

	m_file->seek(bytes, SEEK_CUR);
}

//-----------------------------------------------------------------------------
uint8_t FileStream::read_byte()
{
	check_valid();

	if (!m_last_was_read)
	{
		m_last_was_read = true;
		m_file->seek(0, SEEK_CUR);
	}

	uint8_t buffer;
	
	if (m_file->read(&buffer, 1, 1) != 1)
	{
		Log::E("Could not read from file");
	}

	return buffer;
}

//-----------------------------------------------------------------------------
void FileStream::read(void* buffer, size_t size)
{
	check_valid();

	if (!m_last_was_read)
	{
		m_last_was_read = true;
		m_file->seek(0, SEEK_CUR);
	}

	if (m_file->read(buffer, size, 1) != 1)
	{
		Log::E("Could not read from file.");
	}
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

	size_t totReadBytes = 0;

	while (totReadBytes < size)
	{
		int32_t readBytes;
		int32_t expectedReadBytes = math::min(size - totReadBytes, chunksize);

		readBytes = m_file->read(buff, 1, expectedReadBytes);

		if (readBytes < expectedReadBytes)
		{
			if (m_file->eof())
			{
				if (readBytes != 0)
				{
					stream->write(buff, readBytes);
				}
			}

			delete[] buff;
			//Either the file gave an error, or ended before size bytes could be copied
			return false;
		}

		stream->write(buff, readBytes);
		totReadBytes += readBytes;
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
	{
		if (!m_file)
		{
			return false;
		}

		return m_file->is_valid();
	}
}

//-----------------------------------------------------------------------------
void FileStream::write_byte(uint8_t val)
{
	check_valid();

	if (m_last_was_read)
	{
		m_last_was_read = false;
		m_file->seek(0, SEEK_CUR);
	}

	if (m_file->write(&val, 1, 1) != 1)
	{
		Log::E("Could not write to file.");
	}
}

//-----------------------------------------------------------------------------
void FileStream::write(const void* buffer, size_t size)
{
	check_valid();

	if (m_last_was_read)
	{
		m_last_was_read = false;
		m_file->seek(0, SEEK_CUR);
	}

	if (m_file->write(buffer, size, 1) != 1)
	{
		Log::E("Could not write to file.");
	}
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

	return m_file->tell();
}

//-----------------------------------------------------------------------------
size_t FileStream::size() const
{
	check_valid();
	
	return m_file->size();
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

