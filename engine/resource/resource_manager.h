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

#include "types.h"
#include "container_types.h"
#include "resource.h"
#include "proxy_allocator.h"
#include "resource_loader.h"

namespace crown
{

struct ResourceEntry
{
	bool operator==(const ResourceId& res) const { return id == res; }
	bool operator==(const ResourceEntry& b) const { return id == b.id; }

	ResourceId id;
	uint32_t references;
	void* resource;
};

class Bundle;

/// @defgroup Resource Resource

/// Keeps track and manages resources loaded by ResourceLoader.
///
/// @ingroup Resource
class ResourceManager
{
public:

	/// The resources will be loaded from @a bundle.
	ResourceManager(Bundle& bundle);

	/// Loads the resource @a id.
	/// You can check whether the resource is loaded with can_get().
	void load(ResourceId id);

	/// Unloads the resource @a id.
	/// If @a force is true, the resource is unloaded even if its reference count
	/// is greater than 1.
	/// @warning
	/// Use @a force option only if you know *exactly* what you are doing.
	void unload(ResourceId id, bool force = false);

	/// Returns whether the manager has the given resource. 
	bool can_get(const char* type, const char* name);

	/// Returns whether the manager has the resource @a id.
	bool can_get(ResourceId id) const;

	/// Returns the resource data by @a type and @a name.
	const void* get(const char* type, const char* name) const;

	/// Returns the resource data by @a id.
	const void* get(ResourceId id) const;

	/// Returns the number of references to resource @a id;
	uint32_t references(ResourceId id) const;

	/// Blocks until all load() requests have been completed.
	void flush();

	/// Completes all load() requests which have been loaded by ResourceLoader.
	void complete_requests();

private:

	ResourceEntry* find(ResourceId id) const;
	void complete_request(ResourceId id, void* data);

private:

	ProxyAllocator m_resource_heap;
	ResourceLoader m_loader;
	Array<ResourceEntry> m_resources;
};

} // namespace crown
