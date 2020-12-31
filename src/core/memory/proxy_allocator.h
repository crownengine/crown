/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/memory/allocator.h"

namespace crown
{
/// Offers the facility to tag allocators by a string identifier.
///
/// @ingroup Memory
struct ProxyAllocator : public Allocator
{
	Allocator& _allocator;
	const char* _name;

	/// Tag all allocations made with @a allocator by the given @a name
	ProxyAllocator(Allocator& allocator, const char* name);

	/// @copydoc Allocator::allocate()
	void* allocate(u32 size, u32 align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	void deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	u32 allocated_size(const void* ptr) { return _allocator.allocated_size(ptr); }

	/// @copydoc Allocator::total_allocated()
	u32 total_allocated() { return _allocator.total_allocated(); }

	/// Returns the name of the proxy allocator
	const char* name() const;
};

} // namespace crown
