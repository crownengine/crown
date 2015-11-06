/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "filesystem_types.h"
#include "device_options.h"

namespace crown
{
	/// Initializes the engine.
	bool init(DeviceOptions& opts);

	/// Updates all the subsystems.
	void update();

	/// Shutdowns the engine.
	void shutdown();
} // namespace crown
