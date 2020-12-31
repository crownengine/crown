/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/strings/types.h"

#if CROWN_PLATFORM_POSIX
	#define EXE_PREFIX "./"
	#define EXE_SUFFIX ""
#elif CROWN_PLATFORM_WINDOWS
	#define EXE_PREFIX ""
	#define EXE_SUFFIX ".exe"
#else
	#error "Unknown platform"
#endif // CROWN_PLATFORM_POSIX

#define EXE_PATH(exe) EXE_PREFIX exe EXE_SUFFIX

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
	struct Private* _priv;
	CE_ALIGN_DECL(16, u8 _data[48]);

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

	/// Reads at most @a len bytes from the process' output and returns the
	/// pointer to the first byte read.
	/// Returns NULL on EOF or when an error occurs. In case of error, it sets
	/// @a num_bytes_read to UINT32_MAX, otherwise it sets it to the actual
	/// number of bytes read.
	char* read(u32* num_bytes_read, char* data, u32 len);
};

} // namespace crown
