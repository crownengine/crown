/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/json/types.h"
#include "core/memory/proxy_allocator.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "device/console_server.h"
#include "resource/resource_id.h"
#include "resource/types.h"

namespace crown
{
/// Keeps track and manages resources loaded by ResourceLoader.
///
/// @ingroup Resource
struct ResourceManager
{
	typedef void * (*LoadFunction)(File &file, Allocator &a);
	typedef void (*OnlineFunction)(StringId64 name, ResourceManager &rm);
	typedef void (*OfflineFunction)(StringId64 name, ResourceManager &rm);
	typedef void (*UnloadFunction)(Allocator &allocator, void *resource);

	struct ResourcePair
	{
		StringId64 type;
		StringId64 name;
	};

	struct ResourceData
	{
		StringId64 package_name;
		u32 references;
		u32 online_sequence_num;
		Allocator *allocator;
		void *data;

		static const ResourceData NOT_FOUND;
	};

	struct ResourceTypeData
	{
		u32 version;
		LoadFunction load;
		OnlineFunction online;
		OfflineFunction offline;
		UnloadFunction unload;

		static const ResourceTypeData NOT_FOUND;
	};

	ProxyAllocator _resource_heap;
	ResourceLoader *_resource_loader;
	HashMap<StringId64, ResourceTypeData> _types;
	HashMap<ResourcePair, ResourceData> _resources;
	bool _autoload;

	void on_online(StringId64 type, StringId64 name);
	void on_offline(StringId64 type, StringId64 name);
	void on_unload(StringId64 type, Allocator *allocator, void *data);

	/// Uses @a rl to load resources.
	explicit ResourceManager(ResourceLoader &rl);

	///
	~ResourceManager();

	/// Tries to load the resource (@a type, @a name) from @a package.
	/// When the load queue is full, it may fail returning false. In such case,
	/// you must call complete_requests() and try again later until true is returned.
	/// Use can_get() to check whether the resource can be used.
	bool try_load(StringId64 package_name, StringId64 type, StringId64 name, u32 online_order);

	/// Unloads the resource @a type @a name.
	void unload(StringId64 type, StringId64 name);

	/// Reloads the resource (@a type, @a name) and returns its new data.
	/// If the resource is not loaded, it returns NULL.
	/// @note The user has to manually update all the references to the old resource.
	void *reload(StringId64 type, StringId64 name);

	/// Returns whether the manager has the resource (@a type, @a name).
	/// @note Always returns true if autoload is enabled.
	bool can_get(StringId64 type, StringId64 name);

	/// Returns the data of the resource (@a type, @a name).
	const void *get(StringId64 type, StringId64 name);

	/// Opens the stream of the resource (@a type, @a name).
	File *open_stream(StringId64 type, StringId64 name);

	/// Closes a stream opened by open_stream().
	void close_stream(File *stream);

	/// Sets whether resources should be automatically loaded when accessed.
	void enable_autoload(bool enable);

	/// Completes all load requests which have been loaded by ResourceLoader.
	void complete_requests();

	/// Registers a new resource @a type into the resource manager.
	void register_type(StringId64 type, u32 version, LoadFunction load, UnloadFunction unload, OnlineFunction online, OfflineFunction offline);
};

} // namespace crown
