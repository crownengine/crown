/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "aabb.h"
#include "color4.h"
#include "debug_line.h"
#include "hash_map.h"
#include "intersection.h"
#include "material.h"
#include "material_manager.h"
#include "matrix4x4.h"
#include "mesh_resource.h"
#include "render_world.h"
#include "resource_manager.h"
#include "sprite_resource.h"
#include "unit_manager.h"
#include <bgfx/bgfx.h>

namespace crown
{
RenderWorld::RenderWorld(Allocator& a, ResourceManager& rm, ShaderManager& sm, MaterialManager& mm, UnitManager& um)
	: _marker(RENDER_WORLD_MARKER)
	, _allocator(&a)
	, _resource_manager(&rm)
	, _shader_manager(&sm)
	, _material_manager(&mm)
	, _unit_manager(&um)
	, _debug_drawing(false)
	, _mesh_manager(a)
	, _sprite_manager(a)
	, _light_manager(a)
{
	um.register_destroy_function(RenderWorld::unit_destroyed_callback, this);

	_u_light_pos = bgfx::createUniform("u_light_pos", bgfx::UniformType::Vec4);
	_u_light_dir = bgfx::createUniform("u_light_dir", bgfx::UniformType::Vec4);
	_u_light_col = bgfx::createUniform("u_light_col", bgfx::UniformType::Vec4);
}

RenderWorld::~RenderWorld()
{
	_unit_manager->unregister_destroy_function(this);

	bgfx::destroyUniform(_u_light_pos);
	bgfx::destroyUniform(_u_light_dir);
	bgfx::destroyUniform(_u_light_col);

	_mesh_manager.destroy();
	_sprite_manager.destroy();
	_light_manager.destroy();

	_marker = 0;
}

MeshInstance RenderWorld::mesh_create(UnitId id, const MeshRendererDesc& mrd, const Matrix4x4& tr)
{
	const MeshResource* mr = (const MeshResource*)_resource_manager->get(RESOURCE_TYPE_MESH, mrd.mesh_resource);
	const MeshGeometry* mg = mr->geometry(mrd.geometry_name);
	_material_manager->create_material(mrd.material_resource);

	return _mesh_manager.create(id, mr, mg, mrd.material_resource, tr);
}

void RenderWorld::mesh_destroy(MeshInstance i)
{
	_mesh_manager.destroy(i);
}

void RenderWorld::mesh_instances(UnitId id, Array<MeshInstance>& instances)
{
	MeshInstance inst = _mesh_manager.first(id);

	while (_mesh_manager.is_valid(inst))
	{
		array::push_back(instances, inst);
		inst = _mesh_manager.next(inst);
	}
}

void RenderWorld::mesh_set_material(MeshInstance i, StringId64 id)
{
	CE_ASSERT(i.i < _mesh_manager._data.size, "Index out of bounds");
	_mesh_manager._data.material[i.i] = id;
}

void RenderWorld::mesh_set_visible(MeshInstance i, bool visible)
{
	CE_ASSERT(i.i < _mesh_manager._data.size, "Index out of bounds");
}

OBB RenderWorld::mesh_obb(MeshInstance i)
{
	CE_ASSERT(i.i < _mesh_manager._data.size, "Index out of bounds");

	const Matrix4x4& world = _mesh_manager._data.world[i.i];
	const OBB& obb = _mesh_manager._data.obb[i.i];

	OBB o;
	o.tm = obb.tm * world;
	o.half_extents = obb.half_extents;

	return o;
}

f32 RenderWorld::mesh_raycast(MeshInstance i, const Vector3& from, const Vector3& dir)
{
	CE_ASSERT(i.i < _mesh_manager._data.size, "Index out of bounds");
	const MeshGeometry* mg = _mesh_manager._data.geometry[i.i];
	return ray_mesh_intersection(from
		, dir
		, _mesh_manager._data.world[i.i]
		, mg->vertices.data
		, mg->vertices.stride
		, (u16*)mg->indices.data
		, mg->indices.num
		);
}

SpriteInstance RenderWorld::sprite_create(UnitId id, const SpriteRendererDesc& srd, const Matrix4x4& tr)
{
	const SpriteResource* sr = (const SpriteResource*)_resource_manager->get(RESOURCE_TYPE_SPRITE, srd.sprite_resource);
	_material_manager->create_material(srd.material_resource);

	return _sprite_manager.create(id, sr, srd.material_resource, tr);
}

void RenderWorld::sprite_destroy(SpriteInstance i)
{
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager.destroy(i);
}

void RenderWorld::sprite_instances(UnitId id, Array<SpriteInstance>& instances)
{
	SpriteInstance inst = _sprite_manager.first(id);

	while (_sprite_manager.is_valid(inst))
	{
		array::push_back(instances, inst);
		inst = _sprite_manager.next(inst);
	}
}

void RenderWorld::sprite_set_material(SpriteInstance i, StringId64 id)
{
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.material[i.i] = id;
}

void RenderWorld::sprite_set_visible(SpriteInstance i, bool visible)
{
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
}

void RenderWorld::sprite_set_frame(SpriteInstance i, u32 index)
{
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.frame[i.i] = index;
}

LightInstance RenderWorld::light_create(UnitId id, const LightDesc& ld, const Matrix4x4& tr)
{
	return _light_manager.create(id, ld, tr);
}

void RenderWorld::light_destroy(LightInstance i)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager.destroy(i);
}

LightInstance RenderWorld::light(UnitId id)
{
	return _light_manager.light(id);
}

Color4 RenderWorld::light_color(LightInstance i)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.color[i.i];
}

LightType::Enum RenderWorld::light_type(LightInstance i)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	return (LightType::Enum)_light_manager._data.type[i.i];
}

f32 RenderWorld::light_range(LightInstance i)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.range[i.i];
}

f32 RenderWorld::light_intensity(LightInstance i)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.intensity[i.i];
}

f32 RenderWorld::light_spot_angle(LightInstance i)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.spot_angle[i.i];
}

void RenderWorld::light_set_color(LightInstance i, const Color4& col)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.color[i.i] = col;
}

void RenderWorld::light_set_type(LightInstance i, LightType::Enum type)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.type[i.i] = type;
}

void RenderWorld::light_set_range(LightInstance i, f32 range)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.range[i.i] = range;
}

void RenderWorld::light_set_intensity(LightInstance i, f32 intensity)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.intensity[i.i] = intensity;
}

void RenderWorld::light_set_spot_angle(LightInstance i, f32 angle)
{
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.spot_angle[i.i] = angle;
}

void RenderWorld::update_transforms(const UnitId* begin, const UnitId* end, const Matrix4x4* world)
{
	MeshManager::MeshInstanceData& mid = _mesh_manager._data;
	SpriteManager::SpriteInstanceData& sid = _sprite_manager._data;
	LightManager::LightInstanceData& lid = _light_manager._data;

	for (; begin != end; ++begin, ++world)
	{
		if (_mesh_manager.has(*begin))
		{
			MeshInstance inst = _mesh_manager.first(*begin);
			mid.world[inst.i] = *world;
		}

		if (_sprite_manager.has(*begin))
		{
			SpriteInstance inst = _sprite_manager.first(*begin);
			sid.world[inst.i] = *world;
		}

		if (_light_manager.has(*begin))
		{
			LightInstance inst = _light_manager.light(*begin);
			lid.world[inst.i] = *world;
		}
	}
}

void RenderWorld::render(const Matrix4x4& view, const Matrix4x4& projection)
{
	MeshManager::MeshInstanceData& mid = _mesh_manager._data;
	SpriteManager::SpriteInstanceData& sid = _sprite_manager._data;
	LightManager::LightInstanceData& lid = _light_manager._data;

	for (u32 ll = 0; ll < lid.size; ++ll)
	{
		const Vector4 ldir = normalize(lid.world[ll].z) * view;
		const Vector3 lpos = translation(lid.world[ll]);

		bgfx::setUniform(_u_light_pos, to_float_ptr(lpos));
		bgfx::setUniform(_u_light_dir, to_float_ptr(ldir));
		bgfx::setUniform(_u_light_col, to_float_ptr(lid.color[ll]));

		// Render meshes
		for (u32 i = 0; i < mid.first_hidden; ++i)
		{
			bgfx::setTransform(to_float_ptr(mid.world[i]));
			bgfx::setVertexBuffer(mid.mesh[i].vbh);
			bgfx::setIndexBuffer(mid.mesh[i].ibh);

			_material_manager->get(mid.material[i])->bind(*_resource_manager, *_shader_manager);
		}
	}

	// Render sprites
	for (u32 i = 0; i < sid.first_hidden; ++i)
	{
		bgfx::setVertexBuffer(sid.sprite[i].vbh);
		bgfx::setIndexBuffer(sid.sprite[i].ibh, sid.frame[i] * 6, 6);
		bgfx::setTransform(to_float_ptr(sid.world[i]));

		_material_manager->get(sid.material[i])->bind(*_resource_manager, *_shader_manager);
	}
}

void RenderWorld::light_debug_draw(LightInstance i, DebugLine& dl)
{
	LightManager::LightInstanceData& lid = _light_manager._data;

	const Vector3 pos = translation(lid.world[i.i]);
	const Vector3 dir = -z(lid.world[i.i]);

	switch (lid.type[i.i])
	{
	case LightType::DIRECTIONAL:
		{
			const Vector3 end = pos + dir*3.0f;
			dl.add_line(pos, end, COLOR4_YELLOW);
			dl.add_cone(pos + dir*2.8f, end, 0.1f, COLOR4_YELLOW);
		}
		break;

	case LightType::OMNI:
		dl.add_sphere(pos, lid.range[i.i], COLOR4_YELLOW);
		break;

	case LightType::SPOT:
		{
			const f32 angle  = lid.spot_angle[i.i];
			const f32 range  = lid.range[i.i];
			const f32 radius = tan(angle)*range;
			dl.add_cone(pos + range*dir, pos, radius, COLOR4_YELLOW);
		}
		break;

	default:
		CE_FATAL("Unknown light type");
		break;
	}
}

void RenderWorld::debug_draw(DebugLine& dl)
{
	if (!_debug_drawing)
		return;

	MeshManager::MeshInstanceData& mid = _mesh_manager._data;
	SpriteManager::SpriteInstanceData& sid = _sprite_manager._data;
	LightManager::LightInstanceData& lid = _light_manager._data;

	for (u32 i = 0; i < mid.size; ++i)
	{
		const OBB& obb = mid.obb[i];
		const Matrix4x4& world = mid.world[i];
		dl.add_obb(obb.tm * world, obb.half_extents, COLOR4_RED);
	}

	for (u32 i = 0; i < lid.size; ++i)
		light_debug_draw({ i }, dl);
}

void RenderWorld::enable_debug_drawing(bool enable)
{
	_debug_drawing = enable;
}

void RenderWorld::unit_destroyed_callback(UnitId id)
{
	{
		MeshInstance curr = _mesh_manager.first(id);
		MeshInstance next;

		while (_mesh_manager.is_valid(curr))
		{
			next = _mesh_manager.next(curr);
			mesh_destroy(curr);
			curr = next;
		}
	}

	{
		SpriteInstance curr = _sprite_manager.first(id);
		SpriteInstance next;

		while (_sprite_manager.is_valid(curr))
		{
			next = _sprite_manager.next(curr);
			sprite_destroy(curr);
			curr = next;
		}
	}

	{
		LightInstance first = light(id);

		if (_light_manager.is_valid(first))
			light_destroy(first);
	}
}

void RenderWorld::MeshManager::allocate(u32 num)
{
	CE_ENSURE(num > _data.size);

	const u32 bytes = num * (0
		+ sizeof(UnitId)
		+ sizeof(MeshResource*)
		+ sizeof(MeshGeometry*)
		+ sizeof(MeshData)
		+ sizeof(StringId64)
		+ sizeof(Matrix4x4)
		+ sizeof(OBB)
		+ sizeof(MeshInstance)
		);

	MeshInstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);
	new_data.first_hidden = _data.first_hidden;

	new_data.unit = (UnitId*)(new_data.buffer);
	new_data.resource = (const MeshResource**)(new_data.unit + num);
	new_data.geometry = (const MeshGeometry**)(new_data.resource + num);
	new_data.mesh = (MeshData*)(new_data.geometry + num);
	new_data.material = (StringId64*)(new_data.mesh + num);
	new_data.world = (Matrix4x4*)(new_data.material + num);
	new_data.obb = (OBB*)(new_data.world + num);
	new_data.next_instance = (MeshInstance*)(new_data.obb + num);

	memcpy(new_data.unit, _data.unit, _data.size * sizeof(UnitId));
	memcpy(new_data.resource, _data.resource, _data.size * sizeof(MeshResource*));
	memcpy(new_data.geometry, _data.geometry, _data.size * sizeof(MeshGeometry*));
	memcpy(new_data.mesh, _data.mesh, _data.size * sizeof(MeshData));
	memcpy(new_data.material, _data.material, _data.size * sizeof(StringId64));
	memcpy(new_data.world, _data.world, _data.size * sizeof(Matrix4x4));
	memcpy(new_data.obb, _data.obb, _data.size * sizeof(OBB));
	memcpy(new_data.next_instance, _data.next_instance, _data.size * sizeof(MeshInstance));

	_allocator->deallocate(_data.buffer);
	_data = new_data;
}

void RenderWorld::MeshManager::grow()
{
	allocate(_data.capacity * 2 + 1);
}

MeshInstance RenderWorld::MeshManager::create(UnitId id, const MeshResource* mr, const MeshGeometry* mg, StringId64 mat, const Matrix4x4& tr)
{
	if (_data.size == _data.capacity)
		grow();

	const u32 last = _data.size;

	_data.unit[last]          = id;
	_data.resource[last]      = mr;
	_data.geometry[last]      = mg;
	_data.mesh[last].vbh      = mg->vertex_buffer;
	_data.mesh[last].ibh      = mg->index_buffer;
	_data.material[last]      = mat;
	_data.world[last]         = tr;
	_data.obb[last]           = mg->obb;
	_data.next_instance[last] = make_instance(UINT32_MAX);

	++_data.size;
	++_data.first_hidden;

	MeshInstance curr = first(id);
	if (!is_valid(curr))
	{
		hash_map::set(_map, id, last);
	}
	else
	{
		add_node(curr, make_instance(last));
	}

	return make_instance(last);
}

void RenderWorld::MeshManager::destroy(MeshInstance i)
{
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	const u32 last             = _data.size - 1;
	const UnitId u             = _data.unit[i.i];
	const MeshInstance first_i = first(u);
	const MeshInstance last_i  = make_instance(last);

	swap_node(last_i, i);
	remove_node(first_i, i);

	_data.unit[i.i]          = _data.unit[last];
	_data.resource[i.i]      = _data.resource[last];
	_data.geometry[i.i]      = _data.geometry[last];
	_data.mesh[i.i].vbh      = _data.mesh[last].vbh;
	_data.mesh[i.i].ibh      = _data.mesh[last].ibh;
	_data.material[i.i]      = _data.material[last];
	_data.world[i.i]         = _data.world[last];
	_data.obb[i.i]           = _data.obb[last];
	_data.next_instance[i.i] = _data.next_instance[last];

	--_data.size;
	--_data.first_hidden;
}

bool RenderWorld::MeshManager::has(UnitId id)
{
	return is_valid(first(id));
}

MeshInstance RenderWorld::MeshManager::first(UnitId id)
{
	return make_instance(hash_map::get(_map, id, UINT32_MAX));
}

MeshInstance RenderWorld::MeshManager::next(MeshInstance i)
{
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	return _data.next_instance[i.i];
}

MeshInstance RenderWorld::MeshManager::previous(MeshInstance i)
{
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	const UnitId u = _data.unit[i.i];

	MeshInstance curr = first(u);
	MeshInstance prev = { UINT32_MAX };

	while (curr.i != i.i)
	{
		prev = curr;
		curr = next(curr);
	}

	return prev;
}

void RenderWorld::MeshManager::add_node(MeshInstance first, MeshInstance i)
{
	CE_ASSERT(first.i < _data.size, "Index out of bounds");
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	MeshInstance curr = first;
	while (is_valid(next(curr)))
		curr = next(curr);

	_data.next_instance[curr.i] = i;
}

void RenderWorld::MeshManager::remove_node(MeshInstance first, MeshInstance i)
{
	CE_ASSERT(first.i < _data.size, "Index out of bounds");
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	const UnitId u = _data.unit[first.i];

	if (i.i == first.i)
	{
		if (!is_valid(next(i)))
			hash_map::remove(_map, u);
		else
			hash_map::set(_map, u, next(i).i);
	}
	else
	{
		MeshInstance prev = previous(i);
		_data.next_instance[prev.i] = next(i);
	}
}

void RenderWorld::MeshManager::swap_node(MeshInstance a, MeshInstance b)
{
	CE_ASSERT(a.i < _data.size, "Index out of bounds");
	CE_ASSERT(b.i < _data.size, "Index out of bounds");

	const UnitId u = _data.unit[a.i];
	const MeshInstance first_i = first(u);

	if (a.i == first_i.i)
	{
		hash_map::set(_map, u, b.i);
	}
	else
	{
		const MeshInstance prev_a = previous(a);
		CE_ENSURE(prev_a.i != a.i);
		_data.next_instance[prev_a.i] = b;
	}
}

void RenderWorld::MeshManager::destroy()
{
	_allocator->deallocate(_data.buffer);
}

void RenderWorld::SpriteManager::allocate(u32 num)
{
	CE_ENSURE(num > _data.size);

	const u32 bytes = num * (0
		+ sizeof(UnitId)
		+ sizeof(SpriteResource**)
		+ sizeof(SpriteData)
		+ sizeof(StringId64)
		+ sizeof(u32)
		+ sizeof(Matrix4x4)
		+ sizeof(AABB)
		+ sizeof(SpriteInstance)
		);

	SpriteInstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);
	new_data.first_hidden = _data.first_hidden;

	new_data.unit = (UnitId*)(new_data.buffer);
	new_data.resource = (const SpriteResource**)(new_data.unit + num);
	new_data.sprite = (SpriteData*)(new_data.resource + num);
	new_data.material = (StringId64*)(new_data.sprite + num);
	new_data.frame = (u32*)(new_data.material + num);
	new_data.world = (Matrix4x4*)(new_data.frame + num);
	new_data.aabb = (AABB*)(new_data.world + num);
	new_data.next_instance = (SpriteInstance*)(new_data.aabb + num);

	memcpy(new_data.unit, _data.unit, _data.size * sizeof(UnitId));
	memcpy(new_data.resource, _data.resource, _data.size * sizeof(SpriteResource**));
	memcpy(new_data.sprite, _data.sprite, _data.size * sizeof(SpriteData));
	memcpy(new_data.material, _data.material, _data.size * sizeof(StringId64));
	memcpy(new_data.frame, _data.frame, _data.size * sizeof(u32));
	memcpy(new_data.world, _data.world, _data.size * sizeof(Matrix4x4));
	memcpy(new_data.aabb, _data.aabb, _data.size * sizeof(AABB));
	memcpy(new_data.next_instance, _data.next_instance, _data.size * sizeof(SpriteInstance));

	_allocator->deallocate(_data.buffer);
	_data = new_data;
}

void RenderWorld::SpriteManager::grow()
{
	allocate(_data.capacity * 2 + 1);
}

SpriteInstance RenderWorld::SpriteManager::create(UnitId id, const SpriteResource* sr, StringId64 mat, const Matrix4x4& tr)
{
	if (_data.size == _data.capacity)
		grow();

	const u32 last = _data.size;

	_data.unit[last]          = id;
	_data.resource[last]      = sr;
	_data.sprite[last].vbh    = sr->vb;
	_data.sprite[last].ibh    = sr->ib;
	_data.material[last]      = mat;
	_data.frame[last]         = 0;
	_data.world[last]         = tr;
	_data.aabb[last]          = AABB();
	_data.next_instance[last] = make_instance(UINT32_MAX);

	++_data.size;
	++_data.first_hidden;

	SpriteInstance curr = first(id);
	if (!is_valid(curr))
	{
		hash_map::set(_map, id, last);
	}
	else
	{
		add_node(curr, make_instance(last));
	}

	return make_instance(last);
}

void RenderWorld::SpriteManager::destroy(SpriteInstance i)
{
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	const u32 last               = _data.size - 1;
	const UnitId u               = _data.unit[i.i];
	const SpriteInstance first_i = first(u);
	const SpriteInstance last_i  = make_instance(last);

	swap_node(last_i, i);
	remove_node(first_i, i);

	_data.unit[i.i]          = _data.unit[last];
	_data.resource[i.i]      = _data.resource[last];
	_data.sprite[i.i].vbh    = _data.sprite[last].vbh;
	_data.sprite[i.i].ibh    = _data.sprite[last].ibh;
	_data.material[i.i]      = _data.material[last];
	_data.frame[i.i]         = _data.frame[last];
	_data.world[i.i]         = _data.world[last];
	_data.aabb[i.i]          = _data.aabb[last];
	_data.next_instance[i.i] = _data.next_instance[last];

	--_data.size;
	--_data.first_hidden;
}

bool RenderWorld::SpriteManager::has(UnitId id)
{
	return is_valid(first(id));
}

SpriteInstance RenderWorld::SpriteManager::first(UnitId id)
{
	return make_instance(hash_map::get(_map, id, UINT32_MAX));
}

SpriteInstance RenderWorld::SpriteManager::next(SpriteInstance i)
{
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	return _data.next_instance[i.i];
}

SpriteInstance RenderWorld::SpriteManager::previous(SpriteInstance i)
{
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	const UnitId u = _data.unit[i.i];

	SpriteInstance curr = first(u);
	SpriteInstance prev = { UINT32_MAX };

	while (curr.i != i.i)
	{
		prev = curr;
		curr = next(curr);
	}

	return prev;
}

void RenderWorld::SpriteManager::add_node(SpriteInstance first, SpriteInstance i)
{
	CE_ASSERT(first.i < _data.size, "Index out of bounds");
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	SpriteInstance curr = first;
	while (is_valid(next(curr)))
		curr = next(curr);

	_data.next_instance[curr.i] = i;
}

void RenderWorld::SpriteManager::remove_node(SpriteInstance first, SpriteInstance i)
{
	CE_ASSERT(first.i < _data.size, "Index out of bounds");
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	const UnitId u = _data.unit[first.i];

	if (i.i == first.i)
	{
		if (!is_valid(next(i)))
			hash_map::remove(_map, u);
		else
			hash_map::set(_map, u, next(i).i);
	}
	else
	{
		SpriteInstance prev = previous(i);
		_data.next_instance[prev.i] = next(i);
	}
}

void RenderWorld::SpriteManager::swap_node(SpriteInstance a, SpriteInstance b)
{
	CE_ASSERT(a.i < _data.size, "Index out of bounds");
	CE_ASSERT(b.i < _data.size, "Index out of bounds");

	const UnitId u = _data.unit[a.i];
	const SpriteInstance first_i = first(u);

	if (a.i == first_i.i)
	{
		hash_map::set(_map, u, b.i);
	}
	else
	{
		const SpriteInstance prev_a = previous(a);
		_data.next_instance[prev_a.i] = b;
	}
}

void RenderWorld::SpriteManager::destroy()
{
	_allocator->deallocate(_data.buffer);
}

void RenderWorld::LightManager::allocate(u32 num)
{
	CE_ENSURE(num > _data.size);

	const u32 bytes = num * (0
		+ sizeof(UnitId)
		+ sizeof(Matrix4x4)
		+ sizeof(f32)
		+ sizeof(f32)
		+ sizeof(f32)
		+ sizeof(Color4)
		+ sizeof(u32)
		);

	LightInstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);

	new_data.unit = (UnitId*)(new_data.buffer);
	new_data.world = (Matrix4x4*)(new_data.unit + num);
	new_data.range = (f32*)(new_data.world + num);
	new_data.intensity = (f32*)(new_data.range + num);
	new_data.spot_angle = (f32*)(new_data.intensity + num);
	new_data.color = (Color4*)(new_data.spot_angle + num);
	new_data.type = (u32*)(new_data.color + num);

	memcpy(new_data.unit, _data.unit, _data.size * sizeof(UnitId));
	memcpy(new_data.world, _data.world, _data.size * sizeof(Matrix4x4));
	memcpy(new_data.range, _data.range, _data.size * sizeof(f32));
	memcpy(new_data.intensity, _data.intensity, _data.size * sizeof(f32));
	memcpy(new_data.spot_angle, _data.spot_angle, _data.size * sizeof(f32));
	memcpy(new_data.color, _data.color, _data.size * sizeof(Color4));
	memcpy(new_data.type, _data.type, _data.size * sizeof(u32));

	_allocator->deallocate(_data.buffer);
	_data = new_data;
}

void RenderWorld::LightManager::grow()
{
	allocate(_data.capacity * 2 + 1);
}

LightInstance RenderWorld::LightManager::create(UnitId id, const LightDesc& ld, const Matrix4x4& tr)
{
	CE_ASSERT(!hash_map::has(_map, id), "Unit already has light");

	if (_data.size == _data.capacity)
		grow();

	const u32 last = _data.size;

	_data.unit[last]       = id;
	_data.world[last]      = tr;
	_data.range[last]      = ld.range;
	_data.intensity[last]  = ld.intensity;
	_data.spot_angle[last] = ld.spot_angle;
	_data.color[last]      = vector4(ld.color.x, ld.color.y, ld.color.z, 1.0f);
	_data.type[last]       = ld.type;

	++_data.size;

	hash_map::set(_map, id, last);
	return make_instance(last);
}

void RenderWorld::LightManager::destroy(LightInstance i)
{
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	const u32 last      = _data.size - 1;
	const UnitId u      = _data.unit[i.i];
	const UnitId last_u = _data.unit[last];

	_data.unit[i.i]       = _data.unit[last];
	_data.world[i.i]      = _data.world[last];
	_data.range[i.i]      = _data.range[last];
	_data.intensity[i.i]  = _data.intensity[last];
	_data.spot_angle[i.i] = _data.spot_angle[last];
	_data.color[i.i]      = _data.color[last];
	_data.type[i.i]       = _data.type[last];

	--_data.size;

	hash_map::set(_map, last_u, i.i);
	hash_map::remove(_map, u);
}

bool RenderWorld::LightManager::has(UnitId id)
{
	return is_valid(light(id));
}

LightInstance RenderWorld::LightManager::light(UnitId id)
{
	return make_instance(hash_map::get(_map, id, UINT32_MAX));
}

void RenderWorld::LightManager::destroy()
{
	_allocator->deallocate(_data.buffer);
}

} // namespace crown
