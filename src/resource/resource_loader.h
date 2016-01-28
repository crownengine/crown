/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
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
	Filesystem& _fs;

	Queue<ResourceRequest> _requests;
	Queue<ResourceRequest> _loaded;

	Thread _thread;
	Mutex _mutex;
	Mutex _loaded_mutex;
	bool _exit;

	u32 num_requests();
	void add_loaded(ResourceRequest rr);
	s32 run();
	static s32 thread_proc(void* thiz);

public:

	/// Read resources from @a fs.
	ResourceLoader(Filesystem& fs);
	~ResourceLoader();

	/// Returns whether the resource (type, name) can be loaded.
	bool can_load(StringId64 type, StringId64 name);

	/// Adds a request for loading the resource described by @a rr.
	void add_request(const ResourceRequest& rr);

	/// Blocks until all pending requests have been processed.
	void flush();

	/// Returns all the resources that have been loaded.
	void get_loaded(Array<ResourceRequest>& loaded);
};

} // namespace crown
