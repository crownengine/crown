/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/memory/types.h"
#include "core/strings/string_view.h"

namespace crown
{
/// Asynchronous HTTP request state.
///
/// @ingroup Network
struct HttpStatus
{
	enum Enum
	{
		FINISHED,    ///< Includes HTTP 4xx/5xx responses.
		ERROR,
		DOWNLOADING, ///< Queued or transferring.
		CLOSED
	};
};

/// A response header.
///
/// @ingroup Network
struct HttpHeader
{
	StringView name;
	StringView value;
};

/// Asynchronous HTTP requests.
///
/// @ingroup Network
namespace http
{
	/// Starts an asynchronous GET. Returns 0 if all request slots are occupied.
	u32 get(const char *url, u32 timeout_ms = 10 *1000);

	/// Starts an asynchronous GET for @a size bytes starting at @a offset.
	/// @a size must be nonzero. Fails on encoded responses or ignored ranges.
	u32 get_range(const char *url, u32 offset, u32 size, u32 timeout_ms = 10 *1000);

	/// Downloads a decoded response directly to @a path. The response body is empty.
	/// Removes incomplete files on failure or cancellation. Overwrites existing files.
	/// On HTML5, @a path names a file in OPFS.
	u32 get_to_file(const char *url, const char *path, u32 timeout_ms = 10 *1000);

	/// Starts an asynchronous POST.
	u32 post(const char *url
		, const char *content_type
		, const StringView &body
		, u32 timeout_ms = 10 *1000
		);

	/// Closes the request @a id and free its memory. If the request is in progress it cancels it.
	void close(u32 id);

	/// Returns the current state of request @a id, or CLOSED if the id is invalid.
	HttpStatus::Enum status(u32 id);

	/// Returns the error string for a request @a id with ERROR status, or NULL if the request has
	/// no errors.
	const char *error(u32 id);

	/// Returns true and the response for FINISHED requests, otherwise false with empty outputs.
	bool response(u32 &status_code
		, StringView &reason
		, const HttpHeader *&headers
		, u32 &num_headers
		, StringView &body
		, u32 id
		);

} // namespace http

namespace http_globals
{
	///
	void init(Allocator &a);

	/// Cancels and drains outstanding requests.
	/// On HTML5, call from the engine thread.
	void shutdown();

} // namespace http_globals

} // namespace crown
