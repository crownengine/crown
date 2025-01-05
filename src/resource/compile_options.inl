/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/reader_writer.inl"
#include "resource/compile_options.h"

#if CROWN_CAN_COMPILE
#define DATA_COMPILER_ASSERT(condition, opts, msg, ...) \
	do                                                  \
	{                                                   \
		if (!(condition))                               \
		{                                               \
			opts.error(msg, ## __VA_ARGS__);            \
			return -1;                                  \
		}                                               \
	} while (0)

#define DATA_COMPILER_ENSURE(condition, opts)             \
	DATA_COMPILER_ASSERT(condition                        \
		, opts                                            \
		, "DATA_COMPILER_ENSURE(" # condition ") failed." \
		)

#define DATA_COMPILER_ASSERT_RESOURCE_EXISTS(type, name, opts) \
	DATA_COMPILER_ASSERT(opts.resource_exists(type, name)      \
		, opts                                                 \
		, opts._server                                         \
		? "Resource not found: " RESOURCE_ID_FMT_STR_PAIR      \
		: "Resource not found: %s.%s"                          \
		, name                                                 \
		, type                                                 \
		)

#define DATA_COMPILER_ASSERT_FILE_EXISTS(name, opts) \
	DATA_COMPILER_ASSERT(opts.file_exists(name)      \
		, opts                                       \
		, opts._server                               \
		? "File not found: #FILE(%s)"                \
		: "File not found: %s"                       \
		, name                                       \
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
