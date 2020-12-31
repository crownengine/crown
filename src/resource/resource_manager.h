/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/memory/proxy_allocator.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "resource/types.h"
#include "core/json/types.h"
#include "device/console_server.h"

namespace crown
{
/// Keeps track and manages resources loaded by ResourceLoader.
///
/// @ingroup Resource
struct ResourceManager
{
	typedef void* (*LoadFunction)(File& file, Allocator& a);
	typedef void (*OnlineFunction)(StringId64 name, ResourceManager& rm);
	typedef void (*OfflineFunction)(StringId64 name, ResourceManager& rm);
	typedef void (*UnloadFunction)(Allocator& allocator, void* resource);

	struct ResourcePair
	{
		StringId64 type;
		StringId64 name;
	};

	struct ResourceEntry
	{
		u32 references;
		void* data;

		static const ResourceEntry NOT_FOUND;
	};

	struct ResourceTypeData
	{
		u32 version;
		LoadFunction load;
		OnlineFunction online;
		OfflineFunction offline;
		UnloadFunction unload;
	};

	typedef HashMap<StringId64, ResourceTypeData> TypeMap;
	typedef HashMap<ResourcePair, ResourceEntry> ResourceMap;

	ProxyAllocator _resource_heap;
	ResourceLoader* _loader;
	TypeMap _type_data;
	ResourceMap _rm;
	bool _autoload;

	void on_online(StringId64 type, StringId64 name);
	void on_offline(StringId64 type, StringId64 name);
	void on_unload(StringId64 type, void* data);
	void complete_request(StringId64 type, StringId64 name, void* data);

	/// Uses @a rl to load resources.
	ResourceManager(ResourceLoader& rl);

	///
	~ResourceManager();

	/// Loads the resource (@a type, @a name).
	/// You can check whether the resource is available with can_get().
	void load(StringId64 type, StringId64 name);

	/// Unloads the resource @a type @a name.
	void unload(StringId64 type, StringId64 name);

	/// Reloads the resource (@a type, @a name).
	/// @note The user has to manually update all the references to the old resource.
	void reload(StringId64 type, StringId64 name);

	/// Returns whether the manager has the resource (@a type, @a name).
	bool can_get(StringId64 type, StringId64 name);

	/// Returns the data of the resource (@a type, @a name).
	const void* get(StringId64 type, StringId64 name);

	/// Sets whether resources should be automatically loaded when accessed.
	void enable_autoload(bool enable);

	/// Blocks until all load() requests have been completed.
	void flush();

	/// Completes all load() requests which have been loaded by ResourceLoader.
	void complete_requests();

	/// Registers a new resource @a type into the resource manager.
	void register_type(StringId64 type, u32 version, LoadFunction load, UnloadFunction unload, OnlineFunction online, OfflineFunction offline);
};

} // namespace crown
