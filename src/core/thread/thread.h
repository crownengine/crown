/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

/// @defgroup Thread Thread
/// @ingroup Core
namespace crown
{
typedef s32 (*ThreadFunction)(void* data);

/// Thread.
///
/// @ingroup Thread.
struct Thread
{
	struct Private* _priv;
	CE_ALIGN_DECL(16, u8 _data[256]);

	///
	Thread();

	///
	~Thread();

	///
	Thread(const Thread&) = delete;

	///
	Thread& operator=(const Thread&) = delete;

	///
	void start(ThreadFunction func, void* user_data = NULL, u32 stack_size = 0);

	///
	void stop();

	///
	bool is_running();

	///
	s32 exit_code();
};

} // namespace crown
