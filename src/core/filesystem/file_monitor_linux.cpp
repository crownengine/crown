/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/platform.h"

#if CROWN_PLATFORM_LINUX

#include "core/containers/hash_map.inl"
#include "core/filesystem/file_monitor.h"
#include "core/filesystem/path.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/thread/thread.h"
#include <dirent.h> // opendir, readdir
#include <errno.h>
#include <limits.h> // NAME_MAX
#include <sys/inotify.h>
#include <sys/select.h>
#include <unistd.h> // read

namespace crown
{
struct FileMonitorImpl
{
	Allocator* _allocator;
	int _fd;
	HashMap<int, DynamicString> _watches;
	HashMap<DynamicString, int> _watches_reverse;
	Thread _thread;
	bool _exit;
	bool _recursive;
	FileMonitorFunction _function;
	void* _user_data;

	explicit FileMonitorImpl(Allocator& a)
		: _allocator(&a)
		, _fd(0)
		, _watches(a)
		, _watches_reverse(a)
		, _exit(false)
		, _recursive(false)
		, _function(NULL)
		, _user_data(NULL)
	{
	}

	void add_watch(const char* path, bool recursive, bool generate_create_event = false)
	{
		CE_ENSURE(path != NULL);
		CE_ASSERT(!path::has_trailing_separator(path), "Malformed path");

		int wd = inotify_add_watch(_fd
			, path
			, IN_CREATE
			| IN_DELETE      // File/directory deleted from watched directory.
			| IN_MODIFY
			| IN_ATTRIB
			| IN_MOVE
			| IN_DELETE_SELF // Watched file/directory was itself deleted.
			| IN_DONT_FOLLOW // Don't dereference pathname if it is a symbolic link.
			| IN_ONLYDIR     // Only watch pathname if it is a directory.
			);
		CE_ASSERT(wd != -1, "inotify_add_watch: errno: %d", errno);

		TempAllocator512 ta;
		DynamicString str(ta);
		str = path;
		hash_map::set(_watches, wd, str);
		hash_map::set(_watches_reverse, str, wd);

		// Add all sub-dirs recursively
		if (recursive)
			add_subdirectories(path, generate_create_event);
	}

	void add_subdirectories(const char* path, bool generate_create_event)
	{
		struct dirent *entry;

		DIR *dir = opendir(path);
		if (dir != NULL)
		{
			while ((entry = readdir(dir)))
			{
				const char* dname = entry->d_name;

				if (!strcmp(dname, ".") || !strcmp(dname, ".."))
					continue;

				TempAllocator512 ta;
				DynamicString str(ta);
				path::join(str, path, dname);

				if (generate_create_event)
				{
					_function(_user_data
						, FileMonitorEvent::CREATED
						, entry->d_type == DT_DIR // FIXME: some filesystems do not support DT_DIR.
						, str.c_str()
						, NULL
						);
				}

				if (entry->d_type == DT_DIR)
					add_watch(str.c_str(), _recursive, generate_create_event);
			}

			closedir(dir);
		}
	}

	void start(u32 num, const char** paths, bool recursive, FileMonitorFunction fmf, void* user_data)
	{
		CE_ENSURE(NULL != fmf);

		_recursive = recursive;
		_function = fmf;
		_user_data = user_data;

		_fd = inotify_init();
		CE_ASSERT(_fd != -1, "inotify_init: errno: %d", errno);

		for (u32 i = 0; i < num; ++i)
			add_watch(paths[i], recursive);

		_thread.start([](void* thiz) { return static_cast<FileMonitorImpl*>(thiz)->watch(); }, this);
	}

	void stop()
	{
		_exit = true;
		_thread.stop();

		close(_fd);
	}

	int watch()
	{
		while (!_exit)
		{
			TempAllocator512 ta;

			fd_set set;
			struct timeval timeout;
			u32 cookie = 0;
			u32 cookie_mask = 0;
			DynamicString cookie_path(ta);

			FD_ZERO(&set);
			FD_SET(_fd, &set);

			timeout.tv_sec = 0;
			timeout.tv_usec = 32*1000;

			// select returns 0 if timeout, 1 if input available, -1 if error.
			if (select(FD_SETSIZE, &set, NULL, NULL, &timeout) == 0)
				continue;

#define BUF_LEN sizeof(struct inotify_event) + NAME_MAX + 1
			char buf[BUF_LEN*16];
#undef BUF_LEN
			ssize_t len = read(_fd, buf, sizeof(buf));
			if (len == 0)
				return 0;
			if (len == -1)
				return -1;

			for (char* p = buf; p < buf + len;)
			{
				inotify_event* ev = (inotify_event*)p;

				if (ev->mask & IN_IGNORED)
				{
					// Watch was removed explicitly (inotify_rm_watch(2)) or
					// automatically (file was deleted, or filesystem was
					// unmounted).
					hash_map::remove(_watches, ev->wd);
				}
				if (ev->mask & IN_CREATE)
				{
					DynamicString path(ta);
					full_path(path, ev->wd, ev->name);

					_function(_user_data
						, FileMonitorEvent::CREATED
						, ev->mask & IN_ISDIR
						, path.c_str()
						, NULL
						);

					// From INOTIFY(7), Limitations and caveats:
					// If monitoring an entire directory subtree, and a new subdirectory
					// is created in that tree or an existing directory is renamed into
					// that tree, be aware that by the time you create a watch for the
					// new subdirectory, new files (and subdirectories) may already exist
					// inside the subdirectory.  Therefore, you may want to scan the
					// contents of the subdirectory immediately after adding the watch
					// (and, if desired, recursively add watches for any subdirectories
					// that it contains).
					if (ev->mask & IN_ISDIR)
						add_watch(path.c_str(), _recursive, true);
				}
				if (ev->mask & IN_DELETE)
				{
					DynamicString path(ta);
					full_path(path, ev->wd, ev->name);

					_function(_user_data
						, FileMonitorEvent::DELETED
						, ev->mask & IN_ISDIR
						, path.c_str()
						, NULL
						);
				}
				if (ev->mask & IN_MODIFY || ev->mask & IN_ATTRIB)
				{
					DynamicString path(ta);
					full_path(path, ev->wd, ev->name);

					_function(_user_data
						, FileMonitorEvent::CHANGED
						, ev->mask & IN_ISDIR
						, path.c_str()
						, NULL
						);
				}
				if (ev->mask & IN_MOVED_FROM)
				{
					// Two consecutive IN_MOVED_FROM
					if (cookie != 0)
					{
						_function(_user_data
							, FileMonitorEvent::DELETED
							, cookie_mask & IN_ISDIR
							, cookie_path.c_str()
							, NULL
							);

						u32 wd = hash_map::get(_watches_reverse, cookie_path, INT32_MAX);
						hash_map::remove(_watches_reverse, cookie_path);
						inotify_rm_watch(_fd, wd);

						cookie = 0;
						cookie_mask = 0;
					}
					else
					{
						DynamicString path(ta);
						full_path(path, ev->wd, ev->name);

						cookie = ev->cookie;
						cookie_mask = ev->mask;
						cookie_path = path;
					}
				}
				if (ev->mask & IN_MOVED_TO)
				{
					if (cookie == ev->cookie)
					{
						// File or directory has been renamed
						DynamicString path(ta);
						full_path(path, ev->wd, ev->name);

						_function(_user_data
							, FileMonitorEvent::RENAMED
							, ev->mask & IN_ISDIR
							, cookie_path.c_str()
							, path.c_str()
							);

						cookie = 0;
						cookie_mask = 0;

						if (ev->mask & IN_ISDIR)
						{
							u32 wd = hash_map::get(_watches_reverse, cookie_path, INT32_MAX);
							hash_map::remove(_watches_reverse, cookie_path);
							inotify_rm_watch(_fd, wd);

							add_watch(path.c_str(), _recursive, true);
						}
					}
					else
					{
						// File or directory was moved to this folder
						DynamicString path(ta);
						full_path(path, ev->wd, ev->name);

						_function(_user_data
							, FileMonitorEvent::CREATED
							, ev->mask & IN_ISDIR
							, path.c_str()
							, NULL
							);

						cookie = 0;
						cookie_mask = 0;

						if (ev->mask & IN_ISDIR)
							add_watch(path.c_str(), _recursive, true);
					}
				}

				p += sizeof(inotify_event) + ev->len;
			}

			// Unpaired IN_MOVE_TO
			if (cookie != 0)
			{
				_function(_user_data
					, FileMonitorEvent::DELETED
					, cookie_mask & IN_ISDIR
					, cookie_path.c_str()
					, NULL
					);

				u32 wd = hash_map::get(_watches_reverse, cookie_path, INT32_MAX);
				hash_map::remove(_watches_reverse, cookie_path);
				inotify_rm_watch(_fd, wd);

				cookie = 0;
				cookie_mask = 0;
			}
		}

		return 0;
	}

	void full_path(DynamicString& path, int wd, const char* name)
	{
		TempAllocator512 ta;
		DynamicString path_base(ta);
		path_base = hash_map::get(_watches, wd, path_base);
		path::join(path, path_base.c_str(), name);
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

#endif // CROWN_PLATFORM_LINUX
