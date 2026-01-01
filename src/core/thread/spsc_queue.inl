/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/memory/allocator.h"
#include <atomic>

namespace crown
{
/// Single Producer Single Consumer event queue.
/// Used to pass data efficiently between two threads.
/// https://www.irif.fr/~guatto/papers/sbac13.pdf
///
/// @ingroup Core
template<typename T, int MAX_NUM_ITEMS>
struct SPSCQueue
{
	CE_STATIC_ASSERT(is_power_of_2(MAX_NUM_ITEMS));

	CE_ALIGN_DECL(CROWN_CACHE_LINE_SIZE, std::atomic_int _tail);
	CE_ALIGN_DECL(CROWN_CACHE_LINE_SIZE, std::atomic_int _head);
	Allocator *_allocator;
	T *_queue;

	///
	explicit SPSCQueue(Allocator &a)
		: _tail(0)
		, _head(0)
		, _allocator(&a)
		, _queue(NULL)
	{
		_queue = (T *)_allocator->allocate(sizeof(T) * MAX_NUM_ITEMS, alignof(T));
	}

	///
	~SPSCQueue()
	{
		_allocator->deallocate(_queue);
	}

	///
	SPSCQueue(const SPSCQueue &) = delete;

	///
	SPSCQueue &operator=(const SPSCQueue &) = delete;

	///
	bool push(const T &ev)
	{
		const int tail = _tail.load(std::memory_order_relaxed);
		const int head = _head.load(std::memory_order_acquire);
		const int tail_next = (tail + 1) & (MAX_NUM_ITEMS - 1);

		if (CE_UNLIKELY(tail_next == head))
			return false;

		_queue[tail] = ev;
		_tail.store(tail_next, std::memory_order_release);
		return true;
	}

	///
	bool pop(T &ev)
	{
		const int head = _head.load(std::memory_order_relaxed);
		const int tail = _tail.load(std::memory_order_acquire);
		const int head_next = (head + 1) & (MAX_NUM_ITEMS - 1);

		if (CE_UNLIKELY(head == tail))
			return false;

		ev = _queue[head];
		_head.store(head_next, std::memory_order_release);
		return true;
	}

	///
	bool empty()
	{
		const int head = _head.load(std::memory_order_relaxed);
		const int tail = _tail.load(std::memory_order_acquire);
		return head == tail;
	}
};

} // namespace crown
