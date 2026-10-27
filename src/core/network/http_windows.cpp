/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PLATFORM_WINDOWS && !CROWN_HTTP_NOOP
#include "core/containers/array.inl"
#include "core/filesystem/http_file.h"
#include "core/memory/temp_allocator.inl"
#include "core/network/http_internal.h"
#include "core/strings/string.inl"
#include "core/thread/condition_variable.h"
#include "core/thread/scoped_mutex.inl"
#include <stb_sprintf.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>

namespace crown
{
struct HttpWinRequest;

struct HttpWin
{
	Allocator *_allocator;
	HINTERNET _session;
	PTP_WORK _work;
	Mutex _mutex;
	ConditionVariable _condition;
	HttpWinRequest *_requests[HTTP_MAX_REQUESTS];
	u32 _num_requests;
	u32 _retiring;
	bool _dispatching;

	explicit HttpWin(Allocator &a)
		: _allocator(&a)
		, _session(NULL)
		, _work(NULL)
		, _num_requests(0)
		, _retiring(0)
		, _dispatching(false)
	{
	}
};

struct HttpWinRequest
{
	HttpRequest request;
	HttpDownload download;
	HttpWin *backend;
	HINTERNET connection;
	HINTERNET handle;
	u32 calls;
	bool started;
	bool close_started;
	bool handle_closed;
	bool callback_installed;
	DWORD content_length;
	bool has_content_length;
	char buffer[64 * 1024];
};

static void http_win_error(u32 id, DWORD code)
{
	char message[512];
	DWORD length = FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE
		| FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS
		, GetModuleHandleW(L"winhttp.dll")
		, code
		, 0
		, message
		, sizeof(message)
		, NULL
		);
	if (length == 0)
		stbsp_snprintf(message, sizeof(message), "WinHTTP error %lu", (unsigned long)code);
	http_request::fail(id, message);
}

static bool http_win_timeouts(HINTERNET handle, const HttpRequest &request)
{
	const s32 remaining = http_request::remaining_ms(request);
	if (remaining < 0) {
		SetLastError(ERROR_WINHTTP_TIMEOUT);
		return false;
	}

	return WinHttpSetTimeouts(handle, remaining, remaining, remaining, remaining) != FALSE;
}

static void http_win_leave(HttpWinRequest &rr)
{
	HttpWin &win = *rr.backend;
	bool destroy = false;
	win._mutex.lock();
	--rr.calls;
	if (rr.handle_closed && rr.calls == 0) {
		for (u32 ii = 0; ii < win._num_requests; ++ii) {
			if (win._requests[ii] == &rr) {
				win._requests[ii] = win._requests[--win._num_requests];
				break;
			}
		}

		++win._retiring;
		destroy = true;
	} else if (!http_request::active(rr.request)) {
		SubmitThreadpoolWork(win._work);
	}

	win._mutex.unlock();
	if (destroy) {
		if (rr.connection != NULL)
			WinHttpCloseHandle(rr.connection);
		http_file::close(rr.download);
		http_request::release(&rr.request);
		win._mutex.lock();
		--win._retiring;
		win._condition.broadcast();
		win._mutex.unlock();
	}
}

static bool http_win_wide(Array<wchar_t> &out, const char *text)
{
	const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, NULL, 0);
	if (size == 0)
		return false;

	array::resize(out, (u32)size);
	return MultiByteToWideChar(CP_UTF8
		, MB_ERR_INVALID_CHARS
		, text
		, -1
		, array::begin(out)
		, size
		) != 0;
}

static void http_win_read(HttpWinRequest &rr)
{
	if ((rr.request.timeout_ms != 0 && !http_win_timeouts(rr.handle, rr.request))
		|| !WinHttpReadData(rr.handle, rr.buffer, sizeof(rr.buffer), NULL))
		http_win_error(rr.request.id, GetLastError());
}

static void CALLBACK http_win_callback(HINTERNET handle
	, DWORD_PTR context
	, DWORD notification
	, void *info
	, DWORD size
	)
{
	HttpWinRequest *rr = (HttpWinRequest *)context;
	if (rr == NULL)
		return;

	rr->backend->_mutex.lock();
	++rr->calls;
	if (notification == WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING)
		rr->handle_closed = true;
	rr->backend->_mutex.unlock();
	if (notification != WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING
		&& http_request::active(rr->request)) {
		switch (notification) {
		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
			if (!http_win_timeouts(handle, rr->request) || !WinHttpReceiveResponse(handle, NULL))
				http_win_error(rr->request.id, GetLastError());
			break;

		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: {
			DWORD bytes = 0;
			WinHttpQueryHeaders(rr->handle
				, WINHTTP_QUERY_RAW_HEADERS_CRLF
				, WINHTTP_HEADER_NAME_BY_INDEX
				, NULL
				, &bytes
				, WINHTTP_NO_HEADER_INDEX
				);
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || bytes >= UINT32_MAX / 2)
				goto headers_failed;

			TempAllocator4096 ta;
			Array<wchar_t> wide(ta);
			array::resize(wide, bytes / sizeof(wchar_t) + 1);
			array::back(wide) = L'\0';
			if (!WinHttpQueryHeaders(rr->handle
				, WINHTTP_QUERY_RAW_HEADERS_CRLF
				, WINHTTP_HEADER_NAME_BY_INDEX
				, array::begin(wide)
				, &bytes
				, WINHTTP_NO_HEADER_INDEX
				))
				goto headers_failed;

			const int size = WideCharToMultiByte(CP_UTF8, 0, array::begin(wide), -1, NULL, 0, NULL, NULL);
			if (size == 0 || (u32)size >= UINT32_MAX / 2)
				goto headers_failed;

			Buffer text(ta);
			array::resize(text, (u32)size);
			if (WideCharToMultiByte(CP_UTF8
				, 0
				, array::begin(wide)
				, -1
				, array::begin(text)
				, size
				, NULL
				, NULL
				) != size)
				goto headers_failed;

			const char *line = array::begin(text);
			while (*line != '\0') {
				const char *end = strnl(line);
				if (!http_response::header_line(rr->request.response
					, *rr->request.allocator
					, line
					, (u32)(end - line)
					))
					goto headers_failed;

				line = end;
			}

			if (rr->request.response->status_code != 204 && rr->request.response->status_code != 304) {
				DWORD size = sizeof(rr->content_length);
				rr->has_content_length = WinHttpQueryHeaders(rr->handle
					, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER
					, WINHTTP_HEADER_NAME_BY_INDEX
					, &rr->content_length
					, &size
					, WINHTTP_NO_HEADER_INDEX
					) != FALSE;
				if (!rr->has_content_length && GetLastError() != ERROR_WINHTTP_HEADER_NOT_FOUND)
					goto headers_failed;
			}

			if (!http_request::headers(rr->request))
				goto headers_failed;
			if (rr->request.path != NULL) {
				const StringView encoding = http_response::header_value(*rr->request.response, "Content-Encoding");
				if (encoding == "gzip" || encoding == "deflate")
					rr->has_content_length = false;
				else if (encoding.length() != 0 && encoding != StringView("identity"))
					goto headers_failed;
				if (!http_file::open(rr->download, *rr->request.allocator, rr->request.path)) {
					http_request::fail(rr->request.id, "Cannot open HTTP download file");
					goto headers_failed;
				}
			}
		}
			http_win_read(*rr);
			break;

		headers_failed:
			http_request::fail(rr->request.id, "Unable to read WinHTTP response headers");
			break;

		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
			if (size == 0) {
				if (http_request::expired(rr->request))
					http_request::fail(rr->request.id, "HTTP request timed out");
				else if (rr->has_content_length
					&& rr->request.response->body_size + rr->download.size != rr->content_length)
					http_request::fail(rr->request.id, "Incomplete HTTP response body");
				else if (rr->request.path != NULL && !http_file::finish(rr->download))
					http_request::fail(rr->request.id, "Cannot finish HTTP download file");
				else
					rr->download.complete = http_request::complete(rr->request);
			} else {
				const bool success = rr->request.path != NULL
					? http_file::write(rr->download, rr->buffer, size)
					: http_request::body(rr->request, rr->buffer, size)
					;
				if (!success)
					http_request::fail(rr->request.id, rr->request.path != NULL ? "Cannot write HTTP download file" : "HTTP response is too large");
				else
					http_win_read(*rr);
			}

			break;

		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			http_win_error(rr->request.id, ((WINHTTP_ASYNC_RESULT *)info)->dwError);
			break;

		default:
			break;
		}
	}

	http_win_leave(*rr);
}

static void CALLBACK http_win_dispatch(PTP_CALLBACK_INSTANCE instance, void *context, PTP_WORK work)
{
	CE_UNUSED_2(instance, work);
	HttpWin &win = *(HttpWin *)context;
	win._mutex.lock();
	const bool dispatching = win._dispatching;
	win._dispatching = true;
	win._mutex.unlock();
	if (dispatching)
		return;

	for (;;) {
		HttpWinRequest *next = NULL;
		HINTERNET handle = NULL;
		bool start = false;
		win._mutex.lock();
		for (u32 ii = 0; ii < win._num_requests; ++ii) {
			HttpWinRequest &rr = *win._requests[ii];
			if (!rr.started) {
				rr.started = true;
				next = &rr;
				start = true;
				break;
			}

			if (rr.calls == 0 && !rr.close_started
				&& !http_request::active(rr.request)) {
				rr.close_started = true;
				handle = rr.handle;
				rr.handle = NULL;
				next = &rr;
				break;
			}
		}

		if (next == NULL)
			win._dispatching = false;
		else
			++next->calls;
		win._mutex.unlock();
		if (next == NULL)
			return;

		if (start) {
			HttpWinRequest &rr = *next;
			HttpRequest &request = rr.request;
			if (!http_request::active(request))
				goto request_dispatched;

			request.response = http_response::create(*request.allocator);
			TempAllocator4096 ta;
			Array<wchar_t> wide(ta);
			array::reserve(wide, max(request.url.length(), request.content_type_header.length()) + 1);
			if (!http_win_wide(wide, request.url.data())) {
				http_request::fail(request.id, "Invalid UTF-8 HTTP URL");
				goto request_dispatched;
			}

			URL_COMPONENTS parts = {};
			parts.dwStructSize = sizeof(parts);
			parts.dwHostNameLength = (DWORD)-1;
			parts.dwUrlPathLength = (DWORD)-1;
			parts.dwExtraInfoLength = (DWORD)-1;
			if (!WinHttpCrackUrl(array::begin(wide), 0, 0, &parts)) {
				http_win_error(request.id, GetLastError());
				goto request_dispatched;
			}

			Array<wchar_t> path(ta);
			if (parts.dwUrlPathLength != 0)
				array::push(path, parts.lpszUrlPath, parts.dwUrlPathLength);
			else
				array::push_back(path, L'/');
			for (DWORD ii = 0; ii < parts.dwExtraInfoLength && parts.lpszExtraInfo[ii] != L'#'; ++ii)
				array::push_back(path, parts.lpszExtraInfo[ii]);
			array::push_back(path, L'\0');
			wchar_t *host = parts.lpszHostName == NULL ? &array::back(wide) : parts.lpszHostName;
			host[parts.dwHostNameLength] = L'\0';
			rr.connection = WinHttpConnect(win._session, host, parts.nPort, 0);
			if (rr.connection == NULL) {
				http_win_error(request.id, GetLastError());
				goto request_dispatched;
			}

			rr.handle = WinHttpOpenRequest(rr.connection
				, request.post ? L"POST" : L"GET"
				, array::begin(path)
				, NULL
				, WINHTTP_NO_REFERER
				, WINHTTP_DEFAULT_ACCEPT_TYPES
				, parts.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0
				);
			DWORD_PTR context = (DWORD_PTR)&rr;
			if (rr.handle == NULL
				|| !WinHttpSetOption(rr.handle, WINHTTP_OPTION_CONTEXT_VALUE, &context, sizeof(context))
				|| WinHttpSetStatusCallback(rr.handle
				, http_win_callback
				, WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_FLAG_HANDLES
				, 0
				) == WINHTTP_INVALID_STATUS_CALLBACK) {
				http_win_error(request.id, GetLastError());
				goto request_dispatched;
			}

			rr.callback_installed = true;
			if (request.path != NULL) {
				DWORD flags = WINHTTP_DECOMPRESSION_FLAG_GZIP | WINHTTP_DECOMPRESSION_FLAG_DEFLATE;
				if (!WinHttpSetOption(rr.handle, WINHTTP_OPTION_DECOMPRESSION, &flags, sizeof(flags))) {
					http_win_error(request.id, GetLastError());
					goto request_dispatched;
				}
			}
			if (request.range[0] != '\0') {
				char headers[96];
				stbsp_snprintf(headers, sizeof(headers), "Range: %s\r\nAccept-Encoding: identity", request.range);
				if (!http_win_wide(wide, headers)
					|| !WinHttpAddRequestHeaders(rr.handle, array::begin(wide), (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD)) {
					http_win_error(request.id, GetLastError());
					goto request_dispatched;
				}
			}
			if (request.post) {
				if (!http_win_wide(wide, request.content_type_header.data())) {
					http_request::fail(request.id, "Invalid UTF-8 HTTP content type");
					goto request_dispatched;
				}

				if (!WinHttpAddRequestHeaders(rr.handle
					, array::begin(wide)
					, (DWORD)-1
					, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE
					)) {
					http_win_error(request.id, GetLastError());
					goto request_dispatched;
				}
			}

			if (!http_win_timeouts(rr.handle, request)
				|| !WinHttpSendRequest(rr.handle
				, WINHTTP_NO_ADDITIONAL_HEADERS
				, 0
				, request.upload.length() == 0 ? WINHTTP_NO_REQUEST_DATA : (void *)request.upload.data()
				, request.upload.length()
				, request.upload.length()
				, context
				))
				http_win_error(request.id, GetLastError());
		} else {
			if (handle != NULL) {
				const BOOL success = WinHttpCloseHandle(handle);
				CE_ENSURE(success);
				CE_UNUSED(success);
			}

			if (!next->callback_installed) {
				win._mutex.lock();
				next->handle_closed = true;
				win._mutex.unlock();
			}
		}

	request_dispatched:
		http_win_leave(*next);
	}
}

namespace http_backend
{
	void *init(Allocator &a)
	{
		HttpWin *win = CE_NEW(a, HttpWin)(a);
		win->_session = WinHttpOpen(L"Crown"
			, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY
			, WINHTTP_NO_PROXY_NAME
			, WINHTTP_NO_PROXY_BYPASS
			, WINHTTP_FLAG_ASYNC
			);
		win->_work = CreateThreadpoolWork(http_win_dispatch, win, NULL);
		return win;
	}

	void shutdown(void *backend)
	{
		HttpWin &win = *(HttpWin *)backend;
		if (win._work != NULL) {
			SubmitThreadpoolWork(win._work);
			win._mutex.lock();
			while (win._num_requests != 0 || win._retiring != 0)
				win._condition.wait(win._mutex);
			win._mutex.unlock();
			WaitForThreadpoolWorkCallbacks(win._work, FALSE);
			CloseThreadpoolWork(win._work);
		}

		if (win._session != NULL)
			WinHttpCloseHandle(win._session);
		CE_DELETE(*win._allocator, &win);
	}

	HttpRequest *create(Allocator &a
		, const char *url
		, const char *content_type
		, const StringView *body
		)
	{
		return http_request::create<HttpWinRequest>(a, url, content_type, body);
	}

	void start(void *backend, HttpRequest *request)
	{
		HttpWin &win = *(HttpWin *)backend;
		if (win._session == NULL || win._work == NULL) {
			http_request::fail(request->id, "Unable to initialize WinHTTP");
			http_request::release(request);
			return;
		}

		HttpWinRequest *rr = (HttpWinRequest *)request;
		rr->backend = &win;
		ScopedMutex sm(win._mutex);
		win._requests[win._num_requests++] = rr;
		SubmitThreadpoolWork(win._work);
	}

	void cancel(void *backend)
	{
		HttpWin &win = *(HttpWin *)backend;
		if (win._work != NULL)
			SubmitThreadpoolWork(win._work);
	}

} // namespace http_backend

} // namespace crown
#endif // if CROWN_PLATFORM_WINDOWS && !CROWN_HTTP_NOOP
