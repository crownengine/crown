/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "device/render_pipeline.h"
#include "core/strings/string_id.inl"
#include "resource/material_resource.h"
#include "resource/render_config_resource.h"
#include "resource/shader_resource.h"
#include "world/types.h"
#include <bgfx/bgfx.h>

struct stbrp_context;
struct stbrp_node;

#define LIGHT_SIZE 24     // Size of a light in vec4 units.
#define MAX_NUM_LIGHTS 32 // Maximum number of lights per frame.
#define MAX_NUM_SPRITE_LAYERS 8
#define MAX_NUM_CASCADES 4
#define LIGHTS_DATA_SLOT 12
#define BONES_DATA_SLOT 14
#define CASCADED_SHADOW_MAP_SLOT MATERIAL_MAX_TEXTURE_SLOTS
#define LOCAL_LIGHTS_SHADOW_MAP_SLOT 11
#define LOCAL_LIGHTS_MAX_SHADOW_CASTERS 16 // Maximum number of local shadow-casting lights per frame.
CE_STATIC_ASSERT(LOCAL_LIGHTS_MAX_SHADOW_CASTERS <= MAX_NUM_LIGHTS);
#define LOCAL_LIGHTS_SM_MAX_VIEWS (LOCAL_LIGHTS_MAX_SHADOW_CASTERS * 4) // Worst case all omni casters.
#define LOCAL_LIGHTS_COOKIE_ATLAS_SLOT 13

namespace crown
{
struct RenderFrame;
/// Render pipeline.
///
/// @ingroup Device
struct Pipeline
{
	ShaderManager *_shader_manager;
	RenderSettings _render_settings;
	RenderSettings _requested_render_settings; // Unscaled size sources across resizes.
	RenderPipeline _render_pipeline;
	const RenderConfigResource *_render_config_resource;

	// Non-owning aliases used by Crown's native scene producers.
	// RenderPipeline alone owns and destroys textures/framebuffers.
	// Main output color/depth handles.
	bgfx::FrameBufferHandle _color_sdr;
	bgfx::TextureHandle _color_textures[2];
	bgfx::TextureHandle _depth_texture;
	bgfx::FrameBufferHandle _colors[2];
	bgfx::UniformHandle _color_map;
	bgfx::UniformHandle _depth_map;

	// Selection/outline handles.
	bgfx::TextureHandle _selection_texture;
	bgfx::TextureHandle _selection_depth_texture;
	bgfx::FrameBufferHandle _selection_frame_buffer;
	bgfx::UniformHandle _selection_map;
	bgfx::UniformHandle _selection_depth_map;
	bgfx::TextureHandle _outline_color_texture;
	bgfx::FrameBufferHandle _outline_frame_buffer;
	bgfx::UniformHandle _outline_color_map;
	bgfx::UniformHandle _outline_color;
	bgfx::UniformHandle _unit_id;
	bgfx::UniformHandle _outline_msaa_samples;

	// Cascaded shadow mapping.
	bgfx::TextureHandle _sun_shadow_map_texture;
	bgfx::FrameBufferHandle _sun_shadow_map_frame_buffer;
	bgfx::UniformHandle _u_cascaded_shadow_map;
	bgfx::UniformHandle _u_cascaded_lights;
	bgfx::UniformHandle _u_cascade_shadow_texel_size;
	bgfx::UniformHandle _u_shadow_map_params;
	bgfx::TextureHandle _local_lights_shadow_map_texture;
	bgfx::FrameBufferHandle _local_lights_shadow_map_frame_buffer;
	bgfx::UniformHandle _u_local_lights_shadow_map;
	bgfx::UniformHandle _u_local_lights_params;

	// Lighting.
	bgfx::UniformHandle _lights_num;
	bgfx::UniformHandle _lights_data;
	bgfx::TextureHandle _lights_data_texture;
	bgfx::UniformHandle _fog_data;
	bgfx::UniformHandle _lighting_params;

	// Skinning. The atlas layout matches common.shader from upstream.
	bgfx::UniformHandle _bones_data_sampler;
	bgfx::UniformHandle _bones_data_size;
	bgfx::TextureHandle _bones_texture;
	u32 _bones_texture_height;
	u32 _bones_row;
	Array<Matrix4x4> _bones_data[2]; // makeRef storage survives two bgfx::frame() calls.
	Array<Matrix4x4> *_bones_cpu;
	Array<Matrix4x4> *_bones_gpu;

	// Light cookies.
	bgfx::UniformHandle _u_lights_cookie_atlas;
	bgfx::TextureHandle _lights_cookie_atlas_texture;
	bgfx::FrameBufferHandle _lights_cookie_atlas_frame_buffer;
	stbrp_context *_lights_cookie_atlas_packer; // Re-initialized every frame.
	stbrp_node *_lights_cookie_atlas_packer_nodes; // Sized once for fixed atlas width.

	// Bloom.
	bgfx::UniformHandle _bloom_map;
	bgfx::UniformHandle _map_pixel_size;
	bgfx::UniformHandle _bloom_params;
	BloomDesc _bloom;

	// Color grading and tonemap.
	bgfx::UniformHandle _color_grading_desc_uniform;
	ColorGradingDesc _color_grading_desc;
	bgfx::UniformHandle _tonemap_type;
	TonemapDesc _tonemap;

	// Vignette
	bgfx::UniformHandle _vignette_desc_uniform;
	VignetteDesc _vignette;

	// Default shaders.
	ShaderData _blit_shader = {};
	ShaderData _blit_blend_shader = {};
	ShaderData _gui_shader = {};
	ShaderData _gui_3d_shader = {};
	ShaderData _debug_line_depth_enabled_shader = {};
	ShaderData _debug_line_shader = {};
	ShaderData _outline_shader = {};
	ShaderData _outline_msaa_shader = {};
	ShaderData _selection_shader = {};
	ShaderData _selection_skinning_shader = {};
	ShaderData _shadow_shader = {};
	ShaderData _shadow_skinning_shader = {};
	ShaderData _skydome_shader = {};
	ShaderData _bloom_downsample_shader = {};
	ShaderData _bloom_upsample_shader = {};
	ShaderData _bloom_combine_shader = {};
	ShaderData _tonemap_shader = {};
	ShaderData _vignette_shader = {};
	ShaderData _bloom_copy_shader = {};

	///
	Pipeline(ShaderManager &sm);

	/// Resolve logical layer names at submission time (safe across hot reload).
	u16 view_id(StringId32 layer, u32 index = 0) const { return _render_pipeline.geometry_view(layer, index); }
	u16 shader_view(StringId32 shader, u16 fallback) const { return _render_pipeline.shader_view(shader, fallback); }
	u16 debug_view() const { return _render_pipeline.external_view(StringId32("debug")); }
	u16 screen_gui_view() const { return _render_pipeline.external_view(StringId32("screen_gui")); }
	u16 world_gui_view() const { return _render_pipeline.external_view(StringId32("world_gui")); }
	u16 graph_view() const { return _render_pipeline.external_view(StringId32("graph")); }
	u16 imgui_view() const { return _render_pipeline.external_view(StringId32("imgui")); }

	///
	bool selection_enabled() const;
	bool sun_shadows_enabled() const;
	bool local_shadows_enabled() const;
	bool light_cookies_enabled() const;

	/// Bind only the native lighting samplers actually used by this shader.
	/// A missing input suppresses the batch instead of binding an invalid handle.
	bool bind_lighting(const ShaderData &shader) const;

	///
	void create(u16 width, u16 height, const RenderSettings &render_settings, const RenderConfigResource *resource);

	///
	void destroy();

	///
	void reset(u16 width, u16 height);

	///
	void render(u16 width, u16 height, const Matrix4x4 &view, const Matrix4x4 &proj, RenderFrame *frame = NULL);

	/// Application-frame lifetime, shared by all prepared world/camera draws.
	void begin_frame();
	void end_frame();

	/// Adds a palette to the shared bone atlas.
	void add_bones_data(u32 &row, const Matrix4x4 *bones, u32 num_bones);

	/// Binds a previously added palette for immediate native draws.
	void bind_bones_data(u32 row);

	/// Update execution conditions before any native scene submissions.
	void update_conditions();

	/// Refresh native source dimensions/aliases after changing execution conditions.
	void update_source_resources();

	///
	void prepare_local_lights_stencil(RenderFrame &frame, u16 tile_size, u16 tile_cols);

	///
	void begin_light_cookie_atlas();

	/// Packs @a texture into the atlas, prepares a batch and returns its normalized
	/// texel-center bounds. Returns zero if it does not fit.
	Vector4 add_light_cookie(RenderFrame &frame, u32 &index, bgfx::TextureHandle texture, u16 width, u16 height);

	///
	void reload_shaders(const ShaderResource *old_resource, const ShaderResource *new_resource);

	///
	void set_local_lights_params_uniform();

	///
	void set_global_lighting_params(GlobalLightingDesc *global_lighting);
};

} // namespace crown
