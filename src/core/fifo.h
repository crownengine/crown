/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

namespace crown
{
/// Named pipe.
///
/// @ingroup Core
struct Fifo
{
	struct FifoPrivate *_priv;
	CE_ALIGN_DECL(16, u8 _data[8]);

	///
	Fifo();

	///
	~Fifo();

	///
	Fifo(const Fifo &) = delete;

	///
	Fifo &operator=(const Fifo &) = delete;

	/// Returns whether @a path names a FIFO or a Windows named pipe.
	static bool is_fifo(const char *path);

	/// Opens an existing pipe for writing. Returns 0 on success.
	s32 open(const char *path);

	bool is_open() const;
	void close();

	/// Writes at most @a size bytes. Returns bytes written, or -1 on error.
	s32 write(const void *data, u32 size);

	/// Flushes written data. On Windows, waits for the reader to consume it.
	/// Returns 0 on success; POSIX writes need no additional flush.
	s32 flush();
};

} // namespace crown
