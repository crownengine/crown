/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if CROWN_PLATFORM_EMSCRIPTEN
#include "core/filesystem/http_file.h"
#include "core/strings/dynamic_string.inl"
#include <emscripten/emscripten.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

namespace crown
{
struct HttpFileTask
{
	enum Operation
	{
		SIZE,
		READ,
		REMOVE
	};
	const char *path;
	void *data;
	u32 offset;
	u32 size;
	Operation operation;
	s64 result;
};

extern "C" EMSCRIPTEN_KEEPALIVE void crown_http_file_done(HttpFileTask *task, em_proxying_ctx *ctx, f64 result)
{
	task->result = (s64)result;
	emscripten_proxy_finish(ctx);
}

// code-format off
EM_JS(void, http_file_js, (em_proxying_ctx *ctx
	, HttpFileTask *task
	, const char *path
	, int operation
	, void *data
	, u32 offset
	, u32 size
	), {
	const name = UTF8ToString(Number(path));
	const files = Module.crownHttpFiles || (Module.crownHttpFiles = new Map());
	(async function access_file() {
		let result = -1;
		try {
			if (operation == 2) {
				files.delete(name);
				const root = await navigator.storage.getDirectory();
				await root.removeEntry(name);
				result = 0;
			} else {
				let file = files.get(name);
				if (!file) {
					const root = await navigator.storage.getDirectory();
					const handle = await root.getFileHandle(name);
					file = await handle.getFile();
					files.set(name, file);
				}
				if (operation == 0) {
					if (file.size <= 4294967295) {
						result = file.size;
					}
				} else {
					const bytes = new Uint8Array(await file.slice(offset, offset + size).arrayBuffer());
					HEAPU8.set(bytes, Number(data));
					result = bytes.length;
				}
			}
		} catch (e) {
		}
		_crown_http_file_done(task, ctx, result);
	})();
});
// code-format on

static void http_file_access(em_proxying_ctx *ctx, void *data)
{
	HttpFileTask &task = *(HttpFileTask *)data;
	http_file_js(ctx
		, &task
		, task.path
		, task.operation
		, task.data
		, task.offset
		, task.size
		);
}

static s64 http_file_access(HttpFileTask &task)
{
	const bool ok = emscripten_proxy_sync_with_ctx(emscripten_proxy_get_system_queue()
		, emscripten_main_runtime_thread_id()
		, http_file_access
		, &task
		);
	return ok ? task.result : -1;
}

namespace http_file
{
	bool temporary_path(DynamicString &path)
	{
		char name[64];
		// code-format off
		const int ok = EM_ASM_INT({
			if (!navigator.storage || !navigator.storage.getDirectory) {
				return 0;
			}
			const bytes = new TextEncoder().encode('crown-http-' + crypto.randomUUID() + '\0');
			HEAPU8.set(bytes, Number($0));
			return 1;
		}, name);
		// code-format on
		if (ok)
			path = name;
		return ok != 0;
	}

	s64 size(const char *path)
	{
		HttpFileTask task = { path, NULL, 0, 0, HttpFileTask::SIZE, -1 };
		return http_file_access(task);
	}

	u32 read(void *data, HttpReadFile &file, const char *path, u32 offset, u32 size)
	{
		CE_UNUSED(file);
		HttpFileTask task = { path, data, offset, size, HttpFileTask::READ, -1 };
		const s64 result = http_file_access(task);
		return result < 0 ? 0 : (u32)result;
	}

	void remove(const char *path, HttpReadFile &file)
	{
		CE_UNUSED(file);
		HttpFileTask task = { path, NULL, 0, 0, HttpFileTask::REMOVE, -1 };
		http_file_access(task);
	}

} // namespace http_file

} // namespace crown
#endif // if CROWN_PLATFORM_EMSCRIPTEN
