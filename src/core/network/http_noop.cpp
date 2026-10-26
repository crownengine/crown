/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_HTTP_NOOP
#include "core/network/http_internal.h"

namespace crown
{
namespace http_backend
{
	void *init(Allocator &a)
	{
		CE_UNUSED(a);
		return NULL;
	}

	void shutdown(void *backend)
	{
		CE_UNUSED(backend);
	}

	HttpRequest *create(Allocator &a
		, const char *url
		, const char *content_type
		, const StringView *body
		)
	{
		HttpRequest request;
		void *memory = http_request::create(request
			, a
			, sizeof(HttpRequest)
			, alignof(HttpRequest)
			, url
			, content_type
			, body
			);
		return memory == NULL ? NULL : new (memory) HttpRequest(request);
	}

	void start(void *backend, HttpRequest *request)
	{
		CE_UNUSED(backend);
		http_request::fail(request->id, "HTTP is not supported on this platform");
		http_request::release(request);
	}

	void cancel(void *backend)
	{
		CE_UNUSED(backend);
	}

} // namespace http_backend

} // namespace crown
#endif // if CROWN_HTTP_NOOP
