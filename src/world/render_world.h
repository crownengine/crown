/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/math/types.h"
#include "core/strings/string_id.h"
#include "resource/mesh_resource.h"
#include "resource/types.h"
#include "world/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
/// Manages graphics objects in a World.
///
/// @ingroup World
struct RenderWorld
{
	///
	RenderWorld(Allocator& a, ResourceManager& rm, ShaderManager& sm, MaterialManager& mm, UnitManager& um);

	///
	~RenderWorld();

	/// Creates a new mesh instance.
	MeshInstance mesh_create(UnitId id, const MeshRendererDesc& mrd, const Matrix4x4& tr);

	/// Destroys the mesh @a i.
	void mesh_destroy(MeshInstance i);

	/// Returns the mesh instances of the unit @a id.
	void mesh_instances(UnitId id, Array<MeshInstance>& instances);

	/// Sets the material @a id of the mesh @a i.
	void mesh_set_material(MeshInstance i, StringId64 id);

	/// Sets whether the mesh @a i is @a visible.
	void mesh_set_visible(MeshInstance i, bool visible);

	/// Returns the OBB of the mesh @a i.
	OBB mesh_obb(MeshInstance i);

	/// Returns the distance along ray (from, dir) to intersection point with mesh @a i
	/// or -1.0 if no intersection.
	f32 mesh_raycast(MeshInstance i, const Vector3& from, const Vector3& dir);

	/// Creates a new sprite instance.
	SpriteInstance sprite_create(UnitId id, const SpriteRendererDesc& srd, const Matrix4x4& tr);

	/// Destroys the sprite of the @a unit.
	void sprite_destroy(UnitId unit, SpriteInstance i);

	/// Returns the sprite instances of the @a unit.
	SpriteInstance sprite_instances(UnitId unit);

	/// Sets the material @a id of the sprite.
	void sprite_set_material(UnitId unit, StringId64 id);

	/// Sets the frame @a index of the sprite.
	void sprite_set_frame(UnitId unit, u32 index);

	/// Sets whether the sprite is @a visible.
	void sprite_set_visible(UnitId unit, bool visible);

	/// Sets whether to flip the sprite on the x-axis.
	void sprite_flip_x(UnitId unit, bool flip);

	/// Sets whether to flip the sprite on the y-axis.
	void sprite_flip_y(UnitId unit, bool flip);

	/// Sets the layer of the sprite.
	void sprite_set_layer(UnitId unit, u32 layer);

	/// Sets the depth of the sprite.
	void sprite_set_depth(UnitId unit, u32 depth);

	/// Returns the OBB of the sprite.
	OBB sprite_obb(UnitId unit);

	/// Returns the distance along ray (from, dir) to intersection point with sprite
	/// or -1.0 if no intersection.
	f32 sprite_raycast(UnitId unit, const Vector3& from, const Vector3& dir, u32& layer, u32& depth);

	/// Creates a new light instance.
	LightInstance light_create(UnitId unit, const LightDesc& ld, const Matrix4x4& tr);

	/// Destroys the light.
	void light_destroy(UnitId unit, LightInstance i);

	/// Returns the light of the @a unit.
	LightInstance light_instances(UnitId unit);

	/// Returns the type of the light.
	LightType::Enum light_type(UnitId unit);

	/// Returns the color of the light.
	Color4 light_color(UnitId unit);

	/// Returns the range of the light.
	f32 light_range(UnitId unit);

	/// Returns the intensity of the light.
	f32 light_intensity(UnitId unit);

	/// Returns the spot angle of the light.
	f32 light_spot_angle(UnitId unit);

	/// Sets the @a type of the light.
	void light_set_type(UnitId unit, LightType::Enum type);

	/// Sets the @a color of the light.
	void light_set_color(UnitId unit, const Color4& color);

	/// Sets the @a range of the light.
	void light_set_range(UnitId unit, f32 range);

	/// Sets the @a intensity of the light.
	void light_set_intensity(UnitId unit, f32 intensity);

	/// Sets the spot @a angle of the light.
	void light_set_spot_angle(UnitId unit, f32 angle);

	/// Fills @a dl with debug lines from the light.
	void light_debug_draw(UnitId unit, DebugLine& dl);

	void update_transforms(const UnitId* begin, const UnitId* end, const Matrix4x4* world);

	void render(const Matrix4x4& view);

	/// Sets whether to @a enable debug drawing
	void enable_debug_drawing(bool enable);

	/// Fills @a dl with debug lines
	void debug_draw(DebugLine& dl);

	void unit_destroyed_callback(UnitId id);

	struct MeshManager
	{
		struct MeshData
		{
			bgfx::VertexBufferHandle vbh;
			bgfx::IndexBufferHandle ibh;
		};

		struct MeshInstanceData
		{
			u32 size;
			u32 capacity;
			void* buffer;

			u32 first_hidden;

			UnitId* unit;
			const MeshResource** resource;
			const MeshGeometry** geometry;
			MeshData* mesh;
			StringId64* material;
			Matrix4x4* world;
			OBB* obb;
			MeshInstance* next_instance;
		};

		Allocator* _allocator;
		HashMap<UnitId, u32> _map;
		MeshInstanceData _data;

		MeshManager(Allocator& a)
			: _allocator(&a)
			, _map(a)
		{
			memset(&_data, 0, sizeof(_data));
		}

		void allocate(u32 num);
		void grow();
		MeshInstance create(UnitId id, const MeshResource* mr, const MeshGeometry* mg, StringId64 material, const Matrix4x4& tr);
		void destroy(MeshInstance i);
		bool has(UnitId id);
		MeshInstance first(UnitId id);
		MeshInstance next(MeshInstance i);
		MeshInstance previous(MeshInstance i);
		void add_node(MeshInstance first, MeshInstance i);
		void remove_node(MeshInstance first, MeshInstance i);
		void swap_node(MeshInstance a, MeshInstance b);
		void destroy();

		MeshInstance make_instance(u32 i) { MeshInstance inst = { i }; return inst; }
	};

	struct SpriteManager
	{
		struct SpriteInstanceData
		{
			u32 size;
			u32 capacity;
			void* buffer;

			u32 first_hidden;

			UnitId* unit;
			const SpriteResource** resource;
			StringId64* material;
			u32* frame;
			Matrix4x4* world;
			AABB* aabb;
			bool* flip_x;
			bool* flip_y;
			u32* layer;
			u32* depth;
			SpriteInstance* next_instance;
		};

		Allocator* _allocator;
		HashMap<UnitId, u32> _map;
		SpriteInstanceData _data;

		SpriteManager(Allocator& a)
			: _allocator(&a)
			, _map(a)
		{
			memset(&_data, 0, sizeof(_data));
		}

		SpriteInstance create(UnitId id, const SpriteResource* sr, StringId64 material, u32 layer, u32 depth, const Matrix4x4& tr);
		void destroy(SpriteInstance i);
		bool has(UnitId id);
		SpriteInstance sprite(UnitId id);
		void allocate(u32 num);
		void grow();
		void destroy();

		SpriteInstance make_instance(u32 i) { SpriteInstance inst = { i }; return inst; }
	};

	struct LightManager
	{
		struct LightInstanceData
		{
			u32 size;
			u32 capacity;
			void* buffer;

			UnitId* unit;
			Matrix4x4* world;
			f32* range;
			f32* intensity;
			f32* spot_angle;
			Color4* color;
			u32* type; // LightType::Enum
		};

		Allocator* _allocator;
		HashMap<UnitId, u32> _map;
		LightInstanceData _data;

		LightManager(Allocator& a)
			: _allocator(&a)
			, _map(a)
		{
			memset(&_data, 0, sizeof(_data));
		}

		LightInstance create(UnitId id, const LightDesc& ld, const Matrix4x4& tr);
		void destroy(LightInstance i);
		bool has(UnitId id);
		LightInstance light(UnitId id);
		void debug_draw(u32 start_index, u32 num, DebugLine& dl);

		void allocate(u32 num);
		void grow();
		void destroy();

		LightInstance make_instance(u32 i) { LightInstance inst = { i }; return inst; }
	};

	u32 _marker;
	Allocator* _allocator;
	ResourceManager* _resource_manager;
	ShaderManager* _shader_manager;
	MaterialManager* _material_manager;
	UnitManager* _unit_manager;

	bgfx::UniformHandle _u_light_position;
	bgfx::UniformHandle _u_light_direction;
	bgfx::UniformHandle _u_light_color;
	bgfx::UniformHandle _u_light_range;
	bgfx::UniformHandle _u_light_intensity;

	bool _debug_drawing;
	MeshManager _mesh_manager;
	SpriteManager _sprite_manager;
	LightManager _light_manager;
};

} // namespace crown
