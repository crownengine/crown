/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/memory/allocator.h"
#include "core/memory/globals.h"
#include "core/strings/string_id.inl"
#include "core/error/error.h"
#include "core/types.h"
#include "device/pipeline.h"
#include "world/shader_manager.h"
#include "resource/render_config_resource.inl"
#include <cmath>
#include "core/math/matrix4x4.inl"
#include <bx/math.h>
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

// Device/configuration validation must also run in release builds.
#define PIPELINE_ENSURE(condition, ...) \
	do { if (!(condition)) crown::error::abort(__VA_ARGS__); } while (0)

namespace crown
{
struct PosVertex
{
	float x;
	float y;
	float z;

	static void init()
	{
		pos_layout.begin();
		pos_layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
		pos_layout.end();
	}

	static bgfx::VertexLayout pos_layout;
};

bgfx::VertexLayout PosVertex::pos_layout;

static ShaderData native_shader(Pipeline &pipeline, const char *name, bool needed)
{
	ShaderData shader = {};
	shader.program = BGFX_INVALID_HANDLE;
	return needed ? pipeline._shader_manager->shader(StringId32(name)) : shader;
}

static bool has_geometry_layer(const Pipeline &pipeline, const char *name)
{
	const RenderPipeline &runtime = pipeline._render_pipeline;
	const u32 index = runtime.layer_index(StringId32(name));
	if (index == RENDER_CONFIG_INVALID)
		return false;
	const RenderLayerData &layer = render_config_resource::layers(runtime._resource)[index];
	return layer.generator == RENDER_CONFIG_INVALID && layer.count != 0;
}

static void lookup_default_shaders(Pipeline &pl)
{
	// Look up declared roles, not this frame's enabled roles: execution
	// conditions can turn a producer on later without recreating the pipeline.
	const bool debug = has_geometry_layer(pl, "debug") || has_geometry_layer(pl, "graph");
	const bool shadows = has_geometry_layer(pl, "sm_cascade") || has_geometry_layer(pl, "sm_local");
	const bool selection = has_geometry_layer(pl, "selection");
	pl._blit_shader = native_shader(pl, "blit", has_geometry_layer(pl, "lights_cookie_atlas"));
	pl._gui_shader = native_shader(pl, "gui", has_geometry_layer(pl, "screen_gui"));
	pl._gui_3d_shader = native_shader(pl, "gui+DEPTH_ENABLED", has_geometry_layer(pl, "world_gui"));
	pl._debug_line_depth_enabled_shader = native_shader(pl, "debug_line+DEPTH_ENABLED", debug);
	pl._debug_line_shader = native_shader(pl, "debug_line", debug);
	pl._selection_shader = native_shader(pl, "selection", selection);
	pl._selection_skinning_shader = native_shader(pl, "selection+SKINNING", selection);
	pl._shadow_shader = native_shader(pl, "shadow", shadows);
	pl._shadow_skinning_shader = native_shader(pl, "shadow+SKINNING", shadows);
}

Pipeline::Pipeline(ShaderManager &sm)
	: _shader_manager(&sm)
	, _render_settings()
	, _requested_render_settings()
	, _render_pipeline(default_allocator(), sm)
	, _render_config_resource(NULL)
	, _color_sdr(BGFX_INVALID_HANDLE)
	, _depth_texture(BGFX_INVALID_HANDLE)
	, _color_map(BGFX_INVALID_HANDLE)
	, _depth_map(BGFX_INVALID_HANDLE)
	, _selection_texture(BGFX_INVALID_HANDLE)
	, _selection_depth_texture(BGFX_INVALID_HANDLE)
	, _selection_frame_buffer(BGFX_INVALID_HANDLE)
	, _selection_map(BGFX_INVALID_HANDLE)
	, _selection_depth_map(BGFX_INVALID_HANDLE)
	, _outline_color_texture(BGFX_INVALID_HANDLE)
	, _outline_frame_buffer(BGFX_INVALID_HANDLE)
	, _outline_color_map(BGFX_INVALID_HANDLE)
	, _outline_color(BGFX_INVALID_HANDLE)
	, _unit_id(BGFX_INVALID_HANDLE)
	, _outline_msaa_samples(BGFX_INVALID_HANDLE)
	, _sun_shadow_map_texture(BGFX_INVALID_HANDLE)
	, _sun_shadow_map_frame_buffer(BGFX_INVALID_HANDLE)
	, _local_lights_shadow_map_texture(BGFX_INVALID_HANDLE)
	, _local_lights_shadow_map_frame_buffer(BGFX_INVALID_HANDLE)
	, _u_lights_cookie_atlas(BGFX_INVALID_HANDLE)
	, _lights_cookie_atlas_texture(BGFX_INVALID_HANDLE)
	, _lights_cookie_atlas_frame_buffer(BGFX_INVALID_HANDLE)
	, _lights_cookie_atlas_packer(NULL)
	, _lights_cookie_atlas_packer_nodes(NULL)
	, _bloom_map(BGFX_INVALID_HANDLE)
	, _map_pixel_size(BGFX_INVALID_HANDLE)
	, _bloom_params(BGFX_INVALID_HANDLE)
	, _bloom()
	, _color_grading_desc_uniform(BGFX_INVALID_HANDLE)
	, _color_grading_desc()
	, _tonemap_type(BGFX_INVALID_HANDLE)
	, _tonemap()
	, _vignette_desc_uniform(BGFX_INVALID_HANDLE)
	, _vignette()
{
	for (u32 i = 0; i < countof(_color_textures); ++i)
		_color_textures[i] = BGFX_INVALID_HANDLE;

	for (u32 i = 0; i < countof(_colors); ++i)
		_colors[i] = BGFX_INVALID_HANDLE;
}

bool Pipeline::selection_enabled() const
{
	return (CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS)
		&& (_render_settings.flags & RenderSettingsFlags::SELECTION) != 0
		&& selection_view() != UINT16_MAX
		;
}

bool Pipeline::sun_shadows_enabled() const
{
	return (_render_settings.flags & RenderSettingsFlags::SUN_SHADOWS) != 0
		&& bgfx::isValid(_sun_shadow_map_texture)
		&& cascade_view() != UINT16_MAX;
}

bool Pipeline::local_shadows_enabled() const
{
	return (_render_settings.flags & RenderSettingsFlags::LOCAL_LIGHTS_SHADOWS) != 0
		&& bgfx::isValid(_local_lights_shadow_map_texture)
		&& sm_local_view() != UINT16_MAX;
}

bool Pipeline::light_cookies_enabled() const
{
	return (_render_settings.flags & RenderSettingsFlags::LIGHTS_COOKIE) != 0
		&& _lights_cookie_atlas_packer != NULL
		&& cookie_atlas_view() != UINT16_MAX;
}

bool Pipeline::bind_lighting(const ShaderData &shader) const
{
	static const StringId32 names[] = {
		StringId32("u_cascaded_shadow_map"), StringId32("u_local_lights_shadow_map"),
		StringId32("u_lights_data"), StringId32("u_lights_cookie_atlas")
	};
	const bgfx::TextureHandle textures[] = {
		_sun_shadow_map_texture, _local_lights_shadow_map_texture,
		_lights_data_texture, _lights_cookie_atlas_texture
	};
	const bgfx::UniformHandle samplers[] = {
		_u_cascaded_shadow_map, _u_local_lights_shadow_map,
		_lights_data, _u_lights_cookie_atlas
	};
	CE_STATIC_ASSERT(CASCADED_SHADOW_MAP_SLOT + 1 == LOCAL_LIGHTS_SHADOW_MAP_SLOT);
	CE_STATIC_ASSERT(CASCADED_SHADOW_MAP_SLOT + 2 == LIGHTS_DATA_SLOT);
	CE_STATIC_ASSERT(CASCADED_SHADOW_MAP_SLOT + 3 == LOCAL_LIGHTS_COOKIE_ATLAS_SLOT);
	// The authored sampler-state table usually contains only material textures.
	// Native lighting inputs must be discovered from the compiled shader instead.
	for (u32 i = 0; i < shader.num_sampler_uniforms; ++i) {
		u32 index = 0;
		for (; index < countof(names); ++index) {
			if (shader.sampler_uniforms[i] == names[index]._id)
				break;
		}
		if (index == countof(names))
			continue;
		const u32 stage = CASCADED_SHADOW_MAP_SLOT + index;
		if (!bgfx::isValid(textures[index])) {
			bgfx::discard();
			return false;
		}
		const u32 flags = stage == LOCAL_LIGHTS_COOKIE_ATLAS_SLOT
			? BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
			: UINT32_MAX;
		bgfx::setTexture((u8)stage, samplers[index], textures[index], flags);
	}
	return true;
}

namespace
{
	// These callbacks are Crown's current rendering techniques, not part of
	// RenderPipeline. They do not allocate resources or choose destinations.
	void bloom_draw(RenderPipeline &runtime, const RenderModifierData &m, u16 view, Pipeline &p, bool combine)
	{
		PIPELINE_ENSURE(m.resource < runtime._resource->num_resources, "Bloom needs a resource-array context");
		const u32 levels = render_config_resource::resources(runtime._resource)[m.resource].count;
		f32 gain = 1.0f;
		for (u32 i = 1; i < levels; ++i) gain = 1.0f + bx::abs(p._bloom.intensity) * gain;
		Vector4 parameters = { combine ? 2.0f * gain : 0.5f / gain, p._bloom.threshold, p._bloom.weight, p._bloom.intensity };
		bgfx::setUniform(p._bloom_params, &parameters);
		runtime.draw_fullscreen(m, view);
	}

	void bloom_modifier(RenderPipeline &r, const RenderModifierData &m, u16 view, void *user)
	{
		bloom_draw(r, m, view, *(Pipeline *)user, false);
	}

	void bloom_combine_modifier(RenderPipeline &r, const RenderModifierData &m, u16 view, void *user)
	{
		bloom_draw(r, m, view, *(Pipeline *)user, true);
	}

	void vignette_modifier(RenderPipeline &r, const RenderModifierData &m, u16 view, void *user)
	{
		Pipeline &p = *(Pipeline *)user;
		bgfx::setUniform(p._vignette_desc_uniform, &p._vignette, sizeof(p._vignette)/sizeof(Vector4));
		r.draw_fullscreen(m, view);
	}

	void tonemap_modifier(RenderPipeline &r, const RenderModifierData &m, u16 view, void *user)
	{
		Pipeline &p = *(Pipeline *)user;
		bgfx::setUniform(p._color_grading_desc_uniform, &p._color_grading_desc, sizeof(p._color_grading_desc)/sizeof(Vector4));
		bgfx::setUniform(p._tonemap_type, &p._tonemap, sizeof(p._tonemap)/sizeof(Vector4));
		r.draw_fullscreen(m, view);
	}

	void outline_modifier(RenderPipeline &r, const RenderModifierData &m, u16 view, void *user)
	{
		Pipeline &p = *(Pipeline *)user;
		const Vector4 samples = { f32(1u << p._render_settings.msaa_quality), 0.0f, 0.0f, 0.0f };
		bgfx::setUniform(p._outline_msaa_samples, &samples);
		r.draw_fullscreen(m, view);
	}

	const RenderModifierType modifiers[] = {
		{ StringId32("bloom"), bloom_modifier },
		{ StringId32("bloom_combine"), bloom_combine_modifier },
		{ StringId32("vignette"), vignette_modifier },
		{ StringId32("tonemap"), tonemap_modifier },
		{ StringId32("outline"), outline_modifier }
	};

	RenderResourceSize external_size(const char *name, Vector2 size)
	{
		PIPELINE_ENSURE(std::isfinite(size.x) && std::isfinite(size.y) && size.x >= 1 && size.y >= 1 && size.x <= UINT16_MAX && size.y <= UINT16_MAX
			, "Invalid render setting size '%s'", name);
		return { StringId32(name), (u16)size.x, (u16)size.y };
	}
}

void Pipeline::create(u16 width, u16 height, const RenderSettings &settings, const RenderConfigResource *resource)
{
	PIPELINE_ENSURE(!(settings.flags & RenderSettingsFlags::MSAA) || (settings.msaa_quality >= 1 && settings.msaa_quality <= 4), "MSAA quality must be in the 1..4 range");
	_render_settings = settings;
	_requested_render_settings = settings;
	_render_config_resource = resource;
	if ((_render_settings.flags & RenderSettingsFlags::LIGHTS_COOKIE)
		&& !bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT))
		_render_settings.flags &= ~RenderSettingsFlags::LIGHTS_COOKIE;

	_color_map = bgfx::createUniform("s_color_map", bgfx::UniformType::Sampler);
	{ // Callback uniforms also exist when the selection geometry layer is absent.
		_depth_map = bgfx::createUniform("s_depth_map", bgfx::UniformType::Sampler);
		_selection_map = bgfx::createUniform("s_selection_map", bgfx::UniformType::Sampler);
		_selection_depth_map = bgfx::createUniform("s_selection_depth_map", bgfx::UniformType::Sampler);
		_outline_color_map = bgfx::createUniform("s_color_map", bgfx::UniformType::Sampler);
		_outline_color = bgfx::createUniform("u_outline_color", bgfx::UniformType::Vec4);
		_unit_id = bgfx::createUniform("u_unit_id", bgfx::UniformType::Vec4);
		_outline_msaa_samples = bgfx::createUniform("u_outline_msaa_samples", bgfx::UniformType::Vec4);
	}
	_u_cascaded_shadow_map = bgfx::createUniform("u_cascaded_shadow_map", bgfx::UniformType::Sampler);
	_u_cascaded_lights = bgfx::createUniform("u_cascaded_lights", bgfx::UniformType::Mat4, MAX_NUM_CASCADES);
	_u_cascade_shadow_texel_size = bgfx::createUniform("u_cascade_shadow_texel_size", bgfx::UniformType::Vec4);
	_u_shadow_map_params = bgfx::createUniform("u_shadow_map_params", bgfx::UniformType::Vec4, 2);
	_u_local_lights_shadow_map = bgfx::createUniform("u_local_lights_shadow_map", bgfx::UniformType::Sampler);
	_u_local_lights_params = bgfx::createUniform("u_local_lights_params", bgfx::UniformType::Vec4);
	_lights_num = bgfx::createUniform("u_lights_num", bgfx::UniformType::Vec4);
	_lights_data = bgfx::createUniform("u_lights_data", bgfx::UniformType::Sampler);
	_fog_data = bgfx::createUniform("u_fog_data", bgfx::UniformType::Vec4, 3);
	_lighting_params = bgfx::createUniform("u_lighting_params", bgfx::UniformType::Vec4);
	_u_lights_cookie_atlas = bgfx::createUniform("u_lights_cookie_atlas", bgfx::UniformType::Sampler);
	_bloom_map = bgfx::createUniform("s_bloom_map", bgfx::UniformType::Sampler);
	_map_pixel_size = bgfx::createUniform("u_map_pixel_size", bgfx::UniformType::Vec4);
	_bloom_params = bgfx::createUniform("u_bloom_params", bgfx::UniformType::Vec4);
	_color_grading_desc_uniform = bgfx::createUniform("u_color_grading_desc", bgfx::UniformType::Vec4, 2);
	_tonemap_type = bgfx::createUniform("u_tonemap_type", bgfx::UniformType::Vec4);
	_vignette_desc_uniform = bgfx::createUniform("u_vignette_desc", bgfx::UniformType::Vec4, 2);
	PosVertex::init();
	reset(width, height);
}

void Pipeline::destroy()
{
	_render_pipeline.destroy();
	// Aliases are never destroyed here: framebuffer attachments may be shared.
	_color_sdr = _selection_frame_buffer = _outline_frame_buffer = BGFX_INVALID_HANDLE;
	_sun_shadow_map_frame_buffer = _local_lights_shadow_map_frame_buffer = _lights_cookie_atlas_frame_buffer = BGFX_INVALID_HANDLE;
	for (u32 i = 0; i < countof(_colors); ++i) _colors[i] = BGFX_INVALID_HANDLE;
	for (u32 i = 0; i < countof(_color_textures); ++i) _color_textures[i] = BGFX_INVALID_HANDLE;
	_depth_texture = _selection_texture = _selection_depth_texture = _outline_color_texture = BGFX_INVALID_HANDLE;
	_sun_shadow_map_texture = _local_lights_shadow_map_texture = _lights_data_texture = _lights_cookie_atlas_texture = BGFX_INVALID_HANDLE;

	bgfx::UniformHandle *uniforms[] = {
		&_color_map, &_depth_map, &_selection_map, &_selection_depth_map, &_outline_color_map,
		&_outline_color, &_unit_id, &_outline_msaa_samples, &_u_cascaded_shadow_map,
		&_u_cascaded_lights, &_u_cascade_shadow_texel_size, &_u_shadow_map_params,
		&_u_local_lights_shadow_map, &_u_local_lights_params, &_lights_num, &_lights_data,
		&_fog_data, &_lighting_params, &_u_lights_cookie_atlas, &_bloom_map, &_map_pixel_size,
		&_bloom_params, &_color_grading_desc_uniform, &_tonemap_type, &_vignette_desc_uniform
	};
	for (u32 i = 0; i < countof(uniforms); ++i) {
		if (bgfx::isValid(*uniforms[i])) bgfx::destroy(*uniforms[i]);
		*uniforms[i] = BGFX_INVALID_HANDLE;
	}
	if (_lights_cookie_atlas_packer) {
		default_allocator().deallocate(_lights_cookie_atlas_packer);
		default_allocator().deallocate(_lights_cookie_atlas_packer_nodes);
		_lights_cookie_atlas_packer = NULL;
		_lights_cookie_atlas_packer_nodes = NULL;
	}
}

void Pipeline::reset(u16 width, u16 height)
{
	_render_pipeline.destroy();
	u32 conditions = 0;
	const RenderConfigResource *r = _render_config_resource;
	if ((CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS) && (_render_settings.flags & RenderSettingsFlags::SELECTION))
		conditions |= render_config_resource::condition_mask(r, StringId32("selection"));
	if (_render_settings.flags & RenderSettingsFlags::LIGHTS_COOKIE) conditions |= render_config_resource::condition_mask(r, StringId32("cookies"));
	if (_render_settings.flags & RenderSettingsFlags::BLOOM) conditions |= render_config_resource::condition_mask(r, StringId32("bloom_allocated"));
	if (_render_settings.flags & RenderSettingsFlags::MSAA) conditions |= render_config_resource::condition_mask(r, StringId32("msaa"));
	const RenderResourceSize sizes[] = {
		external_size("sun_shadow_map_size", _requested_render_settings.sun_shadow_map_size),
		external_size("local_lights_shadow_map_size", _requested_render_settings.local_lights_shadow_map_size),
		external_size("lights_cookie_atlas_size", _requested_render_settings.lights_cookie_atlas_size)
	};
	const u32 quality = (_render_settings.flags & RenderSettingsFlags::MSAA) ? _render_settings.msaa_quality : 0;
	_render_pipeline.create(r, width, height, quality, conditions, sizes, countof(sizes), modifiers, countof(modifiers), this);

	// Native destinations are optional. Their absence suppresses their producers;
	// the executor never reconstructs any part of the default layer list.

	_color_textures[0] = _render_pipeline.texture(StringId32("color0"));
	_color_textures[1] = _render_pipeline.texture(StringId32("color1"));
	_depth_texture = _render_pipeline.texture(StringId32("depth"));
	_colors[0] = _render_pipeline.frame_buffer(StringId32("color0_clear"));
	_colors[1] = _render_pipeline.frame_buffer(StringId32("color1_clear"));
	_color_sdr = _render_pipeline.frame_buffer(StringId32("sprite"));
	_selection_texture = _render_pipeline.texture(StringId32("selection_color"));
	_selection_depth_texture = _render_pipeline.texture(StringId32("selection_depth"));
	_selection_frame_buffer = _render_pipeline.frame_buffer(StringId32("selection"));
	_outline_color_texture = _render_pipeline.texture(StringId32("outline_color"));
	_outline_frame_buffer = _render_pipeline.frame_buffer(StringId32("outline"));
	_sun_shadow_map_texture = _render_pipeline.texture(StringId32("sun_shadow_map"));
	_sun_shadow_map_frame_buffer = _render_pipeline.frame_buffer(StringId32("sm_cascade"));
	_local_lights_shadow_map_texture = _render_pipeline.texture(StringId32("local_lights_shadow_map"));
	_local_lights_shadow_map_frame_buffer = _render_pipeline.frame_buffer(StringId32("sm_local"));
	_lights_data_texture = _render_pipeline.texture(StringId32("lights_data"));
	if (bgfx::isValid(_lights_data_texture) && has_geometry_layer(*this, "lights")) {
		const u32 index = _render_pipeline.resource_index(StringId32("lights_data"));
		const RenderTextureState &lights = _render_pipeline.texture_state({ index, 0 });
		PIPELINE_ENSURE(lights.width == MAX_NUM_LIGHTS * LIGHT_SIZE && lights.height == 1
			&& render_config_resource::resources(r)[index].format == RenderResourceFormat::RGBA32F
			, "lights_data must match the native packed-light layout");
	}

	const char *shadow_names[] = { "sun_shadow_map", "local_lights_shadow_map" };
	Vector2 *shadow_sizes[] = { &_render_settings.sun_shadow_map_size, &_render_settings.local_lights_shadow_map_size };
	const bool shadow_producers[] = { has_geometry_layer(*this, "sm_cascade"), has_geometry_layer(*this, "sm_local") };
	for (u32 i = 0; i < countof(shadow_names); ++i) {
		const u32 index = _render_pipeline.resource_index(StringId32(shadow_names[i]));
		if (index == RENDER_CONFIG_INVALID)
			continue;
		const RenderTextureState &texture = _render_pipeline.texture_state({ index, 0 });
		if (shadow_producers[i])
			PIPELINE_ENSURE(texture.width == texture.height && texture.width >= 2, "Native shadow atlases must be square and at least 2x2");
		*shadow_sizes[i] = { f32(texture.width), f32(texture.height) };
	}
	const Vector2 sun = _render_settings.sun_shadow_map_size;
	const Vector2 local = _render_settings.local_lights_shadow_map_size;
	_render_settings.shadow_map_params[0] = { 1.0f/sun.x, 1.0f/sun.y, 1.0f/local.x, 1.0f/local.y };

	if (_lights_cookie_atlas_packer) {
		default_allocator().deallocate(_lights_cookie_atlas_packer);
		default_allocator().deallocate(_lights_cookie_atlas_packer_nodes);
		_lights_cookie_atlas_packer = NULL;
		_lights_cookie_atlas_packer_nodes = NULL;
	}
	_lights_cookie_atlas_frame_buffer = _render_pipeline.frame_buffer(StringId32("lights_cookie_atlas"));
	_lights_cookie_atlas_texture = _render_pipeline.texture(StringId32("lights_cookie_atlas"));
	if ((_render_settings.flags & RenderSettingsFlags::LIGHTS_COOKIE)
		&& bgfx::isValid(_lights_cookie_atlas_texture) && has_geometry_layer(*this, "lights_cookie_atlas")) {
		const u32 index = _render_pipeline.resource_index(StringId32("lights_cookie_atlas"));
		const RenderTextureState &atlas = _render_pipeline.texture_state({ index, 0 });
		PIPELINE_ENSURE(bgfx::isValid(atlas.handle) && render_config_resource::resources(r)[index].format == RenderResourceFormat::RGBA8, "The native cookie atlas requires allocated RGBA8 storage");
		_lights_cookie_atlas_texture = atlas.handle;
		_render_settings.lights_cookie_atlas_size = { f32(atlas.width), f32(atlas.height) };
		_lights_cookie_atlas_packer = (stbrp_context *)default_allocator().allocate(sizeof(stbrp_context));
		_lights_cookie_atlas_packer_nodes = (stbrp_node *)default_allocator().allocate(sizeof(stbrp_node) * atlas.width);
	} else if (!bgfx::isValid(_lights_cookie_atlas_texture)) {
		_lights_cookie_atlas_texture = _render_pipeline.texture(StringId32("lights_cookie_fallback"));
	}
	lookup_default_shaders(*this);
}

void Pipeline::draw_local_lights_stencil(u16 tile_size, u16 tile_cols)
{
	if (sm_local_clear_view() == UINT16_MAX || !local_shadows_enabled() || tile_size == 0 || tile_cols == 0)
		return;
	CE_ENSURE(tile_size > 0);
	CE_ENSURE(tile_cols > 0);

	// Draw stencil "hourglass" pattern for omni lights.
	const u16 sm_w = (u16)_render_settings.local_lights_shadow_map_size.x;
	const f32 step = f32(tile_size) / f32(sm_w) * 0.5f;
	const s32 num_cols = tile_cols;
	const s32 num_rows = num_cols;
	const s32 num_pins = num_cols + 1;
	const s32 num_necks = num_pins - 1;
	const u32 num_vertices = num_pins*num_pins + num_necks*num_necks;
	const u32 num_triangles = num_necks*num_necks * 2;
	const u32 num_indices = num_triangles * 3;

	if (bgfx::getAvailTransientVertexBuffer(num_vertices, PosVertex::pos_layout) == num_vertices
		&& bgfx::getAvailTransientIndexBuffer(num_indices) == num_indices) {
		// Build vertex buffer.
		bgfx::TransientVertexBuffer vb;
		bgfx::allocTransientVertexBuffer(&vb, num_vertices, PosVertex::pos_layout);
		PosVertex *v = (PosVertex *)vb.data;

		for (s32 h = 0; h < num_pins + num_necks; ++h) {
			s32 start_w = h % 2;
			for (s32 w = start_w; w < num_pins + num_necks; w += 2) {
				const f32 xi = w * step;
				const f32 yi = h * step;
				*v++ = { xi, yi, 0.0f };
			}
		}

		// Build index buffer.
		bgfx::TransientIndexBuffer ib;
		bgfx::allocTransientIndexBuffer(&ib, num_indices);
		u16 *ind = (u16 *)ib.data;

		const s32 gap = num_cols + 1;
		const s32 row_stride = 2 * gap - 1;

		for (s32 r = 0; r < num_rows; ++r) {
			for (s32 c = 0; c < num_cols; ++c) {
				const s32 t = r * row_stride + c;
				// Top triangle.
				*ind++ = t;
				*ind++ = t + 1;
				*ind++ = t + gap;
				// Bottom triangle.
				*ind++ = t + gap;
				*ind++ = t + 2 * gap;
				*ind++ = t + 2 * gap - 1;
			}
		}

		bgfx::setState(0);
		bgfx::setStencil(BGFX_STENCIL_TEST_ALWAYS
			| BGFX_STENCIL_FUNC_REF(1)
			| BGFX_STENCIL_FUNC_RMASK(0xff)
			| BGFX_STENCIL_OP_FAIL_S_REPLACE
			| BGFX_STENCIL_OP_FAIL_Z_REPLACE
			| BGFX_STENCIL_OP_PASS_Z_REPLACE
			);
		bgfx::setVertexBuffer(0, &vb);
		bgfx::setIndexBuffer(&ib);
		bgfx::submit(sm_local_clear_view(), _shadow_shader.program);
	}
}

void Pipeline::update_conditions()
{
	_render_pipeline.set_condition(StringId32("bloom"), (_render_settings.flags & RenderSettingsFlags::BLOOM) != 0 && _bloom.enabled);
	_render_pipeline.set_condition(StringId32("vignette"), _vignette.enabled);
}

void Pipeline::render(u16 width, u16 height, const Matrix4x4 &view, const Matrix4x4 &proj)
{
	CE_UNUSED_2(width, height);
	update_conditions();
	_render_pipeline.render(view, proj);
}

void Pipeline::begin_light_cookie_atlas()
{
	if (!light_cookies_enabled())
		return;

	const u16 width = (u16)_render_settings.lights_cookie_atlas_size.x;
	const u16 height = (u16)_render_settings.lights_cookie_atlas_size.y;
	stbrp_init_target(_lights_cookie_atlas_packer
		, width
		, height
		, _lights_cookie_atlas_packer_nodes
		, width
		);
	// Clearing is an explicit layer operation, not a side effect of packing.
}

Vector4 Pipeline::add_light_cookie(u16 &view, bgfx::TextureHandle texture, u16 width, u16 height)
{
	const u16 first = cookie_atlas_view();
	const u32 count = _render_pipeline.geometry_view_count(StringId32("lights_cookie_atlas"));
	if (!light_cookies_enabled() || view < first || u32(view) >= u32(first) + count)
		return VECTOR4_ZERO;

	stbrp_rect packed = { 0, width, height, 0, 0, 0 };
	stbrp_pack_rects(_lights_cookie_atlas_packer, &packed, 1);
	if (!packed.was_packed)
		return VECTOR4_ZERO;

	bgfx::setViewRect(view, packed.x, packed.y, width, height);
	const u32 sampler_flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	bgfx::setTexture(0, _color_map, texture, sampler_flags);
	if (!render_pipeline::fullscreen_triangle(width, height)) {
		bgfx::discard();
		return VECTOR4_ZERO;
	}
	bgfx::setState(_blit_shader.state);
	bgfx::submit(view++, _blit_shader.program);

	const f32 atlas_w = _render_settings.lights_cookie_atlas_size.x;
	const f32 atlas_h = _render_settings.lights_cookie_atlas_size.y;
	const f32 min_v = bgfx::getCaps()->originBottomLeft
		? atlas_h - f32(packed.y) - f32(height) + 0.5f
		: f32(packed.y) + 0.5f
		;
	const f32 max_v = bgfx::getCaps()->originBottomLeft
		? atlas_h - f32(packed.y) - 0.5f
		: f32(packed.y) + f32(height) - 0.5f
		;
	return {
		(f32(packed.x) + 0.5f) / atlas_w,
		min_v / atlas_h,
		(f32(packed.x) + f32(width) - 0.5f) / atlas_w,
		max_v / atlas_h
	};
}

void Pipeline::reload_shaders(const ShaderResource *old_resource, const ShaderResource *new_resource)
{
	CE_UNUSED_2(old_resource, new_resource);
	lookup_default_shaders(*this);
}

void Pipeline::set_local_lights_params_uniform()
{
	Vector4 params;
	params.x = f32((_render_settings.flags & RenderSettingsFlags::LOCAL_LIGHTS_DISTANCE_CULLING) != 0);
	params.y = _render_settings.local_lights_distance_culling_fade;
	params.z = _render_settings.local_lights_distance_culling_cutoff;
	bgfx::setUniform(_u_local_lights_params, &params, sizeof(params)/sizeof(Vector4));
}

void Pipeline::set_global_lighting_params(GlobalLightingDesc *global_lighting)
{
	Vector4 params;
	params.x = global_lighting->ambient_color.x;
	params.y = global_lighting->ambient_color.y;
	params.z = global_lighting->ambient_color.z;
	params.w = global_lighting->shadow_distance;

	bgfx::setUniform(_lighting_params, &params, sizeof(params)/sizeof(Vector4));
}

} // namespace crown

#undef PIPELINE_ENSURE
