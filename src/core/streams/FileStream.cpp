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
uint8_t FileStream::read_byte()
{
	check_valid();

	if (!m_last_was_read)
	{
		m_last_was_read = true;
		fseek(m_file->get_handle(), 0, SEEK_CUR);
	}

	uint8_t buffer;

	if (fread(&buffer, 1, 1, m_file->get_handle()) != 1)
	{
		Log::E("Could not read from file.");
	}

	return buffer;
}

//-----------------------------------------------------------------------------
void FileStream::read_data_block(void* buffer, size_t size)
{
	check_valid();

	if (!m_last_was_read)
	{
		m_last_was_read = true;
		fseek(m_file->get_handle(), 0, SEEK_CUR);
	}

	if (fread(buffer, size, 1, m_file->get_handle()) != 1)
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
		readBytes = fread(buff, 1, expectedReadBytes, m_file->get_handle());

		if (readBytes < expectedReadBytes)
		{
			if (feof(m_file->get_handle()))
			{
				if (readBytes != 0)
				{
					stream->write_data_block(buff, readBytes);
				}
			}

			delete[] buff;
			//Either the file gave an error, or ended before size bytes could be copied
			return false;
		}

		stream->write_data_block(buff, readBytes);
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
		fseek(m_file->get_handle(), 0, SEEK_CUR);
	}

	if (fputc(val, m_file->get_handle()) == EOF)
	{
		Log::E("Could not write to file.");
	}
}

//-----------------------------------------------------------------------------
void FileStream::write_data_block(const void* buffer, size_t size)
{
	check_valid();

	if (m_last_was_read)
	{
		m_last_was_read = false;
		fseek(m_file->get_handle(), 0, SEEK_CUR);
	}

	if (fwrite(buffer, size, 1, m_file->get_handle()) != 1)
	{
		Log::E("Could not write to file.");
	}
}

//-----------------------------------------------------------------------------
void FileStream::flush()
{
	check_valid();
	fflush(m_file->get_handle());
}

//-----------------------------------------------------------------------------
void FileStream::seek(int32_t position, SeekMode mode)
{
	check_valid();
	//flush(); <<<---?
	fseek(m_file->get_handle(), position, (mode==SM_FROM_BEGIN)?SEEK_SET:(mode==SM_FROM_CURRENT)?SEEK_CUR:SEEK_END);
}

//-----------------------------------------------------------------------------
size_t FileStream::position() const
{
	check_valid();
	return ftell(m_file->get_handle());
}

//-----------------------------------------------------------------------------
size_t FileStream::size() const
{
	size_t pos = position();
	fseek(m_file->get_handle(), 0, SEEK_END);
	size_t size = position();
	fseek(m_file->get_handle(), pos, SEEK_SET);
	return size;
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

