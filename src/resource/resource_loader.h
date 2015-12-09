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
#include "string_id.h"

namespace crown
{

struct ResourceRequest
{
	typedef void* (*LoadFunction)(File& file, Allocator& a);

	StringId64 type;
	StringId64 name;
	LoadFunction load_function;
	Allocator* allocator;
	void* data;
};

/// Loads resources in a background thread.
///
/// @ingroup Resource
class ResourceLoader
{
public:

	/// Read resources from @a fs.
	ResourceLoader(Filesystem& fs);
	~ResourceLoader();

	/// Adds a request for loading the resource described by @a rr.
	void add_request(const ResourceRequest& rr);

	/// Blocks until all pending requests have been processed.
	void flush();

	/// Returns all the resources that have been loaded.
	void get_loaded(Array<ResourceRequest>& loaded);

private:

	uint32_t num_requests();
	void add_loaded(ResourceRequest rr);

	// Loads resources in the loading queue.
	int32_t run();

	static int32_t thread_proc(void* thiz)
	{
		return ((ResourceLoader*)thiz)->run();
	}

private:

	Thread _thread;
	Filesystem& _fs;

	Queue<ResourceRequest> _requests;
	Queue<ResourceRequest> _loaded;
	Mutex _mutex;
	Mutex _loaded_mutex;
	bool _exit;
};

} // namespace crown
