/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/list.inl"
#include "core/math/aabb.h"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/intersection.h"
#include "core/math/matrix4x4.inl"
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
static void unit_destroyed_callback_bridge(UnitId unit, void* user_ptr)
{
	((RenderWorld*)user_ptr)->unit_destroyed_callback(unit);
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
	_unit_destroy_callback.destroy = unit_destroyed_callback_bridge;
	_unit_destroy_callback.user_data = this;
	_unit_destroy_callback.node.next = NULL;
	_unit_destroy_callback.node.prev = NULL;
	um.register_destroy_callback(&_unit_destroy_callback);

	_u_light_position  = bgfx::createUniform("u_light_position", bgfx::UniformType::Vec4);
	_u_light_direction = bgfx::createUniform("u_light_direction", bgfx::UniformType::Vec4);
	_u_light_color     = bgfx::createUniform("u_light_color", bgfx::UniformType::Vec4);
	_u_light_range     = bgfx::createUniform("u_light_range", bgfx::UniformType::Vec4);
	_u_light_intensity = bgfx::createUniform("u_light_intensity", bgfx::UniformType::Vec4);
}

RenderWorld::~RenderWorld()
{
	_unit_manager->unregister_destroy_callback(&_unit_destroy_callback);

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

MeshInstance RenderWorld::mesh_create(UnitId unit, const MeshRendererDesc& mrd, const Matrix4x4& tr)
{
	const MeshResource* mr = (const MeshResource*)_resource_manager->get(RESOURCE_TYPE_MESH, mrd.mesh_resource);
	_material_manager->create_material(mrd.material_resource);
	return _mesh_manager.create(unit, mr, mrd, tr);
}

void RenderWorld::mesh_destroy(MeshInstance mesh)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	_mesh_manager.destroy(mesh);
}

MeshInstance RenderWorld::mesh_instance(UnitId unit)
{
	return _mesh_manager.mesh(unit);
}

Material* RenderWorld::mesh_material(MeshInstance mesh)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	return _material_manager->get(_mesh_manager._data.material[mesh.i]);
}

void RenderWorld::mesh_set_material(MeshInstance mesh, StringId64 id)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	_material_manager->create_material(id);
	_mesh_manager._data.material[mesh.i] = id;
}

void RenderWorld::mesh_set_visible(MeshInstance mesh, bool visible)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	_mesh_manager.set_visible(mesh, visible);
}

OBB RenderWorld::mesh_obb(MeshInstance mesh)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");

	const Matrix4x4& world = _mesh_manager._data.world[mesh.i];
	const OBB& obb = _mesh_manager._data.obb[mesh.i];

	OBB o;
	o.tm = obb.tm * world;
	o.half_extents = obb.half_extents;

	return o;
}

f32 RenderWorld::mesh_cast_ray(MeshInstance mesh, const Vector3& from, const Vector3& dir)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	const MeshGeometry* mg = _mesh_manager._data.geometry[mesh.i];
	return ray_mesh_intersection(from
		, dir
		, _mesh_manager._data.world[mesh.i]
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
	return _sprite_manager.create(unit, sr, srd, tr);
}

void RenderWorld::sprite_destroy(SpriteInstance sprite)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager.destroy(sprite);
}

SpriteInstance RenderWorld::sprite_instance(UnitId unit)
{
	return _sprite_manager.sprite(unit);
}

Material* RenderWorld::sprite_material(SpriteInstance sprite)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	return _material_manager->get(_sprite_manager._data.material[sprite.i]);
}

void RenderWorld::sprite_set_material(SpriteInstance sprite, StringId64 id)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	_material_manager->create_material(id);
	_sprite_manager._data.material[sprite.i] = id;
}

void RenderWorld::sprite_set_frame(SpriteInstance sprite, u32 index)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.frame[sprite.i] = index;
}

void RenderWorld::sprite_set_visible(SpriteInstance sprite, bool visible)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager.set_visible(sprite, visible);
}

void RenderWorld::sprite_flip_x(SpriteInstance sprite, bool flip)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.flip_x[sprite.i] = flip;
}

void RenderWorld::sprite_flip_y(SpriteInstance sprite, bool flip)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.flip_y[sprite.i] = flip;
}

void RenderWorld::sprite_set_layer(SpriteInstance sprite, u32 layer)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.layer[sprite.i] = layer;
}

void RenderWorld::sprite_set_depth(SpriteInstance sprite, u32 depth)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	_sprite_manager._data.depth[sprite.i] = depth;
}

OBB RenderWorld::sprite_obb(SpriteInstance sprite)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");

	const OBB& obb = _sprite_manager._data.resource[sprite.i]->obb;
	const Matrix4x4& world = _sprite_manager._data.world[sprite.i];

	OBB o;
	o.tm = obb.tm * world;
	o.half_extents = obb.half_extents;

	return o;
}

f32 RenderWorld::sprite_cast_ray(SpriteInstance sprite, const Vector3& from, const Vector3& dir, u32& layer, u32& depth)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");

	const SpriteManager::SpriteInstanceData& sid = _sprite_manager._data;
	const f32* frame = sprite_resource::frame_data(sid.resource[sprite.i], sid.frame[sprite.i]);

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

	layer = _sprite_manager._data.layer[sprite.i];
	depth = _sprite_manager._data.depth[sprite.i];

	return ray_mesh_intersection(from
		, dir
		, _sprite_manager._data.world[sprite.i]
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

void RenderWorld::light_destroy(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager.destroy(light);
}

LightInstance RenderWorld::light_instance(UnitId unit)
{
	return _light_manager.light(unit);
}

Color4 RenderWorld::light_color(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.color[light.i];
}

LightType::Enum RenderWorld::light_type(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	return (LightType::Enum)_light_manager._data.type[light.i];
}

f32 RenderWorld::light_range(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.range[light.i];
}

f32 RenderWorld::light_intensity(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.intensity[light.i];
}

f32 RenderWorld::light_spot_angle(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.spot_angle[light.i];
}

void RenderWorld::light_set_color(LightInstance light, const Color4& col)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.color[light.i] = col;
}

void RenderWorld::light_set_type(LightInstance light, LightType::Enum type)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.type[light.i] = type;
}

void RenderWorld::light_set_range(LightInstance light, f32 range)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.range[light.i] = range;
}

void RenderWorld::light_set_intensity(LightInstance light, f32 intensity)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.intensity[light.i] = intensity;
}

void RenderWorld::light_set_spot_angle(LightInstance light, f32 angle)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.spot_angle[light.i] = angle;
}

void RenderWorld::light_debug_draw(LightInstance light, DebugLine& dl)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager.debug_draw(light.i, 1, dl);
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
			MeshInstance mesh = _mesh_manager.mesh(*begin);
			mid.world[mesh.i] = *world;
		}

		if (_sprite_manager.has(*begin))
		{
			SpriteInstance sprite = _sprite_manager.sprite(*begin);
			sid.world[sprite.i] = *world;
		}

		if (_light_manager.has(*begin))
		{
			LightInstance light = _light_manager.light(*begin);
			lid.world[light.i] = *world;
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
		bgfx::VertexLayout layout;
		layout.begin()
			.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, false)
			.end()
			;
		bgfx::TransientVertexBuffer tvb;
		bgfx::allocTransientVertexBuffer(&tvb, 4*sid.first_hidden, layout);
		bgfx::TransientIndexBuffer tib;
		bgfx::allocTransientIndexBuffer(&tib, 6*sid.first_hidden);

		f32* vdata = (f32*)tvb.data;
		u16* idata = (u16*)tib.data;

		// Render sprites
		for (u32 i = 0; i < sid.first_hidden; ++i)
		{
			const f32* frame = sprite_resource::frame_data(sid.resource[i], sid.frame[i] % sid.resource[i]->num_frames);

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

void RenderWorld::unit_destroyed_callback(UnitId unit)
{
	{
		MeshInstance first = mesh_instance(unit);

		if (is_valid(first))
			mesh_destroy(first);
	}

	{
		SpriteInstance first = sprite_instance(unit);

		if (is_valid(first))
			sprite_destroy(first);
	}

	{
		LightInstance first = light_instance(unit);

		if (is_valid(first))
			light_destroy(first);
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

	memcpy(new_data.unit, _data.unit, _data.size * sizeof(UnitId));
	memcpy(new_data.resource, _data.resource, _data.size * sizeof(MeshResource*));
	memcpy(new_data.geometry, _data.geometry, _data.size * sizeof(MeshGeometry*));
	memcpy(new_data.mesh, _data.mesh, _data.size * sizeof(MeshData));
	memcpy(new_data.material, _data.material, _data.size * sizeof(StringId64));
	memcpy(new_data.world, _data.world, _data.size * sizeof(Matrix4x4));
	memcpy(new_data.obb, _data.obb, _data.size * sizeof(OBB));

	_allocator->deallocate(_data.buffer);
	_data = new_data;
}

void RenderWorld::MeshManager::grow()
{
	allocate(_data.capacity * 2 + 1);
}

MeshInstance RenderWorld::MeshManager::create(UnitId unit, const MeshResource* mr, const MeshRendererDesc& mrd, const Matrix4x4& tr)
{
	CE_ASSERT(!hash_map::has(_map, unit), "Unit already has a mesh component");

	if (_data.size == _data.capacity)
		grow();

	const u32 last = _data.size;

	const MeshGeometry* mg = mr->geometry(mrd.geometry_name);

	_data.unit[last]     = unit;
	_data.resource[last] = mr;
	_data.geometry[last] = mg;
	_data.mesh[last].vbh = mg->vertex_buffer;
	_data.mesh[last].ibh = mg->index_buffer;
	_data.material[last] = mrd.material_resource;
	_data.world[last]    = tr;
	_data.obb[last]      = mg->obb;

	hash_map::set(_map, unit, last);
	++_data.size;

	if (mrd.visible)
	{
		if (last >= _data.first_hidden)
		{
			// _data now contains a visible item in its hidden partition.
			swap(last, _data.first_hidden);
			++_data.first_hidden;
			return make_instance(_data.first_hidden-1);
		}
		++_data.first_hidden;
	}

	CE_ENSURE(last >= _data.first_hidden);
	return make_instance(last);
}

void RenderWorld::MeshManager::destroy(MeshInstance inst)
{
	CE_ASSERT(inst.i < _data.size, "Index out of bounds");

	const u32 last      = _data.size - 1;
	const UnitId u      = _data.unit[inst.i];
	const UnitId last_u = _data.unit[last];

	_data.unit[inst.i]     = _data.unit[last];
	_data.resource[inst.i] = _data.resource[last];
	_data.geometry[inst.i] = _data.geometry[last];
	_data.mesh[inst.i].vbh = _data.mesh[last].vbh;
	_data.mesh[inst.i].ibh = _data.mesh[last].ibh;
	_data.material[inst.i] = _data.material[last];
	_data.world[inst.i]    = _data.world[last];
	_data.obb[inst.i]      = _data.obb[last];

	hash_map::set(_map, last_u, inst.i);
	hash_map::remove(_map, u);
	--_data.size;

	// If item was hidden.
	if (inst.i >= _data.first_hidden)
		return;

	// If item was visible *and* last item was hidden.
	if (last >= _data.first_hidden)
		swap(inst.i, _data.first_hidden-1);

	--_data.first_hidden;
}

void RenderWorld::MeshManager::swap(u32 inst_a, u32 inst_b)
{
	if (inst_a == inst_b)
		return;

	const UnitId unit_a = _data.unit[inst_a];
	const UnitId unit_b = _data.unit[inst_b];

	exchange(_data.unit[inst_a],     _data.unit[inst_b]);
	exchange(_data.resource[inst_a], _data.resource[inst_b]);
	exchange(_data.geometry[inst_a], _data.geometry[inst_b]);
	exchange(_data.mesh[inst_a],     _data.mesh[inst_b]);
	exchange(_data.material[inst_a], _data.material[inst_b]);
	exchange(_data.world[inst_a],    _data.world[inst_b]);
	exchange(_data.obb[inst_a],      _data.obb[inst_b]);

	hash_map::set(_map, unit_a, inst_b);
	hash_map::set(_map, unit_b, inst_a);
}

bool RenderWorld::MeshManager::has(UnitId unit)
{
	return is_valid(mesh(unit));
}

void RenderWorld::MeshManager::set_visible(MeshInstance inst, bool visible)
{
	if (visible)
	{
		if (inst.i < _data.first_hidden)
			return; // Already visible.

		swap(inst.i, _data.first_hidden);
		++_data.first_hidden;
	}
	else
	{
		if (inst.i >= _data.first_hidden)
			return; // Already hidden.

		swap(inst.i, _data.first_hidden-1);
		--_data.first_hidden;
	}
}

MeshInstance RenderWorld::MeshManager::mesh(UnitId unit)
{
	return make_instance(hash_map::get(_map, unit, UINT32_MAX));
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

SpriteInstance RenderWorld::SpriteManager::create(UnitId unit, const SpriteResource* sr, const SpriteRendererDesc& srd, const Matrix4x4& tr)
{
	CE_ASSERT(!hash_map::has(_map, unit), "Unit already has a sprite component");

	if (_data.size == _data.capacity)
		grow();

	const u32 last = _data.size;

	_data.unit[last]     = unit;
	_data.resource[last] = sr;
	_data.material[last] = srd.material_resource;
	_data.frame[last]    = 0;
	_data.world[last]    = tr;
	_data.aabb[last]     = AABB();
	_data.flip_x[last]   = false;
	_data.flip_y[last]   = false;
	_data.layer[last]    = srd.layer;
	_data.depth[last]    = srd.depth;

	hash_map::set(_map, unit, last);
	++_data.size;

	if (srd.visible)
	{
		if (last >= _data.first_hidden)
		{
			// _data now contains a visible item in its hidden partition.
			swap(last, _data.first_hidden);
			++_data.first_hidden;
			return make_instance(_data.first_hidden-1);
		}
		++_data.first_hidden;
	}

	CE_ENSURE(last >= _data.first_hidden);
	return make_instance(last);
}

void RenderWorld::SpriteManager::destroy(SpriteInstance inst)
{
	CE_ASSERT(inst.i < _data.size, "Index out of bounds");

	const u32 last      = _data.size - 1;
	const UnitId u      = _data.unit[inst.i];
	const UnitId last_u = _data.unit[last];

	_data.unit[inst.i]     = _data.unit[last];
	_data.resource[inst.i] = _data.resource[last];
	_data.material[inst.i] = _data.material[last];
	_data.frame[inst.i]    = _data.frame[last];
	_data.world[inst.i]    = _data.world[last];
	_data.aabb[inst.i]     = _data.aabb[last];
	_data.flip_x[inst.i]   = _data.flip_x[last];
	_data.flip_y[inst.i]   = _data.flip_y[last];
	_data.layer[inst.i]    = _data.layer[last];
	_data.depth[inst.i]    = _data.depth[last];

	hash_map::set(_map, last_u, inst.i);
	hash_map::remove(_map, u);
	--_data.size;

	// If item was hidden.
	if (inst.i >= _data.first_hidden)
		return;

	// If item was visible *and* last item was hidden.
	if (last >= _data.first_hidden)
		swap(inst.i, _data.first_hidden-1);

	--_data.first_hidden;
}

void RenderWorld::SpriteManager::swap(u32 inst_a, u32 inst_b)
{
	if (inst_a == inst_b)
		return;

	const UnitId unit_a = _data.unit[inst_a];
	const UnitId unit_b = _data.unit[inst_b];

	exchange(_data.unit[inst_a],     _data.unit[inst_b]);
	exchange(_data.resource[inst_a], _data.resource[inst_b]);
	exchange(_data.material[inst_a], _data.material[inst_b]);
	exchange(_data.frame[inst_a],    _data.frame[inst_b]);
	exchange(_data.world[inst_a],    _data.world[inst_b]);
	exchange(_data.aabb[inst_a],     _data.aabb[inst_b]);
	exchange(_data.flip_x[inst_a],   _data.flip_x[inst_b]);
	exchange(_data.flip_y[inst_a],   _data.flip_y[inst_b]);
	exchange(_data.layer[inst_a],    _data.layer[inst_b]);
	exchange(_data.depth[inst_a],    _data.depth[inst_b]);

	hash_map::set(_map, unit_a, inst_b);
	hash_map::set(_map, unit_b, inst_a);
}

bool RenderWorld::SpriteManager::has(UnitId unit)
{
	return is_valid(sprite(unit));
}

void RenderWorld::SpriteManager::set_visible(SpriteInstance inst, bool visible)
{
	if (visible)
	{
		if (inst.i < _data.first_hidden)
			return; // Already visible.

		swap(inst.i, _data.first_hidden);
		++_data.first_hidden;
	}
	else
	{
		if (inst.i >= _data.first_hidden)
			return; // Already hidden.

		swap(inst.i, _data.first_hidden-1);
		--_data.first_hidden;
	}
}

SpriteInstance RenderWorld::SpriteManager::sprite(UnitId unit)
{
	return make_instance(hash_map::get(_map, unit, UINT32_MAX));
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

LightInstance RenderWorld::LightManager::create(UnitId unit, const LightDesc& ld, const Matrix4x4& tr)
{
	CE_ASSERT(!hash_map::has(_map, unit), "Unit already has a light component");

	if (_data.size == _data.capacity)
		grow();

	const u32 last = _data.size;

	_data.unit[last]       = unit;
	_data.world[last]      = tr;
	_data.range[last]      = ld.range;
	_data.intensity[last]  = ld.intensity;
	_data.spot_angle[last] = ld.spot_angle;
	_data.color[last]      = vector4(ld.color.x, ld.color.y, ld.color.z, 1.0f);
	_data.type[last]       = ld.type;

	++_data.size;

	hash_map::set(_map, unit, last);
	return make_instance(last);
}

void RenderWorld::LightManager::destroy(LightInstance light)
{
	CE_ASSERT(light.i < _data.size, "Index out of bounds");

	const u32 last      = _data.size - 1;
	const UnitId u      = _data.unit[light.i];
	const UnitId last_u = _data.unit[last];

	_data.unit[light.i]       = _data.unit[last];
	_data.world[light.i]      = _data.world[last];
	_data.range[light.i]      = _data.range[last];
	_data.intensity[light.i]  = _data.intensity[last];
	_data.spot_angle[light.i] = _data.spot_angle[last];
	_data.color[light.i]      = _data.color[last];
	_data.type[light.i]       = _data.type[last];

	--_data.size;

	hash_map::set(_map, last_u, light.i);
	hash_map::remove(_map, u);
}

bool RenderWorld::LightManager::has(UnitId unit)
{
	return is_valid(light(unit));
}

LightInstance RenderWorld::LightManager::light(UnitId unit)
{
	return make_instance(hash_map::get(_map, unit, UINT32_MAX));
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
