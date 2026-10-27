/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/strings/types.h"
#include "core/types.h"

namespace crown
{
struct HttpDownload
{
	File *file;
	Allocator *allocator;
	const char *path;
	u64 size;
	bool complete;
};

struct HttpReadFile
{
	File *file;
	u32 position;
};

namespace http_file
{
	bool open(HttpDownload &file, Allocator &a, const char *path);
	bool write(HttpDownload &file, const void *data, u32 size);
	bool finish(HttpDownload &file);
	void close(HttpDownload &file);

	bool temporary_path(DynamicString &path);
	s64 size(const char *path);
	u32 read(void *data, HttpReadFile &file, const char *path, u32 offset, u32 size);
	void remove(const char *path, HttpReadFile &file);

} // namespace http_file

} // namespace crown
