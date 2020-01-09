/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/platform.h"

#if !CROWN_PLATFORM_LINUX

#include "core/filesystem/file_monitor.h"

namespace crown
{
FileMonitor::FileMonitor(Allocator& /*a*/)
	: _impl(NULL)
{
}

FileMonitor::~FileMonitor()
{
}

void FileMonitor::start(u32 /*num*/, const char** /*path*/, bool /*recursive*/, FileMonitorFunction /*fmf*/, void* /*user_data*/)
{
}

void FileMonitor::stop()
{
}

} // namespace crown

#endif // !CROWN_PLATFORM_LINUX

