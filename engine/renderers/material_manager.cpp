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
