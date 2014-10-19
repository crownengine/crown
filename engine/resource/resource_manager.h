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
	~ResourceManager();

	/// Loads the resource (@a type, @a name).
	/// You can check whether the resource is available with can_get().
	void load(StringId64 type, StringId64 name);

	/// Unloads the resource @a type @a name.
	void unload(StringId64 type, StringId64 name);

	/// Returns whether the manager has the resource (@a type, @a name).
	bool can_get(const char* type, const char* name);

	/// Returns whether the manager has the resource (@a type, @a name).
	bool can_get(StringId64 type, StringId64 name);

	/// Returns the data of the resource (@a type, @a name).
	const void* get(const char* type, const char* name);

	/// Returns the data of the resource (@a type, @a name).
	const void* get(StringId64 type, StringId64 name);

	/// Sets whether resources should be automatically loaded when accessed.
	void enable_autoload(bool enable);

	/// Blocks until all load() requests have been completed.
	void flush();

	/// Completes all load() requests which have been loaded by ResourceLoader.
	void complete_requests();

private:

	void complete_request(ResourceId id, void* data);

private:

	struct ResourceEntry
	{
		bool operator==(const ResourceEntry& e)
		{
			return references == e.references && data == e.data;
		}

		uint32_t references;
		void* data;
	};

	typedef SortMap<ResourceId, ResourceEntry> ResourceMap;

	ProxyAllocator _resource_heap;
	ResourceLoader _loader;
	ResourceMap _rm;
	bool _autoload;

	static const ResourceEntry NOT_FOUND;
};

} // namespace crown
