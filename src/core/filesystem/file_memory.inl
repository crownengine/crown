/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/file.h"
#include <string.h> // memcpy

namespace crown
{
/// A struct to access read-only memory as a File.
///
/// @ingroup Filesystem
struct FileMemory : public File
{
	const u8 *_memory;
	u32 _size;
	u32 _position;

	explicit FileMemory(const void *mem, u32 size)
		: _memory((u8 *)mem)
		, _size(size)
		, _position(0)
	{
	}

	virtual ~FileMemory()
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
		_memory = NULL;
		_size = 0;
		_position = 0;
	}

	virtual bool is_open() override
	{
		return _memory != NULL;
	}

	virtual u32 size() override
	{
		return _size;
	}

	virtual u32 position() override
	{
		return _position;
	}

	virtual bool end_of_file() override
	{
		return _position == _size;
	}

	virtual void seek(u32 position) override
	{
		_position = position;
	}

	virtual void seek_to_end() override
	{
		_position = _size;
	}

	virtual void skip(u32 bytes) override
	{
		seek(_position + bytes);
	}

	virtual u32 read(void *data, u32 size) override
	{
		const u32 rest = _size - _position;
		const u32 num = min(size, rest);
		memcpy(data, _memory + _position, num);
		_position += num;
		return num;
	}

	virtual u32 write(const void *data, u32 size) override
	{
		CE_UNUSED_2(data, size);
		CE_NOOP();
		return 0;
	}

	virtual s32 sync() override
	{
		CE_NOOP();
		return 0;
	}
};

} // namespace crown
