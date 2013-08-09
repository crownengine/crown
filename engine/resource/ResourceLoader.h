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
#include "Thread.h"
#include "Queue.h"
#include "List.h"
#include "Mutex.h"
#include "Cond.h"

namespace crown
{

struct LoadedResource
{
	LoadedResource(ResourceId r, void* d) : resource(r), data(d) {}

	ResourceId	resource;
	void*		data;
};

class Bundle;
class Allocator;

/// Loads resources in a background thread.
class ResourceLoader : public Thread
{
public:

	/// Reads the resources data from the given @a bundle using
	/// @a resource_heap to allocate memory for them.
				ResourceLoader(Bundle& bundle, Allocator& resource_heap);

	/// Loads the @a resource in a background thread.
	void		load(ResourceId resource);

	/// Returns the number of resources still in the loading queue.
	uint32_t	remaining() const;

	/// Returns the number of resources already loaded.
	uint32_t	num_loaded() const;

	/// Returns a list of the last loaded resources.
	void		get_loaded(List<LoadedResource>& l);

	// Loads resources in the loading queue.
	int32_t		run();

private:

	// Whether to look for resources
	Bundle&					m_bundle;

	// Used to strore resource memory
	Allocator&				m_resource_heap;

	Queue<ResourceId>		m_load_queue;
	List<LoadedResource>	m_done_queue;
	Mutex					m_load_mutex;
	Mutex					m_done_mutex;
	Cond					m_load_requests;
};

} // namespace crown
