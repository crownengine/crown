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
#include "ContainerTypes.h"
#include "Resource.h"
#include "ProxyAllocator.h"
#include "ResourceLoader.h"

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

struct PendingRequest
{
	LoadResourceId id;
	ResourceId resource;
};

class Bundle;

/// Keeps track and manages resources loaded by ResourceLoader.
class ResourceManager
{
public:

	/// The resources will be loaded from @a bundle.
	ResourceManager(Bundle& bundle);
	~ResourceManager();

	/// Loads the resource by @a type and @a name and returns its ResourceId.
	/// @note
	/// You have to call is_loaded() to check if the loading process is actually completed.
	void load(ResourceId id);

	/// Unloads the resource @a name, freeing up all the memory associated by it
	/// and eventually any global object associated with it.
	/// If @a force is true, the resource is unloaded even if its reference count
	/// is greater than 1.
	/// @warning
	/// Use @a force option only if you know - exactly - what you are doing.
	void unload(ResourceId id, bool force = false);

	/// Returns whether the manager has the @a name resource into
	/// its list of resources.
	/// @warning
	/// Having a resource does not mean that the resource is
	/// ready to be used; See is_loaded().
	bool has(ResourceId id) const;

	/// Returns the resource instance associated to the given @a type and @a name.
	const void* get(const char* type, const char* name) const;

	/// Returns the data associated with the @a name resource.
	/// You will have to cast the returned pointer accordingly.
	const void* get(ResourceId id) const;

	/// Returns whether the @a name resource is loaded (i.e. whether
	/// you can use the data associated with it).
	bool is_loaded(ResourceId id) const;

	/// Returns the number of references to the resource @a name;
	uint32_t references(ResourceId id) const;

	/// Forces all of the loading requests to complete before preceeding.
	void flush();

private:


	// Returns the entry of the given id.
	ResourceEntry* find(ResourceId id) const;

	// Polls the resource loader for loaded resources.
	void poll_resource_loader();

	void online(ResourceId id, void* resource);

private:

	ProxyAllocator m_resource_heap;
	ResourceLoader m_loader;

	Queue<PendingRequest> m_pendings;
	Array<ResourceEntry> m_resources;

private:

	friend class ResourcePackage;
	friend class Device;
};

} // namespace crown
