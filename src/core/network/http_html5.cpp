/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PLATFORM_EMSCRIPTEN && !CROWN_HTTP_NOOP
#include "core/memory/globals.h"
#include "core/memory/memory.inl"
#include "core/network/http_internal.h"
#include "core/strings/string.inl"
#include <emscripten/emscripten.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

namespace crown
{
struct HttpFetch
{
	Allocator *_allocator;
	em_proxying_queue *_queue;

	explicit HttpFetch(Allocator &a)
		: _allocator(&a)
		, _queue(em_proxying_queue_create())
	{
		CE_ENSURE(_queue != NULL);
	}
};

struct HttpFetchRequest
{
	HttpRequest request;
};

extern "C" EMSCRIPTEN_KEEPALIVE void crown_http_finish(em_proxying_ctx *ctx)
{
	emscripten_proxy_finish(ctx);
}

extern "C" EMSCRIPTEN_KEEPALIVE void *crown_http_allocate(u32 size)
{
	return default_allocator().allocate(size);
}

extern "C" EMSCRIPTEN_KEEPALIVE void crown_http_free(void *data)
{
	default_allocator().deallocate(data);
}

extern "C" EMSCRIPTEN_KEEPALIVE void crown_http_release(HttpRequest *request)
{
	http_request::release(request);
}

extern "C" EMSCRIPTEN_KEEPALIVE int crown_http_active(HttpRequest *request)
{
	return http_request::active(*request) && !http_request::expired(*request);
}

extern "C" EMSCRIPTEN_KEEPALIVE int crown_http_headers(HttpRequest *request
	, u32 code
	, const char *reason
	)
{
	const u32 reason_size = strlen32(reason);
	const char *headers = reason + reason_size + 1;
	request->response = http_response::create(*request->allocator);
	request->response->status_code = code;
	if (!http_response::reason(request->response
		, *request->allocator
		, reason
		, reason_size
		))
		return false;
	while (*headers != '\0') {
		const char *end = strnl(headers);
		if (!http_response::header_line(request->response
			, *request->allocator
			, headers
			, (u32)(end - headers)
			))
			return false;
		headers = end;
	}
	return http_request::headers(*request);
}

extern "C" EMSCRIPTEN_KEEPALIVE int crown_http_body(HttpRequest *request, const char *data, u32 size)
{
	return http_request::active(*request)
		&& http_request::body(*request, data, size);
}

extern "C" EMSCRIPTEN_KEEPALIVE int crown_http_done(HttpRequest *request, const char *error)
{
	bool complete = false;
	if (error[0] != '\0')
		http_request::fail(request->id, error);
	else if (http_request::expired(*request))
		http_request::fail(request->id, "Browser HTTP request timed out");
	else
		complete = http_request::complete(*request);
	return complete;
}

// code-format off
EM_JS(void, http_fetch_js, (HttpRequest *request
	, const char *url
	, const char *range
	, const char *path
	, const char *content_type
	, const char *upload
	, u32 upload_size
	, int post
	, u32 timeout_ms
	), {
	const tasks = Module.crownHttpRequests || (Module.crownHttpRequests = new Map());
	const controller = new AbortController();
	const task = { controller, promise: null };
	const destination = path ? UTF8ToString(Number(path)) : "";
	const headers = {};
	const byte_range = UTF8ToString(Number(range));
	if (byte_range) {
		headers['Range'] = byte_range;
	}
	if (post) {
		headers['Content-Type'] = UTF8ToString(Number(content_type));
	}
	const options = { method: post ? 'POST' : 'GET', headers, signal: controller.signal };
	if (post) {
		options.body = HEAPU8.slice(Number(upload), Number(upload) + upload_size);
	}
	const address = UTF8ToString(Number(url));

	const encoder = Module.crownHttpEncoder || (Module.crownHttpEncoder = new TextEncoder());
	tasks.set(request, task);
	task.promise = (async function download() {
		let buffer = 0, capacity = 0, reader, root, file;
		let error = "";
		const timer = timeout_ms ? setTimeout(function timeout() {
			controller.abort();
		}, timeout_ms) : 0;
		try {
			const response = await fetch(address, options);
			if (!_crown_http_active(request)) {
				throw new Error('HTTP request cancelled or timed out');
			}
			let text = "";
			for (const [name, value] of response.headers) {
				text += name + ': ' + value + '\n';
			}
			const fields = encoder.encode(response.statusText + '\0' + text + '\0');
			capacity = Math.max(fields.length, destination ? 1 : 64 * 1024);
			buffer = _crown_http_allocate(capacity);
			HEAPU8.set(fields, Number(buffer));
			if (!_crown_http_headers(request, response.status, buffer)) {
				throw new Error('Unusable HTTP response');
			}
			if (destination) {
				root = await navigator.storage.getDirectory();
				file = await root.getFileHandle(destination, { create: true });
				await response.body.pipeTo(await file.createWritable(), { signal: controller.signal });
				if ((await file.getFile()).size > 4294967295) {
					throw new Error('HTTP download is too large');
				}
			} else if (response.body) {
				reader = response.body.getReader();
				for (;;) {
					const chunk = await reader.read();
					if (chunk.done) {
						break;
					}
					for (let offset = 0; offset < chunk.value.length; offset += 64 * 1024) {
						const bytes = chunk.value.length <= 64 * 1024
							? chunk.value : chunk.value.subarray(offset, offset + 64 * 1024);
						HEAPU8.set(bytes, Number(buffer));
						if (!_crown_http_body(request, buffer, bytes.length)) {
							throw new Error('Cannot read HTTP response');
						}
					}
				}
			}
			if (!_crown_http_active(request)) {
				throw new Error('HTTP request cancelled or timed out');
			}
		} catch (e) {
			error = String(e);
			controller.abort();
		} finally {
			clearTimeout(timer);
			if (reader) {
				try {
					await reader.cancel();
				} catch (e) {
				}
			}
			if (error) {
				const message = encoder.encode(error + '\0');
				if (message.length > capacity) {
					if (buffer) {
						_crown_http_free(buffer);
					}
					buffer = _crown_http_allocate(message.length);
				}
				HEAPU8.set(message, Number(buffer));
			} else {
				HEAPU8[Number(buffer)] = 0;
			}
			const complete = _crown_http_done(request, buffer);
			_crown_http_free(buffer);
			if (file && !complete) {
				try {
					await root.removeEntry(destination);
				} catch (e) {
				}
			}
			tasks.delete(request);
			_crown_http_release(request);
		}
	})();
});
// code-format on

static void http_fetch_start(void *data)
{
	HttpRequest &request = *(HttpRequest *)data;
	const s32 remaining = http_request::remaining_ms(request);
	if (remaining < 0)
		http_request::fail(request.id, "HTTP request timed out");
	if (!http_request::active(request)) {
		http_request::release(&request);
		return;
	}
	http_fetch_js(&request
		, request.url.data()
		, request.range
		, request.path
		, request.content_type.data()
		, request.upload.data()
		, request.upload.length()
		, request.post
		, remaining
		);
}

// code-format off
EM_JS(void, http_fetch_cancel, (void *data), {
	for (const [request, task] of Module.crownHttpRequests || []) {
		if (!_crown_http_active(request)) {
			task.controller.abort();
		}
	}
});
// code-format on

// code-format off
EM_JS(void, http_fetch_shutdown, (em_proxying_ctx *ctx, void *data), {
	const pending = [];
	for (const task of (Module.crownHttpRequests || new Map()).values()) {
		task.controller.abort();
		pending.push(task.promise);
	}
	Promise.all(pending).then(function drained() {
		_crown_http_finish(ctx);
	});
});
// code-format on

namespace http_backend
{
	void *init(Allocator &a)
	{
		return CE_NEW(a, HttpFetch)(a);
	}

	void shutdown(void *context)
	{
		HttpFetch &backend = *(HttpFetch *)context;
		const bool ok = emscripten_proxy_sync_with_ctx(backend._queue
			, emscripten_main_runtime_thread_id()
			, http_fetch_shutdown
			, NULL
			);
		CE_ENSURE(ok);
		CE_UNUSED(ok);
		em_proxying_queue_destroy(backend._queue);
		CE_DELETE(*backend._allocator, &backend);
	}

	HttpRequest *create(Allocator &a
		, const char *url
		, const char *content_type
		, const StringView *body
		)
	{
		return http_request::create<HttpFetchRequest>(a, url, content_type, body);
	}

	void start(void *context, HttpRequest *request)
	{
		HttpFetch &backend = *(HttpFetch *)context;
		if (!emscripten_proxy_async(backend._queue
			, emscripten_main_runtime_thread_id()
			, http_fetch_start
			, request
			)) {
			http_request::fail(request->id, "Unable to schedule browser HTTP request");
			http_request::release(request);
		}
	}

	void cancel(void *context)
	{
		HttpFetch &backend = *(HttpFetch *)context;
		const bool ok = emscripten_proxy_async(backend._queue
			, emscripten_main_runtime_thread_id()
			, http_fetch_cancel
			, NULL
			);
		CE_ENSURE(ok);
		CE_UNUSED(ok);
	}

} // namespace http_backend

} // namespace crown
#endif // if CROWN_PLATFORM_EMSCRIPTEN && !CROWN_HTTP_NOOP
