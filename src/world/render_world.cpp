/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/hash_set.inl"
#include "core/list.inl"
#include "core/math/aabb.h"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/frustum.inl"
#include "core/math/intersection.h"
#include "core/math/matrix4x4.inl"
#include "core/strings/string_id.inl"
#include "device/pipeline.h"
#include "resource/mesh_resource.h"
#include "resource/render_config_resource.h"
#include "resource/resource_manager.h"
#include "resource/sprite_resource.h"
#include "world/debug_line.h"
#include "world/material.h"
#include "world/material_manager.h"
#include "world/render_world.h"
#include "world/scene_graph.h"
#include "world/shader_manager.h"
#include "world/unit_manager.h"
#include <algorithm> // std::sort
#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace crown
{
static void unit_destroyed_callback_bridge(UnitId unit, void *user_ptr)
{
	((RenderWorld *)user_ptr)->unit_destroyed_callback(unit);
}

RenderWorld::RenderWorld(Allocator &a
	, ResourceManager &rm
	, ShaderManager &sm
	, MaterialManager &mm
	, UnitManager &um
	, Pipeline &pl
	, SceneGraph &sg
	)
	: _marker(RENDER_WORLD_MARKER)
	, _resource_manager(&rm)
	, _shader_manager(&sm)
	, _material_manager(&mm)
	, _unit_manager(&um)
	, _pipeline(&pl)
	, _scene_graph(&sg)
	, _debug_drawing(false)
	, _mesh_manager(a, this)
	, _sprite_manager(a, this)
	, _light_manager(a)
	, _selection(a)
	, _fog_unit(UNIT_INVALID)
{
	_unit_destroy_callback.destroy = unit_destroyed_callback_bridge;
	_unit_destroy_callback.user_data = this;
	_unit_destroy_callback.node.next = NULL;
	_unit_destroy_callback.node.prev = NULL;
	um.register_destroy_callback(&_unit_destroy_callback);

	// Outlines.
	_u_unit_id = bgfx::createUniform("u_unit_id", bgfx::UniformType::Vec4);

	// Lighting.
	_u_lights_num = bgfx::createUniform("u_lights_num", bgfx::UniformType::Vec4, 1);
	_u_lights_data = bgfx::createUniform("u_lights_data", bgfx::UniformType::Vec4, LIGHT_SIZE*MAX_NUM_LIGHTS);

	// Fog.
	memset(&_fog_desc, 0, sizeof(_fog_desc));
	_u_fog_data = bgfx::createUniform("u_fog_data", bgfx::UniformType::Vec4, 2);
}

RenderWorld::~RenderWorld()
{
	// Destroy fog uniform.
	bgfx::destroy(_u_fog_data);

	// Destroy lighting uniforms.
	bgfx::destroy(_u_lights_data);
	bgfx::destroy(_u_lights_num);

	// Destroy outlines uniforms.
	bgfx::destroy(_u_unit_id);

	_unit_manager->unregister_destroy_callback(&_unit_destroy_callback);

	_mesh_manager.destroy();
	_sprite_manager.destroy();
	_light_manager.destroy();

	_marker = 0;
}

MeshInstance RenderWorld::mesh_create(UnitId unit, const MeshRendererDesc &mrd, const Matrix4x4 &tr)
{
	const MeshResource *mr = (MeshResource *)_resource_manager->get(RESOURCE_TYPE_MESH, mrd.mesh_resource);
	const MaterialResource *mat_res = (MaterialResource *)_resource_manager->get(RESOURCE_TYPE_MATERIAL, mrd.material_resource);
	_material_manager->create_material(mat_res);
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

void RenderWorld::mesh_set_geometry(MeshInstance mesh, StringId64 mesh_resource, StringId32 geometry)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	const MeshResource *mr = (MeshResource *)_resource_manager->get(RESOURCE_TYPE_MESH, mesh_resource);
	_mesh_manager.set_geometry(mesh, mr, geometry);
}

void RenderWorld::mesh_set_skeleton(MeshInstance mesh, const AnimationSkeletonInstance *bones)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	_mesh_manager._data.skeleton[mesh.i] = (AnimationSkeletonInstance *)bones;

	UnitId unit = _mesh_manager._data.unit[mesh.i];
	TransformInstance ti = _scene_graph->instance(unit);
	_scene_graph->set_local_pose(ti, MATRIX4X4_IDENTITY);
}

Material *RenderWorld::mesh_material(MeshInstance mesh)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	return _mesh_manager._data.material[mesh.i];
}

void RenderWorld::mesh_set_material(MeshInstance mesh, StringId64 id)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	const MaterialResource *mat_res = (MaterialResource *)_resource_manager->get(RESOURCE_TYPE_MATERIAL, id);
	_mesh_manager._data.material[mesh.i] = _material_manager->create_material(mat_res);
}

void RenderWorld::mesh_set_visible(MeshInstance mesh, bool visible)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	_mesh_manager.set_visible(mesh, visible);
}

void RenderWorld::mesh_set_cast_shadows(MeshInstance mesh, bool cast_shadows)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	if (cast_shadows)
		_mesh_manager._data.flags[mesh.i] |= RenderableFlags::SHADOW_CASTER;
	else
		_mesh_manager._data.flags[mesh.i] &= ~RenderableFlags::SHADOW_CASTER;
}

OBB RenderWorld::mesh_obb(MeshInstance mesh)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");

	const Matrix4x4 &world = _mesh_manager._data.world[mesh.i];
	const OBB &obb = _mesh_manager._data.obb[mesh.i];

	OBB o;
	o.tm = obb.tm * world;
	o.half_extents = obb.half_extents;

	return o;
}

f32 RenderWorld::mesh_cast_ray(MeshInstance mesh, const Vector3 &from, const Vector3 &dir)
{
	CE_ASSERT(mesh.i < _mesh_manager._data.size, "Index out of bounds");
	const MeshGeometry *mg = _mesh_manager._data.geometry[mesh.i];
	return ray_mesh_intersection(from
		, dir
		, _mesh_manager._data.world[mesh.i]
		, mg->vertices.data
		, mg->vertices.stride
		, (u16 *)mg->indices.data
		, mg->indices.num
		);
}

SpriteInstance RenderWorld::sprite_create(UnitId unit, const SpriteRendererDesc &srd, const Matrix4x4 &tr)
{
	const SpriteResource *sr = (SpriteResource *)_resource_manager->get(RESOURCE_TYPE_SPRITE, srd.sprite_resource);
	const MaterialResource *mat_res = (MaterialResource *)_resource_manager->get(RESOURCE_TYPE_MATERIAL, srd.material_resource);
	_material_manager->create_material(mat_res);
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

void RenderWorld::sprite_set_sprite(SpriteInstance sprite, StringId64 sprite_resource_name)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	const SpriteResource *resource = (SpriteResource *)_resource_manager->get(RESOURCE_TYPE_SPRITE, sprite_resource_name);
	_sprite_manager._data.resource[sprite.i] = resource;
}

Material *RenderWorld::sprite_material(SpriteInstance sprite)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	return _sprite_manager._data.material[sprite.i];
}

void RenderWorld::sprite_set_material(SpriteInstance sprite, StringId64 id)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	const MaterialResource *mat_res = (MaterialResource *)_resource_manager->get(RESOURCE_TYPE_MATERIAL, id);
	_sprite_manager._data.material[sprite.i] = _material_manager->create_material(mat_res);
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
	if (flip)
		_sprite_manager._data.flags[sprite.i] |= SpriteFlags::FLIP_X;
	else
		_sprite_manager._data.flags[sprite.i] &= ~SpriteFlags::FLIP_X;
}

void RenderWorld::sprite_flip_y(SpriteInstance sprite, bool flip)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");
	if (flip)
		_sprite_manager._data.flags[sprite.i] |= SpriteFlags::FLIP_Y;
	else
		_sprite_manager._data.flags[sprite.i] &= ~SpriteFlags::FLIP_Y;
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

	const OBB &obb = _sprite_manager._data.resource[sprite.i]->obb;
	const Matrix4x4 &world = _sprite_manager._data.world[sprite.i];

	OBB o;
	o.tm = obb.tm * world;
	o.half_extents = obb.half_extents;

	return o;
}

f32 RenderWorld::sprite_cast_ray(SpriteInstance sprite, const Vector3 &from, const Vector3 &dir, u32 &layer, u32 &depth)
{
	CE_ASSERT(sprite.i < _sprite_manager._data.size, "Index out of bounds");

	const SpriteManager::SpriteInstanceData &sid = _sprite_manager._data;
	const f32 *frame = sprite_resource::frame_data(sid.resource[sprite.i], sid.frame[sprite.i]);

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

LightInstance RenderWorld::light_create(UnitId unit, const LightDesc &ld, const Matrix4x4 &tr)
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
	Vector3 c = _light_manager._data.shader[light.i].color;
	return { c.x, c.y, c.z, 0.0f };
}

LightType::Enum RenderWorld::light_type(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	return (LightType::Enum)_light_manager._data.type[light.i];
}

f32 RenderWorld::light_range(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.shader[light.i].range;
}

f32 RenderWorld::light_intensity(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.shader[light.i].intensity;
}

f32 RenderWorld::light_spot_angle(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.shader[light.i].spot_angle;
}

f32 RenderWorld::light_shadow_bias(LightInstance light)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	return _light_manager._data.shader[light.i].shadow_bias;
}

void RenderWorld::light_set_color(LightInstance light, const Color4 &col)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.shader[light.i].color = { col.x, col.y, col.z };
}

void RenderWorld::light_set_type(LightInstance light, LightType::Enum type)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.type[light.i] = type;
}

void RenderWorld::light_set_range(LightInstance light, f32 range)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.shader[light.i].range = range;
}

void RenderWorld::light_set_intensity(LightInstance light, f32 intensity)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.shader[light.i].intensity = intensity;
}

void RenderWorld::light_set_spot_angle(LightInstance light, f32 angle)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.shader[light.i].spot_angle = angle;
}

void RenderWorld::light_set_shadow_bias(LightInstance light, f32 bias)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager._data.shader[light.i].shadow_bias = bias;
}

void RenderWorld::light_set_cast_shadows(LightInstance light, bool cast_shadows)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	if (cast_shadows)
		_light_manager._data.flag[light.i] |= RenderableFlags::SHADOW_CASTER;
	else
		_light_manager._data.flag[light.i] &= ~RenderableFlags::SHADOW_CASTER;
}

void RenderWorld::light_debug_draw(LightInstance light, DebugLine &dl)
{
	CE_ASSERT(light.i < _light_manager._data.size, "Index out of bounds");
	_light_manager.debug_draw(light.i, 1, dl);
}

FogInstance RenderWorld::fog_create(UnitId unit, const FogDesc &desc)
{
	_fog_unit = unit;
	_fog = { 0u };
	_fog_desc = desc;
	return _fog;
}

void RenderWorld::fog_destroy(FogInstance fog)
{
	CE_ASSERT(fog.i == _fog.i, "Instance not found");
	_fog_desc = {};
	_fog = { UINT32_MAX };
	_fog_unit = UNIT_INVALID;
}

FogInstance RenderWorld::fog_instance(UnitId unit)
{
	if (_fog_unit == unit)
		return _fog;

	return { UINT32_MAX };
}

void RenderWorld::fog_set_color(FogInstance fog, Vector3 color)
{
	CE_ASSERT(fog.i == _fog.i, "Instance not found");
	_fog_desc.color = color;
}

void RenderWorld::fog_set_density(FogInstance fog, float density)
{
	CE_ASSERT(fog.i == _fog.i, "Instance not found");
	_fog_desc.density = density;
}

void RenderWorld::fog_set_range_min(FogInstance fog, float range)
{
	CE_ASSERT(fog.i == _fog.i, "Instance not found");
	_fog_desc.range_min = range;
}

void RenderWorld::fog_set_range_max(FogInstance fog, float range)
{
	CE_ASSERT(fog.i == _fog.i, "Instance not found");
	_fog_desc.range_max = range;
}

void RenderWorld::fog_set_sun_blend(FogInstance fog, float sun_blend)
{
	CE_ASSERT(fog.i == _fog.i, "Instance not found");
	_fog_desc.sun_blend = sun_blend;
}

void RenderWorld::fog_set_enabled(FogInstance fog, bool enable)
{
	CE_ASSERT(fog.i == _fog.i, "Instance not found");
	_fog_desc.enabled = (f32)enable;
}

void RenderWorld::update_transforms(const UnitId *begin, const UnitId *end, const Matrix4x4 *world)
{
	MeshManager::MeshInstanceData &mid = _mesh_manager._data;
	SpriteManager::SpriteInstanceData &sid = _sprite_manager._data;
	LightManager::LightInstanceData &lid = _light_manager._data;

	for (; begin != end; ++begin, ++world) {
		if (_mesh_manager.has(*begin)) {
			MeshInstance mesh = _mesh_manager.mesh(*begin);
			mid.world[mesh.i] = *world;
		}

		if (_sprite_manager.has(*begin)) {
			SpriteInstance sprite = _sprite_manager.sprite(*begin);
			sid.world[sprite.i] = *world;
		}

		if (_light_manager.has(*begin)) {
			LightInstance light = _light_manager.light(*begin);

			Vector3 pos = translation(*world);
			Vector3 dir = -z(*world);
			normalize(dir);

			lid.shader[light.i].position = pos;
			lid.shader[light.i].direction = dir;
		}
	}
}

void RenderWorld::render(const Matrix4x4 &view, const Matrix4x4 &proj)
{
	LightManager &lm = _light_manager;
	LightManager::LightInstanceData &lid = lm._data;

	const bgfx::Caps *caps = bgfx::getCaps();
	Matrix4x4 inv_view = view;
	invert(inv_view);
	const Vector3 camera_pos = translation(inv_view);

	const float sy = caps->originBottomLeft ? 0.5f : -0.5f;
	const float sz = caps->homogeneousDepth ? 0.5f :  1.0f;
	const float tz = caps->homogeneousDepth ? 0.5f :  0.0f;
	Matrix4x4 crop = {
		{ 0.5f, 0.0f, 0.0f, 0.0f },
		{ 0.0f,   sy, 0.0f, 0.0f },
		{ 0.0f, 0.0f, sz,   0.0f },
		{ 0.5f, 0.5f, tz,   1.0f }
	};

	Matrix4x4 cascaded_lights[MAX_NUM_CASCADES];

	array::clear(lm._directional_lights);
	array::clear(lm._local_lights);
	array::clear(lm._local_lights_spot);
	array::clear(lm._local_lights_omni);
	array::clear(lm._lights_data);
	u32 num_lights = 0; // Total lights to render this frame.

	// Collect indices to all directional and local lights.
	for (u32 i = 0; i < lid.size; ++i) {
		if (lid.type[i] == LightType::DIRECTIONAL)
			array::push_back(lm._directional_lights, i);
		else
			array::push_back(lm._local_lights, i);
	}

	// Sort directional lights by intensity.
	std::sort(array::begin(lm._directional_lights)
		, array::end(lm._directional_lights)
		, [lm](const u32 &in_a, const u32 &in_b) {
			return lm._data.shader[in_a].intensity > lm._data.shader[in_b].intensity;
		});

	// Render directional lights.
	for (u32 i = 0; i < array::size(lm._directional_lights) && num_lights < MAX_NUM_LIGHTS; ++i) {
		u32 L = lm._directional_lights[i];

		// CSMs are only computed for the brightest directional light (index = 0) in the scene.
		if (i == 0 && (lid.flag[L] & RenderableFlags::SHADOW_CASTER) != 0
			&& (_pipeline->_render_settings.flags & RenderSettingsFlags::SUN_SHADOWS) != 0) {
			Matrix4x4 light_proj;
			Matrix4x4 light_view;
			Frustum splits[MAX_NUM_CASCADES];
			Frustum frustum;

			// Compute light view matrix.
			const Vector3 &light_dir = lid.shader[L].direction;
			const bx::Vec3 at  = { -light_dir.x,  -light_dir.y, -light_dir.z };
			const bx::Vec3 eye = { 0.0, 0.0, 0.0 };
			const bx::Vec3 up = { 0.0f, 0.0f, 1.0f };
			bx::mtxLookAt(to_float_ptr(light_view), eye, at, up, bx::Handedness::Right);

			// Split the view frustum into MAX_NUM_CASCADES frustums.
			frustum::from_matrix(frustum, proj, caps->homogeneousDepth, bx::Handedness::Right);
			frustum::split(splits, MAX_NUM_CASCADES, frustum, 0.75f);

			// Render the scene once per cascade.
			for (u32 i = 0; i < MAX_NUM_CASCADES; ++i) {
				Frustum split_i_world;
				frustum::transform(split_i_world, splits[i], inv_view);

				// Compute light projection matrix.
				// Get frustum corners in view space.
				Vector3 vertices[8];
				frustum::vertices(vertices, splits[i]);

				for (u32 j = 0; j < 8; ++j) {
					// Transform frustum corners to light space.
					vertices[j] = vertices[j] * inv_view;   // To world space.
					vertices[j] = vertices[j] * light_view; // To light space.
				}

				// Compute frustum bounding box in light space.
				AABB box;
				aabb::from_points(box, countof(vertices), vertices);
				// debug_draw_box(box, get_inverted(light_view), _lines, COLOR4_YELLOW); // Debug draw in world space.

				bx::mtxOrtho(to_float_ptr(light_proj)
					, box.min.x
					, box.max.x
					, box.min.y
					, box.max.y
					, -1000.0f
					,  1000.0f
					, 0.0f
					, caps->homogeneousDepth
					, bx::Handedness::Right
					);

				cascaded_lights[i] = light_view * light_proj * crop;

				// Render scene into atlas.
				//
				// Screen-space     Texture-space
				//
				// (0;0)  (w;0)     (0;1)
				//   +------>         |
				//   |                |
				//   |                |
				//   |                +------>
				// (0;h)            (0;0)  (1;0)
				//
				const f32 tile_size_x = 0.5f * _pipeline->_render_settings.sun_shadow_map_size.x;
				const f32 tile_size_y = 0.5f * _pipeline->_render_settings.sun_shadow_map_size.y;
				Vector4 rects[] =
				{
					{           0, tile_size_y, tile_size_x, tile_size_y },
					{ tile_size_x, tile_size_y, tile_size_x, tile_size_y },
					{           0,           0, tile_size_x, tile_size_y },
					{ tile_size_x,           0, tile_size_x, tile_size_y },
				};
				CE_STATIC_ASSERT(countof(rects) == MAX_NUM_CASCADES);

				lid.shader[L].atlas_u = 0.0f;
#if CROWN_PLATFORM_WINDOWS
				lid.shader[L].atlas_v = 1.0f;
#else
				lid.shader[L].atlas_v = 0.0f;
#endif
				lid.shader[L].map_size = 0.5f;

				bgfx::setViewRect(View::CASCADE_0 + i, rects[i].x, rects[i].y, rects[i].z, rects[i].w);
				bgfx::setViewFrameBuffer(View::CASCADE_0 + i, _pipeline->_sun_shadow_map_frame_buffer);
				bgfx::setViewClear(View::CASCADE_0 + i, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xffffffff, 1.0f, 0);

				bgfx::setViewTransform(View::CASCADE_0 + i, to_float_ptr(light_view), to_float_ptr(light_proj));

				_mesh_manager.draw_shadow_casters(View::CASCADE_0 + i, *_scene_graph);
			}
		}

		array::push_back(lm._lights_data, lid.shader[L]);
		++num_lights;
	}

	// Render local lights.
	if (_pipeline->_render_settings.flags & RenderSettingsFlags::LOCAL_LIGHTS) {
		// Sort culled (local) lights by distance to camera.
		// TODO: no culling is performed yet, culled lights here means *all* local lights.
		std::sort(array::begin(lm._local_lights)
			, array::end(lm._local_lights)
			, [lm, camera_pos](const u32 &in_a, const u32 &in_b) {
				const f32 dist_a = distance_squared(camera_pos, lm._data.shader[in_a].position);
				const f32 dist_b = distance_squared(camera_pos, lm._data.shader[in_b].position);
				return dist_a < dist_b;
			});

		for (u32 i = 0; i < array::size(lm._local_lights) && num_lights < MAX_NUM_LIGHTS; ++i) {
			LightManager::ShaderData &shader = lid.shader[lm._local_lights[i]];

			if (lid.type[lm._local_lights[i]] == LightType::SPOT) {
				array::push_back(lm._local_lights_spot, lm._local_lights[i]);
			} else if (lid.type[lm._local_lights[i]] == LightType::OMNI) {
				array::push_back(lm._local_lights_omni, lm._local_lights[i]);
			} else {
				CE_FATAL("Unknown local light type");
			}

			++num_lights;
		}

		for (u32 i = 0; i < array::size(lm._local_lights_omni); ++i)
			array::push_back(lm._lights_data, lid.shader[lm._local_lights_omni[i]]);
		for (u32 i = 0; i < array::size(lm._local_lights_spot); ++i)
			array::push_back(lm._lights_data, lid.shader[lm._local_lights_spot[i]]);
	}

	// Send lights data to GPU.
	Vector4 h;
	h.x = array::size(lm._directional_lights);
	h.y = array::size(lm._local_lights_omni);
	h.z = array::size(lm._local_lights_spot);
	h.w = 0.0f;
	bgfx::setUniform(_u_lights_num, &h);
	CE_ENSURE(array::size(lm._lights_data) <= MAX_NUM_LIGHTS);
	bgfx::setUniform(_u_lights_data, (char *)array::begin(lm._lights_data), array::size(lm._lights_data)*sizeof(LightManager::ShaderData)/sizeof(Vector4));
	bgfx::touch(View::LIGHTS);

	// Render objects.
	_mesh_manager.draw_visibles(View::MESH, *_scene_graph, &cascaded_lights[0]);
	_sprite_manager.draw_visibles(View::SPRITE_0);

	// Render outlines.
	_mesh_manager.draw_selected(View::SELECTION, *_scene_graph);
	_sprite_manager.draw_selected(View::SELECTION);
}

void RenderWorld::debug_draw(DebugLine &dl)
{
	if (!_debug_drawing)
		return;

	const MeshManager::MeshInstanceData &mid = _mesh_manager._data;

	for (u32 i = 0; i < mid.size; ++i) {
		const OBB &obb = mid.obb[i];
		const Matrix4x4 &world = mid.world[i];
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

void RenderWorld::reload_materials(const MaterialResource *old_resource, const MaterialResource *new_resource)
{
#if CROWN_CAN_RELOAD
	for (u32 i = 0; i < _mesh_manager._data.size; ++i) {
		if (_mesh_manager._data.material_resource[i] == old_resource) {
			_mesh_manager._data.material[i] = _material_manager->get(new_resource);
			_mesh_manager._data.material_resource[i] = new_resource;
		}
	}

	for (u32 i = 0; i < _sprite_manager._data.size; ++i) {
		if (_sprite_manager._data.material_resource[i] == old_resource) {
			_sprite_manager._data.material[i] = _material_manager->get(new_resource);
			_sprite_manager._data.material_resource[i] = new_resource;
		}
	}
#else
	CE_UNUSED_2(old_resource, new_resource);
#endif
}

void RenderWorld::MeshManager::allocate(u32 num)
{
	CE_ENSURE(num > _data.size);

	const u32 bytes = 0
		+ num*sizeof(UnitId) + alignof(UnitId)
		+ num*sizeof(MeshResource *) + alignof(MeshResource *)
		+ num*sizeof(MeshGeometry *) + alignof(MeshGeometry *)
		+ num*sizeof(MeshData) + alignof(MeshData)
		+ num*sizeof(Material *) + alignof(Material *)
		+ num*sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ num*sizeof(OBB) + alignof(OBB)
		+ num*sizeof(AnimationSkeletonInstance *) + alignof(AnimationSkeletonInstance *)
		+ num*sizeof(u32) + alignof(u32)
#if CROWN_CAN_RELOAD
		+ num*sizeof(MaterialResource *) + alignof(MaterialResource *)
#endif
		+ 0
		;

	MeshInstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);
	new_data.first_hidden = _data.first_hidden;

	new_data.unit          = (UnitId *             )memory::align_top(new_data.buffer,         alignof(UnitId));
	new_data.resource      = (const MeshResource **)memory::align_top(new_data.unit + num,     alignof(MeshResource *));
	new_data.geometry      = (const MeshGeometry **)memory::align_top(new_data.resource + num, alignof(MeshGeometry *));
	new_data.mesh          = (MeshData *           )memory::align_top(new_data.geometry + num, alignof(MeshData));
	new_data.material      = (Material **          )memory::align_top(new_data.mesh + num,     alignof(Material *));
	new_data.world         = (Matrix4x4 *          )memory::align_top(new_data.material + num, alignof(Matrix4x4));
	new_data.obb           = (OBB *                )memory::align_top(new_data.world + num,    alignof(OBB));
	new_data.skeleton      = (const AnimationSkeletonInstance **)memory::align_top(new_data.obb + num, alignof(AnimationSkeletonInstance *));
	new_data.flags         = (u32 *                )memory::align_top(new_data.skeleton + num, alignof(u32));
#if CROWN_CAN_RELOAD
	new_data.material_resource = (const MaterialResource **)memory::align_top(new_data.flags + num, alignof(MaterialResource *));
#endif

	memcpy(new_data.unit, _data.unit, _data.size * sizeof(UnitId));
	memcpy(new_data.resource, _data.resource, _data.size * sizeof(MeshResource *));
	memcpy(new_data.geometry, _data.geometry, _data.size * sizeof(MeshGeometry *));
	memcpy(new_data.mesh, _data.mesh, _data.size * sizeof(MeshData));
	memcpy(new_data.material, _data.material, _data.size * sizeof(Material *));
	memcpy(new_data.world, _data.world, _data.size * sizeof(Matrix4x4));
	memcpy(new_data.obb, _data.obb, _data.size * sizeof(OBB));
	memcpy(new_data.skeleton, _data.skeleton, _data.size * sizeof(AnimationSkeletonInstance *));
	memcpy(new_data.flags, _data.flags, _data.size * sizeof(u32));
#if CROWN_CAN_RELOAD
	memcpy(new_data.material_resource, _data.material_resource, _data.size * sizeof(MaterialResource *));
#endif

	_allocator->deallocate(_data.buffer);
	_data = new_data;
}

void RenderWorld::MeshManager::grow()
{
	allocate(_data.capacity * 2 + 1);
}

MeshInstance RenderWorld::MeshManager::create(UnitId unit, const MeshResource *mr, const MeshRendererDesc &mrd, const Matrix4x4 &tr)
{
	CE_ASSERT(!hash_map::has(_map, unit), "Unit already has a mesh component");

	if (_data.size == _data.capacity)
		grow();

	const u32 last = _data.size;

	const MeshGeometry *mg = mr->geometry(mrd.geometry_name);
	const MaterialResource *mat_res = (MaterialResource *)_render_world->_resource_manager->get(RESOURCE_TYPE_MATERIAL, mrd.material_resource);

	_data.unit[last]     = unit;
	_data.resource[last] = mr;
	_data.geometry[last] = mg;
	_data.mesh[last].vbh = mg->vertex_buffer;
	_data.mesh[last].ibh = mg->index_buffer;
	_data.material[last] = _render_world->_material_manager->get(mat_res);
	_data.world[last]    = tr;
	_data.obb[last]      = mg->obb;
	_data.skeleton[last] = NULL;
	_data.flags[last]    = mrd.flags;
#if CROWN_CAN_RELOAD
	_data.material_resource[last] = mat_res;
#endif

	hash_map::set(_map, unit, last);
	++_data.size;

	if (mrd.flags & RenderableFlags::VISIBLE) {
		if (last >= _data.first_hidden) {
			// _data now contains a visible item in its hidden partition.
			swap(last, _data.first_hidden);
			++_data.first_hidden;
			return make_instance(_data.first_hidden - 1);
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
	_data.skeleton[inst.i] = _data.skeleton[last];
	_data.flags[inst.i]    = _data.flags[last];
#if CROWN_CAN_RELOAD
	_data.material_resource[inst.i] = _data.material_resource[last];
#endif

	hash_map::set(_map, last_u, inst.i);
	hash_map::remove(_map, u);
	--_data.size;

	// If item was hidden.
	if (inst.i >= _data.first_hidden)
		return;

	// If item was visible *and* last item was hidden.
	if (last >= _data.first_hidden)
		swap(inst.i, _data.first_hidden - 1);

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
	exchange(_data.skeleton[inst_a], _data.skeleton[inst_b]);
	exchange(_data.flags[inst_a],    _data.flags[inst_b]);
#if CROWN_CAN_RELOAD
	exchange(_data.material_resource[inst_a], _data.material_resource[inst_b]);
#endif

	hash_map::set(_map, unit_a, inst_b);
	hash_map::set(_map, unit_b, inst_a);
}

bool RenderWorld::MeshManager::has(UnitId unit)
{
	return is_valid(mesh(unit));
}

void RenderWorld::MeshManager::set_geometry(MeshInstance mesh, const MeshResource *mr, StringId32 geometry)
{
	const MeshGeometry *mg = mr->geometry(geometry);
	CE_ENSURE(mg != NULL);

	_data.resource[mesh.i] = mr;
	_data.geometry[mesh.i] = mg;
	_data.mesh[mesh.i].vbh = mg->vertex_buffer;
	_data.mesh[mesh.i].ibh = mg->index_buffer;
	_data.obb[mesh.i]      = mg->obb;
}

void RenderWorld::MeshManager::set_visible(MeshInstance inst, bool visible)
{
	if (visible) {
		if (inst.i < _data.first_hidden)
			return; // Already visible.

		swap(inst.i, _data.first_hidden);
		++_data.first_hidden;
	} else {
		if (inst.i >= _data.first_hidden)
			return; // Already hidden.

		swap(inst.i, _data.first_hidden - 1);
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

void RenderWorld::MeshManager::set_instance_data(u32 ii, SceneGraph &scene_graph)
{
	if (_data.skeleton[ii] != NULL) {
		AnimationSkeletonInstance *skeleton = (AnimationSkeletonInstance *)_data.skeleton[ii];

		for (u32 b = 0; b < skeleton->num_bones; ++b) {
			TransformInstance bone_ti = scene_graph.instance(skeleton->bone_lookup[b]);
			skeleton->bones[b] = skeleton->offsets[b] * scene_graph.world_pose(bone_ti);
		}

		TransformInstance ti = scene_graph.instance(_data.unit[ii]);
		Matrix4x4 world_pose = scene_graph.world_pose(ti);
		skeleton->bones[0] = world_pose;

		bgfx::setTransform(skeleton->bones, skeleton->num_bones);
	} else {
		TransformInstance ti = scene_graph.instance(_data.unit[ii]);
		Matrix4x4 world_pose = scene_graph.world_pose(ti);
		bgfx::setTransform(to_float_ptr(world_pose));
	}

	bgfx::setVertexBuffer(0, _data.mesh[ii].vbh);
	bgfx::setIndexBuffer(_data.mesh[ii].ibh);
}

void RenderWorld::MeshManager::draw_shadow_casters(u8 view_id, SceneGraph &scene_graph)
{
	for (u32 ii = 0; ii < _data.first_hidden; ++ii) {
		if ((_data.flags[ii] & RenderableFlags::SHADOW_CASTER) == 0) // FIXME: put in a separate list.
			continue;

		set_instance_data(ii, scene_graph);

		ShaderData sd = _data.skeleton[ii] != NULL
			? _render_world->_pipeline->_shadow_skinning_shader
			: _render_world->_pipeline->_shadow_shader
			;
		bgfx::setState(sd.state);
		bgfx::submit(view_id, sd.program);
	}
}

void RenderWorld::MeshManager::draw_visibles(u8 view_id, SceneGraph &scene_graph, const Matrix4x4 *cascaded_lights)
{
	const Vector4 texel_size =
	{
		1.0f/_render_world->_pipeline->_render_settings.sun_shadow_map_size.x,
		1.0f/_render_world->_pipeline->_render_settings.sun_shadow_map_size.y,
		0.0f,
		0.0f
	};

	for (u32 ii = 0; ii < _data.first_hidden; ++ii) {
		bgfx::setTexture(CASCADED_SHADOW_MAP_SLOT, _render_world->_pipeline->_u_cascaded_shadow_map, _render_world->_pipeline->_sun_shadow_map_texture);
		bgfx::setUniform(_render_world->_pipeline->_u_cascaded_texel_size, &texel_size);
		bgfx::setUniform(_render_world->_pipeline->_u_cascaded_lights, cascaded_lights, MAX_NUM_CASCADES);
		bgfx::setUniform(_render_world->_u_fog_data, (char *)&_render_world->_fog_desc, sizeof(_render_world->_fog_desc) / sizeof(Vector4));
		_render_world->_pipeline->set_local_lights_params_uniform();

		set_instance_data(ii, scene_graph);
		_data.material[ii]->bind(view_id);
	}
}

void RenderWorld::MeshManager::draw_selected(u8 view_id, SceneGraph &scene_graph)
{
	union
	{
		u32 u;
		f32 f;
	} u2f;

	for (u32 ii = 0; ii < _data.size; ++ii) {
		UnitId unit_id = _data.unit[ii];
		if (!hash_set::has(_render_world->_selection, unit_id)) // FIXME: put selected objects in a separate list.
			continue;

		u2f.u = unit_id._idx;
		Vector4 data = { u2f.f, 0.0f, 0.0f, 0.0f };
		bgfx::setUniform(_render_world->_u_unit_id, &data);

		set_instance_data(ii, scene_graph);
		bgfx::setState(_render_world->_pipeline->_selection_shader.state);
		bgfx::submit(view_id, _render_world->_pipeline->_selection_shader.program);
	}
}

void RenderWorld::SpriteManager::allocate(u32 num)
{
	CE_ENSURE(num > _data.size);

	const u32 bytes = 0
		+ num*sizeof(UnitId) + alignof(UnitId)
		+ num*sizeof(SpriteResource **) + alignof(SpriteResource *)
		+ num*sizeof(Material **) + alignof(Material *)
		+ num*sizeof(u32) + alignof(u32)
		+ num*sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ num*sizeof(AABB) + alignof(AABB)
		+ num*sizeof(u32) + alignof(u32)
		+ num*sizeof(u32) + alignof(u32)
		+ num*sizeof(u32) + alignof(u32)
#if CROWN_CAN_RELOAD
		+ num*sizeof(MaterialResource **) + alignof(MaterialResource *)
#endif
		+ 0
		;

	SpriteInstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);
	new_data.first_hidden = _data.first_hidden;

	new_data.unit     = (UnitId *               )memory::align_top(new_data.buffer,         alignof(UnitId));
	new_data.resource = (const SpriteResource **)memory::align_top(new_data.unit + num,     alignof(SpriteResource *));
	new_data.material = (Material **            )memory::align_top(new_data.resource + num, alignof(Material *));
	new_data.frame    = (u32 *                  )memory::align_top(new_data.material + num, alignof(u32));
	new_data.world    = (Matrix4x4 *            )memory::align_top(new_data.frame + num,    alignof(Matrix4x4));
	new_data.aabb     = (AABB *                 )memory::align_top(new_data.world + num,    alignof(AABB));
	new_data.flags    = (u32 *                  )memory::align_top(new_data.aabb + num,     alignof(u32));
	new_data.layer    = (u32 *                  )memory::align_top(new_data.flags + num,    alignof(u32));
	new_data.depth    = (u32 *                  )memory::align_top(new_data.layer + num,    alignof(u32));
#if CROWN_CAN_RELOAD
	new_data.material_resource = (const MaterialResource **)memory::align_top(new_data.depth + num, alignof(MaterialResource *));
#endif

	memcpy(new_data.unit, _data.unit, _data.size * sizeof(UnitId));
	memcpy(new_data.resource, _data.resource, _data.size * sizeof(SpriteResource**));
	memcpy(new_data.material, _data.material, _data.size * sizeof(Material **));
	memcpy(new_data.frame, _data.frame, _data.size * sizeof(u32));
	memcpy(new_data.world, _data.world, _data.size * sizeof(Matrix4x4));
	memcpy(new_data.aabb, _data.aabb, _data.size * sizeof(AABB));
	memcpy(new_data.flags, _data.flags, _data.size * sizeof(u32));
	memcpy(new_data.layer, _data.layer, _data.size * sizeof(u32));
	memcpy(new_data.depth, _data.depth, _data.size * sizeof(u32));
#if CROWN_CAN_RELOAD
	memcpy(new_data.material_resource, _data.material_resource, _data.size * sizeof(MaterialResource *));
#endif

	_allocator->deallocate(_data.buffer);
	_data = new_data;
}

void RenderWorld::SpriteManager::grow()
{
	allocate(_data.capacity * 2 + 1);
}

SpriteInstance RenderWorld::SpriteManager::create(UnitId unit, const SpriteResource *sr, const SpriteRendererDesc &srd, const Matrix4x4 &tr)
{
	CE_ASSERT(!hash_map::has(_map, unit), "Unit already has a sprite component");

	if (_data.size == _data.capacity)
		grow();

	const u32 last = _data.size;

	const MaterialResource *mat_res = (MaterialResource *)_render_world->_resource_manager->get(RESOURCE_TYPE_MATERIAL, srd.material_resource);

	_data.unit[last]     = unit;
	_data.resource[last] = sr;
	_data.material[last] = _render_world->_material_manager->get(mat_res);
	_data.frame[last]    = 0;
	_data.world[last]    = tr;
	_data.aabb[last]     = AABB();
	_data.flags[last]    = srd.flags;
	_data.layer[last]    = srd.layer;
	_data.depth[last]    = srd.depth;
#if CROWN_CAN_RELOAD
	_data.material_resource[last] = mat_res;
#endif

	hash_map::set(_map, unit, last);
	++_data.size;

	if (srd.flags & RenderableFlags::VISIBLE) {
		if (last >= _data.first_hidden) {
			// _data now contains a visible item in its hidden partition.
			swap(last, _data.first_hidden);
			++_data.first_hidden;
			return make_instance(_data.first_hidden - 1);
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
	_data.flags[inst.i]    = _data.flags[last];
	_data.layer[inst.i]    = _data.layer[last];
	_data.depth[inst.i]    = _data.depth[last];
#if CROWN_CAN_RELOAD
	_data.material_resource[inst.i] = _data.material_resource[last];
#endif

	hash_map::set(_map, last_u, inst.i);
	hash_map::remove(_map, u);
	--_data.size;

	// If item was hidden.
	if (inst.i >= _data.first_hidden)
		return;

	// If item was visible *and* last item was hidden.
	if (last >= _data.first_hidden)
		swap(inst.i, _data.first_hidden - 1);

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
	exchange(_data.flags[inst_a],    _data.flags[inst_b]);
	exchange(_data.layer[inst_a],    _data.layer[inst_b]);
	exchange(_data.depth[inst_a],    _data.depth[inst_b]);
#if CROWN_CAN_RELOAD
	exchange(_data.material_resource[inst_a], _data.material_resource[inst_b]);
#endif

	hash_map::set(_map, unit_a, inst_b);
	hash_map::set(_map, unit_b, inst_a);
}

bool RenderWorld::SpriteManager::has(UnitId unit)
{
	return is_valid(sprite(unit));
}

void RenderWorld::SpriteManager::set_visible(SpriteInstance inst, bool visible)
{
	if (visible) {
		if (inst.i < _data.first_hidden)
			return; // Already visible.

		swap(inst.i, _data.first_hidden);
		++_data.first_hidden;
	} else {
		if (inst.i >= _data.first_hidden)
			return; // Already hidden.

		swap(inst.i, _data.first_hidden - 1);
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

void RenderWorld::SpriteManager::set_instance_data(f32 **vdata_, u16 **idata_, bgfx::TransientVertexBuffer &tvb, bgfx::TransientIndexBuffer &tib, u32 ii)
{
	f32 *vdata = *vdata_;
	u16 *idata = *idata_;

	const f32 *frame = sprite_resource::frame_data(_data.resource[ii]
		, _data.frame[ii] % _data.resource[ii]->num_frames
		);

	f32 u0 = frame[ 3]; // u
	f32 v0 = frame[ 4]; // v

	f32 u1 = frame[ 8]; // u
	f32 v1 = frame[ 9]; // v

	f32 u2 = frame[13]; // u
	f32 v2 = frame[14]; // v

	f32 u3 = frame[18]; // u
	f32 v3 = frame[19]; // v

	if ((_data.flags[ii] & SpriteFlags::FLIP_X) != 0) {
		f32 u;
		u = u0; u0 = u1; u1 = u;
		u = u2; u2 = u3; u3 = u;
	}

	if ((_data.flags[ii] & SpriteFlags::FLIP_Y) != 0) {
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

	*vdata_ += 20;

	idata[0] = ii*4 + 0;
	idata[1] = ii*4 + 1;
	idata[2] = ii*4 + 2;
	idata[3] = ii*4 + 0;
	idata[4] = ii*4 + 2;
	idata[5] = ii*4 + 3;

	*idata_ += 6;

	bgfx::setTransform(to_float_ptr(_data.world[ii]));
	bgfx::setVertexBuffer(0, &tvb);
	bgfx::setIndexBuffer(&tib, ii*6, 6);
}

void RenderWorld::SpriteManager::draw_visibles(u8 view_id)
{
	bgfx::VertexLayout layout;
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	f32 *vdata;
	u16 *idata;

	// Allocate vertex and index buffers.
	if (_data.first_hidden) {
		layout.begin();
		layout.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float);
		layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, false);
		layout.end();

		bgfx::allocTransientVertexBuffer(&tvb, 4*_data.first_hidden, layout);
		bgfx::allocTransientIndexBuffer(&tib, 6*_data.first_hidden);

		vdata = (f32 *)tvb.data;
		idata = (u16 *)tib.data;
	}

	// Render all sprites.
	for (u32 ii = 0; ii < _data.first_hidden; ++ii) {
		set_instance_data(&vdata, &idata, tvb, tib, ii);
		_data.material[ii]->bind(_data.layer[ii] + view_id, _data.depth[ii]);
	}
}

void RenderWorld::SpriteManager::draw_selected(u8 view_id)
{
	union
	{
		u32 u;
		f32 f;
	} u2f;

	bgfx::VertexLayout layout;
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	f32 *vdata;
	u16 *idata;

	// Allocate vertex and index buffers.
	if (_data.size) {
		layout.begin();
		layout.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float);
		layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, false);
		layout.end();

		bgfx::allocTransientVertexBuffer(&tvb, 4*_data.size, layout);
		bgfx::allocTransientIndexBuffer(&tib, 6*_data.size);

		vdata = (f32 *)tvb.data;
		idata = (u16 *)tib.data;
	}

	// Render all sprites.
	for (u32 ii = 0; ii < _data.size; ++ii) {
		set_instance_data(&vdata, &idata, tvb, tib, ii);

		UnitId unit_id = _data.unit[ii];
		if (!hash_set::has(_render_world->_selection, unit_id)) { // FIXME: put selected objects in a separate list.
			bgfx::discard();
			continue;
		}

		u2f.u = unit_id._idx;
		Vector4 data = { u2f.f, 0.0f, 0.0f, 0.0f };
		bgfx::setUniform(_render_world->_u_unit_id, &data);

		bgfx::setState(_render_world->_pipeline->_selection_shader.state);
		bgfx::submit(view_id, _render_world->_pipeline->_selection_shader.program);
	}
}

void RenderWorld::LightManager::allocate(u32 num)
{
	CE_ENSURE(num > _data.size);

	const u32 bytes = 0
		+ num*sizeof(UnitId) + alignof(UnitId)
		+ num*sizeof(u32) + alignof(u32)
		+ num*sizeof(u32) + alignof(u32)
		+ num*sizeof(ShaderData) + alignof(ShaderData)
		+ num*sizeof(f32) + alignof(f32)
		+ num*sizeof(u32) + alignof(u32)
		;

	LightInstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);

	new_data.unit = (UnitId *)memory::align_top(new_data.buffer, alignof(UnitId));
	new_data.flag = (u32 *)memory::align_top(new_data.unit + num, alignof(u32));
	new_data.type = (u32 *)memory::align_top(new_data.flag + num, alignof(u32));
	new_data.shader = (ShaderData *)memory::align_top(new_data.type + num, alignof(ShaderData));

	memcpy(new_data.unit, _data.unit, _data.size * sizeof(*new_data.unit));
	memcpy(new_data.flag, _data.flag, _data.size * sizeof(*new_data.flag));
	memcpy(new_data.type, _data.type, _data.size * sizeof(*new_data.type));
	memcpy(new_data.shader, _data.shader, _data.size * sizeof(ShaderData));

	_allocator->deallocate(_data.buffer);
	_data = new_data;
}

void RenderWorld::LightManager::grow()
{
	allocate(_data.capacity * 2 + 1);
}

LightInstance RenderWorld::LightManager::create(UnitId unit, const LightDesc &ld, const Matrix4x4 &tr)
{
	CE_ASSERT(!hash_map::has(_map, unit), "Unit already has a light component");

	if (_data.size == _data.capacity)
		grow();

	const u32 last = _data.size;

	Vector3 dir = -z(tr);
	normalize(dir);

	_data.unit[last]               = unit;
	_data.flag[last]               = ld.flags;
	_data.type[last]               = ld.type;
	_data.shader[last].color       = ld.color;
	_data.shader[last].intensity   = ld.intensity;
	_data.shader[last].position    = translation(tr);
	_data.shader[last].range       = ld.range;
	_data.shader[last].direction   = dir;
	_data.shader[last].spot_angle  = ld.spot_angle;
	_data.shader[last].shadow_bias = ld.shadow_bias;

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

	_data.unit[light.i] = _data.unit[last];
	_data.flag[light.i] = _data.flag[last];
	_data.type[light.i] = _data.type[last];
	_data.shader[light.i] = _data.shader[last];

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

void RenderWorld::LightManager::debug_draw(u32 start_index, u32 num, DebugLine &dl)
{
	for (u32 i = start_index; i < start_index + num; ++i) {
		const Vector3 pos = _data.shader[i].position;
		const Vector3 dir = _data.shader[i].direction;

		switch (_data.type[i]) {
		case LightType::DIRECTIONAL: {
			const Vector3 end = pos + dir*3.0f;
			dl.add_line(pos, end, COLOR4_YELLOW);
			dl.add_cone(pos + dir*2.8f, end, 0.1f, COLOR4_YELLOW);
			break;
		}

		case LightType::OMNI:
			dl.add_sphere(pos, _data.shader[i].range, COLOR4_YELLOW);
			break;

		case LightType::SPOT: {
			const f32 angle  = _data.shader[i].spot_angle;
			const f32 range  = _data.shader[i].range;
			const f32 radius = ftan(angle)*range;
			dl.add_cone(pos + range*dir, pos, radius, COLOR4_YELLOW, 36, 4);
			break;
		}

		default:
			CE_FATAL("Unknown light type");
			break;
		}
	}
}

} // namespace crown
