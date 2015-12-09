/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "disk_file.h"
#include "types.h"
#include "memory.h"
#include <algorithm>

namespace crown
{

DiskFile::DiskFile(FileOpenMode mode, const char* path)
	: File(mode)
	, _last_was_read(true)
{
	_file.open(path, mode);
}

DiskFile::~DiskFile()
{
	_file.close();
}

void DiskFile::seek(uint32_t position)
{
	check_valid();

	_file.seek(position);
}

void DiskFile::seek_to_end()
{
	check_valid();

	_file.seek_to_end();
}

void DiskFile::skip(uint32_t bytes)
{
	check_valid();

	_file.skip(bytes);
}

uint32_t DiskFile::read(void* buffer, uint32_t size)
{
	check_valid();

	if (!_last_was_read)
	{
		_last_was_read = true;
		_file.seek(0);
	}

	return _file.read(buffer, size);
}

uint32_t DiskFile::write(const void* buffer, uint32_t size)
{
	check_valid();

	if (_last_was_read)
	{
		_last_was_read = false;
		_file.seek(0);
	}

	return _file.write(buffer, size);
}

bool DiskFile::copy_to(File& file, uint32_t size)
{
	check_valid();

	const uint32_t chunksize = 1024*1024;

	char* buff = (char*) default_allocator().allocate(chunksize * sizeof(char));

	uint32_t tot_read_bytes = 0;

	while (tot_read_bytes < size)
	{
		uint32_t expected_read_bytes = std::min(size - tot_read_bytes, chunksize);
		uint32_t read_bytes = _file.read(buff, expected_read_bytes);

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

uint32_t DiskFile::position()
{
	check_valid();

	return _file.position();
}

uint32_t DiskFile::size()
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
