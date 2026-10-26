/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"
#include "core/functional.h"
#include "core/network/http.h"
#include "core/strings/string_view.inl"
#include <atomic>
#include <new>

namespace crown
{
static const u32 HTTP_MAX_REQUESTS = 64;

struct HttpResponse
{
	u32 status_code;
	u32 num_headers;
	HttpHeader *headers;
	u32 header_capacity;
	u32 data_capacity;
	u32 reason_size;
	u32 header_size;
	u32 body_size;
};

struct HttpRequest
{
	Allocator *allocator;
	HttpResponse *response;
	const std::atomic<HttpStatus::Enum> *state;
	u32 id;
	u32 timeout_ms;
	s64 submitted;
	StringView url;
	StringView content_type_header;
	StringView content_type;
	StringView upload;
	bool post;
};

namespace http_response
{
	HttpResponse *create(Allocator &a);

	bool reason(HttpResponse *&response, Allocator &a, const char *text, u32 size);

	bool header(HttpResponse *&response
		, Allocator &a
		, const char *name
		, u32 name_size
		, const char *value
		, u32 value_size
		);

	bool header_line(HttpResponse *&response, Allocator &a, const char *line, u32 size);

	void finish(HttpResponse &response);

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
		);

	template<typename T>
	HttpRequest *create(Allocator &a
		, const char *url
		, const char *content_type
		, const StringView *body
		)
	{
		CE_STATIC_ASSERT(offsetof(T, request) == 0);
		HttpRequest request;
		void *memory = create(request, a, sizeof(T), alignof(T), url, content_type, body);
		if (memory == NULL)
			return NULL;

		T *backend = new (memory) T();
		backend->request = request;
		return &backend->request;
	}

	void fail(u32 id, const char *message);

	bool body(HttpRequest &request, const char *data, u32 size);

	void complete(HttpRequest &request);

	void release(HttpRequest *request);

	inline bool active(const HttpRequest &request)
	{
		return request.state->load(std::memory_order_relaxed) == HttpStatus::DOWNLOADING;
	}

	bool expired(const HttpRequest &request);

	s32 remaining_ms(const HttpRequest &request);

} // namespace http_request

namespace http_backend
{
	void *init(Allocator &a);

	void shutdown(void *backend);

	HttpRequest *create(Allocator &a
		, const char *url
		, const char *content_type
		, const StringView *body
		);

	void start(void *backend, HttpRequest *request);

	void cancel(void *backend);

} // namespace http_backend

} // namespace crown
