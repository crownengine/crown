/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "input_types.h"

namespace crown
{
/// @defgroup Input Input

/// Global input-related functions.
///
/// @ingroup Input
namespace input_globals
{
	/// Initializes the input system.
	void init();

	/// Shutdowns the input system.
	void shutdown();

	/// Updates input devices.
	void update();

	/// Returns the default keyboard.
	Keyboard& keyboard();

	/// Returns the default mouse.
	Mouse& mouse();

	/// Rettuns the default touch panel.
	Touch& touch();
} // namespace input_globals
} // namespace crown
