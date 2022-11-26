/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
 * License: https://github.com/crownengine/crown/blob/master/LICENSE
 */

#pragma once

#include "core/thread/spsc_queue.inl"
#include "core/thread/scoped_mutex.inl"

namespace crown
{
/// Multi Producer Single Consumer event queue.
/// Used to pass data efficiently between multiple producers and a consumer thread.
///
/// @ingroup Core
template<typename T, int MAX_NUM_ITEMS>
struct MPSCQueue
{
	SPSCQueue<T, MAX_NUM_ITEMS> _queue;
	Mutex _mutex;

	///
	explicit MPSCQueue(Allocator &a)
		: _queue(a)
	{
	}

	///
	MPSCQueue(const MPSCQueue &) = delete;

	///
	MPSCQueue &operator=(const MPSCQueue &) = delete;

	///
	bool push(const T &ev)
	{
		ScopedMutex sm(_mutex);
		return _queue.push(ev);
	}

	///
	bool pop(T &ev)
	{
		ScopedMutex sm(_mutex);
		return _queue.pop(ev);
	}
};

} // namespace crown
