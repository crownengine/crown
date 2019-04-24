/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/strings/types.h"

namespace crown
{
/// ProcessFlags
///
/// @ingroup Core
struct ProcessFlags
{
	enum Enum
	{
		STDIN_PIPE   = 1 << 0, ///< Create stdin pipe.
		STDOUT_PIPE  = 1 << 1, ///< Create stdout pipe.
		STDERR_MERGE = 1 << 2  ///< Merge stderr with stdout.
	};
};

/// Process.
///
/// @ingroup Core
struct Process
{
	CE_ALIGN_DECL(16, u8 _data[32]);

	///
	Process();

	///
	~Process();

	/// Spawns the process described by @a argv with the given @a flags.
	/// Returns 0 on success, non-zero otherwise.
	s32 spawn(const char* const* argv, u32 flags = 0);

	/// Returns whether the process has been spawned
	/// due to a previous successful call to spawn().
	bool spawned();

	/// Focefully terminates the process.
	/// On Linux, this function sends SIGKILL.
	void force_exit();

	/// Waits synchronously for the process to terminate and returns its exit code.
	s32 wait();

	///
	char* fgets(char* data, u32 len);
};

} // namespace crown
