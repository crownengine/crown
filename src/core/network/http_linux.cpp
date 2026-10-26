/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PLATFORM_LINUX && !CROWN_HTTP_NOOP
#include "core/memory/memory.inl"
#include "core/network/http_internal.h"
#include "core/os.h"
#include "core/thread/mutex.h"
#include "core/thread/thread.h"
#include <atomic>
#include <string.h>
#define CURL_DISABLE_DEPRECATION
#include <curl/curl.h>

namespace crown
{
#define HTTP_CURL_FUNCTIONS(F)  \
	F(curl_global_init)         \
	F(curl_global_cleanup)      \
	F(curl_version_info)        \
	F(curl_easy_init)           \
	F(curl_easy_cleanup)        \
	F(curl_easy_setopt)         \
	F(curl_easy_getinfo)        \
	F(curl_easy_strerror)       \
	F(curl_multi_init)          \
	F(curl_multi_cleanup)       \
	F(curl_multi_add_handle)    \
	F(curl_multi_remove_handle) \
	F(curl_multi_perform)       \
	F(curl_multi_poll)          \
	F(curl_multi_wakeup)        \
	F(curl_multi_info_read)

struct HttpCurlRequest;

struct HttpCurl
{
	Allocator *_allocator;
	void *_library;
	CURLM *_multi;
	bool _initialized;
	const char *_error;
	HttpCurlRequest *_requests[HTTP_MAX_REQUESTS];
	u32 _num_requests;
	Mutex _mutex;
	Thread _thread;
	HttpRequest *_pending[HTTP_MAX_REQUESTS];
	u32 _num_pending;
	std::atomic_bool _exit;
#define HTTP_CURL_DECLARE(name) decltype(&::name) _##name;
	HTTP_CURL_FUNCTIONS(HTTP_CURL_DECLARE)
#undef HTTP_CURL_DECLARE

	explicit HttpCurl(Allocator &a)
		: _allocator(&a)
		, _library(NULL)
		, _multi(NULL)
		, _initialized(false)
		, _error("System libcurl is unavailable")
		, _num_requests(0)
		, _num_pending(0)
		, _exit(false)
	{
	}
};

struct HttpCurlRequest
{
	HttpRequest request;
	CURL *easy;
	curl_slist headers;
	bool added;
	char diagnostic[CURL_ERROR_SIZE];
};

static size_t http_curl_body(char *bytes, size_t size, size_t count, void *user_data)
{
	HttpCurlRequest &rr = *(HttpCurlRequest *)user_data;
	if (size != 0 && count > UINT32_MAX / size)
		return 0;

	const size_t length = size * count;
	if (!http_request::active(rr.request))
		return 0;

	if (!http_request::body(rr.request, bytes, (u32)length)) {
		http_request::fail(rr.request.id, "HTTP response is too large");
		return 0;
	}

	return length;
}

static size_t http_curl_header(char *bytes, size_t size, size_t count, void *user_data)
{
	HttpCurlRequest &rr = *(HttpCurlRequest *)user_data;
	if (size != 0 && count > UINT32_MAX / size)
		return 0;

	const size_t length = size * count;
	if (!http_response::header_line(rr.request.response
		, *rr.request.allocator
		, bytes
		, (u32)length
		)) {
		http_request::fail(rr.request.id, "Invalid or oversized HTTP response headers");
		return 0;
	}

	return length;
}

static void http_curl_wakeup(HttpCurl &curl)
{
	if (curl._multi == NULL)
		return;

	const CURLMcode result = curl._curl_multi_wakeup(curl._multi);
	CE_ENSURE(result == CURLM_OK);
	CE_UNUSED(result);
}

static s32 http_curl_run(void *user_data)
{
	HttpCurl &curl = *(HttpCurl *)user_data;
	for (;;) {
		HttpRequest *pending[HTTP_MAX_REQUESTS];
		curl._mutex.lock();
		const u32 count = curl._num_pending;
		memcpy(pending, curl._pending, count * sizeof(*pending));
		curl._num_pending = 0;
		curl._mutex.unlock();
		for (u32 ii = 0; ii < count; ++ii) {
			HttpRequest *request = pending[ii];
			if (curl._exit.load() || !http_request::active(*request)) {
				http_request::release(request);
				continue;
			}

			const s32 timeout = http_request::remaining_ms(*request);
			if (timeout < 0) {
				http_request::fail(request->id, "HTTP request timed out");
				http_request::release(request);
				continue;
			}

			HttpCurlRequest *rr = (HttpCurlRequest *)request;
			request->response = http_response::create(*request->allocator);
			curl._requests[curl._num_requests++] = rr;
			rr->easy = curl._curl_easy_init();
			if (rr->easy == NULL) {
				http_request::fail(request->id, "Unable to create curl request");
				continue;
			}
		#define HTTP_CURL_OPTION(option, value)                                \
			if (curl._curl_easy_setopt(rr->easy, option, value) != CURLE_OK) { \
				http_request::fail(request->id, "Unable to set " #option);     \
				continue;                                                      \
			}

			HTTP_CURL_OPTION(CURLOPT_URL, request->url.data());
			HTTP_CURL_OPTION(CURLOPT_PRIVATE, rr);
			HTTP_CURL_OPTION(CURLOPT_ERRORBUFFER, rr->diagnostic);
			HTTP_CURL_OPTION(CURLOPT_NOSIGNAL, 1L);
			HTTP_CURL_OPTION(CURLOPT_PROTOCOLS, (long)(CURLPROTO_HTTP | CURLPROTO_HTTPS));
			HTTP_CURL_OPTION(CURLOPT_REDIR_PROTOCOLS, (long)(CURLPROTO_HTTP | CURLPROTO_HTTPS));
			HTTP_CURL_OPTION(CURLOPT_FOLLOWLOCATION, 1L);
			HTTP_CURL_OPTION(CURLOPT_MAXREDIRS, 20L);
			HTTP_CURL_OPTION(CURLOPT_ACCEPT_ENCODING, "");
			HTTP_CURL_OPTION(CURLOPT_WRITEFUNCTION, http_curl_body);
			HTTP_CURL_OPTION(CURLOPT_WRITEDATA, rr);
			HTTP_CURL_OPTION(CURLOPT_HEADERFUNCTION, http_curl_header);
			HTTP_CURL_OPTION(CURLOPT_HEADERDATA, rr);
			HTTP_CURL_OPTION(CURLOPT_TIMEOUT_MS, (long)timeout);
			if (request->post) {
				rr->headers.data = (char *)request->content_type_header.data();
				HTTP_CURL_OPTION(CURLOPT_HTTPHEADER, &rr->headers);
				HTTP_CURL_OPTION(CURLOPT_POST, 1L);
				HTTP_CURL_OPTION(CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)request->upload.length());
				HTTP_CURL_OPTION(CURLOPT_POSTFIELDS
					, request->upload.length() == 0 ? "" : request->upload.data()
					);
			}
		#undef HTTP_CURL_OPTION
			if (curl._curl_multi_add_handle(curl._multi, rr->easy) != CURLM_OK) {
				http_request::fail(request->id, "Unable to start curl request");
				continue;
			}

			rr->added = true;
		}

		int running;
		const CURLMcode result = curl._curl_multi_perform(curl._multi, &running);
		if (result != CURLM_OK) {
			for (u32 ii = 0; ii < curl._num_requests; ++ii)
				http_request::fail(curl._requests[ii]->request.id, "curl multi transfer failed");
		}

		int remaining;
		CURLMsg *message;
		while ((message = curl._curl_multi_info_read(curl._multi, &remaining)) != NULL) {
			if (message->msg != CURLMSG_DONE)
				continue;

			HttpCurlRequest *rr = NULL;
			curl._curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &rr);
			if (http_request::expired(rr->request)) {
				http_request::fail(rr->request.id, "HTTP request timed out");
			} else if (message->data.result != CURLE_OK) {
				http_request::fail(rr->request.id, rr->diagnostic[0] != '\0'
					? rr->diagnostic : curl._curl_easy_strerror(message->data.result));
			} else {
				http_request::complete(rr->request);
			}
		}
		for (u32 ii = 0; ii < curl._num_requests;) {
			HttpCurlRequest &rr = *curl._requests[ii];
			if (curl._exit.load() || !http_request::active(rr.request)) {
				if (rr.added)
					curl._curl_multi_remove_handle(curl._multi, rr.easy);
				if (rr.easy != NULL)
					curl._curl_easy_cleanup(rr.easy);
				curl._requests[ii] = curl._requests[--curl._num_requests];
				http_request::release(&rr.request);
			} else {
				++ii;
			}
		}

		if (curl._exit.load()) {
			curl._mutex.lock();
			const bool empty = curl._num_pending == 0;
			curl._mutex.unlock();
			if (empty)
				break;

			continue;
		}

		if (curl._curl_multi_poll(curl._multi, NULL, 0, INT32_MAX, NULL) != CURLM_OK) {
			for (u32 ii = 0; ii < curl._num_requests; ++ii)
				http_request::fail(curl._requests[ii]->request.id, "curl multi poll failed");
		}
	}

	return 0;
}

namespace http_backend
{
	void *init(Allocator &a)
	{
		HttpCurl *curl = CE_NEW(a, HttpCurl)(a);
		curl->_library = os::library_open("libcurl.so.4");
		if (curl->_library == NULL)
			return curl;
#define HTTP_CURL_LOAD(name)                                                                \
		curl->_##name = (decltype(curl->_##name))os::library_symbol(curl->_library, #name); \
		if (curl->_##name == NULL) {                                                        \
			curl->_error = "Missing libcurl symbol: " #name;                                \
			return curl;                                                                    \
		}

		HTTP_CURL_FUNCTIONS(HTTP_CURL_LOAD)
#undef HTTP_CURL_LOAD
		if (curl->_curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
			curl->_error = "Unable to initialize libcurl";
			return curl;
		}

		curl->_initialized = true;
		if (!(curl->_curl_version_info(CURLVERSION_NOW)->features & CURL_VERSION_ASYNCHDNS)) {
			curl->_error = "System libcurl requires asynchronous DNS support";
			return curl;
		}

		curl->_multi = curl->_curl_multi_init();
		if (curl->_multi == NULL)
			curl->_error = "Unable to initialize curl multi interface";
		else
			curl->_thread.start(http_curl_run, curl);
		return curl;
	}

	void shutdown(void *backend)
	{
		HttpCurl &curl = *(HttpCurl *)backend;
		if (curl._thread.is_running()) {
			curl._exit.store(true);
			http_curl_wakeup(curl);
			curl._thread.stop();
		}

		if (curl._multi != NULL)
			curl._curl_multi_cleanup(curl._multi);
		if (curl._initialized)
			curl._curl_global_cleanup();
		if (curl._library != NULL)
			os::library_close(curl._library);
		CE_DELETE(*curl._allocator, &curl);
	}

	HttpRequest *create(Allocator &a
		, const char *url
		, const char *content_type
		, const StringView *body
		)
	{
		return http_request::create<HttpCurlRequest>(a, url, content_type, body);
	}

	void start(void *backend, HttpRequest *request)
	{
		HttpCurl &curl = *(HttpCurl *)backend;
		if (curl._multi == NULL) {
			http_request::fail(request->id, curl._error);
			http_request::release(request);
			return;
		}

		curl._mutex.lock();
		curl._pending[curl._num_pending++] = request;
		curl._mutex.unlock();
		http_curl_wakeup(curl);
	}

	void cancel(void *backend)
	{
		http_curl_wakeup(*(HttpCurl *)backend);
	}

} // namespace http_backend

} // namespace crown
#endif // if CROWN_PLATFORM_LINUX && !CROWN_HTTP_NOOP
