/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/memory/memory.inl"
#include "core/network/http_internal.h"
#include "core/strings/string.inl"
#include "core/thread/scoped_mutex.inl"
#include "core/time.h"
#include <string.h>
#include <stb_sprintf.h>

namespace crown
{
struct HttpSlot
{
	u32 id;
	std::atomic<HttpStatus::Enum> state;
	bool backend_active;
	HttpResponse *response;
	char diagnostic[512];

	HttpSlot()
		: id(0)
		, state(HttpStatus::CLOSED)
		, backend_active(false)
		, response(NULL)
	{
	}
};

struct Http
{
	Allocator *_allocator;
	Mutex _mutex;
	void *_backend;
	HttpSlot _slots[HTTP_MAX_REQUESTS];
	u32 _next_id;

	explicit Http(Allocator &a)
		: _allocator(&a)
		, _backend(NULL)
		, _next_id(0)
	{
	}

	HttpSlot *find(u32 id)
	{
		for (u32 ii = 0; ii < countof(_slots); ++ii) {
			if (_slots[ii].id == id)
				return &_slots[ii];
		}

		return NULL;
	}
};

static Http *s_http;

static void http_error(HttpSlot &slot, const char *message)
{
	strncpy(slot.diagnostic, message, sizeof(slot.diagnostic) - 1);
	slot.diagnostic[sizeof(slot.diagnostic) - 1] = '\0';
	slot.state = HttpStatus::ERROR;
}

namespace http_response
{
	struct HeaderOffset
	{
		u32 name;
		u32 name_size;
		u32 value_size;
	};

	static const u32 MAX_SIZE = UINT32_MAX / 2;
	CE_STATIC_ASSERT(sizeof(HeaderOffset) <= sizeof(HttpHeader));
	CE_STATIC_ASSERT(alignof(HttpResponse) >= alignof(HttpHeader));

	static char *descriptors(HttpResponse &response)
	{
		return (char *)(&response + 1);
	}

	static char *data(HttpResponse &response)
	{
		return descriptors(response) + response.header_capacity * sizeof(HttpHeader);
	}

	static bool reserve(HttpResponse *&response, Allocator &a, u32 num_headers, u64 data_size)
	{
		if (sizeof(HttpResponse) + (u64)num_headers * sizeof(HttpHeader) + data_size > MAX_SIZE)
			return false;

		if (response != NULL
			&& num_headers <= response->header_capacity
			&& data_size <= response->data_capacity)
			return true;

		u64 header_capacity = response == NULL ? 8 : response->header_capacity;
		u64 data_capacity = response == NULL ? 4096 : response->data_capacity;
		if (num_headers > header_capacity)
			header_capacity = max((u64)num_headers, header_capacity * 2);
		if (data_size > data_capacity)
			data_capacity = max(data_size, data_capacity * 2);
		u64 size = sizeof(HttpResponse) + header_capacity * sizeof(HttpHeader) + data_capacity;
		if (size > MAX_SIZE) {
			header_capacity = num_headers;
			data_capacity = data_size;
			size = sizeof(HttpResponse) + header_capacity * sizeof(HttpHeader) + data_capacity;
		}

		void *memory = a.allocate((u32)size, alignof(HttpResponse));
		HttpResponse *next = new (memory) HttpResponse(response == NULL
			? HttpResponse()
			: *response
			);
		next->header_capacity = (u32)header_capacity;
		next->data_capacity = (u32)data_capacity;
		if (response != NULL) {
			memcpy(descriptors(*next)
				, descriptors(*response)
				, response->num_headers * sizeof(HttpHeader)
				);
			const u32 used = response->reason_size + response->header_size + response->body_size;
			memcpy(data(*next), data(*response), used);
			a.deallocate(response);
		}

		response = next;
		return true;
	}

	HttpResponse *create(Allocator &a)
	{
		HttpResponse *response = NULL;
		reserve(response, a, 0, 0);
		return response;
	}

	bool reason(HttpResponse *&response, Allocator &a, const char *text, u32 size)
	{
		if (!reserve(response, a, 0, size))
			return false;

		if (size != 0)
			memcpy(data(*response), text, size);
		response->reason_size = size;
		return true;
	}

	bool header(HttpResponse *&response
		, Allocator &a
		, const char *name
		, u32 name_size
		, const char *value
		, u32 value_size
		)
	{
		const u64 data_size = (u64)response->reason_size + response->header_size
			+ name_size + value_size + response->body_size
			;
		if (!reserve(response, a, response->num_headers + 1, data_size))
			return false;

		HeaderOffset h = { response->header_size, name_size, value_size };
		char *bytes = data(*response) + response->reason_size + response->header_size;
		memmove(bytes + name_size + value_size, bytes, response->body_size);
		if (name_size != 0)
			memcpy(bytes, name, name_size);
		if (value_size != 0)
			memcpy(bytes + name_size, value, value_size);
		memcpy(descriptors(*response) + response->num_headers * sizeof(HttpHeader), &h, sizeof(h));
		++response->num_headers;
		response->header_size += name_size + value_size;
		return true;
	}

	bool header_line(HttpResponse *&response, Allocator &a, const char *line, u32 size)
	{
		while (size != 0 && (line[size - 1] == '\r' || line[size - 1] == '\n'))
			--size;
		if (size >= 5 && memcmp(line, "HTTP/", 5) == 0) {
			response->status_code = 0;
			response->num_headers = 0;
			response->reason_size = 0;
			response->header_size = 0;
			response->body_size = 0;
			const char *end = line + size;
			const char *code = crown::find(StringView(line, size), ' ');
			if (code == NULL)
				return false;

			while (code < end && *code == ' ')
				++code;
			if (end - code < 3)
				return false;

			for (u32 ii = 0; ii < 3; ++ii) {
				if (code[ii] < '0' || code[ii] > '9')
					return false;

				response->status_code = response->status_code * 10 + (u32)(code[ii] - '0');
			}

			code += 3;
			while (code < end && *code == ' ')
				++code;
			return reason(response, a, code, (u32)(end - code));
		}

		if (size == 0)
			return true;

		const char *colon = crown::find(StringView(line, size), ':');
		if (colon == NULL || colon == line)
			return false;

		const char *value = colon + 1;
		const char *end = line + size;
		while (value < end && (*value == ' ' || *value == '\t'))
			++value;
		while (end > value && (end[-1] == ' ' || end[-1] == '\t'))
			--end;
		return header(response, a, line, (u32)(colon - line), value, (u32)(end - value));
	}

	StringView header_value(HttpResponse &response, const char *name)
	{
		const u32 length = strlen32(name);
		for (u32 ii = 0; ii < response.num_headers; ++ii) {
			HeaderOffset h;
			memcpy(&h, descriptors(response) + ii * sizeof(HttpHeader), sizeof(h));
			const char *text = data(response) + response.reason_size + h.name;
			if (h.name_size == length && crown::strncasecmp(text, name, length) == 0)
				return StringView(text + h.name_size, h.value_size);
		}
		return StringView();
	}

	void finish(HttpResponse &response)
	{
		char *bytes = data(response);
		response.headers = (HttpHeader *)descriptors(response);
		for (u32 ii = 0; ii < response.num_headers; ++ii) {
			HeaderOffset offset;
			memcpy(&offset, descriptors(response) + ii * sizeof(HttpHeader), sizeof(offset));
			HttpHeader *h = new (response.headers + ii) HttpHeader;
			h->name = StringView(bytes + response.reason_size + offset.name, offset.name_size);
			h->value = StringView(h->name.data() + offset.name_size, offset.value_size);
		}
	}

} // namespace http_response

namespace http_request
{
	void *create(HttpRequest &request
		, Allocator &a
		, u32 struct_size
		, u32 alignment
		, const char *url
		, const char *content_type
		, const StringView *body
		)
	{
		static const char CONTENT_TYPE[] = "Content-Type: ";
		const u32 prefix_size = body != NULL ? sizeof(CONTENT_TYPE) - 1 : 0;
		const size_t url_size = strlen(url);
		const size_t type_size = body != NULL ? strlen(content_type) : 0;
		const u32 body_size = body != NULL ? body->length() : 0;
		const u64 total_size = (u64)struct_size + url_size + prefix_size
			+ type_size + body_size + 2
			;
		if (url_size > UINT32_MAX / 2 || type_size > UINT32_MAX / 2 || total_size > UINT32_MAX / 2)
			return NULL;

		const u32 size = (u32)total_size;
		void *memory = a.allocate(size, alignment);
		request = HttpRequest();
		request.allocator = &a;
		char *data = (char *)memory + struct_size;
		memcpy(data, url, url_size + 1);
		request.url = StringView(data, (u32)url_size);
		data += url_size + 1;
		request.content_type_header = StringView(data, (u32)(prefix_size + type_size));
		if (prefix_size != 0)
			memcpy(data, CONTENT_TYPE, prefix_size);
		data += prefix_size;
		if (type_size != 0)
			memcpy(data, content_type, type_size);
		data[type_size] = '\0';
		request.content_type = StringView(data, (u32)type_size);
		data += type_size + 1;
		if (body_size != 0)
			memcpy(data, body->data(), body_size);
		request.upload = StringView(data, body_size);
		request.post = body != NULL;
		return memory;
	}

	bool expired(const HttpRequest &request)
	{
		return request.timeout_ms > 0
			&& time::seconds(time::now() - request.submitted) * 1000.0 >= request.timeout_ms;
	}

	s32 remaining_ms(const HttpRequest &request)
	{
		if (request.timeout_ms == 0)
			return 0;

		const f64 elapsed_ms = time::seconds(time::now() - request.submitted) * 1000.0;
		const f64 remaining = request.timeout_ms - elapsed_ms;
		if (remaining <= 0.0)
			return -1;

		if (remaining >= INT32_MAX)
			return INT32_MAX;

		return (s32)remaining + 1;
	}

	void fail(u32 id, const char *message)
	{
		ScopedMutex sm(s_http->_mutex);
		HttpSlot *slot = s_http->find(id);
		if (slot != NULL && slot->state == HttpStatus::DOWNLOADING)
			http_error(*slot, message);
	}

	bool headers(HttpRequest &request)
	{
		HttpResponse &response = *request.response;
		if (request.range_size != 0) {
			const StringView encoding = http_response::header_value(response, "Content-Encoding");
			if ((response.status_code != 206 && response.status_code != 416)
				|| (encoding.length() != 0 && encoding != StringView("identity"))) {
				fail(request.id, "Server does not support uncompressed byte ranges");
				return false;
			}
			return http_response::reserve(request.response, *request.allocator, response.num_headers
				, (u64)response.reason_size + response.header_size + request.range_size
				);
		}
		if (request.path != NULL && response.status_code != 200) {
			fail(request.id, "Cannot download file: expected HTTP 200");
			return false;
		}
		return true;
	}

	bool body(HttpRequest &request, const char *data, u32 size)
	{
		if (request.range_size != 0 && (u64)request.response->body_size + size > request.range_size) {
			fail(request.id, "HTTP response exceeds requested range");
			return false;
		}
		HttpResponse *&response = request.response;
		const u64 used = (u64)response->reason_size + response->header_size + response->body_size;
		if (used + size > response->data_capacity
			&& !http_response::reserve(response, *request.allocator, response->num_headers, used + size))
			return false;
		memcpy(http_response::data(*response) + used, data, size);
		response->body_size += size;
		return true;
	}

	bool complete(HttpRequest &request)
	{
		ScopedMutex sm(s_http->_mutex);
		HttpSlot *slot = s_http->find(request.id);
		if (slot == NULL || slot->state != HttpStatus::DOWNLOADING)
			return false;
		http_response::finish(*request.response);
		slot->response = request.response;
		slot->state = HttpStatus::FINISHED;
		request.response = NULL;
		return true;
	}

	void release(HttpRequest *request)
	{
		const u32 id = request->id;
		request->allocator->deallocate(request->path);
		request->allocator->deallocate(request->response);
		request->allocator->deallocate(request);
		ScopedMutex sm(s_http->_mutex);
		HttpSlot *slot = s_http->find(id);
		slot->backend_active = false;
		if (slot->state == HttpStatus::CLOSED)
			slot->id = 0;
	}

} // namespace http_request

static u32 http_submit(const char *url
	, const char *content_type
	, const StringView *body
	, u32 timeout_ms
	, u32 offset = 0
	, u32 size = 0
	, const char *path = NULL
	)
{
	const s64 submitted = time::now();
	HttpRequest *request = NULL;
	u32 id;
	s_http->_mutex.lock();
	HttpSlot *slot = NULL;
	for (u32 ii = 0; ii < countof(s_http->_slots); ++ii) {
		if (s_http->_slots[ii].id == 0) {
			slot = &s_http->_slots[ii];
			break;
		}
	}

	if (slot == NULL) {
		s_http->_mutex.unlock();
		return 0;
	}

	do {
		id = ++s_http->_next_id;
	} while (id == 0 || s_http->find(id) != NULL);
	slot->id = id;
	slot->state = HttpStatus::DOWNLOADING;
	if (url == NULL
		|| (!str_has_prefix(url, "http://") && !str_has_prefix(url, "https://"))
		|| crown::find(StringView(url), '\r') != NULL || crown::find(StringView(url), '\n') != NULL) {
		http_error(*slot, "Expected an absolute HTTP or HTTPS URL");
	} else if (body != NULL && (content_type == NULL || content_type[0] == '\0'
		|| crown::find(StringView(content_type), '\r') != NULL || crown::find(StringView(content_type), '\n') != NULL)) {
		http_error(*slot, "Invalid HTTP content type");
	} else {
		request = http_backend::create(*s_http->_allocator, url, content_type, body);
		if (request == NULL)
			http_error(*slot, "HTTP request is too large");
	}

	if (request != NULL) {
		if (size != 0)
			stbsp_snprintf(request->range, sizeof(request->range), "bytes=%u-%u", offset, offset + size - 1);
		request->range_size = size;
		if (path != NULL) {
			const u32 length = strlen32(path) + 1;
			request->path = (char *)request->allocator->allocate(length);
			memcpy(request->path, path, length);
		}
		request->id = id;
		request->state = &slot->state;
		request->timeout_ms = timeout_ms;
		request->submitted = submitted;
		slot->backend_active = true;
	}

	s_http->_mutex.unlock();
	if (request != NULL)
		http_backend::start(s_http->_backend, request);
	return id;
}

namespace http
{
	u32 get(const char *url, u32 timeout_ms)
	{
		return http_submit(url, NULL, NULL, timeout_ms);
	}

	u32 get_range(const char *url, u32 offset, u32 size, u32 timeout_ms)
	{
		CE_ASSERT(size != 0 && offset <= UINT32_MAX - (size - 1), "Invalid HTTP byte range");
		return http_submit(url, NULL, NULL, timeout_ms, offset, size);
	}

	u32 get_to_file(const char *url, const char *path, u32 timeout_ms)
	{
		CE_ENSURE(path != NULL && path[0] != '\0');
		return http_submit(url, NULL, NULL, timeout_ms, 0, 0, path);
	}

	u32 post(const char *url, const char *content_type, const StringView &body, u32 timeout_ms)
	{
		return http_submit(url, content_type, &body, timeout_ms);
	}

	void close(u32 id)
	{
		bool cancel = false;
		s_http->_mutex.lock();
		HttpSlot *slot = s_http->find(id);
		if (slot != NULL && slot->state != HttpStatus::CLOSED) {
			slot->state = HttpStatus::CLOSED;
			s_http->_allocator->deallocate(slot->response);
			slot->response = NULL;
			cancel = slot->backend_active;
			if (!cancel)
				slot->id = 0;
		}

		s_http->_mutex.unlock();
		if (cancel)
			http_backend::cancel(s_http->_backend);
	}

	HttpStatus::Enum status(u32 id)
	{
		ScopedMutex sm(s_http->_mutex);
		HttpSlot *slot = s_http->find(id);
		return slot == NULL ? HttpStatus::CLOSED : slot->state.load(std::memory_order_relaxed);
	}

	const char *error(u32 id)
	{
		ScopedMutex sm(s_http->_mutex);
		HttpSlot *slot = s_http->find(id);
		return slot != NULL && slot->state == HttpStatus::ERROR ? slot->diagnostic : NULL;
	}

	bool response(u32 &status_code
		, StringView &reason
		, const HttpHeader *&headers
		, u32 &num_headers
		, StringView &body
		, u32 id
		)
	{
		status_code = 0;
		reason = StringView();
		headers = NULL;
		num_headers = 0;
		body = StringView();
		ScopedMutex sm(s_http->_mutex);
		HttpSlot *slot = s_http->find(id);
		if (slot == NULL || slot->state != HttpStatus::FINISHED)
			return false;

		HttpResponse &response = *slot->response;
		const char *data = http_response::data(response);
		status_code = response.status_code;
		reason = StringView(data, response.reason_size);
		headers = response.headers;
		num_headers = response.num_headers;
		body = StringView(data + response.reason_size + response.header_size, response.body_size);
		return true;
	}

} // namespace http

namespace http_globals
{
	void init(Allocator &a)
	{
		s_http = CE_NEW(a, Http)(a);
		s_http->_backend = http_backend::init(a);
	}

	void shutdown()
	{
		s_http->_mutex.lock();
		for (u32 ii = 0; ii < countof(s_http->_slots); ++ii) {
			HttpSlot &slot = s_http->_slots[ii];
			slot.state = HttpStatus::CLOSED;
			s_http->_allocator->deallocate(slot.response);
			slot.response = NULL;
		}

		s_http->_mutex.unlock();
		http_backend::shutdown(s_http->_backend);
		Allocator *a = s_http->_allocator;
		CE_DELETE(*a, s_http);
		s_http = NULL;
	}

} // namespace http_globals

} // namespace crown
