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
#include "resource.h"
#include "thread.h"
#include "container_types.h"
#include "mutex.h"
#include "semaphore.h"
#include "os.h"

namespace crown
{

class Bundle;
class Allocator;

struct ResourceData
{
	ResourceId id;
	void* data;
};

/// Loads resources in a background thread.
class ResourceLoader
{
public:

	/// Reads the resources data from the given @a bundle using
	/// @a resource_heap to allocate memory for them.
	ResourceLoader(Bundle& bundle, Allocator& resource_heap);
	~ResourceLoader();

	/// Loads the @a resource in a background thread.
	void load(ResourceId id);

	/// Blocks until all pending requests have been processed.
	void flush();

	void get_loaded(Array<ResourceData>& loaded);

private:

	void add_request(ResourceId id);
	uint32_t num_requests();
	void add_loaded(ResourceData data);

	// Loads resources in the loading queue.
	int32_t run();

	static int32_t thread_proc(void* thiz)
	{
		ResourceLoader* rl = (ResourceLoader*)thiz;
		return rl->run();
	}

private:

	Thread m_thread;
	Bundle& m_bundle;
	Allocator& m_resource_heap;

	Queue<ResourceId> m_requests;
	Queue<ResourceData> m_loaded;
	Mutex m_mutex;
	Mutex m_loaded_mutex;
	bool m_exit;
};

} // namespace crown
