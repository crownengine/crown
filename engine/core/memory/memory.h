/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "assert.h"
#include "allocator.h"
#include "macros.h"
#include <new>

namespace crown
{

Allocator& default_allocator();

/// @defgroup Memory Memory
namespace memory
{
	/// Value used to fill unused memory
	const uint32_t PADDING_VALUE = 0xFFFFFFFFu;

	/// Returns the pointer @a p aligned to the desired @a align byte
	inline void* align_top(void* p, size_t align)
	{
		CE_ASSERT(align >= 1, "Alignment must be > 1");
		CE_ASSERT(align % 2 == 0 || align == 1, "Alignment must be a power of two");

		uintptr_t ptr = (uintptr_t)p;

		const size_t mod = ptr % align;

		if (mod)
		{
			ptr += align - mod;
		}

		return (void*) ptr;
	}

	/// Respects standard behaviour when calling on NULL @a ptr
	template <typename T>
	inline void call_destructor_and_deallocate(Allocator& a, T* ptr)
	{
		if (!ptr)
			return;

		ptr->~T();
		a.deallocate(ptr);
	}
} // namespace memory

namespace memory_globals
{
	/// Constructs the initial default allocators.
	/// @note
	/// Has to be called before anything else during the engine startup.
	void init();

	/// Destroys the allocators created with memory_globals::init().
	/// @note
	/// Should be the last call of the program.
	void shutdown();
} // namespace memory_globals
} // namespace crown

/// Allocates memory with @a allocator for the given @a T type
/// and calls constructor on it.
/// @note
/// @a allocator must be a reference to an existing allocator.
#define CE_NEW(allocator, T) new ((allocator).allocate(sizeof(T), CE_ALIGNOF(T))) T

/// Calls destructor on @a ptr and deallocates memory using the
/// given @a allocator.
/// @note
/// @a allocator must be a reference to an existing allocator.
#define CE_DELETE(allocator, ptr) crown::memory::call_destructor_and_deallocate(allocator, ptr)
