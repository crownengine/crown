/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "disk_file.h"
#include "types.h"
#include "memory.h"
#include <algorithm>

namespace crown
{

DiskFile::DiskFile()
{
}

DiskFile::~DiskFile()
{
	close();
}

void DiskFile::open(const char* path, FileOpenMode::Enum mode)
{
	_file.open(path, mode);
}

void DiskFile::close()
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

uint32_t DiskFile::read(void* data, uint32_t size)
{
	check_valid();
	return _file.read(data, size);
}

uint32_t DiskFile::write(const void* data, uint32_t size)
{
	check_valid();
	return _file.write(data, size);
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
	_file.flush();
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

} // namespace crown
