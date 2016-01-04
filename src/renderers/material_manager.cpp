/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "material_manager.h"
#include "memory.h"
#include "sort_map.h"
#include "resource_manager.h"

namespace crown
{

namespace material_manager
{
	static MaterialManager* s_mmgr = NULL;

	void init(ResourceManager& rm)
	{
		s_mmgr = CE_NEW(default_allocator(), MaterialManager)(rm);
	}

	void shutdown()
	{
		CE_DELETE(default_allocator(), s_mmgr);
	}

	MaterialManager* get()
	{
		return s_mmgr;
	}
} // namespace material_manager

MaterialManager::MaterialManager(ResourceManager& rm)
	: _resource_manager(&rm)
	, _materials(default_allocator())
{
}

MaterialManager::~MaterialManager()
{
	const SortMap<StringId64, Material*>::Entry* begin = sort_map::begin(_materials);
	const SortMap<StringId64, Material*>::Entry* end = sort_map::end(_materials);

	for (; begin != end; ++begin)
	{
		begin->pair.second->destroy();
		CE_DELETE(default_allocator(), begin->pair.second);
	}
}

void MaterialManager::create_material(StringId64 id)
{
	if (sort_map::has(_materials, id))
		return;

	Material* mat = CE_NEW(default_allocator(), Material);
	mat->create((MaterialResource*)_resource_manager->get(MATERIAL_TYPE, id), *this);

	sort_map::set(_materials, id, mat);
	sort_map::sort(_materials);
}

void MaterialManager::destroy_material(StringId64 id)
{
	Material* mat = sort_map::get(_materials, id, (Material*)NULL);
	mat->destroy();
	CE_DELETE(default_allocator(), mat);
	sort_map::remove(_materials, id);
	sort_map::sort(_materials);
}

Material* MaterialManager::lookup_material(StringId64 id)
{
	return sort_map::get(_materials, id, (Material*)NULL);
}

} // namespace crown
