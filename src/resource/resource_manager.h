/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "filesystem_types.h"
#include "proxy_allocator.h"
#include "resource_types.h"
#include "string_id.h"
#include "types.h"

namespace crown
{

/// @defgroup Resource Resource

/// Keeps track and manages resources loaded by ResourceLoader.
///
/// @ingroup Resource
class ResourceManager
{
public:

	/// Uses @a rl to load resources.
	ResourceManager(ResourceLoader& rl);
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

private:

	typedef void* (*LoadFunction)(File& file, Allocator& a);
	typedef void (*OnlineFunction)(StringId64 name, ResourceManager& rm);
	typedef void (*OfflineFunction)(StringId64 name, ResourceManager& rm);
	typedef void (*UnloadFunction)(Allocator& allocator, void* resource);

	void register_resource_type(StringId64 type, LoadFunction load, OnlineFunction online, OfflineFunction offline, UnloadFunction unload);

	void on_online(StringId64 type, StringId64 name);
	void on_offline(StringId64 type, StringId64 name);
	void on_unload(StringId64 type, void* data);

	void complete_request(StringId64 type, StringId64 name, void* data);

private:

	struct ResourcePair
	{
		StringId64 type;
		StringId64 name;

		bool operator<(const ResourcePair& a) const
		{
			return type < a.type || (type == a.type && name < a.name);
		}
	};

	ResourcePair make_pair(StringId64 type, StringId64 name)
	{
		ResourcePair pair = { type, name };
		return pair;
	}

	struct ResourceEntry
	{
		bool operator==(const ResourceEntry& e)
		{
			return references == e.references && data == e.data;
		}

		uint32_t references;
		void* data;

		static const ResourceEntry NOT_FOUND;
	};

	struct ResourceTypeData
	{
		LoadFunction load;
		OnlineFunction online;
		OfflineFunction offline;
		UnloadFunction unload;
	};

	typedef SortMap<StringId64, ResourceTypeData> TypeMap;
	typedef SortMap<ResourcePair, ResourceEntry> ResourceMap;

	ProxyAllocator _resource_heap;
	ResourceLoader* _loader;
	TypeMap _type_data;
	ResourceMap _rm;
	bool _autoload;
};

} // namespace crown
