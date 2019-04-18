/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/strings/types.h"

namespace crown
{
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

	/// Spawns the process described by @a argv.
	/// Returns 0 on success, non-zero otherwise.
	s32 spawn(const char* const* argv);

	/// Waits for the process to terminate and returns its exit code.
	/// If @a output is not NULL it reads stdout and stderr into
	/// the StringStream.
	s32 wait(StringStream* output);
};

} // namespace crown
