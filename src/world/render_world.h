/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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
	MeshId mesh_create(UnitId unit, const MeshRendererDesc &mrd);

	/// Destroys the @a mesh.
	void mesh_destroy(MeshId mesh);

	/// Returns the ID of the mesh owned by the *unit*.
	MeshId mesh_instance(UnitId unit);

	/// Sets the @a geometry of the @a mesh. @a geometry must be a valid geometry name inside @a
	/// mesh_resource.
	void mesh_set_geometry(MeshId mesh, StringId64 mesh_resource, StringId32 geometry);

	///
	void mesh_set_skeleton(MeshId mesh, const AnimationSkeletonInstance *bones);

	/// Returns the material of the @a mesh.
	Material *mesh_material(MeshId mesh);

	/// Sets the material @a id of the @a mesh.
	void mesh_set_material(MeshId mesh, StringId64 id);

	/// Sets whether the @a mesh is @a visible.
	void mesh_set_visible(MeshId mesh, bool visible);

	/// Sets whether the @a mesh casts shadows.
	void mesh_set_cast_shadows(MeshId mesh, bool cast_shadows);

	/// Returns the OBB of the @a mesh.
	OBB mesh_obb(MeshId mesh);

	/// Returns the distance along ray (from, dir) to intersection point with @a mesh
	/// or -1.0 if no intersection.
	f32 mesh_cast_ray(MeshId mesh, const Vector3 &from, const Vector3 &dir);

	/// Creates a new sprite instance.
	SpriteId sprite_create(UnitId unit, const SpriteRendererDesc &srd);

	/// Destroys the sprite @a i.
	void sprite_destroy(SpriteId sprite);

	/// Returns the ID of the sprite owned by the *unit*.
	SpriteId sprite_instance(UnitId unit);

	/// Sets the @a sprite_resource of the @a sprite.
	void sprite_set_sprite(SpriteId sprite, StringId64 sprite_resource_name);

	/// Returns the material of the @a sprite.
	Material *sprite_material(SpriteId sprite);

	/// Sets the material @a id of the @a sprite.
	void sprite_set_material(SpriteId sprite, StringId64 id);

	/// Sets the frame @a index of the @a sprite.
	/// The @a index automatically wraps if it greater than
	/// the total number of frames in the @a sprite.
	void sprite_set_frame(SpriteId sprite, u32 index);

	/// Sets whether the @a sprite is @a visible.
	void sprite_set_visible(SpriteId sprite, bool visible);

	/// Sets whether to flip the @a sprite on the x-axis.
	void sprite_flip_x(SpriteId sprite, bool flip);

	/// Sets whether to flip the @a sprite on the y-axis.
	void sprite_flip_y(SpriteId sprite, bool flip);

	/// Sets the layer of the @a sprite.
	void sprite_set_layer(SpriteId sprite, u32 layer);

	/// Sets the depth of the @a sprite.
	void sprite_set_depth(SpriteId sprite, u32 depth);

	/// Returns the OBB of the @a sprite.
	OBB sprite_obb(SpriteId sprite);

	/// Returns the distance along ray (from, dir) to intersection point with
	/// @a sprite or -1.0 if no intersection.
	f32 sprite_cast_ray(SpriteId sprite, const Vector3 &from, const Vector3 &dir, u32 &layer, u32 &depth);

	/// Creates a new light instance.
	LightId light_create(UnitId unit, const LightDesc &ld);

	/// Destroys the @a light.
	void light_destroy(LightId light);

	/// Returns the ID of the light owned by the *unit*.
	LightId light_instance(UnitId unit);

	/// Returns the type of the @a light.
	LightType::Enum light_type(LightId light);

	/// Returns the color of the @a light.
	Color4 light_color(LightId light);

	/// Returns the range of the @a light.
	f32 light_range(LightId light);

	/// Returns the intensity of the @a light.
	f32 light_intensity(LightId light);

	/// Returns the spot angle of the @a light.
	f32 light_spot_angle(LightId light);

	/// Returns the shadow bias of the @a light.
	f32 light_shadow_bias(LightId light);

	/// Sets the @a type of the @a light.
	void light_set_type(LightId light, LightType::Enum type);

	/// Sets the @a color of the @a light.
	void light_set_color(LightId light, const Color4 &color);

	/// Sets the @a range of the @a light.
	void light_set_range(LightId light, f32 range);

	/// Sets the @a intensity of the @a light.
	void light_set_intensity(LightId light, f32 intensity);

	/// Sets the spot @a angle of the @a light.
	void light_set_spot_angle(LightId light, f32 angle);

	/// Sets the shadow @a bias of the @a light.
	void light_set_shadow_bias(LightId light, f32 bias);

	/// Sets whether the @a light casts shadows.
	void light_set_cast_shadows(LightId light, bool cast_shadows);

	/// Fills @a dl with debug lines from the @a light.
	void light_debug_draw(LightId light, DebugLine &dl);

	///
	void fog_create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	/// Creates a new fog instance for the @a unit and returns its id.
	/// Note that the fog component is limited to one instance per
	/// World. Every call to this function will overwrite any
	/// previously created fog instance.
	FogId fog_create(UnitId unit, const FogDesc &desc);

	/// Destroys the @a fog instance.
	void fog_destroy(FogId fog);

	/// Returns the ID of the fog owned by the @a unit.
	FogId fog_instance(UnitId unit);

	/// Sets the @a color of the @a fog.
	void fog_set_color(FogId fog, Vector3 color);

	/// Sets the @a density of the @a fog.
	void fog_set_density(FogId fog, float density);

	/// Sets the minimum @a range of the @a fog.
	void fog_set_range_min(FogId fog, float range);

	/// Sets the maximum @a range of the @a fog.
	void fog_set_range_max(FogId fog, float range);

	/// Sets the sun @a blend of the @a fog.
	void fog_set_sun_blend(FogId fog, float sun_blend);

	/// Sets whether the @a fog is @a enabled.
	void fog_set_enabled(FogId fog, bool enable);

	///
	void global_lighting_create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	/// Creates a new global lighting instance for @a unit and returns its id.
	GlobalLightingId global_lighting_create(UnitId unit, const GlobalLightingDesc &desc);

	///
	void global_lighting_destroy(u32 global_lighting);

	///
	GlobalLightingId global_lighting_instance(UnitId unit);

	///
	void global_lighting_set_skydome_map(StringId64 texture_name);

	///
	void global_lighting_set_skydome_intensity(f32 intensity);

	///
	void global_lighting_set_ambient_color(Color4 color);

	///
	void bloom_create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	/// Creates a new bloom instance for @a unit and returns its id.
	BloomId bloom_create(UnitId unit, const BloomDesc &desc);

	///
	void bloom_destroy(u32 bloom);

	///
	BloomId bloom_instance(UnitId unit);

	///
	void bloom_set_enabled(bool enabled);

	///
	void bloom_set_weight(float mix);

	///
	void bloom_set_intensity(float intensity);

	///
	void bloom_set_threshold(float threshold);

	///
	void tonemap_create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	///
	TonemapId tonemap_create(UnitId unit, const TonemapDesc &desc);

	///
	void tonemap_destroy(u32 tonemap);

	///
	TonemapId tonemap_instance(UnitId unit);

	///
	void tonemap_set_type(TonemapType::Enum type);

	///
	void update_transforms(const UnitId *begin, const UnitId *end, const Matrix4x4 *world);

	///
	void render(const Matrix4x4 &view, const Matrix4x4 &proj, const Matrix4x4 &persp, UnitId skydome_unit, DebugLine &dl);

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
		void create_instances(const void *components_data
			, u32 num
			, const UnitId *unit_lookup
			, const u32 *unit_index
			);

		///
		void destroy(MeshId mesh);

		///
		bool has(UnitId unit);

		///
		void set_geometry(MeshId mesh, const MeshResource *mr, StringId32 geometry);

		///
		void set_visible(MeshId mesh, bool visible);

		///
		MeshId mesh(UnitId unit);

		///
		void destroy();

		///
		void swap(u32 inst_a, u32 inst_b);

		///
		void set_instance_data(u32 ii, SceneGraph &scene_graph);

		///
		void draw_shadow_casters(u8 view, SceneGraph &scene_graph, u32 stencil = BGFX_STENCIL_NONE);

		///
		void draw_visibles(u8 view, SceneGraph &scene_graph, const Matrix4x4 *cascaded_lights);

		///
		void draw_selected(u8 view, SceneGraph &scene_graph);

		///
		MeshId make_instance(u32 i)
		{
			MeshId inst = { i }; return inst;
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
			u32 *flags;
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
		void create_instances(const void *components_data
			, u32 num
			, const UnitId *unit_lookup
			, const u32 *unit_index
			);

		///
		void destroy(SpriteId sprite);

		///
		bool has(UnitId unit);

		///
		void set_visible(SpriteId sprite, bool visible);

		///
		SpriteId sprite(UnitId unit);

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
		SpriteId make_instance(u32 i)
		{
			SpriteId inst = { i }; return inst;
		}
	};

	struct LightManager
	{
		// This data is fed to the shader as-is.
		// Keep it in sync with "core/shaders/lighting.shader"!
		struct ShaderData
		{                      // Vec4 offset
			Vector3 color;     // 0
			f32 intensity;
			Vector3 position;  // 1
			f32 range;
			Vector3 direction; // 2
			f32 spot_angle;
			Matrix4x4 mvp[4];  // 3-18 Model-View-Proj-Crop.
			Vector4 atlas_u;   // 19   U-coord in shadow map atlas.
			Vector4 atlas_v;   // 20   V-coord in shadow map atlas.
			f32 map_size;      // 21   Tile size in shadow map atlas.
			f32 shadow_bias;
			f32 cast_shadows;
			f32 _pad;
		};

		struct LightInstanceData
		{
			u32 size;
			u32 capacity;
			void *buffer;

			UnitId *unit;
			u32 *flag;            // RenderableFlags::Enum
			u32 *type;            // LightType::Enum
			ShaderData *shader;
		};

		Allocator *_allocator;
		RenderWorld *_render_world;
		HashMap<UnitId, u32> _map;
		LightInstanceData _data;
		Array<ShaderData> _lights_data; // Shader array to send to GPU.
		Array<u32> _directional_lights; // Indices to directional lights sorted by intensity.
		Array<u32> _local_lights;       // Indices to local lights sorted by distance to camera.
		Array<u32> _local_lights_omni;  // Indices to spot lights that will be rendered this frame.
		Array<u32> _local_lights_spot;  // Indices to omni lights that will be rendered this frame.

		///
		explicit LightManager(Allocator &a, RenderWorld *rw)
			: _allocator(&a)
			, _render_world(rw)
			, _map(a)
			, _lights_data(a)
			, _directional_lights(a)
			, _local_lights(a)
			, _local_lights_omni(a)
			, _local_lights_spot(a)
		{
			memset(&_data, 0, sizeof(_data));
		}

		///
		void create_instances(const void *components_data
			, u32 num
			, const UnitId *unit_lookup
			, const u32 *unit_index
			);

		///
		void destroy(LightId light);

		///
		bool has(UnitId unit);

		///
		LightId light(UnitId unit);

		///
		void debug_draw(u32 start_index, u32 num, DebugLine &dl);

		///
		void allocate(u32 num);

		///
		void grow();

		///
		void destroy();

		///
		LightId make_instance(u32 i)
		{
			LightId inst = { i }; return inst;
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

	// Fog.
	UnitId _fog_unit;
	FogId _fog;
	FogDesc _fog_desc;

	// Global lighting.
	UnitId _global_lighting_unit;
	GlobalLightingDesc _global_lighting_desc;

	// Bloom.
	UnitId _bloom_unit;
	BloomDesc _bloom_desc;

	// Tonemap.
	UnitId _tonemap_unit;
	TonemapDesc _tonemap_desc;
};

} // namespace crown
