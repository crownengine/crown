/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/path.h"
#include "core/memory/globals.h"
#include "core/memory/memory.inl"
#include "core/os.h"
#include "core/platform.h"
#include "core/strings/dynamic_string.inl"
#include "core/thread/condition_variable.h"
#include "core/thread/mutex.h"
#include "core/thread/spsc_queue.inl"
#include "core/thread/thread.h"
#include "device/log.h"
#include "device/save_game.h"
#include <atomic>
#include <string.h>
#if CROWN_PLATFORM_EMSCRIPTEN
	#include <emscripten/emscripten.h>
#endif

#define CROWN_SAVE_GAME_SIMULATE_SLOW_IO 0
#define CROWN_SAVE_GAME_SIMULATE_SLOW_IO_MS 150
#define CROWN_SAVE_GAME_IO_CHUNK_SIZE (64*1024)

LOG_SYSTEM(SAVE_GAME, "save_game")

namespace crown
{
static SaveGame *s_save_game;

#if CROWN_PLATFORM_EMSCRIPTEN
static void html5_save_game_init(const char *save_dir)
{
	// code-format off
	MAIN_THREAD_EM_ASM({
			var saveDir = UTF8ToString($0);
			if (saveDir.charAt(0) != "/")
				saveDir = "/" + saveDir;

			Module.CrownSaveGame = Module.CrownSaveGame || {};
			var state = Module.CrownSaveGame;

			if (state.mountDir == saveDir && state.ready)
				return;

			state.mountDir = saveDir;
			state.ready = 0;
			state.error = 0;
			state.syncing = 0;
			state.syncError = 0;

			function mkdirTree(dir)
			{
				var path = "";
				var parts = dir.split("/");
				for (var ii = 0; ii < parts.length; ++ii) {
					if (!parts[ii])
						continue;
					path += "/" + parts[ii];
					try {
						FS.mkdir(path);
					} catch (err) {
					}
				}
			}

			try {
				mkdirTree(saveDir);
				FS.mount(IDBFS, {}, saveDir);
				FS.syncfs(true, function(err)
				{
					state.error = err ? 1 : 0;
					state.ready = 1;
				});
			} catch (err) {
				console.error(err);
				state.error = 1;
				state.ready = 1;
			}
		}
		, save_dir
		);
	// code-format on
}

static bool html5_save_game_ready()
{
	// code-format off
	return MAIN_THREAD_EM_ASM_INT({
			return Module.CrownSaveGame && Module.CrownSaveGame.ready ? 1 : 0;
		}) != 0;
	// code-format on
}

static bool html5_save_game_ok()
{
	// code-format off
	return MAIN_THREAD_EM_ASM_INT({
			return Module.CrownSaveGame && !Module.CrownSaveGame.error ? 1 : 0;
		}) != 0;
	// code-format on
}

static void html5_save_game_flush()
{
	// code-format off
	MAIN_THREAD_EM_ASM({
			var state = Module.CrownSaveGame || (Module.CrownSaveGame = {});
			state.syncing = 1;
			state.syncError = 0;

			try {
				FS.syncfs(false, function(err)
				{
					state.syncError = err ? 1 : 0;
					state.syncing = 0;
				});
			} catch (err) {
				console.error(err);
				state.syncError = 1;
				state.syncing = 0;
			}
		});
	// code-format on
}

static bool html5_save_game_syncing()
{
	// code-format off
	return MAIN_THREAD_EM_ASM_INT({
			return Module.CrownSaveGame && Module.CrownSaveGame.syncing ? 1 : 0;
		}) != 0;
	// code-format on
}

static bool html5_save_game_flush_ok()
{
	// code-format off
	return MAIN_THREAD_EM_ASM_INT({
			var s = Module.CrownSaveGame;
			return s && !s.error && !s.syncError ? 1 : 0;
		}) != 0;
	// code-format on
}
#endif // if CROWN_PLATFORM_EMSCRIPTEN

/// Asynchronous save game service.
///
/// @ingroup Device
struct SaveGame
{
	struct Request
	{
		enum Type
		{
			LOAD,
			SAVE
		};

		std::atomic_bool done;
		std::atomic_int error;
		Type type;
		u32 token;
		void *data;
		u32 size;
		std::atomic<f32> progress;
		char basename[256];

		Request()
			: done(true)
			, error(SaveError::SUCCESS)
			, type(LOAD)
			, token(0)
			, data(NULL)
			, size(0)
			, progress(0)
		{
			basename[0] = '\0';
		}

		void set_progress(u32 processed, u32 total)
		{
			if (total == 0) {
				progress.store(1.0f);
				return;
			}

			progress.store(f32(processed) / f32(total));
		}
	};

	Allocator *_allocator;
	FilesystemDisk _filesystem;
	SPSCQueue<u32, 64> _requests_queue;
	Thread _thread;
	Mutex _mutex;
	ConditionVariable _requests_condition;
	Request _requests[64];
	u32 _next_token;
	std::atomic_bool _exit;
	bool _has_save_dir;

	///
	SaveGame(Allocator &a, const char *save_dir)
		: _allocator(&a)
		, _filesystem(a)
		, _requests_queue(a)
		, _next_token(0)
		, _exit(false)
		, _has_save_dir(save_dir != NULL && save_dir[0] != '\0')
	{
		CE_ENSURE(s_save_game == NULL);

		if (_has_save_dir) {
			_filesystem.set_prefix(save_dir);
			CreateResult cr = _filesystem.create_directory("");
			if (cr.error == CreateResult::UNKNOWN)
				logw(SAVE_GAME, "Unable to create save directory '%s'", save_dir);
		}

#if CROWN_PLATFORM_EMSCRIPTEN
		if (_has_save_dir)
			html5_save_game_init(save_dir);
#endif

		_thread.start([](void *thiz) { return ((SaveGame *)thiz)->run(); }, this);
	}

	///
	~SaveGame()
	{
		CE_ENSURE(s_save_game == this);

		_mutex.lock();
		_exit.store(true);
		_requests_condition.signal();
		_mutex.unlock();
		_thread.stop();

		for (u32 ii = 0; ii < countof(_requests); ++ii) {
			Request &rr = _requests[ii];
			_allocator->deallocate(rr.data);
			rr.data = NULL;
			rr.size = 0;
		}
	}

	///
	SaveGame(const SaveGame &) = delete;

	///
	SaveGame &operator=(const SaveGame &) = delete;

	SaveGame::Request *find_request(u32 save_request)
	{
		if (save_request == 0)
			return NULL;

		for (u32 ii = 0; ii < countof(_requests); ++ii) {
			if (_requests[ii].token == save_request)
				return &_requests[ii];
		}

		return NULL;
	}

	u32 queue_request(SaveGame::Request::Type type
		, const char *filename
		, const void *data
		, u32 size
		)
	{
		CE_ENSURE(type == SaveGame::Request::LOAD || data != NULL || size == 0);

		for (u32 ii = 0; ii < countof(_requests); ++ii) {
			SaveGame::Request &rr = _requests[ii];
			if (rr.token == 0) {
				rr.type = type;
				rr.token = ++_next_token;
				if (rr.token == 0)
					rr.token = ++_next_token;
				rr.done.store(true);
				rr.error.store(SaveError::SUCCESS);
				rr.progress.store(0.0f);
				rr.basename[0] = '\0';
				_allocator->deallocate(rr.data);
				rr.data = NULL;
				rr.size = 0;

				if (filename == NULL
					|| !path::is_valid_basename(filename)
					|| strlen32(filename) >= sizeof(SaveGame::Request::basename)
					) {
					loge(SAVE_GAME, "Invalid savegame filename: '%s'", filename ? filename : "<null>");
					rr.progress.store(1.0f);
					rr.error.store(SaveError::INVALID_FILENAME);
					return rr.token;
				}

				strcpy(rr.basename, filename);

				if (!_has_save_dir) {
					rr.progress.store(1.0f);
					rr.error.store(SaveError::SAVE_DIR_UNSET);
					return rr.token;
				}

				if (type == SaveGame::Request::SAVE && data == NULL && size > 0) {
					loge(SAVE_GAME, "Invalid savegame data");
					rr.progress.store(1.0f);
					rr.error.store(SaveError::UNKNOWN);
					return rr.token;
				}

				if (size > 0) {
					rr.data = _allocator->allocate(size);
					memcpy(rr.data, data, size);
					rr.size = size;
				}

				rr.done.store(false);

				_mutex.lock();
				const bool queued = _requests_queue.push(ii);
				if (queued)
					_requests_condition.signal();
				_mutex.unlock();

				if (!queued) {
					rr.progress.store(1.0f);
					rr.error.store(SaveError::UNKNOWN);
					rr.done.store(true);
					return rr.token;
				}

				return rr.token;
			}
		}

		return 0;
	}

	enum IODir { IO_READ, IO_WRITE };

	s32 do_chunked_io(File &file, void *buf, u32 size, IODir dir, Request &rr)
	{
		u32 bytes_done = 0;
		const u32 chunk = u32(CROWN_SAVE_GAME_IO_CHUNK_SIZE);
		while (bytes_done < size) {
			const u32 remaining = size - bytes_done;
			const u32 n_req = remaining < chunk ? remaining : chunk;
#if CROWN_SAVE_GAME_SIMULATE_SLOW_IO
			os::sleep(CROWN_SAVE_GAME_SIMULATE_SLOW_IO_MS);
#endif
			const u32 n = dir == IO_READ
				? file.read((char *)buf + bytes_done, n_req)
				: file.write((const char *)buf + bytes_done, n_req);
			CE_ENSURE(n <= n_req);
			bytes_done += n;
			rr.set_progress(bytes_done, size);
			if (n != n_req)
				break;
		}
		rr.set_progress(bytes_done, size);
		return bytes_done == size ? SaveError::SUCCESS : SaveError::IO_ERROR;
	}

	s32 read_request(SaveGame::Request &rr)
	{
		_allocator->deallocate(rr.data);
		rr.data = NULL;
		rr.size = 0;

		const Stat st = _filesystem.stat(rr.basename);
		if (st.file_type == Stat::NO_ENTRY)
			return SaveError::MISSING;
		if (st.file_type != Stat::REGULAR)
			return SaveError::IO_ERROR;

		File *file = _filesystem.open(rr.basename, FileOpenMode::READ);
		CE_ENSURE(file != NULL);
		if (!file->is_open()) {
			_filesystem.close(*file);
			return SaveError::IO_ERROR;
		}

		const u32 size = file->size();
		rr.data = _allocator->allocate(size + 1);
		rr.size = size;

		const s32 io_result = do_chunked_io(*file, rr.data, size, IO_READ, rr);
		_filesystem.close(*file);
		if (io_result != SaveError::SUCCESS)
			return io_result;
		((char *)rr.data)[rr.size] = '\0';

		return SaveError::SUCCESS;
	}

	s32 write_request(SaveGame::Request &rr)
	{
		DynamicString temp_filename(default_allocator());
		File *file = _filesystem.open_temporary(temp_filename);
		CE_ENSURE(file != NULL);
		if (!file->is_open()) {
			_filesystem.close(*file);
			return SaveError::IO_ERROR;
		}

		const u32 size = rr.size;

		const s32 io_result = do_chunked_io(*file, rr.data, size, IO_WRITE, rr);
		_filesystem.close(*file);

		if (io_result != SaveError::SUCCESS) {
			os::delete_file(temp_filename.c_str());
			return io_result;
		}

		DynamicString filename(default_allocator());
		_filesystem.absolute_path(filename, rr.basename);

		const RenameResult res = os::rename(temp_filename.c_str(), filename.c_str());
		if (res.error != RenameResult::SUCCESS) {
			os::delete_file(temp_filename.c_str());
			return SaveError::IO_ERROR;
		}

		return SaveError::SUCCESS;
	}

#if CROWN_PLATFORM_EMSCRIPTEN
	/// Spins until @a pred returns true or exit is requested.
	/// Returns true if the predicate was satisfied, false if exit was requested.
	bool html5_spin_until(bool (*pred)())
	{
		while (!_exit.load() && !pred())
			os::sleep(1);
		return !_exit.load();
	}
#endif

	///
	void process_request(Request &rr)
	{
		CE_ENSURE(!rr.done.load());
		rr.progress.store(0.0f);
		rr.error.store(SaveError::SUCCESS);

#if CROWN_PLATFORM_EMSCRIPTEN
		if (!html5_spin_until(html5_save_game_ready))
			return;

		if (!html5_save_game_ok()) {
			rr.progress.store(1.0f);
			rr.error.store(SaveError::IO_ERROR);
			rr.done.store(true);
			return;
		}
#endif

		s32 result = rr.type == SaveGame::Request::LOAD
			? read_request(rr)
			: write_request(rr)
			;

#if CROWN_PLATFORM_EMSCRIPTEN
		if (result == SaveError::SUCCESS && rr.type == SaveGame::Request::SAVE) {
			html5_save_game_flush();
			if (!html5_spin_until([]{ return !html5_save_game_syncing(); }))
				return;

			if (!html5_save_game_flush_ok())
				result = SaveError::IO_ERROR;
		}
#endif

		rr.progress.store(1.0f);
		rr.error.store(result);
		rr.done.store(true);
	}

	///
	s32 run()
	{
		while (!_exit.load()) {
			_mutex.lock();
			while (!_exit.load() && _requests_queue.empty())
				_requests_condition.wait(_mutex);
			_mutex.unlock();

			u32 idx;
			while (!_exit.load() && _requests_queue.pop(idx)) {
				CE_ENSURE(idx < countof(_requests));
				process_request(_requests[idx]);
			}
		}

		return 0;
	}
};

namespace save_game
{
	u32 load(const char *filename)
	{
		CE_ASSERT(s_save_game != NULL, "SaveGame not initialized!");
		return s_save_game->queue_request(SaveGame::Request::LOAD, filename, NULL, 0);
	}

	u32 save(const char *filename, const void *data, u32 size)
	{
		CE_ASSERT(s_save_game != NULL, "SaveGame not initialized!");
		return s_save_game->queue_request(SaveGame::Request::SAVE, filename, data, size);
	}

	SaveStatus status(u32 save_request)
	{
		CE_ASSERT(s_save_game != NULL, "SaveGame not initialized!");
		SaveStatus st;
		st.done = true;
		st.progress = 0.0f;
		st.data = NULL;
		st.size = 0;
		st.error = SaveError::INVALID_REQUEST;

		SaveGame::Request *rr = s_save_game->find_request(save_request);
		if (rr == NULL)
			return st;

		const bool done = rr->done.load();
		const s32 error = done ? rr->error.load() : SaveError::SUCCESS;
		const bool load_ok = done && error == SaveError::SUCCESS && rr->type == SaveGame::Request::LOAD;
		st.done = done;
		st.progress = rr->progress.load();
		st.data = load_ok ? rr->data : NULL;
		st.size = load_ok ? rr->size : 0;
		st.error = error;
		return st;
	}

	void free(u32 save_request)
	{
		CE_ASSERT(s_save_game != NULL, "SaveGame not initialized!");
		SaveGame::Request *rr = s_save_game->find_request(save_request);
		if (rr == NULL)
			return;

		CE_ASSERT(rr->done.load(), "Cannot free pending SaveGame request");

		rr->token = 0;
		s_save_game->_allocator->deallocate(rr->data);
		rr->data = NULL;
		rr->size = 0;
		rr->progress.store(0.0f);
		rr->basename[0] = '\0';
		rr->error.store(SaveError::SUCCESS);
	}

} // namespace save_game

namespace save_game_globals
{
	void init(Allocator &a, const char *save_dir)
	{
		CE_ASSERT(s_save_game == NULL, "SaveGame already initialized");
		s_save_game = CE_NEW(a, SaveGame)(a, save_dir);
	}

	void shutdown()
	{
		if (s_save_game == NULL)
			return;

		Allocator &a = *s_save_game->_allocator;
		CE_DELETE(a, s_save_game);
		s_save_game = NULL;
	}

	SaveGame *save_game()
	{
		return s_save_game;
	}

} // namespace save_game_globals

} // namespace crown
