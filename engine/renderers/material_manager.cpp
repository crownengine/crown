/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "material_manager.h"
#include "memory.h"
#include "sort_map.h"

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
	: m_materials(default_allocator())
{
}

void MaterialManager::load(StringId64 id, ResourceManager& rm)
{
	ResourceId res_id;
	res_id.type = MATERIAL_TYPE;
	res_id.name = id;

	MaterialId mat_id = id_table::create(_materials_ids);
	_materials[mat_id.index].create((MaterialResource*) rm.get(res_id), *this);

	sort_map::set(m_materials, id, mat_id);
	sort_map::sort(m_materials);
}

void MaterialManager::unload(StringId64 id, ResourceManager& /*rm*/)
{
	MaterialId deff_id;
	deff_id.id = INVALID_ID;
	deff_id.index = 0;

	MaterialId mat_id = sort_map::get(m_materials, id, deff_id);
	CE_ASSERT(mat_id.id != INVALID_ID, "Material not loaded");

	_materials[mat_id.index].destroy();
	id_table::destroy(_materials_ids, mat_id);

	sort_map::remove(m_materials, id);
	sort_map::sort(m_materials);
}

MaterialId MaterialManager::create_material(StringId64 id)
{
	MaterialId deff_id;
	deff_id.id = INVALID_ID;
	deff_id.index = 0;

	MaterialId idd = sort_map::get(m_materials, id, deff_id);
	CE_ASSERT(idd.id != INVALID_ID, "Material not loaded");

	MaterialId new_id = id_table::create(_materials_ids);
	_materials[new_id.index].clone(_materials[idd.index]);

	return new_id;
}

void MaterialManager::destroy_material(MaterialId id)
{
	id_table::destroy(_materials_ids, id);
}

Material* MaterialManager::lookup_material(MaterialId id)
{
	return &_materials[id.index];
}

} // namespace crown
