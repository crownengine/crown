/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/platform.h"

#if !CROWN_PLATFORM_LINUX && !CROWN_PLATFORM_WINDOWS

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

#endif // !CROWN_PLATFORM_LINUX && !CROWN_PLATFORM_WINDOWS
