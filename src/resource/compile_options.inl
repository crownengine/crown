/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/filesystem/reader_writer.inl"
#include "resource/compile_options.h"
#include "resource/resource_id.inl"

#define RETURN_IF_FALSE(system, condition, opts, msg, ...) \
	do                                                     \
	{                                                      \
		if (!(condition))                                  \
		{                                                  \
			opts.error(system, msg, ## __VA_ARGS__);       \
			return -1;                                     \
		}                                                  \
	} while (0)

#define ENSURE_OR_RETURN(system, condition, opts) \
	RETURN_IF_FALSE(system                        \
		, condition                               \
		, opts                                    \
		, # condition " failed."                  \
		)

#define RETURN_IF_MISSING(system, type, name, opts)       \
	RETURN_IF_FALSE(system                                \
		, opts.resource_exists(type, name)                \
		, opts                                            \
		, opts._server                                    \
		? "Resource not found: " RESOURCE_ID_FMT_STR_PAIR \
		: "Resource not found: %s.%s"                     \
		, name                                            \
		, type                                            \
		)

#define WARN_IF_MISSING(system, type, name, opts)                 \
	do                                                            \
	{                                                             \
		if (!opts.resource_exists(type, name))                    \
		{                                                         \
			opts.warning(system                                   \
				, opts._server                                    \
				? "Resource not found: " RESOURCE_ID_FMT_STR_PAIR \
				: "Resource not found: %s.%s"                     \
				, name                                            \
				, type                                            \
				);                                                \
		}                                                         \
	} while (0)

#define RETURN_IF_FILE_MISSING(system, name, opts) \
	RETURN_IF_FALSE(system                         \
		, opts.file_exists(name)                   \
		, opts                                     \
		, opts._server                             \
		? "File not found: #FILE(%s)"              \
		: "File not found: %s"                     \
		, name                                     \
		)

namespace crown
{
///
template<typename T>
void CompileOptions::write(const T &data)
{
	_binary_writer.write(data);
}

} // namespace crown

#endif // if CROWN_CAN_COMPILE
