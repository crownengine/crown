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
	struct CanGetFlags
	{
		enum Enum : u32
		{
			STRICT = u32(1) << 0
		};
	};

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
	Array<ResourcePair> _scoped_autoloaded;
	Array<u32> _scoped_autoload_markers;

	void on_online(StringId64 type, StringId64 name);
	void on_offline(StringId64 type, StringId64 name);
	void on_unload(StringId64 type, Allocator *allocator, void *data);

	/// Uses @a rl to load resources.
	explicit ResourceManager(ResourceLoader &rl);

	///
	~ResourceManager();

	/// Starts loading the resource (@a type, @a name) from @a resource_package asynchronously. The
	/// function returns before the resource has been loaded. Call can_get() to poll for completion.
	void load(ResourcePackage *resource_package, StringId64 type, StringId64 name, u32 online_order);

	/// Unloads the resource @a type @a name.
	void unload(StringId64 type, StringId64 name);

	/// Reloads the resource (@a type, @a name) and returns its new data.
	/// If the resource is not loaded, it returns NULL.
	/// @note The user has to manually update all the references to the old resource.
	void *reload(StringId64 type, StringId64 name);

	/// Returns whether the manager has the resource (@a type, @a name).
	/// @note Returns true if autoload is enabled or a scoped autoload is active unless
	/// CanGetFlags::STRICT is specified.
	bool can_get(StringId64 type, StringId64 name, u32 flags = 0);

	/// Returns the data of the resource (@a type, @a name).
	const void *get(StringId64 type, StringId64 name);

	/// Opens the stream of the resource (@a type, @a name).
	File *open_stream(StringId64 type, StringId64 name);

	/// Closes a stream opened by open_stream().
	void close_stream(File *stream);

	/// Sets whether resources should be automatically loaded when accessed.
	void enable_autoload(bool enable);

	/// Begins a scope in which resources are automatically loaded when accessed.
	void scoped_autoload_begin();

	/// Ends the current autoload scope and unloads resources automatically loaded within it.
	void scoped_autoload_end();

	/// Completes all load requests which have been loaded by ResourceLoader.
	void complete_requests();

	/// Registers a new resource @a type into the resource manager.
	void register_type(StringId64 type, u32 version, LoadFunction load, UnloadFunction unload, OnlineFunction online, OfflineFunction offline);
};

} // namespace crown
