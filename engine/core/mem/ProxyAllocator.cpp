/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Assert.h"
#include "ProxyAllocator.h"
#include "Allocator.h"
#include "StringUtils.h"

namespace crown
{

static ProxyAllocator* g_proxy_allocators_head = NULL;

//-----------------------------------------------------------------------------
ProxyAllocator::ProxyAllocator(const char* name, Allocator& allocator) :
	m_allocator(allocator),
	m_name(name),
	m_next(NULL)
{
	CE_ASSERT(name != NULL, "Name must be != NULL");

	if(g_proxy_allocators_head != NULL)
	{
		m_next = g_proxy_allocators_head;
	}

	g_proxy_allocators_head = this;
}

//-----------------------------------------------------------------------------
void* ProxyAllocator::allocate(size_t size, size_t align)
{
	return m_allocator.allocate(size, align);
}

//-----------------------------------------------------------------------------
void ProxyAllocator::deallocate(void* data)
{
	m_allocator.deallocate(data);
}

//-----------------------------------------------------------------------------
const char* ProxyAllocator::name() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
uint32_t ProxyAllocator::count()
{
	const ProxyAllocator* head = g_proxy_allocators_head;
	uint32_t count = 0;

	while (head != NULL)
	{
		++count;
		head = head->m_next;
	}

	return count;
}

//-----------------------------------------------------------------------------
ProxyAllocator* ProxyAllocator::find(const char* name)
{
	ProxyAllocator* head = g_proxy_allocators_head;

	while (head != NULL)
	{
		if (string::strcmp(name, head->name()) == 0)
		{
			return head;
		}

		head = head->m_next;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
ProxyAllocator* ProxyAllocator::begin()
{
	return g_proxy_allocators_head;
}

//-----------------------------------------------------------------------------
ProxyAllocator* ProxyAllocator::next(ProxyAllocator* a)
{
	if (a == NULL)
	{
		return NULL;
	}

	return a->m_next;
}

} // namespace crown
