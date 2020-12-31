/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/error/error.inl"
#include "core/memory/allocator.h"
#include "core/memory/types.h"
#include <new>

namespace crown
{
namespace memory
{
	/// Returns the pointer @a p aligned to the desired @a align byte
	inline void* align_top(const void* p, u32 align)
	{
		CE_ASSERT(align >= 1, "Alignment must be > 1");
		CE_ASSERT(align % 2 == 0 || align == 1, "Alignment must be a power of two");

		const uintptr_t mask = align-1;
		uintptr_t ptr = (uintptr_t)p;
		ptr = (ptr + mask) & ~mask;
		return (void*)ptr;
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

/// Convert integer to type.
template <int v>
struct Int2Type { enum {value=v}; };

template <typename T>
inline T &construct(void *p, Allocator& a, Int2Type<true>)
{
	return *(T*)new (p) T(a);
}

template <typename T>
inline T &construct(void *p, Allocator& /*a*/, Int2Type<false>)
{
	return *(T*)new (p) T();
}

template <typename T>
inline T &construct(void *p, Allocator& a)
{
	return construct<T>(p, a, IS_ALLOCATOR_AWARE_TYPE(T)());
}

} // namespace crown

/// Allocates memory with @a allocator for the given @a T type
/// and calls constructor on it.
/// @note
/// @a allocator must be a reference to an existing allocator.
#define CE_NEW(allocator, T) new ((allocator).allocate(sizeof(T), alignof(T))) T

/// Calls destructor on @a ptr and deallocates memory using the
/// given @a allocator.
/// @note
/// @a allocator must be a reference to an existing allocator.
#define CE_DELETE(allocator, ptr) crown::memory::call_destructor_and_deallocate(allocator, ptr)
