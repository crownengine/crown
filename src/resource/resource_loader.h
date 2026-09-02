/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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
#include <atomic>

namespace crown
{
struct ResourceRequest
{
	typedef void * (*LoadFunction)(File &file, Allocator &a);

	ResourcePackage *resource_package;
	StringId64 type;
	StringId64 name;
	u32 online_order;
	LoadFunction load_function;
	Allocator *allocator;
	void *data;

	bool is_spurious() const
	{
		return data == NULL && allocator == NULL;
	}
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
	Mutex _loaded_mutex;
	ConditionVariable _requests_condition;
	std::atomic_bool _exit;

	/// Do not call explicitly.
	s32 run();

	/// Read resources from @a data_filesystem. Is bundle specifies whether
	/// the filesystem contains bundled data.
	explicit ResourceLoader(Filesystem &data_filesystem, bool is_bundle);

	///
	~ResourceLoader();

	/// Adds a request for loading the resource described by @a rr.
	void add_request(const ResourceRequest &rr);

	/// Adds @a rr to the loaded queue.
	void add_loaded(const ResourceRequest &rr);

	/// Pops a request from the loaded queue. Returns false if the queue is empty.
	bool pop_loaded(ResourceRequest &rr);

	/// Registers a fallback resource @a name for the specified resource @a type.
	void register_fallback(StringId64 type, StringId64 name);

	///
	File *open_stream(StringId64 type, StringId64 name);

	///
	void close_stream(File *stream);
};

} // namespace crown
