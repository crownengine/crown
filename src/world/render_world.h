/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "math_types.h"
#include "resource_types.h"
#include "string_id.h"
#include "world_types.h"
#include <bgfx/bgfx.h>

namespace crown
{

/// Manages graphics objects in a World.
///
/// @ingroup World
class RenderWorld
{
public:

	RenderWorld(Allocator& a, ResourceManager& rm, ShaderManager& sm, MaterialManager& mm, UnitManager& um);
	~RenderWorld();

	/// Creates a new mesh instance.
	MeshInstance create_mesh(UnitId id, const MeshRendererDesc& mrd, const Matrix4x4& tr);

	/// Destroys the mesh @a i.
	void destroy_mesh(MeshInstance i);

	/// Returns the first mesh of the unit @a id.
	MeshInstance first_mesh(UnitId id);

	/// Returns the next mesh in the chain.
	MeshInstance next_mesh(MeshInstance id);

	/// Returns the previous mesh in the chain.
	MeshInstance previous_mesh(MeshInstance id);

	void set_mesh_material(MeshInstance i, StringId64 id);
	void set_mesh_visible(MeshInstance i, bool visible);
	OBB mesh_obb(MeshInstance i);

	/// Creates a new sprite instance.
	SpriteInstance create_sprite(UnitId id, const SpriteRendererDesc& srd, const Matrix4x4& tr);

	/// Destroys the sprite @a i.
	void destroy_sprite(SpriteInstance i);

	/// Returns the first sprite in the chain.
	SpriteInstance first_sprite(UnitId id);

	/// Returns the next sprite in the chain.
	SpriteInstance next_sprite(SpriteInstance i);

	/// Returns the previous sprite in the chain.
	SpriteInstance previous_sprite(SpriteInstance id);

	void set_sprite_material(SpriteInstance i, StringId64 id);
	void set_sprite_frame(SpriteInstance i, uint32_t index);
	void set_sprite_visible(SpriteInstance i, bool visible);

	/// Creates a new light instance.
	LightInstance create_light(UnitId id, const LightDesc& ld, const Matrix4x4& tr);

	/// Destroys the light @a i.
	void destroy_light(LightInstance i);

	/// Returns the light of the unit @a id.
	LightInstance light(UnitId id);

	/// Returns the type of the light @a i.
	LightType::Enum light_type(LightInstance i);

	/// Returns the color of the light @a i.
	Color4 light_color(LightInstance i);

	/// Returns the range of the light @a i.
	float light_range(LightInstance i);

	/// Returns the intensity of the light @a i.
	float light_intensity(LightInstance i);

	/// Returns the spot angle of the light @a i.
	float light_spot_angle(LightInstance i);

	/// Sets the @a type of the light @a i.
	void set_light_type(LightInstance i, LightType::Enum type);

	/// Sets the @a color of the light @a i.
	void set_light_color(LightInstance i, const Color4& color);

	/// Sets the @a range of the light @a i.
	void set_light_range(LightInstance i, float range);

	/// Sets the @a intensity of the light @a i.
	void set_light_intensity(LightInstance i, float intensity);

	/// Sets the spot @a angle of the light @a i.
	void set_light_spot_angle(LightInstance i, float angle);

	void update_transforms(const UnitId* begin, const UnitId* end, const Matrix4x4* world);

	void render(const Matrix4x4& view, const Matrix4x4& projection, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

	/// Sets whether to @a enable debug drawing
	void enable_debug_drawing(bool enable);

	/// Fills @a dl with debug lines
	void draw_debug(DebugLine& dl);

	bool is_valid(MeshInstance i) { return i.i != UINT32_MAX; }
	bool is_valid(SpriteInstance i) { return i.i != UINT32_MAX; }
	bool is_valid(LightInstance i) { return i.i != UINT32_MAX; }

	static const uint32_t MARKER = 0xc82277de;

private:

	static void unit_destroyed_callback(UnitId id, void* user_ptr)
	{
		((RenderWorld*)user_ptr)->unit_destroyed_callback(id);
	}

	void unit_destroyed_callback(UnitId id);

	void allocate_mesh(uint32_t num);
	void allocate_sprite(uint32_t num);
	void allocate_light(uint32_t num);
	void grow_mesh();
	void grow_sprite();
	void grow_light();

	MeshInstance make_mesh_instance(uint32_t i) { MeshInstance inst = { i }; return inst; }
	SpriteInstance make_sprite_instance(uint32_t i) { SpriteInstance inst = { i }; return inst; }
	LightInstance make_light_instance(uint32_t i) { LightInstance inst = { i }; return inst; }

	void add_mesh_node(MeshInstance first, MeshInstance i);
	void remove_mesh_node(MeshInstance first, MeshInstance i);
	void swap_mesh_node(MeshInstance a, MeshInstance b);
	void add_sprite_node(SpriteInstance first, SpriteInstance i);
	void remove_sprite_node(SpriteInstance first, SpriteInstance i);
	void swap_sprite_node(SpriteInstance a, SpriteInstance b);

	struct MeshData
	{
		bgfx::VertexBufferHandle vbh;
		bgfx::IndexBufferHandle ibh;
	};

	struct MeshInstanceData
	{
		MeshInstanceData()
			: size(0)
			, capacity(0)
			, buffer(NULL)

			, first_hidden(0)

			, unit(NULL)
			, mr(NULL)
			, mesh(NULL)
			, material(NULL)
			, world(NULL)
			, obb(NULL)
			, next_instance(NULL)
		{
		}

		uint32_t size;
		uint32_t capacity;
		void* buffer;

		uint32_t first_hidden;

		UnitId* unit;
		const MeshResource** mr;
		MeshData* mesh;
		StringId64* material;
		Matrix4x4* world;
		OBB* obb;
		MeshInstance* next_instance;
	};

	struct SpriteData
	{
		bgfx::VertexBufferHandle vbh;
		bgfx::IndexBufferHandle ibh;
	};

	struct SpriteInstanceData
	{
		SpriteInstanceData()
			: size(0)
			, capacity(0)
			, buffer(NULL)

			, first_hidden(0)

			, unit(NULL)
			, sr(NULL)
			, sprite(NULL)
			, material(NULL)
			, frame(NULL)
			, world(NULL)
			, aabb(NULL)
			, next_instance(NULL)
		{
		}

		uint32_t size;
		uint32_t capacity;
		void* buffer;

		uint32_t first_hidden;

		UnitId* unit;
		const SpriteResource** sr;
		SpriteData* sprite;
		StringId64* material;
		uint32_t* frame;
		Matrix4x4* world;
		AABB* aabb;
		SpriteInstance* next_instance;
	};

	struct LightInstanceData
	{
		LightInstanceData()
			: size(0)
			, capacity(0)
			, buffer(NULL)

			, unit(NULL)
			, world(NULL)
			, range(NULL)
			, intensity(NULL)
			, spot_angle(NULL)
			, color(NULL)
			, type(NULL)
		{
		}

		uint32_t size;
		uint32_t capacity;
		void* buffer;

		UnitId* unit;
		Matrix4x4* world;
		float* range;
		float* intensity;
		float* spot_angle;
		Color4* color;
		uint32_t* type; // LightType::Enum
	};

	uint32_t _marker;

	Allocator* _allocator;
	ResourceManager* _resource_manager;
	ShaderManager* _shader_manager;
	MaterialManager* _material_manager;

	bgfx::UniformHandle _u_light_pos;
	bgfx::UniformHandle _u_light_dir;
	bgfx::UniformHandle _u_light_col;

	bool _debug_drawing;
	Hash<uint32_t> _mesh_map;
	MeshInstanceData _mesh_data;
	Hash<uint32_t> _sprite_map;
	SpriteInstanceData _sprite_data;
	Hash<uint32_t> _light_map;
	LightInstanceData _light_data;
};

} // namespace crown
