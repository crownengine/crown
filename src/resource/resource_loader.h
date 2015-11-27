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

	ResourceLoader(Filesystem& fs);
	~ResourceLoader();

	/// Adds a request for loading the resource @a type @a name.
	void add_request(StringId64 type, StringId64 name, Allocator& a);

	/// Blocks until all pending requests have been processed.
	void flush();

	/// Returns all the resources that have been loaded.
	void get_loaded(Array<ResourceData>& loaded);

private:

	uint32_t num_requests();
	void add_loaded(ResourceData data);

	// Loads resources in the loading queue.
	int32_t run();

	static int32_t thread_proc(void* thiz)
	{
		return ((ResourceLoader*)thiz)->run();
	}

private:

	struct ResourceRequest
	{
		StringId64 type;
		StringId64 name;
		Allocator* allocator;
	};

	ResourceRequest make_request(StringId64 type, StringId64 name, Allocator& a)
	{
		ResourceRequest rr;
		rr.type = type;
		rr.name = name;
		rr.allocator = &a;
		return rr;
	}

	Thread _thread;
	Filesystem& _fs;

	Queue<ResourceRequest> _requests;
	Queue<ResourceData> _loaded;
	Mutex _mutex;
	Mutex _loaded_mutex;
	bool _exit;
};

} // namespace crown
