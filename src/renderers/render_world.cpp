/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "aabb.h"
#include "color4.h"
#include "debug_line.h"
#include "hash.h"
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

RenderWorld::RenderWorld(Allocator& a, ResourceManager& rm, MaterialManager& mm, UnitManager& um)
	: _marker(MARKER)
	, _allocator(&a)
	, _resource_manager(&rm)
	, _material_manager(&mm)
	, _debug_drawing(false)
	, _mesh_map(a)
	, _sprite_map(a)
	, _light_map(a)
{
	um.register_destroy_function(RenderWorld::unit_destroyed_callback, this);

	_u_light_pos = bgfx::createUniform("u_light_pos", bgfx::UniformType::Vec4);
	_u_light_dir = bgfx::createUniform("u_light_dir", bgfx::UniformType::Vec4);
	_u_light_col = bgfx::createUniform("u_light_col", bgfx::UniformType::Vec4);
}

RenderWorld::~RenderWorld()
{
	bgfx::destroyUniform(_u_light_pos);
	bgfx::destroyUniform(_u_light_dir);
	bgfx::destroyUniform(_u_light_col);

	_allocator->deallocate(_mesh_data.buffer);
	_allocator->deallocate(_sprite_data.buffer);
	_allocator->deallocate(_light_data.buffer);

	_marker = 0;
}

MeshInstance RenderWorld::create_mesh(UnitId id, const MeshRendererDesc& mrd, const Matrix4x4& tr)
{
	if (_mesh_data.size == _mesh_data.capacity)
		grow_mesh();

	const MeshResource* mr = (const MeshResource*)_resource_manager->get(MESH_TYPE, mrd.mesh_resource);
	_material_manager->create_material(mrd.material_resource);

	const uint32_t last = _mesh_data.size;

	_mesh_data.unit[last]          = id;
	_mesh_data.mr[last]            = mr;
	_mesh_data.mesh[last].vbh      = mr->geometry(mrd.mesh_name)->vertex_buffer;
	_mesh_data.mesh[last].ibh      = mr->geometry(mrd.mesh_name)->index_buffer;
	_mesh_data.material[last]      = mrd.material_resource;
	_mesh_data.world[last]         = tr;
	_mesh_data.obb[last]           = mr->geometry(mrd.mesh_name)->obb;
	_mesh_data.next_instance[last] = make_mesh_instance(UINT32_MAX);

	++_mesh_data.size;
	++_mesh_data.first_hidden;

	MeshInstance first = first_mesh(id);
	if (!is_valid(first))
	{
		hash::set(_mesh_map, id.encode(), last);
	}
	else
	{
		add_mesh_node(first, make_mesh_instance(last));
	}

	return make_mesh_instance(last);
}

void RenderWorld::destroy_mesh(MeshInstance i)
{
	const uint32_t last       = _mesh_data.size - 1;
	const UnitId u            = _mesh_data.unit[i.i];
	const MeshInstance first  = first_mesh(u);
	const MeshInstance last_i = make_mesh_instance(last);

	swap_mesh_node(last_i, i);
	remove_mesh_node(first, i);

	_mesh_data.unit[i.i]          = _mesh_data.unit[last];
	_mesh_data.mr[i.i]            = _mesh_data.mr[last];
	_mesh_data.mesh[i.i].vbh      = _mesh_data.mesh[last].vbh;
	_mesh_data.mesh[i.i].ibh      = _mesh_data.mesh[last].ibh;
	_mesh_data.material[i.i]      = _mesh_data.material[last];
	_mesh_data.world[i.i]         = _mesh_data.world[last];
	_mesh_data.obb[i.i]           = _mesh_data.obb[last];
	_mesh_data.next_instance[i.i] = _mesh_data.next_instance[last];

	--_mesh_data.size;
	--_mesh_data.first_hidden;
}

void RenderWorld::add_mesh_node(MeshInstance first, MeshInstance i)
{
	MeshInstance curr = first;
	while (is_valid(next_mesh(curr)))
		curr = next_mesh(curr);

	_mesh_data.next_instance[curr.i] = i;
}

void RenderWorld::remove_mesh_node(MeshInstance first, MeshInstance i)
{
	const UnitId u = _mesh_data.unit[first.i];

	if (i.i == first.i)
	{
		if (!is_valid(next_mesh(i)))
			hash::set(_mesh_map, u.encode(), UINT32_MAX);
		else
			hash::set(_mesh_map, u.encode(), next_mesh(i).i);
	}
	else
	{
		MeshInstance prev = previous_mesh(i);
		_mesh_data.next_instance[prev.i] = next_mesh(i);
	}
}

void RenderWorld::swap_mesh_node(MeshInstance a, MeshInstance b)
{
	const UnitId u = _mesh_data.unit[a.i];
	const MeshInstance first = first_mesh(u);

	if (a.i == first.i)
	{
		hash::set(_mesh_map, u.encode(), b.i);
	}
	else
	{
		const MeshInstance prev_a = previous_mesh(a);
		CE_ENSURE(prev_a.i != a.i);
		_mesh_data.next_instance[prev_a.i] = b;
	}
}

MeshInstance RenderWorld::first_mesh(UnitId id)
{
	return make_mesh_instance(hash::get(_mesh_map, id.encode(), UINT32_MAX));
}

MeshInstance RenderWorld::next_mesh(MeshInstance i)
{
	CE_ASSERT(i.i < _mesh_data.size, "Index out of bounds: i.i = %d", i.i);
	return _mesh_data.next_instance[i.i];
}

MeshInstance RenderWorld::previous_mesh(MeshInstance i)
{
	const UnitId u = _mesh_data.unit[i.i];

	MeshInstance first = first_mesh(u);
	MeshInstance curr = first;
	MeshInstance prev = { UINT32_MAX };

	while (curr.i != i.i)
	{
		prev = curr;
		curr = next_mesh(curr);
	}

	return prev;
}

void RenderWorld::set_mesh_material(MeshInstance i, StringId64 id)
{
	_mesh_data.material[i.i] = id;
}

void RenderWorld::set_mesh_visible(MeshInstance i, bool visible)
{
}

OBB RenderWorld::mesh_obb(MeshInstance i)
{
	return _mesh_data.obb[i.i];
}

SpriteInstance RenderWorld::create_sprite(UnitId id, const SpriteRendererDesc& srd, const Matrix4x4& tr)
{
	if (_sprite_data.size == _sprite_data.capacity)
		grow_sprite();

	const SpriteResource* sr = (const SpriteResource*)_resource_manager->get(SPRITE_TYPE, srd.sprite_resource);
	_material_manager->create_material(srd.material_resource);

	const uint32_t last = _sprite_data.size;

	_sprite_data.unit[last]          = id;
	_sprite_data.sr[last]            = sr;
	_sprite_data.sprite[last].vbh    = sr->vb;
	_sprite_data.sprite[last].ibh    = sr->ib;
	_sprite_data.material[last]      = srd.material_resource;
	_sprite_data.frame[last]         = 0;
	_sprite_data.world[last]         = tr;
	_sprite_data.aabb[last]          = AABB();
	_sprite_data.next_instance[last] = make_sprite_instance(UINT32_MAX);

	++_sprite_data.size;
	++_sprite_data.first_hidden;

	SpriteInstance first = first_sprite(id);
	if (!is_valid(first))
	{
		hash::set(_sprite_map, id.encode(), last);
	}
	else
	{
		add_sprite_node(first, make_sprite_instance(last));
	}

	return make_sprite_instance(last);
}

void RenderWorld::destroy_sprite(SpriteInstance i)
{
	const uint32_t last         = _sprite_data.size - 1;
	const UnitId u              = _sprite_data.unit[i.i];
	const SpriteInstance first  = first_sprite(u);
	const SpriteInstance last_i = make_sprite_instance(last);

	swap_sprite_node(last_i, i);
	remove_sprite_node(first, i);

	_sprite_data.unit[i.i]          = _sprite_data.unit[last];
	_sprite_data.sr[i.i]            = _sprite_data.sr[last];
	_sprite_data.sprite[i.i].vbh    = _sprite_data.sprite[last].vbh;
	_sprite_data.sprite[i.i].ibh    = _sprite_data.sprite[last].ibh;
	_sprite_data.material[i.i]      = _sprite_data.material[last];
	_sprite_data.frame[i.i]         = _sprite_data.frame[last];
	_sprite_data.world[i.i]         = _sprite_data.world[last];
	_sprite_data.aabb[i.i]          = _sprite_data.aabb[last];
	_sprite_data.next_instance[i.i] = _sprite_data.next_instance[last];

	--_sprite_data.size;
	--_sprite_data.first_hidden;
}

void RenderWorld::add_sprite_node(SpriteInstance first, SpriteInstance i)
{
	SpriteInstance curr = first;
	while (is_valid(next_sprite(curr)))
		curr = next_sprite(curr);

	_sprite_data.next_instance[curr.i] = i;
}

void RenderWorld::remove_sprite_node(SpriteInstance first, SpriteInstance i)
{
	const UnitId u = _sprite_data.unit[first.i];

	if (i.i == first.i)
	{
		if (!is_valid(next_sprite(i)))
			hash::set(_sprite_map, u.encode(), UINT32_MAX);
		else
			hash::set(_sprite_map, u.encode(), next_sprite(i).i);
	}
	else
	{
		SpriteInstance prev = previous_sprite(i);
		_sprite_data.next_instance[prev.i] = next_sprite(i);
	}
}

void RenderWorld::swap_sprite_node(SpriteInstance a, SpriteInstance b)
{
	const UnitId u = _sprite_data.unit[a.i];
	const SpriteInstance first = first_sprite(u);

	if (a.i == first.i)
	{
		hash::set(_sprite_map, u.encode(), b.i);
	}
	else
	{
		const SpriteInstance prev_a = previous_sprite(a);
		_sprite_data.next_instance[prev_a.i] = b;
	}
}

SpriteInstance RenderWorld::first_sprite(UnitId id)
{
	return make_sprite_instance(hash::get(_sprite_map, id.encode(), UINT32_MAX));
}

SpriteInstance RenderWorld::next_sprite(SpriteInstance i)
{
	CE_ASSERT(i.i < _sprite_data.size, "Index out of bounds");
	return _sprite_data.next_instance[i.i];
}

SpriteInstance RenderWorld::previous_sprite(SpriteInstance i)
{
	const UnitId u = _sprite_data.unit[i.i];

	SpriteInstance first = first_sprite(u);
	SpriteInstance curr = first;
	SpriteInstance prev = { UINT32_MAX };

	while (curr.i != i.i)
	{
		prev = curr;
		curr = next_sprite(curr);
	}

	return prev;
}

void RenderWorld::set_sprite_material(SpriteInstance i, StringId64 id)
{
	_sprite_data.material[i.i] = id;
}

void RenderWorld::set_sprite_visible(SpriteInstance i, bool visible)
{
}

void RenderWorld::set_sprite_frame(SpriteInstance i, uint32_t index)
{
	_sprite_data.frame[i.i] = index;
}

LightInstance RenderWorld::create_light(UnitId id, const LightDesc& ld, const Matrix4x4& tr)
{
	CE_ASSERT(!hash::has(_light_map, id.encode()), "Unit already has light");

	if (_light_data.size == _light_data.capacity)
		grow_light();

	const uint32_t last = _light_data.size;

	_light_data.unit[last]       = id;
	_light_data.world[last]      = tr;
	_light_data.range[last]      = ld.range;
	_light_data.intensity[last]  = ld.intensity;
	_light_data.spot_angle[last] = ld.spot_angle;
	_light_data.color[last]      = vector4(ld.color.x, ld.color.y, ld.color.z, 1.0f);
	_light_data.type[last]       = ld.type;

	++_light_data.size;

	hash::set(_light_map, id.encode(), last);
	return make_light_instance(last);
}

void RenderWorld::destroy_light(LightInstance i)
{
	const uint32_t last = _light_data.size - 1;
	const UnitId u = _light_data.unit[i.i];
	const UnitId last_u = _light_data.unit[last];

	_light_data.unit[i.i]       = _light_data.unit[last];
	_light_data.world[i.i]      = _light_data.world[last];
	_light_data.range[i.i]      = _light_data.range[last];
	_light_data.intensity[i.i]  = _light_data.intensity[last];
	_light_data.spot_angle[i.i] = _light_data.spot_angle[last];
	_light_data.color[i.i]      = _light_data.color[last];
	_light_data.type[i.i]       = _light_data.type[last];

	--_light_data.size;

	hash::set(_light_map, last_u.encode(), i.i);
	hash::remove(_light_map, u.encode());
}

LightInstance RenderWorld::light(UnitId id)
{
	return make_light_instance(hash::get(_light_map, id.encode(), UINT32_MAX));
}

Color4 RenderWorld::light_color(LightInstance i)
{
	return _light_data.color[i.i];
}

LightType::Enum RenderWorld::light_type(LightInstance i)
{
	return (LightType::Enum)_light_data.type[i.i];
}

float RenderWorld::light_range(LightInstance i)
{
	return _light_data.range[i.i];
}

float RenderWorld::light_intensity(LightInstance i)
{
	return _light_data.intensity[i.i];
}

float RenderWorld::light_spot_angle(LightInstance i)
{
	return _light_data.spot_angle[i.i];
}

void RenderWorld::set_light_color(LightInstance i, const Color4& col)
{
	_light_data.color[i.i] = col;
}

void RenderWorld::set_light_type(LightInstance i, LightType::Enum type)
{
	_light_data.type[i.i] = type;
}

void RenderWorld::set_light_range(LightInstance i, float range)
{
	_light_data.range[i.i] = range;
}

void RenderWorld::set_light_intensity(LightInstance i, float intensity)
{
	_light_data.intensity[i.i] = intensity;
}

void RenderWorld::set_light_spot_angle(LightInstance i, float angle)
{
	_light_data.spot_angle[i.i] = angle;
}

void RenderWorld::update_transforms(const UnitId* begin, const UnitId* end, const Matrix4x4* world)
{
	for (; begin != end; ++begin, ++world)
	{
		uint32_t inst = hash::get(_mesh_map, begin->encode(), UINT32_MAX);

		if (inst != UINT32_MAX)
			_mesh_data.world[inst] = *world;

		inst = hash::get(_sprite_map, begin->encode(), UINT32_MAX);

		if (inst != UINT32_MAX)
			_sprite_data.world[inst] = *world;

		inst = hash::get(_light_map, begin->encode(), UINT32_MAX);

		if (inst != UINT32_MAX)
			_light_data.world[inst] = *world;
	}
}

void RenderWorld::render(const Matrix4x4& view, const Matrix4x4& projection, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0x353839FF
		, 1.0f
		, 0
		);

	// Set view and projection matrix for view 0.
	bgfx::setViewTransform(0, to_float_ptr(view), to_float_ptr(projection));
	bgfx::setViewRect(0, x, y, width, height);

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::touch(0);

	for (uint32_t ll = 0; ll < _light_data.size; ++ll)
	{
		const Vector4 ldir = normalize(_light_data.world[ll].z) * view;
		const Vector3 lpos = translation(_light_data.world[ll]);

		bgfx::setUniform(_u_light_pos, to_float_ptr(lpos));
		bgfx::setUniform(_u_light_dir, to_float_ptr(ldir));
		bgfx::setUniform(_u_light_col, to_float_ptr(_light_data.color[ll]));

		// Render meshes
		for (uint32_t i = 0; i < _mesh_data.first_hidden; ++i)
		{
			bgfx::setTransform(to_float_ptr(_mesh_data.world[i]));
			bgfx::setVertexBuffer(_mesh_data.mesh[i].vbh);
			bgfx::setIndexBuffer(_mesh_data.mesh[i].ibh);

			_material_manager->get(_mesh_data.material[i])->bind();
		}
	}

	// Render sprites
	for (uint32_t i = 0; i < _sprite_data.first_hidden; ++i)
	{
		bgfx::setVertexBuffer(_sprite_data.sprite[i].vbh);
		bgfx::setIndexBuffer(_sprite_data.sprite[i].ibh, _sprite_data.frame[i] * 6, 6);
		bgfx::setTransform(to_float_ptr(_sprite_data.world[i]));

		_material_manager->get(_sprite_data.material[i])->bind();
	}
}

void RenderWorld::draw_debug(DebugLine& dl)
{
	if (!_debug_drawing)
		return;

	for (uint32_t i = 0; i < _mesh_data.size; ++i)
	{
		const OBB& obb = _mesh_data.obb[i];
		const Matrix4x4& world = _mesh_data.world[i];
		dl.add_obb(obb.tm * world, obb.half_extents, COLOR4_RED);
	}

	for (uint32_t i = 0; i < _light_data.size; ++i)
	{
		const Vector3 pos = translation(_light_data.world[i]);
		const Vector3 dir = -z(_light_data.world[i]);

		// Draw tiny sphere for all light types
		dl.add_sphere(pos, 0.1f, _light_data.color[i]);

		switch (_light_data.type[i])
		{
			case LightType::DIRECTIONAL:
			{
				const Vector3 end = pos + dir*3.0f;
				dl.add_line(pos, end, COLOR4_YELLOW);
				dl.add_cone(pos + dir*2.8f, end, 0.1f, COLOR4_YELLOW);
				break;
			}
			case LightType::OMNI:
			{
				dl.add_sphere(pos, _light_data.range[i], COLOR4_YELLOW);
				break;
			}
			case LightType::SPOT:
			{
				const float angle = _light_data.spot_angle[i];
				const float range = _light_data.range[i];
				const float radius = tan(angle)*range;
				dl.add_cone(pos + range*dir, pos, radius, COLOR4_YELLOW);
				break;
			}
			default:
			{
				CE_ASSERT(false, "Bad light type");
				break;
			}
		}
	}
}

void RenderWorld::enable_debug_drawing(bool enable)
{
	_debug_drawing = enable;
}

void RenderWorld::unit_destroyed_callback(UnitId id)
{
	{
		MeshInstance first = first_mesh(id);
		MeshInstance curr = first;
		MeshInstance next = make_mesh_instance(UINT32_MAX);

		while (is_valid(curr))
		{
			next = next_mesh(curr);
			destroy_mesh(curr);
			curr = next;
		}
	}

	{
		SpriteInstance first = first_sprite(id);
		SpriteInstance curr = first;
		SpriteInstance next = make_sprite_instance(UINT32_MAX);

		while (is_valid(curr))
		{
			next = next_sprite(curr);
			destroy_sprite(curr);
			curr = next;
		}
	}

	{
		LightInstance first = light(id);

		if (is_valid(first))
			destroy_light(first);
	}
}

void RenderWorld::allocate_mesh(uint32_t num)
{
	CE_ENSURE(num > _mesh_data.size);

	const uint32_t bytes = num * (0
		+ sizeof(UnitId)
		+ sizeof(MeshResource*)
		+ sizeof(MeshData)
		+ sizeof(StringId64)
		+ sizeof(Matrix4x4)
		+ sizeof(OBB)
		+ sizeof(MeshInstance)
		);

	MeshInstanceData new_data;
	new_data.size = _mesh_data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);
	new_data.first_hidden = _mesh_data.first_hidden;

	new_data.unit = (UnitId*)(new_data.buffer);
	new_data.mr = (const MeshResource**)(new_data.unit + num);
	new_data.mesh = (MeshData*)(new_data.mr + num);
	new_data.material = (StringId64*)(new_data.mesh + num);
	new_data.world = (Matrix4x4*)(new_data.material + num);
	new_data.obb = (OBB*)(new_data.world + num);
	new_data.next_instance = (MeshInstance*)(new_data.obb + num);

	memcpy(new_data.unit, _mesh_data.unit, _mesh_data.size * sizeof(UnitId));
	memcpy(new_data.mr, _mesh_data.mr, _mesh_data.size * sizeof(MeshResource*));
	memcpy(new_data.mesh, _mesh_data.mesh, _mesh_data.size * sizeof(MeshData));
	memcpy(new_data.material, _mesh_data.material, _mesh_data.size * sizeof(StringId64));
	memcpy(new_data.world, _mesh_data.world, _mesh_data.size * sizeof(Matrix4x4));
	memcpy(new_data.obb, _mesh_data.obb, _mesh_data.size * sizeof(OBB));
	memcpy(new_data.next_instance, _mesh_data.next_instance, _mesh_data.size * sizeof(MeshInstance));

	_allocator->deallocate(_mesh_data.buffer);
	_mesh_data = new_data;
}

void RenderWorld::allocate_sprite(uint32_t num)
{
	CE_ENSURE(num > _sprite_data.size);

	const uint32_t bytes = num * (0
		+ sizeof(UnitId)
		+ sizeof(SpriteResource**)
		+ sizeof(SpriteData)
		+ sizeof(StringId64)
		+ sizeof(uint32_t)
		+ sizeof(Matrix4x4)
		+ sizeof(AABB)
		+ sizeof(SpriteInstance)
		);

	SpriteInstanceData new_data;
	new_data.size = _sprite_data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);
	new_data.first_hidden = _sprite_data.first_hidden;

	new_data.unit = (UnitId*)(new_data.buffer);
	new_data.sr = (const SpriteResource**)(new_data.unit + num);
	new_data.sprite = (SpriteData*)(new_data.sr + num);
	new_data.material = (StringId64*)(new_data.sprite + num);
	new_data.frame = (uint32_t*)(new_data.material + num);
	new_data.world = (Matrix4x4*)(new_data.frame + num);
	new_data.aabb = (AABB*)(new_data.world + num);
	new_data.next_instance = (SpriteInstance*)(new_data.aabb + num);

	memcpy(new_data.unit, _sprite_data.unit, _sprite_data.size * sizeof(UnitId));
	memcpy(new_data.sr, _sprite_data.sr, _sprite_data.size * sizeof(SpriteResource**));
	memcpy(new_data.sprite, _sprite_data.sprite, _sprite_data.size * sizeof(SpriteData));
	memcpy(new_data.material, _sprite_data.material, _sprite_data.size * sizeof(StringId64));
	memcpy(new_data.frame, _sprite_data.frame, _sprite_data.size * sizeof(uint32_t));
	memcpy(new_data.world, _sprite_data.world, _sprite_data.size * sizeof(Matrix4x4));
	memcpy(new_data.aabb, _sprite_data.aabb, _sprite_data.size * sizeof(AABB));
	memcpy(new_data.next_instance, _sprite_data.next_instance, _sprite_data.size * sizeof(SpriteInstance));

	_allocator->deallocate(_sprite_data.buffer);
	_sprite_data = new_data;
}

void RenderWorld::allocate_light(uint32_t num)
{
	CE_ENSURE(num > _light_data.size);

	const uint32_t bytes = num * (0
		+ sizeof(UnitId)
		+ sizeof(Matrix4x4)
		+ sizeof(float)
		+ sizeof(float)
		+ sizeof(float)
		+ sizeof(Color4)
		+ sizeof(uint32_t)
		);

	LightInstanceData new_data;
	new_data.size = _light_data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);

	new_data.unit = (UnitId*)(new_data.buffer);
	new_data.world = (Matrix4x4*)(new_data.unit + num);
	new_data.range = (float*)(new_data.world + num);
	new_data.intensity = (float*)(new_data.range + num);
	new_data.spot_angle = (float*)(new_data.intensity + num);
	new_data.color = (Color4*)(new_data.spot_angle + num);
	new_data.type = (uint32_t*)(new_data.color + num);

	memcpy(new_data.unit, _light_data.unit, _light_data.size * sizeof(UnitId));
	memcpy(new_data.world, _light_data.world, _light_data.size * sizeof(Matrix4x4));
	memcpy(new_data.range, _light_data.range, _light_data.size * sizeof(float));
	memcpy(new_data.intensity, _light_data.intensity, _light_data.size * sizeof(float));
	memcpy(new_data.spot_angle, _light_data.spot_angle, _light_data.size * sizeof(float));
	memcpy(new_data.color, _light_data.color, _light_data.size * sizeof(Color4));
	memcpy(new_data.type, _light_data.type, _light_data.size * sizeof(uint32_t));

	_allocator->deallocate(_light_data.buffer);
	_light_data = new_data;
}

void RenderWorld::grow_mesh()
{
	allocate_mesh(_mesh_data.capacity * 2 + 1);
}
void RenderWorld::grow_sprite()
{
	allocate_sprite(_sprite_data.capacity * 2 + 1);
}
void RenderWorld::grow_light()
{
	allocate_light(_light_data.capacity * 2 + 1);
}

} // namespace crown
