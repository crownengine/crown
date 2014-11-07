/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "os_event_queue.h"

#pragma once

namespace crown
{

bool next_event(OsEvent& ev);
bool process_events();

} // namespace crown
