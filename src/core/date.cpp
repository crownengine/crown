/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/date.h"
#include <stb_sprintf.h>

#if CROWN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#else
	#include <time.h>
#endif

namespace crown
{
namespace date
{
	void date(Date &date)
	{
#if CROWN_PLATFORM_WINDOWS
		SYSTEMTIME now;
		GetLocalTime(&now);
		date.year  = now.wYear;
		date.month = now.wMonth;
		date.day   = now.wDay;
#else
		time_t t;
		struct tm now;
		::time(&t);
		localtime_r(&t, &now);
		date.year  = now.tm_year + 1900;
		date.month = now.tm_mon + 1;
		date.day   = now.tm_mday;
#endif
	}

	void utc_date(Date &date)
	{
#if CROWN_PLATFORM_WINDOWS
		SYSTEMTIME now;
		GetSystemTime(&now);
		date.year  = now.wYear;
		date.month = now.wMonth;
		date.day   = now.wDay;
#else
		time_t t;
		struct tm now;
		::time(&t);
		gmtime_r(&t, &now);
		date.year  = now.tm_year + 1900;
		date.month = now.tm_mon + 1;
		date.day   = now.tm_mday;
#endif
	}

	void time(Time &time)
	{
#if CROWN_PLATFORM_WINDOWS
		SYSTEMTIME now;
		GetLocalTime(&now);
		time.hour    = now.wHour;
		time.minutes = now.wMinute;
		time.seconds = now.wSecond;
#else
		time_t t;
		struct tm now;
		::time(&t);
		localtime_r(&t, &now);
		time.hour    = now.tm_hour;
		time.minutes = now.tm_min;
		time.seconds = now.tm_sec;
#endif
	}

	void utc_time(Time &time)
	{
#if CROWN_PLATFORM_WINDOWS
		SYSTEMTIME now;
		GetSystemTime(&now);
		time.hour    = now.wHour;
		time.minutes = now.wMinute;
		time.seconds = now.wSecond;
#else
		time_t t;
		struct tm now;
		::time(&t);
		gmtime_r(&t, &now);
		time.hour    = now.tm_hour;
		time.minutes = now.tm_min;
		time.seconds = now.tm_sec;
#endif
	}

	const char *to_string(char *buf, u32 len, const Date &date)
	{
		stbsp_snprintf(buf, len, "%04d-%02d-%02d", date.year, date.month, date.day);
		return buf;
	}

	const char *to_string(char *buf, u32 len, const Time &time)
	{
		stbsp_snprintf(buf, len, "%02d-%02d-%02d", time.hour, time.minutes, time.seconds);
		return buf;
	}

} // namespace date

} // namespace crown
