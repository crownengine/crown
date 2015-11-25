/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "filesystem_types.h"
#include "thread.h"
#include "container_types.h"
#include "mutex.h"
#include "memory_types.h"
#include "string_id.h"

namespace crown
{

struct ResourceData
{
	StringId64 type;
	StringId64 name;
	void* data;
};

/// Loads resources in a background thread.
///
/// @ingroup Resource
class ResourceLoader
{
public:

	/// Reads the resources data from the given @a fs using
	/// @a resource_heap to allocate memory for them.
	ResourceLoader(Filesystem& fs, Allocator& resource_heap);
	~ResourceLoader();

	/// Loads the @a resource in a background thread.
	void load(StringId64 type, StringId64 name);

	/// Blocks until all pending requests have been processed.
	void flush();

	void get_loaded(Array<ResourceData>& loaded);

private:

	void add_request(StringId64 type, StringId64 name);
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

	struct ResourceRequest
	{
		StringId64 type;
		StringId64 name;
	};

	ResourceRequest make_request(StringId64 type, StringId64 name)
	{
		ResourceRequest request = { type, name };
		return request;
	}

	Thread _thread;
	Filesystem& _fs;
	Allocator& _resource_heap;

	Queue<ResourceRequest> _requests;
	Queue<ResourceData> _loaded;
	Mutex _mutex;
	Mutex _loaded_mutex;
	bool _exit;
};

} // namespace crown
