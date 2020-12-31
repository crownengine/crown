/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/platform.h"

#if CROWN_PLATFORM_WINDOWS

#include "core/containers/hash_map.inl"
#include "core/filesystem/file_monitor.h"
#include "core/filesystem/path.h"
#include "core/memory/temp_allocator.inl"
#include "core/os.h"
#include "core/strings/dynamic_string.inl"
#include "core/thread/thread.h"
#include <windows.h>

namespace crown
{
struct FileMonitorImpl
{
	struct Watch
	{
		ALLOCATOR_AWARE;

		HANDLE _handle;
		DynamicString _path;
		char _buffer[4096];
		OVERLAPPED _overlapped;

		explicit Watch(Allocator& a)
			: _path(a)
		{
			memset(&_buffer, 0, sizeof(_buffer));
			memset(&_overlapped, 0, sizeof(_overlapped));
		}

		~Watch()
		{
			CloseHandle(_handle);
		}
	};

	Allocator* _allocator;
	HANDLE _iocp;
	HashMap<u32, Watch*> _watches;
	Thread _thread;
	bool _exit;
	bool _recursive;
	FileMonitorFunction _function;
	void* _user_data;
	u32 _key;

	explicit FileMonitorImpl(Allocator& a)
		: _allocator(&a)
		, _iocp(0)
		, _watches(a)
		, _exit(false)
		, _recursive(false)
		, _function(NULL)
		, _user_data(NULL)
		, _key(1000)
	{
	}

	void add_watch(const char* path, bool recursive)
	{
		CE_ENSURE(path != NULL);
		CE_ASSERT(!path::has_trailing_separator(path), "Malformed path");

		HANDLE fh = CreateFile(path
			, FILE_LIST_DIRECTORY
			, FILE_SHARE_DELETE
			| FILE_SHARE_READ
			| FILE_SHARE_WRITE
			, NULL
			, OPEN_EXISTING
			, FILE_FLAG_BACKUP_SEMANTICS // Required by directories
			| FILE_FLAG_OVERLAPPED       // Required by CreateIoCompletionPort
			, NULL
			);
		CE_ASSERT(fh != INVALID_HANDLE_VALUE, "CreateFile: GetLastError: %d", GetLastError());

		// Create new IOCP or return already created one
		_iocp = CreateIoCompletionPort(fh, _iocp, _key, 0);
		CE_ASSERT(_iocp != NULL, "CreateIoCompletionPort: GetLastError: %d", GetLastError());

		Watch* wh = CE_NEW(*_allocator, Watch)(*_allocator);
		wh->_path = path;
		wh->_handle = fh;

		BOOL rdc = ReadDirectoryChangesW(fh
			, &wh->_buffer
			, sizeof(wh->_buffer)
			, _recursive
			, FILE_NOTIFY_CHANGE_FILE_NAME
			| FILE_NOTIFY_CHANGE_DIR_NAME
			| FILE_NOTIFY_CHANGE_ATTRIBUTES
			| FILE_NOTIFY_CHANGE_SIZE
			, NULL
			, &wh->_overlapped
			, NULL
			);
		CE_ASSERT(rdc != 0, "ReadDirectoryChangesW: GetLastError: %d", GetLastError());

		hash_map::set(_watches, _key, wh);
		++_key;
	}

	void scan_subdirectories(const char* path)
	{
		TempAllocator256 dir_ta;
		DynamicString dir(dir_ta);
		dir += path;
		dir += "\\*";

		WIN32_FIND_DATA ffd;
		HANDLE file = FindFirstFile(dir.c_str(), &ffd);
		if (file != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (!strcmp(ffd.cFileName, ".") || !strcmp(ffd.cFileName, ".."))
					continue;

				TempAllocator512 ta;
				DynamicString str(ta);
				path::join(str, path, ffd.cFileName);

				_function(_user_data
					, FileMonitorEvent::CREATED
					, ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
					, str.c_str()
					, NULL
					);

				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					scan_subdirectories(str.c_str());
			}
			while (FindNextFile(file, &ffd) != 0);

			FindClose(file);
		}
	}

	void start(u32 num, const char** paths, bool recursive, FileMonitorFunction fmf, void* user_data)
	{
		CE_ENSURE(NULL != fmf);

		_recursive = recursive;
		_function = fmf;
		_user_data = user_data;

		for (u32 i = 0; i < num; ++i)
			add_watch(paths[i], recursive);

		_thread.start([](void* thiz) { return static_cast<FileMonitorImpl*>(thiz)->watch(); }, this);
	}

	void stop()
	{
		_exit = true;
		// Post dummy packet to force thread to evaluate _exit
		PostQueuedCompletionStatus(_iocp, 0, 999, 0);
		_thread.stop();

		// Release all handles
		auto cur = hash_map::begin(_watches);
		auto end = hash_map::end(_watches);
		for (; cur != end; ++cur)
		{
			HASH_MAP_SKIP_HOLE(_watches, cur);

			CE_DELETE(*_allocator, cur->second);
		}

		CloseHandle(_iocp);
	}

	int watch()
	{
		while (!_exit)
		{
			DWORD bytes_transferred;
			ULONG_PTR key;
			OVERLAPPED* ov;
			BOOL ret = GetQueuedCompletionStatus(_iocp
				, &bytes_transferred
				, &key
				, &ov
				, INFINITE
				);
			if (ret == FALSE)
				continue;

			// Re-evaluate _exit
			if (bytes_transferred == 0)
				continue;

			Watch* wh = hash_map::get(_watches, (u32)(uintptr_t)key, (Watch*)NULL);

			// Read packets
			DWORD last_action = -1;
			DynamicString path_old_name(default_allocator());
			char* cur = (char*)wh->_buffer;
			for(;;)
			{
				const FILE_NOTIFY_INFORMATION* fni = (const FILE_NOTIFY_INFORMATION*)cur;

				TempAllocator512 ta;
				DynamicString path(ta);
				full_path(path, (u32)(uintptr_t)key, fni->FileName, fni->FileNameLength);

				if (fni->Action == FILE_ACTION_ADDED)
				{
					Stat st;
					os::stat(st, path.c_str());

					_function(_user_data
						, FileMonitorEvent::CREATED
						, st.file_type == Stat::FileType::DIRECTORY
						, path.c_str()
						, NULL
					);

					if (st.file_type == Stat::FileType::DIRECTORY)
						scan_subdirectories(path.c_str());
				}
				else if (fni->Action == FILE_ACTION_REMOVED)
				{
					_function(_user_data
						, FileMonitorEvent::DELETED
						, false // FIXME: add "unknown" type or always assume file and let client handle that?
						, path.c_str()
						, NULL
					);
				}
				else if (fni->Action == FILE_ACTION_MODIFIED)
				{
					Stat st;
					os::stat(st, path.c_str());

					_function(_user_data
						, FileMonitorEvent::CHANGED
						, st.file_type == Stat::FileType::DIRECTORY
						, path.c_str()
						, NULL
						);
				}
				else if (fni->Action == FILE_ACTION_RENAMED_OLD_NAME)
				{
					last_action = fni->Action;
					full_path(path_old_name, (u32)(uintptr_t)key, fni->FileName, fni->FileNameLength);
				}
				else if (fni->Action == FILE_ACTION_RENAMED_NEW_NAME)
				{
					if (last_action == FILE_ACTION_RENAMED_OLD_NAME)
					{
						last_action = -1;

						Stat st;
						os::stat(st, path.c_str());

						_function(_user_data
							, FileMonitorEvent::RENAMED
							, st.file_type == Stat::FileType::DIRECTORY
							, path_old_name.c_str()
							, path.c_str()
							);

						if (st.file_type == Stat::FileType::DIRECTORY)
							scan_subdirectories(path.c_str());
					}
				}

				// advance to next entry in buffer (variable length)
				if (fni->NextEntryOffset == 0)
					break;
				cur += fni->NextEntryOffset;
			}

			BOOL rdc = ReadDirectoryChangesW(wh->_handle
				, &wh->_buffer
				, sizeof(wh->_buffer)
				, _recursive
				, FILE_NOTIFY_CHANGE_FILE_NAME
				| FILE_NOTIFY_CHANGE_DIR_NAME
				| FILE_NOTIFY_CHANGE_ATTRIBUTES
				| FILE_NOTIFY_CHANGE_SIZE
				, NULL
				, &wh->_overlapped
				, NULL
				);
			CE_ASSERT(rdc != 0, "ReadDirectoryChangesW: GetLastError: %d", GetLastError());
		}

		return 0;
	}

	void full_path(DynamicString& path, u32 key, const WCHAR* name, u32 name_len)
	{
		Watch* wh = hash_map::get(_watches, key, (Watch*)NULL);

		TempAllocator512 ta;
		DynamicString path_base(ta);
		path_base = wh->_path;
		DynamicString filename(ta);
		for (u32 ii = 0; ii < name_len/2; ++ii)
			filename += (char)name[ii];
		path::join(path, path_base.c_str(), filename.c_str());
	}

};

FileMonitor::FileMonitor(Allocator& a)
{
	_impl = CE_NEW(a, FileMonitorImpl)(a);
}

FileMonitor::~FileMonitor()
{
	CE_DELETE(*_impl->_allocator, _impl);
}

void FileMonitor::start(u32 num, const char** paths, bool recursive, FileMonitorFunction fmf, void* user_data)
{
	_impl->start(num, paths, recursive, fmf, user_data);
}

void FileMonitor::stop()
{
	_impl->stop();
}

} // namespace crown

#endif // CROWN_PLATFORM_WINDOWS
