/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "assert.h"
#include "proxy_allocator.h"
#include "string_utils.h"
#include "mutex.h"

namespace crown
{

static ProxyAllocator* g_proxy_allocators_head = NULL;
static Mutex g_proxy_allocators_mutex;

ProxyAllocator::ProxyAllocator(const char* name, Allocator& allocator)
	: _allocator(allocator)
	, _name(name)
	, _total_allocated(0)
	, _next(NULL)
{
	ScopedMutex sm(g_proxy_allocators_mutex);

	CE_ASSERT(name != NULL, "Name must be != NULL");

	if(g_proxy_allocators_head != NULL)
	{
		_next = g_proxy_allocators_head;
	}

	g_proxy_allocators_head = this;
}

void* ProxyAllocator::allocate(size_t size, size_t align)
{
	_total_allocated += size;

	return _allocator.allocate(size, align);
}

void ProxyAllocator::deallocate(void* data)
{
	_allocator.deallocate(data);
}

size_t ProxyAllocator::allocated_size()
{
	return _total_allocated;
}

const char* ProxyAllocator::name() const
{
	return _name;
}

uint32_t ProxyAllocator::count()
{
	ScopedMutex sm(g_proxy_allocators_mutex);

	const ProxyAllocator* head = g_proxy_allocators_head;
	uint32_t count = 0;

	while (head != NULL)
	{
		++count;
		head = head->_next;
	}

	return count;
}

ProxyAllocator* ProxyAllocator::find(const char* name)
{
	ScopedMutex sm(g_proxy_allocators_mutex);

	ProxyAllocator* head = g_proxy_allocators_head;

	while (head != NULL)
	{
		if (strcmp(name, head->name()) == 0)
		{
			return head;
		}

		head = head->_next;
	}

	return NULL;
}

ProxyAllocator* ProxyAllocator::begin()
{
	return g_proxy_allocators_head;
}

ProxyAllocator* ProxyAllocator::next(ProxyAllocator* a)
{
	if (a == NULL)
	{
		return NULL;
	}

	return a->_next;
}

} // namespace crown
