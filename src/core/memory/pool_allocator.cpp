/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.inl"
#include "core/memory/memory.inl"
#include "core/memory/pool_allocator.h"

namespace crown
{
struct PoolHeader
{
	PoolHeader *next;
	char *first_block;
};

static PoolHeader *make_pool(Allocator &backing, u32 num_blocks, u32 block_size, u32 block_align)
{
	const u32 actual_block_size = block_size + block_align - (block_size & (block_align - 1));
	const u32 total_pool_size = sizeof(PoolHeader)
		+ block_align
		+ num_blocks * actual_block_size
		;

	PoolHeader *h = (PoolHeader *)backing.allocate(total_pool_size, alignof(PoolHeader));
	h->next = NULL;
	h->first_block = (char *)memory::align_top((char *)&h[1], block_align);

	// Initialize freelist.
	char *cur = h->first_block;
	for (u32 bb = 0; bb < num_blocks - 1; bb++) {
		uintptr_t *next = (uintptr_t *)cur;
		*next = (uintptr_t)cur + actual_block_size;
		cur += actual_block_size;
	}
	*(uintptr_t *)cur = (uintptr_t)NULL;

	return h;
}

PoolAllocator::PoolAllocator(Allocator &backing, u32 num_blocks, u32 block_size, u32 block_align)
	: _backing(backing)
	, _start(NULL)
	, _freelist(NULL)
	, _num_blocks(num_blocks)
	, _block_size(block_size)
	, _block_align(block_align)
{
	CE_ENSURE(num_blocks > 0);
	CE_ENSURE(block_size >= sizeof(void *));
	CE_ENSURE(is_power_of_2(block_align));
}

PoolAllocator::~PoolAllocator()
{
	PoolHeader *next;
	PoolHeader *tmp = _start;
	while (tmp != NULL) {
		next = tmp->next;
		_backing.deallocate(tmp);
		tmp = next;
	}
}

void *PoolAllocator::allocate(u32 size, u32 align)
{
	CE_ENSURE(size <= _block_size);
	CE_ENSURE(align <= _block_align);
	CE_ENSURE(is_power_of_2(align));
	CE_UNUSED_2(size, align);

	if (CE_UNLIKELY(_freelist == NULL)) {
		PoolHeader *new_pool = make_pool(_backing, _num_blocks, _block_size, _block_align);
		new_pool->next = _start;
		_start = new_pool;
		_freelist = new_pool->first_block;
	}

	uintptr_t next_free = *(uintptr_t *)_freelist;
	void *data = _freelist;
	_freelist = (void *)next_free;

	return data;
}

void PoolAllocator::deallocate(void *data)
{
	if (CE_UNLIKELY(!data))
		return;

	uintptr_t *next = (uintptr_t *)data;
	*next = (uintptr_t)_freelist;

	_freelist = data;
}

} // namespace crown
