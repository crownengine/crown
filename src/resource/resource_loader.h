/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/strings/string_id.h"
#include "core/thread/condition_variable.h"
#include "core/thread/mutex.h"
#include "core/thread/thread.h"
#include "core/types.h"
#include "resource/types.h"

namespace crown
{
struct ResourceRequest
{
	typedef void * (*LoadFunction)(File &file, Allocator &a);

	ResourceManager *resource_manager;
	StringId64 package_name;
	StringId64 type;
	StringId64 name;
	u32 version;
	LoadFunction load_function;
	Allocator *allocator;
	void *data;
};

/// Loads resources in a background thread.
///
/// @ingroup Resource
struct ResourceLoader
{
	Filesystem &_data_filesystem;
	bool _is_bundle;

	Queue<ResourceRequest> _requests;
	Queue<ResourceRequest> _loaded;
	HashMap<StringId64, StringId64> _fallback;

	Thread _thread;
	Mutex _mutex;
	ConditionVariable _requests_condition;
	Mutex _loaded_mutex;
	bool _exit;

	///
	u32 num_requests();

	///
	void add_loaded(ResourceRequest rr);

	/// Do not call explicitly.
	s32 run();

	/// Read resources from @a data_filesystem. Is bundle specifies whether
	/// the filesystem contains bundled data.
	explicit ResourceLoader(Filesystem &data_filesystem, bool is_bundle);

	///
	~ResourceLoader();

	/// Adds a request for loading the resource described by @a rr.
	void add_request(const ResourceRequest &rr);

	/// Blocks until all pending requests have been processed.
	void flush();

	/// Returns all the resources that have been loaded.
	void get_loaded(Array<ResourceRequest> &loaded);

	/// Registers a fallback resource @a name for the given resource @a type.
	void register_fallback(StringId64 type, StringId64 name);
};

} // namespace crown
