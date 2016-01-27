/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "disk_file.h"

namespace crown
{
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

u32 DiskFile::size()
{
	return _file.size();
}

u32 DiskFile::position()
{
	return _file.position();
}

bool DiskFile::end_of_file()
{
	return position() == size();
}

void DiskFile::seek(u32 position)
{
	_file.seek(position);
}

void DiskFile::seek_to_end()
{
	_file.seek_to_end();
}

void DiskFile::skip(u32 bytes)
{
	_file.skip(bytes);
}

u32 DiskFile::read(void* data, u32 size)
{
	return _file.read(data, size);
}

u32 DiskFile::write(const void* data, u32 size)
{
	return _file.write(data, size);
}

void DiskFile::flush()
{
	_file.flush();
}

} // namespace crown
