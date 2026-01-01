/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"
#include "core/strings/types.h"

#if CROWN_PLATFORM_WINDOWS
	#define EXE_PREFIX ""
	#define EXE_SUFFIX ".exe"
#else
	#define EXE_PREFIX "./"
	#define EXE_SUFFIX ""
#endif

#define EXE_PATH(exe) EXE_PREFIX exe EXE_SUFFIX

#define CROWN_PROCESS_STDIN_PIPE   UINT32_C(0x1) ///< Create stdin pipe.
#define CROWN_PROCESS_STDOUT_PIPE  UINT32_C(0x2) ///< Create stdout pipe.
#define CROWN_PROCESS_STDERR_MERGE UINT32_C(0x4) ///< Merge stderr with stdout.

namespace crown
{
/// Process.
///
/// @ingroup Core
struct Process
{
	struct Private *_priv;
	CE_ALIGN_DECL(16, u8 _data[48]);

	///
	Process();

	///
	~Process();

	///
	Process(const Process &) = delete;

	///
	Process &operator=(const Process &) = delete;

	/// Spawns the process described by @a argv with the given @a flags.
	/// Returns 0 on success, non-zero otherwise.
	s32 spawn(const char * const *argv, u32 flags = 0);

	/// Returns whether the process has been spawned
	/// due to a previous successful call to spawn().
	bool spawned();

	/// Focefully terminates the process.
	/// On Linux, this function sends SIGKILL.
	void force_exit();

	/// Waits synchronously for the process to terminate and returns its exit code.
	s32 wait();

	/// Reads at most @a len bytes from the process's output and returns the
	/// pointer to the first byte read.
	/// Returns NULL on EOF or when an error occurs. In case of error, it sets
	/// @a num_bytes_read to UINT32_MAX, otherwise it sets it to the actual
	/// number of bytes read.
	char *read(u32 *num_bytes_read, char *data, u32 len);
};

} // namespace crown
