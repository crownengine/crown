/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "material_manager.h"
#include "sort_map.h"
#include "resource_manager.h"

namespace crown
{

MaterialManager::MaterialManager(Allocator& a, ResourceManager& rm)
	: _allocator(&a)
	, _resource_manager(&rm)
	, _materials(a)
{
}

MaterialManager::~MaterialManager()
{
	auto begin = sort_map::begin(_materials);
	auto end = sort_map::end(_materials);

	for (; begin != end; ++begin)
	{
		begin->pair.second->destroy();
		CE_DELETE(*_allocator, begin->pair.second);
	}
}

void MaterialManager::create_material(StringId64 id)
{
	if (sort_map::has(_materials, id))
		return;

	Material* mat = CE_NEW(*_allocator, Material);
	mat->create((MaterialResource*)_resource_manager->get(MATERIAL_TYPE, id));

	sort_map::set(_materials, id, mat);
	sort_map::sort(_materials);
}

void MaterialManager::destroy_material(StringId64 id)
{
	Material* mat = sort_map::get(_materials, id, (Material*)NULL);
	mat->destroy();
	CE_DELETE(*_allocator, mat);
	sort_map::remove(_materials, id);
	sort_map::sort(_materials);
}

Material* MaterialManager::get(StringId64 id)
{
	CE_ASSERT(sort_map::has(_materials, id), "Material not found");
	return sort_map::get(_materials, id, (Material*)NULL);
}

} // namespace crown
