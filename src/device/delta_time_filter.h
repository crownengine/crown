/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

namespace crown
{
/// Produces a smoothed delta time based on a history of previous delta times.
///
/// @ingroup Device
struct DeltaTimeFilter
{
	struct HistoryData
	{
		enum { MAX_NUM_HISTORY = 64 };

		u32 _size;
		u32 _capacity;
		u32 _head;
		s64 _raw[MAX_NUM_HISTORY];
		s64 _sorted[MAX_NUM_HISTORY];
	};

	HistoryData _history;
	s64 _simulated_time;      ///< Total simulated time.
	s64 _start_time;          ///< Time when the simulation started.
	s64 _drift;               ///< Difference between _simulated_time and _start_time.
	u32 _num_outliers;        ///< Number of outliers to discard (both higher and lower).
	s64 _previous_average;    ///< Previous delta time average.
	f32 _average_cap;         ///< Maximum allowed distance from previous average, in percent. Use 1.0 to disable.
	f32 _max_wall_clock_debt; ///< Maximum drift (debt) between simulated time and wall-clock time.
	u32 _num_debt_frames;     ///< Maximum number of frames to pay back debt.
	u32 _debt_frames_left;    ///< Number of frames left to pay back debt.
	s64 _debt_cost;           ///< Debt payback cost.

	///
	DeltaTimeFilter();

	/// Sets the number of samples to be averaged, the outliers to be excluded and the maximum
	/// allowed deviation in percent from the previous average.  Given a delta time series DTs, and
	/// a previous average AVGp:
	///
	/// * DTs = [ .33; .30; .32; .33; .24; .33; .25; .35; .33; .42 ]
	/// * AVGp = 0.33
	///
	/// A call to set_smoothing(10, 2, 0.1) would compute the new average like so:
	///
	/// 1. Remove the 2 minimum and maximum values in DTs;
	/// 2. Compute the average for the remaining 6 values in DTs: AVG = 0.323.
	/// 3. Smooth the newly computed average: AVG = lerp(AVGp, AVG, 0.1) = 0.329.
	void set_smoothing(u32 num_samples, u32 num_outliers, f32 average_cap);

	/// Returns a smoothed delta time based on @a dt and a history of previous delta times.
	f32 filter(s64 dt);
};

} // namespace crown
