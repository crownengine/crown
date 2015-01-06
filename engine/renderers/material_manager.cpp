/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "material_manager.h"
#include "memory.h"
#include "sort_map.h"
#include "device.h"

namespace crown
{

namespace material_manager
{
	static MaterialManager* s_mmgr = NULL;

	void init()
	{
		s_mmgr = CE_NEW(default_allocator(), MaterialManager)();
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

MaterialManager::MaterialManager()
{
}

MaterialId MaterialManager::create_material(StringId64 id)
{
	MaterialId new_id = id_table::create(_materials_ids);
	_materials[new_id.index].create((MaterialResource*) device()->resource_manager()->get(MATERIAL_TYPE, id), *this);
	return new_id;
}

void MaterialManager::destroy_material(MaterialId id)
{
	_materials[id.index].destroy();
	id_table::destroy(_materials_ids, id);
}

Material* MaterialManager::lookup_material(MaterialId id)
{
	return &_materials[id.index];
}

} // namespace crown
