/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

/// @defgroup Filesystem Filesystem
/// @ingroup Core
namespace crown
{
struct File;
struct FileMonitor;
struct Filesystem;

/// Enumerates file open modes.
///
/// @ingroup Filesystem
struct FileOpenMode
{
	enum Enum
	{
		READ,
		WRITE
	};
};

} // namespace crown
