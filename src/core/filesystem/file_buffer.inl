/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	Buffer *_buffer;
	u32 _position;

	explicit FileBuffer(Buffer &buffer)
		: _buffer(&buffer)
		, _position(0)
	{
	}

	virtual ~FileBuffer()
	{
		CE_NOOP();
	}

	virtual void open(const char *path, FileOpenMode::Enum mode) override
	{
		CE_UNUSED(path);
		CE_UNUSED(mode);
		CE_NOOP();
	}

	virtual void close() override
	{
		_buffer = NULL;
		_position = 0;
	}

	virtual bool is_open() override
	{
		return _buffer != NULL;
	}

	virtual u32 size() override
	{
		return array::size(*_buffer);
	}

	virtual u32 position() override
	{
		return _position;
	}

	virtual bool end_of_file() override
	{
		return _position == array::size(*_buffer);
	}

	virtual void seek(u32 position) override
	{
		_position = min(array::size(*_buffer), position);
	}

	virtual void seek_to_end() override
	{
		_position = array::size(*_buffer);
	}

	virtual void skip(u32 bytes) override
	{
		seek(_position + bytes);
	}

	virtual u32 read(void *data, u32 size) override
	{
		const u32 rest = array::size(*_buffer) - _position;
		const u32 num = min(size, rest);
		memcpy(data, array::begin(*_buffer) + _position, num);
		_position += num;
		return num;
	}

	virtual u32 write(const void *data, u32 size) override
	{
		_buffer->_size = _position;
		array::push(*_buffer, (char *)data, size);
		_position += size;
		return size;
	}

	virtual s32 sync() override
	{
		CE_NOOP();
		return 0;
	}
};

} // namespace crown
