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
#include "OS.h"

namespace crown
{

class Bundle;
class Allocator;

#define MAX_LOAD_REQUESTS 1024
typedef uint32_t LoadResourceId;

enum LoadResourceStatus
{
	LRS_NO_INFORMATION,
	LRS_QUEUED,
	LRS_LOADING,
	LRS_LOADED
};

struct LoadResource
{
	LoadResourceId id;
	uint32_t type;
	ResourceId resource;
};

struct LoadResourceData
{
	LoadResourceStatus status;
	void* data;
};

/// Loads resources in a background thread.
class ResourceLoader
{
public:

	/// Reads the resources data from the given @a bundle using
	/// @a resource_heap to allocate memory for them.
							ResourceLoader(Bundle& bundle, Allocator& resource_heap);

	/// Loads the @a resource in a background thread.
	LoadResourceId			load_resource(uint32_t type, ResourceId resource);

	/// Returns the status of the given load request @a id.
	LoadResourceStatus		load_resource_status(LoadResourceId id) const;

	/// Returns the data which has been loaded for the given request @a id.
	void*					load_resource_data(LoadResourceId id) const;

	// Loads resources in the loading queue.
	int32_t					run();

	void start()
	{
		m_should_run = true;
		m_thread.start(background_run, this);
	}

	void stop()
	{
		m_should_run = false;
		m_full.signal();
		m_thread.stop();
	}

private:

	static int32_t background_run(void* thiz)
	{
		return ((ResourceLoader*)thiz)->run();
	}

private:

	Thread					m_thread;
	bool					m_should_run;

	// Whether to look for resources
	Bundle&					m_bundle;

	// Used to strore resource memory
	Allocator&				m_resource_heap;

	uint32_t				m_num_requests;
	Queue<LoadResource>		m_requests;
	LoadResourceData		m_results[MAX_LOAD_REQUESTS];

	Mutex					m_requests_mutex;
	Mutex					m_results_mutex;
	Cond					m_full;
};

} // namespace crown
