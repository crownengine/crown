/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/hash_map.h"
#include "core/math/aabb.h"
#include "core/math/color4.h"
#include "core/math/intersection.h"
#include "core/math/matrix4x4.h"
#include "device/pipeline.h"
#include "resource/mesh_resource.h"
#include "resource/resource_manager.h"
#include "resource/sprite_resource.h"
#include "world/debug_line.h"
#include "world/material.h"
#include "world/material_manager.h"
#include "world/render_world.h"
#include "world/unit_manager.h"
#include <bgfx/bgfx.h>

namespace crown
{
static void unit_destroyed_callback_bridge(UnitId id, void* user_ptr)
{
	((RenderWorld*)user_ptr)->unit_destroyed_callback(id);
}

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
	um.register_destroy_function(unit_destroyed_callback_bridge, this);

	_u_light_position  = bgfx::createUniform("u_light_position", bgfx::UniformType::Vec4);
	_u_light_direction = bgfx::createUniform("u_light_direction", bgfx::UniformType::Vec4);
	_u_light_color     = bgfx::createUniform("u_light_color", bgfx::UniformType::Vec4);
	_u_light_range     = bgfx::createUniform("u_light_range", bgfx::UniformType::Vec4);
	_u_light_intensity = bgfx::createUniform("u_light_intensity", bgfx::UniformType::Vec4);
}

RenderWorld::~RenderWorld()
{
	_unit_manager->unregister_destroy_function(this);

	bgfx::destroy(_u_light_intensity);
	bgfx::destroy(_u_light_range);
	bgfx::destroy(_u_light_color);
	bgfx::destroy(_u_light_direction);
	bgfx::destroy(_u_light_position);

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

	while (is_valid(inst))
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

f32 RenderWorld::mesh_cast_ray(MeshInstance i, const Vector3& from, const Vector3& dir)
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

SpriteInstance RenderWorld::sprite_create(UnitId unit, const SpriteRendererDesc& srd, const Matrix4x4& tr)
{
	const SpriteResource* sr = (const SpriteResource*)_resource_manager->get(RESOURCE_TYPE_SPRITE, srd.sprite_resource);
	_material_manager->create_material(srd.material_resource);

	return _sprite_manager.create(unit
		, sr
		, srd.material_resource
		, srd.layer
		, srd.depth
		, tr
		);
}

void RenderWorld::sprite_destroy(UnitId unit, SpriteInstance /*i*/)
{
	SpriteInstance i = _sprite_manager.sprite(unit);
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager.destroy(i);
}

SpriteInstance RenderWorld::sprite_instances(UnitId unit)
{
	return _sprite_manager.sprite(unit);
}

void RenderWorld::sprite_set_material(UnitId unit, StringId64 id)
{
	SpriteInstance i = _sprite_manager.sprite(unit);
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.material[i.i] = id;
}

void RenderWorld::sprite_set_frame(UnitId unit, u32 index)
{
	SpriteInstance i = _sprite_manager.sprite(unit);
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.frame[i.i] = index;
}

void RenderWorld::sprite_set_visible(UnitId unit, bool visible)
{
	SpriteInstance i = _sprite_manager.sprite(unit);
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager.set_visible(i, visible);
}

void RenderWorld::sprite_flip_x(UnitId unit, bool flip)
{
	SpriteInstance i = _sprite_manager.sprite(unit);
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.flip_x[i.i] = flip;
}

void RenderWorld::sprite_flip_y(UnitId unit, bool flip)
{
	SpriteInstance i = _sprite_manager.sprite(unit);
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.flip_y[i.i] = flip;
}

void RenderWorld::sprite_set_layer(UnitId unit, u32 layer)
{
	SpriteInstance i = _sprite_manager.sprite(unit);
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.layer[i.i] = layer;
}

void RenderWorld::sprite_set_depth(UnitId unit, u32 depth)
{
	SpriteInstance i = _sprite_manager.sprite(unit);
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.depth[i.i] = depth;
}

OBB RenderWorld::sprite_obb(UnitId unit)
{
	SpriteInstance i = _sprite_manager.sprite(unit);
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");

	const OBB& obb = _sprite_manager._data.resource[i.i]->obb;
	const Matrix4x4& world = _sprite_manager._data.world[i.i];

	OBB o;
	o.tm = obb.tm * world;
	o.half_extents = obb.half_extents;

	return o;
}

f32 RenderWorld::sprite_cast_ray(UnitId unit, const Vector3& from, const Vector3& dir, u32& layer, u32& depth)
{
	SpriteInstance i = _sprite_manager.sprite(unit);
	CE_ASSERT(i.i < _sprite_manager._data.size, "Index out of bounds");

	const SpriteManager::SpriteInstanceData& sid = _sprite_manager._data;
	const f32* frame = sprite_resource::frame_data(sid.resource[i.i], sid.frame[i.i]);

	const f32 vertices[] =
	{
		frame[ 0], frame[ 1], frame[ 2],
		frame[ 5], frame[ 6], frame[ 7],
		frame[10], frame[11], frame[12],
		frame[15], frame[16], frame[17]
	};

	const u16 indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	layer = _sprite_manager._data.layer[i.i];
	depth = _sprite_manager._data.depth[i.i];

	return ray_mesh_intersection(from
		, dir
		, _sprite_manager._data.world[i.i]
		, vertices
		, sizeof(Vector3)
		, indices
		, 6
		);
}

LightInstance RenderWorld::light_create(UnitId unit, const LightDesc& ld, const Matrix4x4& tr)
{
	return _light_manager.create(unit, ld, tr);
}

void RenderWorld::light_destroy(UnitId unit, LightInstance /*i*/)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager.destroy(i);
}

LightInstance RenderWorld::light_instances(UnitId unit)
{
	return _light_manager.light(unit);
}

Color4 RenderWorld::light_color(UnitId unit)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.color[i.i];
}

LightType::Enum RenderWorld::light_type(UnitId unit)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	return (LightType::Enum)_light_manager._data.type[i.i];
}

f32 RenderWorld::light_range(UnitId unit)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.range[i.i];
}

f32 RenderWorld::light_intensity(UnitId unit)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.intensity[i.i];
}

f32 RenderWorld::light_spot_angle(UnitId unit)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.spot_angle[i.i];
}

void RenderWorld::light_set_color(UnitId unit, const Color4& col)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.color[i.i] = col;
}

void RenderWorld::light_set_type(UnitId unit, LightType::Enum type)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.type[i.i] = type;
}

void RenderWorld::light_set_range(UnitId unit, f32 range)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.range[i.i] = range;
}

void RenderWorld::light_set_intensity(UnitId unit, f32 intensity)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.intensity[i.i] = intensity;
}

void RenderWorld::light_set_spot_angle(UnitId unit, f32 angle)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.spot_angle[i.i] = angle;
}

void RenderWorld::light_debug_draw(UnitId unit, DebugLine& dl)
{
	LightInstance i = _light_manager.light(unit);
	CE_ASSERT(i.i < _light_manager._data.size, "Index out of bounds");
	_light_manager.debug_draw(i.i, 1, dl);
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
			SpriteInstance inst = _sprite_manager.sprite(*begin);
			sid.world[inst.i] = *world;
		}

		if (_light_manager.has(*begin))
		{
			LightInstance inst = _light_manager.light(*begin);
			lid.world[inst.i] = *world;
		}
	}
}

void RenderWorld::render(const Matrix4x4& view)
{
	MeshManager::MeshInstanceData& mid = _mesh_manager._data;
	SpriteManager::SpriteInstanceData& sid = _sprite_manager._data;
	LightManager::LightInstanceData& lid = _light_manager._data;

	for (u32 ll = 0; ll < lid.size; ++ll)
	{
		const Vector4 ldir = normalize(lid.world[ll].z) * view;
		const Vector3 lpos = translation(lid.world[ll]);

		bgfx::setUniform(_u_light_position, to_float_ptr(lpos));
		bgfx::setUniform(_u_light_direction, to_float_ptr(ldir));
		bgfx::setUniform(_u_light_color, to_float_ptr(lid.color[ll]));
		bgfx::setUniform(_u_light_range, &lid.range[ll]);
		bgfx::setUniform(_u_light_intensity, &lid.intensity[ll]);

		// Render meshes
		for (u32 i = 0; i < mid.first_hidden; ++i)
		{
			bgfx::setTransform(to_float_ptr(mid.world[i]));
			bgfx::setVertexBuffer(0, mid.mesh[i].vbh);
			bgfx::setIndexBuffer(mid.mesh[i].ibh);

			_material_manager->get(mid.material[i])->bind(*_resource_manager, *_shader_manager, VIEW_MESH);
		}
	}

	// Render sprites
	if (sid.first_hidden)
	{
		bgfx::VertexDecl decl;
		decl.begin()
			.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, false)
			.end()
			;
		bgfx::TransientVertexBuffer tvb;
		bgfx::allocTransientVertexBuffer(&tvb, 4*sid.first_hidden, decl);
		bgfx::TransientIndexBuffer tib;
		bgfx::allocTransientIndexBuffer(&tib, 6*sid.first_hidden);

		f32* vdata = (f32*)tvb.data;
		u16* idata = (u16*)tib.data;

		// Render sprites
		for (u32 i = 0; i < sid.first_hidden; ++i)
		{
			const f32* frame = sprite_resource::frame_data(sid.resource[i], sid.frame[i]);

			f32 u0 = frame[ 3]; // u
			f32 v0 = frame[ 4]; // v

			f32 u1 = frame[ 8]; // u
			f32 v1 = frame[ 9]; // v

			f32 u2 = frame[13]; // u
			f32 v2 = frame[14]; // v

			f32 u3 = frame[18]; // u
			f32 v3 = frame[19]; // v

			if (sid.flip_x[i])
			{
				f32 u;
				u = u0; u0 = u1; u1 = u;
				u = u2; u2 = u3; u3 = u;
			}

			if (sid.flip_y[i])
			{
				f32 v;
				v = v0; v0 = v2; v2 = v;
				v = v1; v1 = v3; v3 = v;
			}

			vdata[ 0] = frame[ 0]; // x
			vdata[ 1] = frame[ 1]; // y
			vdata[ 2] = frame[ 2]; // z
			vdata[ 3] = u0;
			vdata[ 4] = v0;

			vdata[ 5] = frame[ 5]; // x
			vdata[ 6] = frame[ 6]; // y
			vdata[ 7] = frame[ 7]; // z
			vdata[ 8] = u1;
			vdata[ 9] = v1;

			vdata[10] = frame[10]; // x
			vdata[11] = frame[11]; // y
			vdata[12] = frame[12]; // z
			vdata[13] = u2;
			vdata[14] = v2;

			vdata[15] = frame[15]; // x
			vdata[16] = frame[16]; // y
			vdata[17] = frame[17]; // z
			vdata[18] = u3;
			vdata[19] = v3;

			vdata += 20;

			*idata++ = i*4+0;
			*idata++ = i*4+1;
			*idata++ = i*4+2;
			*idata++ = i*4+0;
			*idata++ = i*4+2;
			*idata++ = i*4+3;

			bgfx::setTransform(to_float_ptr(sid.world[i]));
			bgfx::setVertexBuffer(0, &tvb);
			bgfx::setIndexBuffer(&tib, i*6, 6);

			_material_manager->get(sid.material[i])->bind(*_resource_manager
				, *_shader_manager
				, sid.layer[i] + VIEW_SPRITE_0
				, sid.depth[i]
				);
		}
	}
}

void RenderWorld::debug_draw(DebugLine& dl)
{
	if (!_debug_drawing)
		return;

	MeshManager::MeshInstanceData& mid = _mesh_manager._data;

	for (u32 i = 0; i < mid.size; ++i)
	{
		const OBB& obb = mid.obb[i];
		const Matrix4x4& world = mid.world[i];
		dl.add_obb(obb.tm * world, obb.half_extents, COLOR4_RED);
	}

	_light_manager.debug_draw(0, _light_manager._data.size, dl);
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

		while (is_valid(curr))
		{
			next = _mesh_manager.next(curr);
			mesh_destroy(curr);
			curr = next;
		}
	}

	{
		SpriteInstance first = sprite_instances(id);

		if (is_valid(first))
			sprite_destroy(id, first);
	}

	{
		LightInstance first = light_instances(id);

		if (is_valid(first))
			light_destroy(id, first);
	}
}

void RenderWorld::MeshManager::allocate(u32 num)
{
	CE_ENSURE(num > _data.size);

	const u32 bytes = 0
		+ num*sizeof(UnitId) + alignof(UnitId)
		+ num*sizeof(MeshResource*) + alignof(MeshResource*)
		+ num*sizeof(MeshGeometry*) + alignof(MeshGeometry*)
		+ num*sizeof(MeshData) + alignof(MeshData)
		+ num*sizeof(StringId64) + alignof(StringId64)
		+ num*sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ num*sizeof(OBB) + alignof(OBB)
		+ num*sizeof(MeshInstance) + alignof(MeshInstance)
		;

	MeshInstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);
	new_data.first_hidden = _data.first_hidden;

	new_data.unit          = (UnitId*             )memory::align_top(new_data.buffer,         alignof(UnitId       ));
	new_data.resource      = (const MeshResource**)memory::align_top(new_data.unit + num,     alignof(MeshResource*));
	new_data.geometry      = (const MeshGeometry**)memory::align_top(new_data.resource + num, alignof(MeshGeometry*));
	new_data.mesh          = (MeshData*           )memory::align_top(new_data.geometry + num, alignof(MeshData     ));
	new_data.material      = (StringId64*         )memory::align_top(new_data.mesh + num,     alignof(StringId64   ));
	new_data.world         = (Matrix4x4*          )memory::align_top(new_data.material + num, alignof(Matrix4x4    ));
	new_data.obb           = (OBB*                )memory::align_top(new_data.world + num,    alignof(OBB          ));
	new_data.next_instance = (MeshInstance*       )memory::align_top(new_data.obb + num,      alignof(MeshInstance ));

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

	const u32 bytes = 0
		+ num*sizeof(UnitId) + alignof(UnitId)
		+ num*sizeof(SpriteResource**) + alignof(SpriteResource*)
		+ num*sizeof(StringId64) + alignof(StringId64)
		+ num*sizeof(u32) + alignof(u32)
		+ num*sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ num*sizeof(AABB) + alignof(AABB)
		+ num*sizeof(bool) + alignof(bool)
		+ num*sizeof(bool) + alignof(bool)
		+ num*sizeof(u32) + alignof(u32)
		+ num*sizeof(u32) + alignof(u32)
		;

	SpriteInstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);
	new_data.first_hidden = _data.first_hidden;

	new_data.unit     = (UnitId*               )memory::align_top(new_data.buffer,         alignof(UnitId         ));
	new_data.resource = (const SpriteResource**)memory::align_top(new_data.unit + num,     alignof(SpriteResource*));
	new_data.material = (StringId64*           )memory::align_top(new_data.resource + num, alignof(StringId64     ));
	new_data.frame    = (u32*                  )memory::align_top(new_data.material + num, alignof(u32            ));
	new_data.world    = (Matrix4x4*            )memory::align_top(new_data.frame + num,    alignof(Matrix4x4      ));
	new_data.aabb     = (AABB*                 )memory::align_top(new_data.world + num,    alignof(AABB           ));
	new_data.flip_x   = (bool*                 )memory::align_top(new_data.aabb + num,     alignof(bool           ));
	new_data.flip_y   = (bool*                 )memory::align_top(new_data.flip_x + num,   alignof(bool           ));
	new_data.layer    = (u32*                  )memory::align_top(new_data.flip_y + num,   alignof(u32            ));
	new_data.depth    = (u32*                  )memory::align_top(new_data.layer + num,    alignof(u32            ));

	memcpy(new_data.unit, _data.unit, _data.size * sizeof(UnitId));
	memcpy(new_data.resource, _data.resource, _data.size * sizeof(SpriteResource**));
	memcpy(new_data.material, _data.material, _data.size * sizeof(StringId64));
	memcpy(new_data.frame, _data.frame, _data.size * sizeof(u32));
	memcpy(new_data.world, _data.world, _data.size * sizeof(Matrix4x4));
	memcpy(new_data.aabb, _data.aabb, _data.size * sizeof(AABB));
	memcpy(new_data.flip_x, _data.flip_x, _data.size * sizeof(bool));
	memcpy(new_data.flip_y, _data.flip_y, _data.size * sizeof(bool));
	memcpy(new_data.layer, _data.layer, _data.size * sizeof(u32));
	memcpy(new_data.depth, _data.depth, _data.size * sizeof(u32));

	_allocator->deallocate(_data.buffer);
	_data = new_data;
}

void RenderWorld::SpriteManager::grow()
{
	allocate(_data.capacity * 2 + 1);
}

SpriteInstance RenderWorld::SpriteManager::create(UnitId id, const SpriteResource* sr, StringId64 mat, u32 layer, u32 depth, const Matrix4x4& tr)
{
	if (_data.size == _data.capacity)
		grow();

	const u32 last = _data.size;

	_data.unit[last]     = id;
	_data.resource[last] = sr;
	_data.material[last] = mat;
	_data.frame[last]    = 0;
	_data.world[last]    = tr;
	_data.aabb[last]     = AABB();
	_data.flip_x[last]   = false;
	_data.flip_y[last]   = false;
	_data.layer[last]    = layer;
	_data.depth[last]    = depth;

	++_data.size;
	++_data.first_hidden;

	hash_map::set(_map, id, last);
	return make_instance(last);
}

void RenderWorld::SpriteManager::destroy(SpriteInstance i)
{
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	const u32 last      = _data.size - 1;
	const UnitId u      = _data.unit[i.i];
	const UnitId last_u = _data.unit[last];

	_data.unit[i.i]     = _data.unit[last];
	_data.resource[i.i] = _data.resource[last];
	_data.material[i.i] = _data.material[last];
	_data.frame[i.i]    = _data.frame[last];
	_data.world[i.i]    = _data.world[last];
	_data.aabb[i.i]     = _data.aabb[last];
	_data.flip_x[i.i]   = _data.flip_x[last];
	_data.flip_y[i.i]   = _data.flip_y[last];
	_data.layer[i.i]    = _data.layer[last];
	_data.depth[i.i]    = _data.depth[last];

	--_data.size;
	--_data.first_hidden;

	hash_map::set(_map, last_u, i.i);
	hash_map::remove(_map, u);
}

bool RenderWorld::SpriteManager::has(UnitId id)
{
	return is_valid(sprite(id));
}

void RenderWorld::SpriteManager::set_visible(SpriteInstance i, bool visible)
{
	u32 swap_index = UINT32_MAX;
	const UnitId unit = _data.unit[i.i];

	if (visible && i.i >= _data.first_hidden)
	{
		const u32 first_hidden = _data.first_hidden;
		const UnitId first_hidden_unit = _data.unit[first_hidden];
		hash_map::set(_map, unit, first_hidden);
		hash_map::set(_map, first_hidden_unit, i.i);
		swap_index = first_hidden;
		++_data.first_hidden;
	}
	else if (!visible && i.i < _data.first_hidden)
	{
		const u32 last_visible = _data.first_hidden - 1;
		const UnitId last_visible_unit = _data.unit[last_visible];
		hash_map::set(_map, unit, last_visible);
		hash_map::set(_map, last_visible_unit, i.i);
		swap_index = last_visible;
		--_data.first_hidden;
	}

	if (swap_index != UINT32_MAX)
	{
		exchange(_data.unit[i.i], _data.unit[swap_index]);
		exchange(_data.resource[i.i], _data.resource[swap_index]);
		exchange(_data.material[i.i], _data.material[swap_index]);
		exchange(_data.frame[i.i], _data.frame[swap_index]);
		exchange(_data.world[i.i], _data.world[swap_index]);
		exchange(_data.aabb[i.i], _data.aabb[swap_index]);
		exchange(_data.flip_x[i.i], _data.flip_x[swap_index]);
		exchange(_data.flip_y[i.i], _data.flip_y[swap_index]);
		exchange(_data.layer[i.i], _data.layer[swap_index]);
		exchange(_data.depth[i.i], _data.depth[swap_index]);
	}
}

SpriteInstance RenderWorld::SpriteManager::sprite(UnitId id)
{
	return make_instance(hash_map::get(_map, id, UINT32_MAX));
}

void RenderWorld::SpriteManager::destroy()
{
	_allocator->deallocate(_data.buffer);
}

void RenderWorld::LightManager::allocate(u32 num)
{
	CE_ENSURE(num > _data.size);

	const u32 bytes = 0
		+ num*sizeof(UnitId) + alignof(UnitId)
		+ num*sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ num*sizeof(f32) + alignof(f32)
		+ num*sizeof(f32) + alignof(f32)
		+ num*sizeof(f32) + alignof(f32)
		+ num*sizeof(Color4) + alignof(Color4)
		+ num*sizeof(u32) + alignof(u32)
		;

	LightInstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);

	new_data.unit       = (UnitId*   )memory::align_top(new_data.buffer,           alignof(UnitId   ));
	new_data.world      = (Matrix4x4*)memory::align_top(new_data.unit + num,       alignof(Matrix4x4));
	new_data.range      = (f32*      )memory::align_top(new_data.world + num,      alignof(f32      ));
	new_data.intensity  = (f32*      )memory::align_top(new_data.range + num,      alignof(f32      ));
	new_data.spot_angle = (f32*      )memory::align_top(new_data.intensity + num,  alignof(f32      ));
	new_data.color      = (Color4*   )memory::align_top(new_data.spot_angle + num, alignof(Color4   ));
	new_data.type       = (u32*      )memory::align_top(new_data.color + num,      alignof(u32      ));

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

void RenderWorld::LightManager::debug_draw(u32 start_index, u32 num, DebugLine& dl)
{
	for (u32 i = start_index; i < start_index + num; ++i)
	{
		const Vector3 pos = translation(_data.world[i]);
		const Vector3 dir = -z(_data.world[i]);

		switch (_data.type[i])
		{
		case LightType::DIRECTIONAL:
			{
				const Vector3 end = pos + dir*3.0f;
				dl.add_line(pos, end, COLOR4_YELLOW);
				dl.add_cone(pos + dir*2.8f, end, 0.1f, COLOR4_YELLOW);
			}
			break;

		case LightType::OMNI:
			dl.add_sphere(pos, _data.range[i], COLOR4_YELLOW);
			break;

		case LightType::SPOT:
			{
				const f32 angle  = _data.spot_angle[i];
				const f32 range  = _data.range[i];
				const f32 radius = ftan(angle)*range;
				dl.add_cone(pos + range*dir, pos, radius, COLOR4_YELLOW);
			}
			break;

		default:
			CE_FATAL("Unknown light type");
			break;
		}
	}
}

} // namespace crown
