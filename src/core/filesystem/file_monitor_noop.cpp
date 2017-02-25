/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "platform.h"

#if !CROWN_PLATFORM_LINUX

#include "file_monitor.h"

namespace crown
{
FileMonitor::FileMonitor(Allocator& /*a*/)
	: _impl(NULL)
{
}

FileMonitor::~FileMonitor()
{
}

void FileMonitor::start(const char* /*path*/, bool /*recursive*/, FileMonitorFunction /*fmf*/, void* /*user_data*/)
{
}

void FileMonitor::stop()
{
}

} // namespace crown

#endif // !CROWN_PLATFORM_LINUX

