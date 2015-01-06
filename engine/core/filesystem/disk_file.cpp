/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "disk_file.h"
#include "types.h"
#include "log.h"
#include "math_utils.h"
#include "memory.h"

namespace crown
{

DiskFile::DiskFile(FileOpenMode mode, const char* filename)
	: File(mode)
	, _file(filename, mode)
	, _last_was_read(true)
{
}

DiskFile::~DiskFile()
{
	_file.close();
}

void DiskFile::seek(size_t position)
{
	check_valid();

	_file.seek(position);
}

void DiskFile::seek_to_end()
{
	check_valid();

	_file.seek_to_end();
}

void DiskFile::skip(size_t bytes)
{
	check_valid();

	_file.skip(bytes);
}

void DiskFile::read(void* buffer, size_t size)
{
	check_valid();

	if (!_last_was_read)
	{
		_last_was_read = true;
		_file.seek(0);
	}

	/*size_t bytes_read =*/ _file.read(buffer, size);
	//CE_ASSERT(bytes_read == size, "Failed to read from file: requested: %llu, read: %llu", size, bytes_read);
}

void DiskFile::write(const void* buffer, size_t size)
{
	check_valid();

	if (_last_was_read)
	{
		_last_was_read = false;
		_file.seek(0);
	}

	/*size_t bytes_written =*/ _file.write(buffer, size);
	//CE_ASSERT(bytes_written == size, "Failed to write to file: requested: %llu, written: %llu", size, bytes_written);
}

bool DiskFile::copy_to(File& file, size_t size)
{
	check_valid();

	const size_t chunksize = 1024*1024;

	char* buff = (char*) default_allocator().allocate(chunksize * sizeof(char));

	size_t tot_read_bytes = 0;

	while (tot_read_bytes < size)
	{
		size_t read_bytes;
		size_t expected_read_bytes = min(size - tot_read_bytes, chunksize);

		read_bytes = _file.read(buff, expected_read_bytes);

		if (read_bytes < expected_read_bytes)
		{
			if (_file.eof())
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

bool DiskFile::end_of_file()
{
	return position() == size();
}

bool DiskFile::is_valid()
{
	return _file.is_open();
}

void DiskFile::flush()
{
	check_valid();

	// FIXME implement flush in File
}

size_t DiskFile::position()
{
	check_valid();

	return _file.position();
}

size_t DiskFile::size()
{
	check_valid();

	return _file.size();
}

bool DiskFile::can_read() const
{
	check_valid();

	return true;
}

bool DiskFile::can_write() const
{
	check_valid();

	return true;
}

bool DiskFile::can_seek() const
{
	return true;
}

} // namespace crown

