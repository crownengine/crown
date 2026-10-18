/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/fifo.h"
#include "core/strings/string.inl"
#include <new>

#if CROWN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#elif CROWN_PLATFORM_POSIX && !CROWN_PLATFORM_EMSCRIPTEN
	#include <errno.h>
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <unistd.h>
#endif

namespace crown
{
struct FifoPrivate
{
#if CROWN_PLATFORM_WINDOWS
	HANDLE handle;
#else
	int fd;
#endif
};

Fifo::Fifo()
{
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(*_priv));
	_priv = new (_data) FifoPrivate();
#if CROWN_PLATFORM_WINDOWS
	_priv->handle = INVALID_HANDLE_VALUE;
#else
	_priv->fd = -1;
#endif
}

Fifo::~Fifo()
{
	close();
	_priv->~FifoPrivate();
}

bool Fifo::is_fifo(const char *path)
{
#if CROWN_PLATFORM_WINDOWS
	return str_has_prefix_case(path, "\\\\.\\pipe\\");
#elif CROWN_PLATFORM_POSIX && !CROWN_PLATFORM_EMSCRIPTEN
	struct stat info;
	return ::stat(path, &info) == 0 && S_ISFIFO(info.st_mode);
#else
	CE_UNUSED(path);
	return false;
#endif
}

s32 Fifo::open(const char *path)
{
	close();
#if CROWN_PLATFORM_WINDOWS
	_priv->handle = CreateFileA(path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
#elif CROWN_PLATFORM_POSIX && !CROWN_PLATFORM_EMSCRIPTEN
	_priv->fd = ::open(path, O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if (is_open()) {
		struct stat info;
		if (fstat(_priv->fd, &info) != 0 || !S_ISFIFO(info.st_mode))
			close();
	}
#else
	CE_UNUSED(path);
#endif
	return is_open() ? 0 : -1;
}

bool Fifo::is_open() const
{
#if CROWN_PLATFORM_WINDOWS
	return _priv->handle != INVALID_HANDLE_VALUE;
#else
	return _priv->fd != -1;
#endif
}

void Fifo::close()
{
	if (!is_open())
		return;
#if CROWN_PLATFORM_WINDOWS
	CloseHandle(_priv->handle);
	_priv->handle = INVALID_HANDLE_VALUE;
#elif CROWN_PLATFORM_POSIX && !CROWN_PLATFORM_EMSCRIPTEN
	::close(_priv->fd);
	_priv->fd = -1;
#endif
}

s32 Fifo::write(const void *data, u32 size)
{
	if (!is_open())
		return -1;
	size = min(size, u32(INT32_MAX));
#if CROWN_PLATFORM_WINDOWS
	DWORD bytes_wrote = 0;
	return WriteFile(_priv->handle, data, size, &bytes_wrote, NULL) ? s32(bytes_wrote) : -1;
#elif CROWN_PLATFORM_POSIX && !CROWN_PLATFORM_EMSCRIPTEN
	ssize_t bytes_wrote;
	do {
		bytes_wrote = ::write(_priv->fd, data, size);
	} while (bytes_wrote < 0 && errno == EINTR);
	return s32(bytes_wrote);
#else
	CE_UNUSED_2(data, size);
	return -1;
#endif
}

s32 Fifo::flush()
{
	if (!is_open())
		return -1;
#if CROWN_PLATFORM_WINDOWS
	return FlushFileBuffers(_priv->handle) ? 0 : -1;
#else
	return 0;
#endif
}

} // namespace crown
