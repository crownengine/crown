/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/array.inl"
#include "core/filesystem/file.h"

namespace crown
{
/// File interface for Buffer.
///
/// @ingroup Filesystem
struct FileBuffer : public File
{
	Buffer* _buffer;
	u32 _position;

	explicit FileBuffer(Buffer& buffer)
		: _buffer(&buffer)
		, _position(0)
	{
	}

	virtual ~FileBuffer()
	{
		CE_NOOP();
	}

	virtual void open(const char* path, FileOpenMode::Enum mode)
	{
		CE_UNUSED(path);
		CE_UNUSED(mode);
		CE_NOOP();
	}

	virtual void close()
	{
		_buffer = NULL;
		_position = 0;
	}

	virtual bool is_open()
	{
		return _buffer != NULL;
	}

	virtual u32 size()
	{
		return array::size(*_buffer);
	}

	virtual u32 position()
	{
		return _position;
	}

	virtual bool end_of_file()
	{
		return _position == array::size(*_buffer);
	}

	virtual void seek(u32 position)
	{
		_position = min(array::size(*_buffer), position);
	}

	virtual void seek_to_end()
	{
		_position = array::size(*_buffer);
	}

	virtual void skip(u32 bytes)
	{
		seek(_position + bytes);
	}

	virtual u32 read(void* data, u32 size)
	{
		const u32 rest = array::size(*_buffer) - _position;
		const u32 num = min(size, rest);
		memcpy(data, array::begin(*_buffer) + _position, num);
		_position += num;
		return num;
	}

	virtual u32 write(const void* data, u32 size)
	{
		_buffer->_size = _position;
		array::push(*_buffer, (const char*)data, size);
		_position += size;
		return size;
	}

	virtual void flush()
	{
		CE_NOOP();
	}
};

} // namespace crown
