/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/filesystem/file.h"
#include "core/filesystem/filesystem_http.h"
#include "core/filesystem/http_file.h"
#include "core/memory/temp_allocator.inl"
#include "core/network/http.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_view.inl"
#include "device/log.h"
#include <stdio.h>
#include <string.h>

LOG_SYSTEM(FILESYSTEM_HTTP, "filesystem_http")

namespace crown
{
struct FileHttp : public File
{
	Allocator *_allocator;
	DynamicString _url;
	DynamicString _download;
	HttpReadFile _file;
	u8 *_cache;
	u32 _cache_offset;
	u32 _cache_size;
	u32 _size;
	u32 _position;
	bool _open;

	explicit FileHttp(Allocator &a)
		: _allocator(&a)
		, _url(a)
		, _download(a)
		, _file()
		, _cache(NULL)
		, _cache_offset(0)
		, _cache_size(0)
		, _size(0)
		, _position(0)
		, _open(false)
	{
	}

	~FileHttp()
	{
		close();
	}

	bool fetch(void *data, u32 offset, u32 size)
	{
		if (_download.empty()) {
			const u32 id = http::get_range(_url.c_str(), offset, size, 30*1000);
			while (http::status(id) == HttpStatus::DOWNLOADING)
				os::sleep(1);

			u32 code, num_headers;
			StringView reason, body;
			const HttpHeader *headers;
			bool success = false;
			if (http::response(code, reason, headers, num_headers, body, id)) {
				char content_range[96] = {};
				for (u32 ii = 0; ii < num_headers; ++ii) {
					if (headers[ii].name.length() == 13
						&& crown::strncasecmp(headers[ii].name.data(), "Content-Range", 13) == 0
						&& headers[ii].value.length() < sizeof(content_range)) {
						memcpy(content_range, headers[ii].value.data(), headers[ii].value.length());
						content_range[headers[ii].value.length()] = '\0';
					}
				}

				unsigned long long first, last, total;
				if (!_open && code == 416 && strcmp(content_range, "bytes */0") == 0) {
					success = true;
				} else if (code == 206
					&& sscanf(content_range, "bytes %llu-%llu/%llu", &first, &last, &total) == 3
					&& total > 0 && total <= UINT32_MAX
					&& first == offset && last == min((u64)offset + size, (u64)total) - 1
					&& last >= first && body.length() == last - first + 1
					&& (!_open || total == _size)) {
					if (data != NULL)
						memcpy(data, body.data(), body.length());
					_size = (u32)total;
					success = true;
				}
			}
			http::close(id);
			if (success)
				return true;

			if (!http_file::temporary_path(_download))
				return false;
			const u32 download = http::get_to_file(_url.c_str(), _download.c_str(), 30*1000);
			while (http::status(download) == HttpStatus::DOWNLOADING)
				os::sleep(1);
			const s64 downloaded = http::status(download) == HttpStatus::FINISHED
				? http_file::size(_download.c_str()) : -1;
			success = downloaded >= 0 && (!_open || downloaded == _size);
			http::close(download);
			if (!success) {
				loge(FILESYSTEM_HTTP, "Cannot download '%s'", _url.c_str());
				http_file::remove(_download.c_str(), _file);
				_download = "";
				return false;
			}
			_size = (u32)downloaded;
		}
		size = min(size, _size - offset);
		return data == NULL || http_file::read(data, _file, _download.c_str(), offset, size) == size;
	}

	void open(const char *path, FileOpenMode::Enum mode) override
	{
		close();
		if (mode != FileOpenMode::READ)
			return;
		_url = path;
		_open = fetch(NULL, 0, 1);
	}

	void close() override
	{
		if (!_download.empty()) {
			http_file::remove(_download.c_str(), _file);
			_download = "";
		}
		_allocator->deallocate(_cache);
		_cache = NULL;
		_cache_offset = _cache_size = _size = _position = 0;
		_open = false;
	}

	bool is_open() override
	{
		return _open;
	}

	u32 size() override
	{
		return _size;
	}

	u32 position() override
	{
		return _position;
	}

	bool end_of_file() override
	{
		return _position == _size;
	}

	void seek(u32 position) override
	{
		_position = min(position, _size);
	}

	void seek_to_end() override
	{
		_position = _size;
	}

	void skip(u32 bytes) override
	{
		_position += min(bytes, _size - _position);
	}

	u32 read(void *data, u32 size) override
	{
		u32 read = 0;
		size = min(size, _size - _position);
		while (read < size) {
			if (size - read >= 64*1024) {
				if (!fetch((u8 *)data + read, _position, 64*1024))
					break;
				_position += 64*1024;
				read += 64*1024;
				continue;
			}
			u32 offset = _position - _cache_offset;
			if (offset >= _cache_size) {
				if (_cache == NULL)
					_cache = (u8 *)_allocator->allocate(min(64*1024u, _size));
				const u32 count = min(64*1024u, _size - _position);
				if (!fetch(_cache, _position, count))
					break;
				_cache_offset = _position;
				_cache_size = count;
				offset = 0;
			}
			const u32 num = min(size - read, _cache_size - offset);
			memcpy((u8 *)data + read, _cache + offset, num);
			_position += num;
			read += num;
		}
		return read;
	}

	u32 write(const void *data, u32 size) override
	{
		CE_UNUSED_2(data, size);
		return 0;
	}

	s32 sync() override
	{
		return 0;
	}
};

FilesystemHttp::FilesystemHttp(Allocator &a, const char *url)
	: _allocator(&a)
	, _prefix(a)
{
	CE_ENSURE(url != NULL);
	_prefix = url;
	if (!_prefix.empty() && _prefix.c_str()[_prefix.length() - 1] != '/')
		_prefix += '/';
}

File *FilesystemHttp::open(const char *path, FileOpenMode::Enum mode)
{
	TempAllocator1024 ta;
	DynamicString url(ta);
	absolute_path(url, path);
	FileHttp *file = CE_NEW(*_allocator, FileHttp)(*_allocator);
	file->open(url.c_str(), mode);
	return file;
}

void FilesystemHttp::close(File &file)
{
	CE_DELETE(*_allocator, &file);
}

Stat FilesystemHttp::stat(const char *path)
{
	File *file = open(path, FileOpenMode::READ);
	Stat st;
	st.file_type = file->is_open() ? Stat::REGULAR : Stat::NO_ENTRY;
	st.size = file->is_open() ? file->size() : 0;
	st.mtime = 0;
	close(*file);
	return st;
}

bool FilesystemHttp::exists(const char *path)
{
	return stat(path).file_type != Stat::NO_ENTRY;
}

bool FilesystemHttp::is_directory(const char *path)
{
	CE_UNUSED(path);
	return false;
}

bool FilesystemHttp::is_file(const char *path)
{
	return stat(path).file_type == Stat::REGULAR;
}

u64 FilesystemHttp::last_modified_time(const char *path)
{
	CE_UNUSED(path);
	return 0;
}

CreateResult FilesystemHttp::create_directory(const char *path)
{
	CE_UNUSED(path);
	CreateResult result;
	result.error = CreateResult::UNKNOWN;
	return result;
}

DeleteResult FilesystemHttp::delete_directory(const char *path)
{
	CE_UNUSED(path);
	DeleteResult result;
	result.error = DeleteResult::UNKNOWN;
	return result;
}

DeleteResult FilesystemHttp::delete_file(const char *path)
{
	CE_UNUSED(path);
	DeleteResult result;
	result.error = DeleteResult::UNKNOWN;
	return result;
}

RenameResult FilesystemHttp::rename(const char *old_path, const char *new_path)
{
	CE_UNUSED_2(old_path, new_path);
	RenameResult result;
	result.error = RenameResult::UNKNOWN;
	return result;
}

void FilesystemHttp::list_files(const char *path, Vector<DynamicString> &files)
{
	CE_UNUSED_2(path, files);
}

void FilesystemHttp::absolute_path(DynamicString &os_path, const char *path)
{
	os_path = _prefix;
	for (const char *p = path; *p != '\0'; ++p)
		os_path += *p == '\\' ? '/' : *p;
}

} // namespace crown
