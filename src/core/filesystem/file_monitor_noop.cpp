/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if !CROWN_PLATFORM_LINUX && !CROWN_PLATFORM_WINDOWS
#include "core/filesystem/file_monitor.h"

namespace crown
{
FileMonitor::FileMonitor(Allocator & /*a*/)
	: _impl(NULL)
{
}

FileMonitor::~FileMonitor()
{
}

void FileMonitor::start(u32 /*num*/, const char ** /*path*/, bool /*recursive*/, FileMonitorFunction /*fmf*/, void * /*user_data*/)
{
}

void FileMonitor::stop()
{
}

} // namespace crown

#endif // if !CROWN_PLATFORM_LINUX && !CROWN_PLATFORM_WINDOWS
