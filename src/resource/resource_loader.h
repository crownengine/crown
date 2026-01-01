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
#include "core/thread/spsc_queue.inl"
#include "core/thread/mpsc_queue.inl"
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
	u32 online_order;
	LoadFunction load_function;
	Allocator *allocator;
	void *data;

	bool is_spurious()
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

	SPSCQueue<ResourceRequest, 128> _requests;
	MPSCQueue<ResourceRequest, 128> _loaded;
	HashMap<StringId64, StringId64> _fallback;

	Thread _thread;
	Mutex _mutex;
	ConditionVariable _requests_condition;
	bool _exit;

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
	/// Returns true on success, false otherwise.
	bool add_request(const ResourceRequest &rr);

	/// Registers a fallback resource @a name for the given resource @a type.
	void register_fallback(StringId64 type, StringId64 name);

	///
	File *open_stream(StringId64 type, StringId64 name);

	///
	void close_stream(File *stream);
};

} // namespace crown
