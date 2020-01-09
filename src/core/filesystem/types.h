/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
