/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

namespace crown
{
namespace date
{
	struct Date
	{
		s32 year;
		s32 month;
		s32 day;
	};

	struct Time
	{
		s32 hour;
		s32 minutes;
		s32 seconds;
	};

	///
	void date(Date &date);

	///
	void utc_date(Date &date);

	///
	void time(Time &time);

	///
	void utc_time(Time &time);

	/// Formats @a date as YYYY-MM-DD.
	const char *to_string(char *buf, u32 len, const Date &date);

	/// Formats @a time as HH-MM-SS.
	const char *to_string(char *buf, u32 len, const Time &time);

} // namespace date

} // namespace crown
