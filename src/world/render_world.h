/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/math/types.h"
#include "core/strings/string_id.h"
#include "device/pipeline.h"
#include "resource/mesh_resource.h"
#include "resource/mesh_skeleton_resource.h"
#include "resource/shader_resource.h"
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
	RenderWorld(Allocator &a
		, ResourceManager &rm
		, ShaderManager &sm
		, MaterialManager &mm
		, UnitManager &um
		, Pipeline &pl
		, SceneGraph &sg
		);

	///
	~RenderWorld();

	/// Creates a new mesh instance.
	MeshInstance mesh_create(UnitId unit, const MeshRendererDesc &mrd, const Matrix4x4 &tr);

	/// Destroys the @a mesh.
	void mesh_destroy(MeshInstance mesh);

	/// Returns the ID of the mesh owned by the *unit*.
	MeshInstance mesh_instance(UnitId unit);

	/// Sets the @a geometry of the @a mesh. @a geometry must be a valid geometry name inside @a
	/// mesh_resource.
	void mesh_set_geometry(MeshInstance mesh, StringId64 mesh_resource, StringId32 geometry);

	///
	void mesh_set_skeleton(MeshInstance mesh, const AnimationSkeletonInstance *bones);

	/// Returns the material of the @a mesh.
	Material *mesh_material(MeshInstance mesh);

	/// Sets the material @a id of the @a mesh.
	void mesh_set_material(MeshInstance mesh, StringId64 id);

	/// Sets whether the @a mesh is @a visible.
	void mesh_set_visible(MeshInstance mesh, bool visible);

	/// Returns the OBB of the @a mesh.
	OBB mesh_obb(MeshInstance mesh);

	/// Returns the distance along ray (from, dir) to intersection point with @a mesh
	/// or -1.0 if no intersection.
	f32 mesh_cast_ray(MeshInstance mesh, const Vector3 &from, const Vector3 &dir);

	/// Creates a new sprite instance.
	SpriteInstance sprite_create(UnitId unit, const SpriteRendererDesc &srd, const Matrix4x4 &tr);

	/// Destroys the sprite @a i.
	void sprite_destroy(SpriteInstance sprite);

	/// Returns the ID of the sprite owned by the *unit*.
	SpriteInstance sprite_instance(UnitId unit);

	/// Sets the @a sprite_resource of the @a sprite.
	void sprite_set_sprite(SpriteInstance sprite, StringId64 sprite_resource_name);

	/// Returns the material of the @a sprite.
	Material *sprite_material(SpriteInstance sprite);

	/// Sets the material @a id of the @a sprite.
	void sprite_set_material(SpriteInstance sprite, StringId64 id);

	/// Sets the frame @a index of the @a sprite.
	/// The @a index automatically wraps if it greater than
	/// the total number of frames in the @a sprite.
	void sprite_set_frame(SpriteInstance sprite, u32 index);

	/// Sets whether the @a sprite is @a visible.
	void sprite_set_visible(SpriteInstance sprite, bool visible);

	/// Sets whether to flip the @a sprite on the x-axis.
	void sprite_flip_x(SpriteInstance sprite, bool flip);

	/// Sets whether to flip the @a sprite on the y-axis.
	void sprite_flip_y(SpriteInstance sprite, bool flip);

	/// Sets the layer of the @a sprite.
	void sprite_set_layer(SpriteInstance sprite, u32 layer);

	/// Sets the depth of the @a sprite.
	void sprite_set_depth(SpriteInstance sprite, u32 depth);

	/// Returns the OBB of the @a sprite.
	OBB sprite_obb(SpriteInstance sprite);

	/// Returns the distance along ray (from, dir) to intersection point with
	/// @a sprite or -1.0 if no intersection.
	f32 sprite_cast_ray(SpriteInstance sprite, const Vector3 &from, const Vector3 &dir, u32 &layer, u32 &depth);

	/// Creates a new light instance.
	LightInstance light_create(UnitId unit, const LightDesc &ld, const Matrix4x4 &tr);

	/// Destroys the @a light.
	void light_destroy(LightInstance light);

	/// Returns the ID of the light owned by the *unit*.
	LightInstance light_instance(UnitId unit);

	/// Returns the type of the @a light.
	LightType::Enum light_type(LightInstance light);

	/// Returns the color of the @a light.
	Color4 light_color(LightInstance light);

	/// Returns the range of the @a light.
	f32 light_range(LightInstance light);

	/// Returns the intensity of the @a light.
	f32 light_intensity(LightInstance light);

	/// Returns the spot angle of the @a light.
	f32 light_spot_angle(LightInstance light);

	/// Returns the shadow bias of the @a light.
	f32 light_shadow_bias(LightInstance light);

	/// Sets the @a type of the @a light.
	void light_set_type(LightInstance light, LightType::Enum type);

	/// Sets the @a color of the @a light.
	void light_set_color(LightInstance light, const Color4 &color);

	/// Sets the @a range of the @a light.
	void light_set_range(LightInstance light, f32 range);

	/// Sets the @a intensity of the @a light.
	void light_set_intensity(LightInstance light, f32 intensity);

	/// Sets the spot @a angle of the @a light.
	void light_set_spot_angle(LightInstance light, f32 angle);

	/// Sets the shadow @a bias of the @a light.
	void light_set_shadow_bias(LightInstance light, f32 bias);

	/// Fills @a dl with debug lines from the @a light.
	void light_debug_draw(LightInstance light, DebugLine &dl);

	void update_transforms(const UnitId *begin, const UnitId *end, const Matrix4x4 *world);

	///
	void render(const Matrix4x4 &view, const Matrix4x4 &proj);

	/// Sets whether to @a enable debug drawing
	void enable_debug_drawing(bool enable);

	/// Fills @a dl with debug lines
	void debug_draw(DebugLine &dl);

	///
	void unit_destroyed_callback(UnitId unit);

	///
	void reload_materials(const MaterialResource *old_resource, const MaterialResource *new_resource);

	/// Callback to customize drawing of objects.
	typedef void (*DrawOverride)(u8 view_id, UnitId unit_id, RenderWorld *rw);

	/// List of meshes to be rendered.
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
			void *buffer;

			u32 first_hidden;

			UnitId *unit;
			const MeshResource **resource;
			const MeshGeometry **geometry;
			MeshData *mesh;
			Material **material;
			Matrix4x4 *world;
			OBB *obb;
			const AnimationSkeletonInstance **skeleton;
			u32 *flags;
#if CROWN_CAN_RELOAD
			const MaterialResource **material_resource;
#endif
		};

		Allocator *_allocator;
		RenderWorld *_render_world;
		HashMap<UnitId, u32> _map;
		MeshInstanceData _data;

		///
		MeshManager(Allocator &a, RenderWorld *rw)
			: _allocator(&a)
			, _render_world(rw)
			, _map(a)
		{
			memset(&_data, 0, sizeof(_data));
		}

		///
		void allocate(u32 num);

		///
		void grow();

		///
		MeshInstance create(UnitId unit, const MeshResource *mr, const MeshRendererDesc &mrd, const Matrix4x4 &tr);

		///
		void destroy(MeshInstance mesh);

		///
		bool has(UnitId unit);

		///
		void set_geometry(MeshInstance mesh, const MeshResource *mr, StringId32 geometry);

		///
		void set_visible(MeshInstance mesh, bool visible);

		///
		MeshInstance mesh(UnitId unit);

		///
		void destroy();

		///
		void swap(u32 inst_a, u32 inst_b);

		///
		void set_instance_data(u32 ii, SceneGraph &scene_graph);

		///
		void draw_shadow_casters(u8 view, SceneGraph &scene_graph);

		///
		void draw_visibles(u8 view, SceneGraph &scene_graph, const Matrix4x4 *cascaded_lights);

		///
		void draw_selected(u8 view, SceneGraph &scene_graph);

		///
		MeshInstance make_instance(u32 i)
		{
			MeshInstance inst = { i }; return inst;
		}
	};

	/// List of meshes to be rendered.
	struct SpriteManager
	{
		struct SpriteInstanceData
		{
			u32 size;
			u32 capacity;
			void *buffer;

			u32 first_hidden;

			UnitId *unit;
			const SpriteResource **resource;
			Material **material;
			u32 *frame;
			Matrix4x4 *world;
			AABB *aabb;
			bool *flip_x;
			bool *flip_y;
			u32 *layer;
			u32 *depth;
#if CROWN_CAN_RELOAD
			const MaterialResource **material_resource;
#endif
		};

		Allocator *_allocator;
		RenderWorld *_render_world;
		HashMap<UnitId, u32> _map;
		SpriteInstanceData _data;

		///
		SpriteManager(Allocator &a, RenderWorld *rw)
			: _allocator(&a)
			, _render_world(rw)
			, _map(a)
		{
			memset(&_data, 0, sizeof(_data));
		}

		///
		SpriteInstance create(UnitId unit, const SpriteResource *sr, const SpriteRendererDesc &srd, const Matrix4x4 &tr);

		///
		void destroy(SpriteInstance sprite);

		///
		bool has(UnitId unit);

		///
		void set_visible(SpriteInstance sprite, bool visible);

		///
		SpriteInstance sprite(UnitId unit);

		///
		void allocate(u32 num);

		///
		void grow();

		///
		void destroy();

		///
		void swap(u32 inst_a, u32 inst_b);

		///
		void set_instance_data(f32 **vdata, u16 **idata, bgfx::TransientVertexBuffer &tvb, bgfx::TransientIndexBuffer &tib, u32 ii);

		///
		void draw_visibles(u8 view);

		///
		void draw_selected(u8 view);

		///
		SpriteInstance make_instance(u32 i)
		{
			SpriteInstance inst = { i }; return inst;
		}
	};

	struct LightManager
	{
		// This data is fed to the shader as-is.
		// Keep it in sync with "core/shaders/lighting.shader"!
		struct ShaderData
		{
			Vector3 color;
			f32 intensity;
			Vector3 position;
			f32 range;
			Vector3 direction;
			f32 spot_angle;
			f32 shadow_bias;
			f32 atlas_u;   // U-coord in shadow map atlas.
			f32 atlas_v;   // V-coord in shadow map atlas.
			f32 map_size;  // Tile size in shadow map atlas.
		};

		struct Index
		{
			UnitId unit;
			u32 flags;
			u32 type  : 4;  // LightType::Enum
			u32 index : 28; // Maps from light-index to sorted light-index.
		};

		struct LightInstanceData
		{
			u32 size;
			u32 num[LightType::COUNT]; // Number of lights for each type.
			bool dirty;                // true if data (potentially) unsorted.
			u32 capacity;
			void *buffer;

			Index *index;
			ShaderData *shader_a;
			ShaderData *shader_b;

			ShaderData *shader; // After sort(), it points to the sorted shader array.
			ShaderData *new_shader; // Temporary storage for sort().
		};

		Allocator *_allocator;
		HashMap<UnitId, u32> _map;
		LightInstanceData _data;

		///
		explicit LightManager(Allocator &a)
			: _allocator(&a)
			, _map(a)
		{
			memset(&_data, 0, sizeof(_data));
		}

		///
		LightInstance create(UnitId unit, const LightDesc &ld, const Matrix4x4 &tr);

		///
		void destroy(LightInstance light);

		///
		bool has(UnitId unit);

		///
		LightInstance light(UnitId unit);

		///
		void debug_draw(u32 start_index, u32 num, DebugLine &dl);

		///
		void allocate(u32 num);

		///
		void grow();

		///
		void destroy();

		///
		LightInstance make_instance(u32 i)
		{
			LightInstance inst = { i }; return inst;
		}
	};

	u32 _marker;
	ResourceManager *_resource_manager;
	ShaderManager *_shader_manager;
	MaterialManager *_material_manager;
	UnitManager *_unit_manager;
	Pipeline *_pipeline;
	SceneGraph *_scene_graph;

	bool _debug_drawing;
	MeshManager _mesh_manager;
	SpriteManager _sprite_manager;
	LightManager _light_manager;

	UnitDestroyCallback _unit_destroy_callback;

	// Outlines.
	HashSet<UnitId> _selection;
	bgfx::UniformHandle _u_unit_id;

	// Lighting.
	bgfx::UniformHandle _u_lights_num;
	bgfx::UniformHandle _u_lights_data;

	// Shadow mapping.
	bgfx::FrameBufferHandle _cascaded_shadow_map_frame_buffer;
	bgfx::UniformHandle _u_cascaded_shadow_map;
	bgfx::UniformHandle _u_cascaded_texel_size;
	bgfx::UniformHandle _u_cascaded_lights;
};

} // namespace crown
