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

#pragma once

#include "Types.h"
#include "Resource.h"
#include "Bundle.h"
#include "Allocator.h"
#include "File.h"

namespace crown
{

// Bump the version whenever a change in the header is made
const uint32_t LUA_RESOURCE_VERSION = 1;

struct LuaHeader
{
	uint32_t	version;	// Lua resource version
	uint32_t	size;		// Size of lua code
};

class LuaResource
{
public:

	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);

		const size_t file_size = file->size() - 12;
		LuaResource* res = (LuaResource*) allocator.allocate(sizeof(LuaResource));
		res->m_data = (uint8_t*) allocator.allocate(file_size);
		file->read(res->m_data, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* /*resource*/) {}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT_NOT_NULL(resource);

		allocator.deallocate(((LuaResource*)resource)->m_data);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* /*resource*/) {}

public:

	uint32_t size() const
	{
		return ((LuaHeader*) m_data)->size;
	}

	const uint8_t* code() const
	{
		return m_data + sizeof(LuaHeader);
	}

private:

	uint8_t* m_data;
};

} // namespace crown
