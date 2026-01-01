/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/math/math.inl"
#include "core/time.h"
#include "device/delta_time_filter.h"
#include <algorithm> // std::sort
#include <string.h>  // memset

namespace crown
{
DeltaTimeFilter::DeltaTimeFilter()
	: _simulated_time(0u)
	, _drift(0u)
	, _num_outliers(2)
	, _previous_average(0.0f)
	, _average_cap(0.1f)
	, _max_wall_clock_debt(0.010f)
	, _num_debt_frames(0u)
	, _debt_frames_left(0u)
	, _debt_cost(0.0f)
{
	_start_time = time::now();

	_history._size = 0;
	_history._capacity = 12;
	_history._head = 0;

	memset(&_history._raw, 0, sizeof(_history._raw));
	memset(&_history._sorted, 0, sizeof(_history._sorted));
}

void DeltaTimeFilter::set_smoothing(u32 num_samples, u32 num_outliers, f32 average_cap)
{
	_history._size = 0;
	_history._capacity = min(num_samples, (u32)HistoryData::MAX_NUM_HISTORY);
	_history._head = 0;

	_num_outliers = num_outliers;
	_average_cap = average_cap;
}

/// Returns a new smoothed delta time.
f32 DeltaTimeFilter::filter(s64 dt)
{
	// How much time we simulated versus how much time has actually passed.
	// If positive we simulated more time than actually passed.
	_drift = _simulated_time - (time::now() - _start_time);

	//         +-----+-----+-----+-----+-----+
	// Frame:  |  0  |  1  |  2  | n-1 |  n  |
	//         +-----+-----+-----+-----+-----+
	// WCt:    0    33    66    99   132   165
	// SIMt:   0     0    33    66    99   132
	// Drift:  0   -33   -33   -33   -33   -33

	_history._raw[_history._head] = dt;
	_history._head = (_history._head + 1) % _history._capacity;
	_history._size = min(_history._size + 1, (u32)_history._capacity);

	// Compute the new delta time based on historic data.
	s64 avg = dt;

	if (_history._size > 2*_num_outliers) {
		// Sort circular buffer data.
		for (u32 i = 0; i < _history._size; ++i) {
			const u32 c = (_history._head - i - 1) % _history._capacity;
			_history._sorted[i] = _history._raw[c];
		}
		std::sort(_history._sorted, _history._sorted + _history._size);

		// Compute average excluding outliers.
		const s64 *begin = _history._sorted + _num_outliers;
		const s64 *end   = _history._sorted + _history._size - _num_outliers;
		for (const s64 *cur = begin + 1; cur != end; ++cur)
			avg += *cur;
		avg /= s64(end - begin);

		// Limit distance from previous average.
		avg = lerp(f32(_previous_average), f32(avg), _average_cap);
	}

	_previous_average = avg;

	// Try to pay back time debt over _num_debt_frames.
	if (_num_debt_frames > 0) {
		if (_debt_frames_left == 0 && fabs(time::seconds(_drift)) >= _max_wall_clock_debt) {
			_debt_frames_left = _num_debt_frames;
			_debt_cost = _drift / _num_debt_frames;
		}

		if (_debt_frames_left > 0) {
			--_debt_frames_left;
			avg -= _debt_cost;
			_drift -= _debt_cost;
		}
	}

	_simulated_time += avg;
	return time::seconds(avg);
}

} // namespace crown
