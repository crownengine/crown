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
#include "mesh_resource.h"
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

	/// Returns the mesh instances of the unit @a id.
	void mesh_instances(UnitId id, Array<MeshInstance>& instances);

	void set_mesh_material(MeshInstance i, StringId64 id);
	void set_mesh_visible(MeshInstance i, bool visible);
	OBB mesh_obb(MeshInstance i);

	f32 mesh_raycast(MeshInstance i, const Vector3& from, const Vector3& dir);

	/// Creates a new sprite instance.
	SpriteInstance create_sprite(UnitId id, const SpriteRendererDesc& srd, const Matrix4x4& tr);

	/// Destroys the sprite @a i.
	void destroy_sprite(SpriteInstance i);

	/// Returns the sprite instances of the unit @a id.
	void sprite_instances(UnitId id, Array<SpriteInstance>& instances);

	void set_sprite_material(SpriteInstance i, StringId64 id);
	void set_sprite_frame(SpriteInstance i, u32 index);
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
	f32 light_range(LightInstance i);

	/// Returns the intensity of the light @a i.
	f32 light_intensity(LightInstance i);

	/// Returns the spot angle of the light @a i.
	f32 light_spot_angle(LightInstance i);

	/// Sets the @a type of the light @a i.
	void set_light_type(LightInstance i, LightType::Enum type);

	/// Sets the @a color of the light @a i.
	void set_light_color(LightInstance i, const Color4& color);

	/// Sets the @a range of the light @a i.
	void set_light_range(LightInstance i, f32 range);

	/// Sets the @a intensity of the light @a i.
	void set_light_intensity(LightInstance i, f32 intensity);

	/// Sets the spot @a angle of the light @a i.
	void set_light_spot_angle(LightInstance i, f32 angle);

	void update_transforms(const UnitId* begin, const UnitId* end, const Matrix4x4* world);

	void render(const Matrix4x4& view, const Matrix4x4& projection);

	/// Sets whether to @a enable debug drawing
	void enable_debug_drawing(bool enable);

	/// Fills @a dl with debug lines from light @a i.
	void debug_draw_light(LightInstance i, DebugLine& dl);

	/// Fills @a dl with debug lines
	void debug_draw(DebugLine& dl);

private:

	static void unit_destroyed_callback(UnitId id, void* user_ptr)
	{
		((RenderWorld*)user_ptr)->unit_destroyed_callback(id);
	}

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
		bool is_valid(MeshInstance i) { return i.i != UINT32_MAX; }
	};

	struct SpriteManager
	{
		struct SpriteData
		{
			bgfx::VertexBufferHandle vbh;
			bgfx::IndexBufferHandle ibh;
		};

		struct SpriteInstanceData
		{
			u32 size;
			u32 capacity;
			void* buffer;

			u32 first_hidden;

			UnitId* unit;
			const SpriteResource** resource;
			SpriteData* sprite;
			StringId64* material;
			u32* frame;
			Matrix4x4* world;
			AABB* aabb;
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

		void allocate(u32 num);
		void grow();
		SpriteInstance create(UnitId id, const SpriteResource* sr, StringId64 material, const Matrix4x4& tr);
		void destroy(SpriteInstance i);
		bool has(UnitId id);
		SpriteInstance first(UnitId id);
		SpriteInstance next(SpriteInstance i);
		SpriteInstance previous(SpriteInstance i);
		void add_node(SpriteInstance first, SpriteInstance i);
		void remove_node(SpriteInstance first, SpriteInstance i);
		void swap_node(SpriteInstance a, SpriteInstance b);
		void destroy();

		SpriteInstance make_instance(u32 i) { SpriteInstance inst = { i }; return inst; }
		bool is_valid(SpriteInstance i) { return i.i != UINT32_MAX; }
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
		void allocate(u32 num);
		void grow();
		void destroy();

		LightInstance make_instance(u32 i) { LightInstance inst = { i }; return inst; }
		bool is_valid(LightInstance i) { return i.i != UINT32_MAX; }
	};

	u32 _marker;

	Allocator* _allocator;
	ResourceManager* _resource_manager;
	ShaderManager* _shader_manager;
	MaterialManager* _material_manager;
	UnitManager* _unit_manager;

	bgfx::UniformHandle _u_light_pos;
	bgfx::UniformHandle _u_light_dir;
	bgfx::UniformHandle _u_light_col;

	bool _debug_drawing;
	MeshManager _mesh_manager;
	SpriteManager _sprite_manager;
	LightManager _light_manager;
};

} // namespace crown
