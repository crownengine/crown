/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/math/matrix4x4.inl"
#include "core/strings/string_id.inl"
#include "core/error/error.h"
#include "device/render_pipeline.h"
#include "resource/render_config_resource.inl"
#include "world/shader_manager.h"
#include <bx/math.h>
#include <cmath>

// Device/configuration validation must also run in release builds.
#define PIPELINE_ENSURE(condition, ...) \
	do { if (!(condition)) crown::error::abort(__VA_ARGS__); } while (0)

namespace crown
{
namespace
{
	bgfx::TextureFormat::Enum texture_format(u32 format)
	{
		static const bgfx::TextureFormat::Enum formats[] = {
			bgfx::TextureFormat::RGBA8, bgfx::TextureFormat::BGRA8,
			bgfx::TextureFormat::RGBA16F, bgfx::TextureFormat::RGBA32F,
			bgfx::TextureFormat::R32U, bgfx::TextureFormat::D16,
			bgfx::TextureFormat::D24, bgfx::TextureFormat::D24S8, bgfx::TextureFormat::D32F
		};
		CE_STATIC_ASSERT(countof(formats) == RenderResourceFormat::COUNT);
		PIPELINE_ENSURE(format < countof(formats), "Invalid render resource format");
		return formats[format];
	}

	u32 sampler_flags(u32 flags)
	{
		u32 result = 0;
		if (flags & RenderResourceFlags::CLAMP) result |= BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
		if (flags & RenderResourceFlags::POINT) result |= BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT;
		if (flags & RenderResourceFlags::ANISOTROPIC) result |= BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC | BGFX_SAMPLER_MIP_POINT;
		if (flags & RenderResourceFlags::COMPARE) result |= BGFX_SAMPLER_COMPARE_LEQUAL;
		return result;
	}

	u64 texture_flags(const RenderResourceData &resource, u32 msaa_quality)
	{
		u64 flags = sampler_flags(resource.flags);
		if (resource.flags & RenderResourceFlags::RENDER_TARGET) flags |= BGFX_TEXTURE_RT;
		if ((resource.flags & RenderResourceFlags::MSAA) && msaa_quality) {
			flags &= ~BGFX_TEXTURE_RT_MSAA_MASK;
			flags |= u64(1 + msaa_quality) << BGFX_TEXTURE_RT_MSAA_SHIFT;
			if (resource.format >= RenderResourceFormat::D16) {
				// Preserve Crown's backend-specific depth/MSAA policy.
				if (CROWN_PLATFORM_LINUX)
					flags |= BGFX_TEXTURE_MSAA_SAMPLE;
				else
					flags |= BGFX_TEXTURE_RT_WRITE_ONLY;
			}
		}
		return flags;
	}

	u16 dimension(u16 source, f32 scale, u32 limit, const char *name)
	{
		const double value = double(source) * double(scale);
		PIPELINE_ENSURE(std::isfinite(value) && value > 0.0 && value <= limit && value <= UINT16_MAX
			, "Render resource '%s' exceeds the device texture-size limit", name);
		return (u16)max(1u, (u32)value);
	}
}

RenderPipeline::RenderPipeline(Allocator &a, ShaderManager &sm)
	: _resource(NULL), _shader_manager(&sm), _user_data(NULL), _conditions(0), _allocation_conditions(0)
	, _resource_textures(a), _textures(a), _views(a), _layer_views(a)
	, _frame_buffers(a), _samplers(a), _uniforms(a), _pixel_sizes(a), _functions(a)
	, _resource_names(a), _layer_names(a)
{
}

RenderPipeline::~RenderPipeline()
{
	destroy();
}

bool RenderPipeline::enabled(const RenderCondition &condition) const
{
	return render_config_resource::enabled(condition, _conditions);
}

void RenderPipeline::set_condition(StringId32 name, bool on)
{
	const u32 bit = render_config_resource::condition_mask(_resource, name);
	PIPELINE_ENSURE(!(bit & _allocation_conditions) || on == ((_conditions & bit) != 0)
		, "Changing a resource allocation condition requires recreating the pipeline");
	if (on) _conditions |= bit;
	else _conditions &= ~bit;
}

u32 RenderPipeline::resource_index(StringId32 name) const
{
	const u32 index = hash_map::get(_resource_names, name, RENDER_CONFIG_INVALID);
	PIPELINE_ENSURE(index != RENDER_CONFIG_INVALID, "Unknown render resource 0x%08x", name._id);
	return index;
}

const RenderTextureState &RenderPipeline::texture_state(RenderResourceRef ref) const
{
	PIPELINE_ENSURE(ref.resource < _resource->num_resources, "Invalid render resource index");
	PIPELINE_ENSURE(ref.index < render_config_resource::resources(_resource)[ref.resource].count, "Invalid render texture index");
	return _textures[_resource_textures[ref.resource] + ref.index];
}

bgfx::TextureHandle RenderPipeline::texture(StringId32 name, u32 index) const
{
	return texture_state({ resource_index(name), index }).handle;
}

u32 RenderPipeline::layer_index(StringId32 name) const
{
	const u32 index = hash_map::get(_layer_names, name, RENDER_CONFIG_INVALID);
	PIPELINE_ENSURE(index != RENDER_CONFIG_INVALID, "Unknown render layer 0x%08x", name._id);
	return index;
}

u16 RenderPipeline::layer_view(u32 layer, u32 index) const
{
	PIPELINE_ENSURE(layer < _resource->num_layers, "Invalid render layer");
	const RenderLayerData &d = render_config_resource::layers(_resource)[layer];
	PIPELINE_ENSURE(index < d.count, "Sub-view index out of range for '%s'", render_config_resource::name(_resource, d.name_offset));
	const u16 view = _layer_views[layer] + (u16)index;
	return enabled(d.condition) && _views[view].usable ? view : UINT16_MAX;
}

u16 RenderPipeline::view_id(StringId32 name, u32 index) const
{
	return layer_view(layer_index(name), index);
}

bgfx::FrameBufferHandle RenderPipeline::frame_buffer(StringId32 name, u32 index) const
{
	const u16 id = view_id(name, index);
	return id != UINT16_MAX ? _views[id].frame_buffer : bgfx::FrameBufferHandle BGFX_INVALID_HANDLE;
}

u16 RenderPipeline::shader_view(StringId32 shader, u16 fallback) const
{
	const RenderShaderLayerData *routes = render_config_resource::shader_layers(_resource);
	for (u32 i = 0; i < _resource->num_shader_layers; ++i) {
		if (routes[i].shader == shader) return layer_view(routes[i].layer, routes[i].index);
	}
	return fallback;
}

RenderViewState RenderPipeline::create_target(const RenderTargetData &d, u16 width, u16 height)
{
	RenderViewState view = { BGFX_INVALID_HANDLE, width, height, true };
	bgfx::TextureHandle attachments[RENDER_CONFIG_MAX_TARGETS];
	u32 count = d.num_colors + (d.depth.resource != RENDER_CONFIG_INVALID);
	PIPELINE_ENSURE(count <= bgfx::getCaps()->limits.maxFBAttachments, "Too many framebuffer attachments for this device");
	u32 quality = 0;
	for (u32 i = 0; i < count; ++i) {
		const RenderResourceRef ref = i < d.num_colors ? d.colors[i] : d.depth;
		const RenderTextureState &t = texture_state(ref);
		if (!bgfx::isValid(t.handle)) view.usable = false;
		if (i == 0) { view.width = t.width; view.height = t.height; quality = t.msaa_quality; }
		PIPELINE_ENSURE(view.width == t.width && view.height == t.height && quality == t.msaa_quality
			, "Framebuffer attachments must have identical dimensions and MSAA quality");
		attachments[i] = t.handle;
	}
	if (count && view.usable) {
		// Views borrow attachments; the resource set is the ONLY texture owner.
		view.frame_buffer = bgfx::createFrameBuffer((u8)count, attachments, false);
		PIPELINE_ENSURE(bgfx::isValid(view.frame_buffer), "Could not create render framebuffer");
		array::push_back(_frame_buffers, view.frame_buffer);
	}
	return view;
}

void RenderPipeline::setup_view(u16 id, const RenderTargetData &d, const char *name)
{
	const RenderViewState &v = _views[id];
	bgfx::resetView(id);
	bgfx::setViewName(id, name);
	if (!v.usable) return;
	bgfx::setViewFrameBuffer(id, v.frame_buffer);
	if (!d.manual_rect) bgfx::setViewRect(id, 0, 0, v.width, v.height);
	static const bgfx::ViewMode::Enum modes[] = { bgfx::ViewMode::Default, bgfx::ViewMode::Sequential, bgfx::ViewMode::DepthAscending, bgfx::ViewMode::DepthDescending };
	PIPELINE_ENSURE(d.sort < countof(modes), "Invalid layer sort mode");
	bgfx::setViewMode(id, modes[d.sort]);
	u16 clear = 0;
	if (d.clear_flags & RenderClearFlags::COLOR) clear |= BGFX_CLEAR_COLOR;
	if (d.clear_flags & RenderClearFlags::DEPTH) clear |= BGFX_CLEAR_DEPTH;
	if (d.clear_flags & RenderClearFlags::STENCIL) clear |= BGFX_CLEAR_STENCIL;
	bgfx::setViewClear(id, clear, d.clear_rgba, d.clear_depth, (u8)d.clear_stencil);
	f32 projection[16];
	const bool homogeneous = bgfx::getCaps()->homogeneousDepth;
	switch (d.transform) {
	case RenderLayerTransform::ORTHO_UNIT:
		bx::mtxOrtho(projection, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 100.0f, 0.0f, homogeneous, bx::Handedness::Right);
		break;
	case RenderLayerTransform::ORTHO_UNIT_FLIPPED:
		bx::mtxOrtho(projection, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, homogeneous);
		break;
	case RenderLayerTransform::ORTHO_SCREEN:
		bx::mtxOrtho(projection, 0.0f, f32(v.width), 0.0f, f32(v.height), 0.0f, 1.0f, 0.0f, homogeneous, bx::Handedness::Right);
		break;
	case RenderLayerTransform::ORTHO_GRAPH:
		bx::mtxOrtho(projection, -v.width/2.0f, v.width/2.0f, -v.height/2.0f, v.height/2.0f, 0.0f, 1.0f, 0.0f, homogeneous, bx::Handedness::Right);
		break;
	default: return; // Camera transforms are supplied per frame, or by a producer.
	}
	bgfx::setViewTransform(id, NULL, projection);
}

void RenderPipeline::create(const RenderConfigResource *r, u16 width, u16 height
	, u32 msaa_quality, u32 conditions
	, const RenderResourceSize *sizes, u32 num_sizes
	, const RenderModifierType *types, u32 num_types, void *user_data)
{
	PIPELINE_ENSURE(_resource == NULL, "Destroy the render pipeline before recreating it");
	PIPELINE_ENSURE(r != NULL && r->version == RESOURCE_HEADER(RESOURCE_VERSION_RENDER_CONFIG), "Invalid render_config version");
	PIPELINE_ENSURE(msaa_quality <= 4, "Invalid MSAA quality");
	_resource = r;
	_user_data = user_data;
	_conditions = conditions;
	width = max(u16(1), width);
	height = max(u16(1), height);
	const bgfx::Caps *caps = bgfx::getCaps();
	const RenderResourceData *resources = render_config_resource::resources(r);
	for (u32 i = 0; i < r->num_resources; ++i) {
		const RenderResourceData &d = resources[i];
		_allocation_conditions |= d.condition.required | d.condition.excluded;
		hash_map::set(_resource_names, d.name, i);
		u16 source_width = (u16)d.width, source_height = (u16)d.height;
		if (!d.width) {
			if (d.size_source == StringId32("backbuffer")) { source_width = width; source_height = height; }
			else {
				bool found = false;
				for (u32 k = 0; k < num_sizes; ++k) {
					if (sizes[k].name == d.size_source) { source_width = sizes[k].width; source_height = sizes[k].height; found = true; break; }
				}
				PIPELINE_ENSURE(found, "Unknown size source for '%s'", render_config_resource::name(r, d.name_offset));
			}
		}
		const char *name = render_config_resource::name(r, d.name_offset);
		const u16 w = dimension(source_width, d.w_scale, caps->limits.maxTextureSize, name);
		const u16 h = dimension(source_height, d.h_scale, caps->limits.maxTextureSize, name);
		array::push_back(_resource_textures, array::size(_textures));
		const u64 flags = texture_flags(d, msaa_quality);
		for (u32 k = 0; k < d.count; ++k) {
			RenderTextureState texture = { BGFX_INVALID_HANDLE, (u16)max(1u, u32(w) >> k), (u16)max(1u, u32(h) >> k), (d.flags & RenderResourceFlags::MSAA) ? msaa_quality : 0 };
			if (enabled(d.condition)) {
				const bgfx::TextureFormat::Enum format = texture_format(d.format);
				PIPELINE_ENSURE(bgfx::isTextureValid(0, false, 1, format, flags), "Unsupported texture format/flags for '%s'", name);
				const u8 rgba[] = { u8(d.initial_value >> 24), u8(d.initial_value >> 16), u8(d.initial_value >> 8), u8(d.initial_value) };
				const bgfx::Memory *initial = d.has_initial_value ? bgfx::copy(rgba, sizeof(rgba)) : NULL;
				texture.handle = bgfx::createTexture2D(texture.width, texture.height, false, 1, format, flags, initial);
				PIPELINE_ENSURE(bgfx::isValid(texture.handle), "Could not create render texture '%s'", name);
				bgfx::setName(texture.handle, name);
			}
			array::push_back(_textures, texture);
		}
	}

	const RenderInputData *inputs = render_config_resource::inputs(r);
	for (u32 i = 0; i < r->num_inputs; ++i) {
		PIPELINE_ENSURE(inputs[i].stage < caps->limits.maxTextureSamplers, "Texture stage exceeds device limit");
		bgfx::UniformHandle handle = bgfx::createUniform(render_config_resource::name(r, inputs[i].sampler_name_offset), bgfx::UniformType::Sampler);
		PIPELINE_ENSURE(bgfx::isValid(handle), "Could not create modifier sampler");
		array::push_back(_samplers, handle);
	}
	const RenderUniformData *uniforms = render_config_resource::uniforms(r);
	for (u32 i = 0; i < r->num_uniforms; ++i) {
		bgfx::UniformHandle handle = bgfx::createUniform(render_config_resource::name(r, uniforms[i].name_offset), bgfx::UniformType::Vec4);
		PIPELINE_ENSURE(bgfx::isValid(handle), "Could not create modifier uniform");
		array::push_back(_uniforms, handle);
	}
	const RenderModifierData *mods = render_config_resource::modifiers(r);
	for (u32 i = 0; i < r->num_modifiers; ++i) {
		RenderModifierFunction function = NULL;
		if (mods[i].type != StringId32("fullscreen_pass")) {
			for (u32 k = 0; k < num_types; ++k) if (types[k].name == mods[i].type) { function = types[k].function; break; }
			PIPELINE_ENSURE(function != NULL, "Unregistered render modifier type 0x%08x", mods[i].type._id);
		}
		array::push_back(_functions, function);
		bgfx::UniformHandle handle = BGFX_INVALID_HANDLE;
		if (mods[i].pixel_size_uniform != RENDER_CONFIG_INVALID) {
			handle = bgfx::createUniform(render_config_resource::name(r, mods[i].pixel_size_uniform), bgfx::UniformType::Vec4);
			PIPELINE_ENSURE(bgfx::isValid(handle), "Could not create pixel-size uniform");
		}
		array::push_back(_pixel_sizes, handle);
	}

	const RenderLayerData *layers = render_config_resource::layers(r);
	u32 view_count = 0;
	for (u32 i = 0; i < r->num_layers; ++i) view_count += layers[i].count;
	PIPELINE_ENSURE(view_count <= caps->limits.maxViews && view_count <= UINT16_MAX, "Pipeline needs %u views, device supports %u", view_count, caps->limits.maxViews);
	for (u32 i = 0; i < r->num_layers; ++i) {
		const RenderLayerData &layer = layers[i];
		hash_map::set(_layer_names, layer.name, i);
		array::push_back(_layer_views, (u16)array::size(_views));
		if (layer.generator == RENDER_CONFIG_INVALID) {
			RenderViewState target = create_target(layer.target, width, height);
			for (u32 j = 0; j < layer.count; ++j) {
				const u16 id = (u16)array::size(_views);
				array::push_back(_views, target);
				setup_view(id, layer.target, render_config_resource::name(r, layer.name_offset));
			}
		} else {
			const RenderGeneratorData &g = render_config_resource::generators(r)[layer.generator];
			for (u32 j = 0; j < g.num_modifiers; ++j) {
				const RenderModifierData &mod = mods[g.first_modifier + j];
				const u16 id = (u16)array::size(_views);
				array::push_back(_views, create_target(mod.target, width, height));
				setup_view(id, mod.target, render_config_resource::name(r, mod.name_offset));
			}
		}
	}
	// Declaration order is backend order; CPU submission order is independent.
	bgfx::setViewOrder(0, (u16)view_count, NULL);
}

void RenderPipeline::destroy()
{
	for (u32 i = 0; i < array::size(_views); ++i) bgfx::resetView((u16)i);
	for (u32 i = 0; i < array::size(_frame_buffers); ++i) bgfx::destroy(_frame_buffers[i]);
	for (u32 i = 0; i < array::size(_textures); ++i) if (bgfx::isValid(_textures[i].handle)) bgfx::destroy(_textures[i].handle);
	for (u32 i = 0; i < array::size(_samplers); ++i) bgfx::destroy(_samplers[i]);
	for (u32 i = 0; i < array::size(_uniforms); ++i) bgfx::destroy(_uniforms[i]);
	for (u32 i = 0; i < array::size(_pixel_sizes); ++i) if (bgfx::isValid(_pixel_sizes[i])) bgfx::destroy(_pixel_sizes[i]);
	array::clear(_resource_textures); array::clear(_textures); array::clear(_views); array::clear(_layer_views);
	array::clear(_frame_buffers); array::clear(_samplers); array::clear(_uniforms); array::clear(_pixel_sizes); array::clear(_functions);
	hash_map::clear(_resource_names); hash_map::clear(_layer_names);
	_resource = NULL;
	_allocation_conditions = 0;
}

void RenderPipeline::render(const Matrix4x4 &view, const Matrix4x4 &projection)
{
	const RenderLayerData *layers = render_config_resource::layers(_resource);
	const RenderModifierData *mods = render_config_resource::modifiers(_resource);
	for (u32 i = 0; i < _resource->num_layers; ++i) {
		const RenderLayerData &layer = layers[i];
		if (!enabled(layer.condition)) continue;
		for (u32 j = 0; j < layer.count; ++j) {
			const u16 id = _layer_views[i] + (u16)j;
			if (!_views[id].usable) continue;
			u32 modifier = RENDER_CONFIG_INVALID;
			const RenderTargetData *target = &layer.target;
			if (layer.generator != RENDER_CONFIG_INVALID) {
				modifier = render_config_resource::generators(_resource)[layer.generator].first_modifier + j;
				if (!enabled(mods[modifier].condition)) continue;
				target = &mods[modifier].target;
			}
			if (target->transform == RenderLayerTransform::CAMERA)
				bgfx::setViewTransform(id, to_float_ptr(view), to_float_ptr(projection));
			if (target->touch) bgfx::touch(id);
			if (modifier != RENDER_CONFIG_INVALID) {
				if (_functions[modifier]) _functions[modifier](*this, mods[modifier], id, _user_data);
				else draw_fullscreen(mods[modifier], id);
			}
		}
	}
}

bool RenderPipeline::draw_fullscreen(const RenderModifierData &mod, u16 view)
{
	PIPELINE_ENSURE(view < array::size(_views) && _views[view].usable, "Invalid fullscreen destination");
	const RenderViewState &output = _views[view];
	if (!render_pipeline::fullscreen_triangle(output.width, output.height)) {
		bgfx::discard();
		return false;
	}
	const RenderInputData *inputs = render_config_resource::inputs(_resource);
	for (u32 i = mod.first_input; i < mod.first_input + mod.num_inputs; ++i) {
		const bgfx::TextureHandle handle = texture_state(inputs[i].texture).handle;
		PIPELINE_ENSURE(bgfx::isValid(handle), "Modifier input is disabled or unavailable");
		const u32 flags = inputs[i].flags == RENDER_CONFIG_INVALID ? UINT32_MAX : sampler_flags(inputs[i].flags);
		bgfx::setTexture((u8)inputs[i].stage, _samplers[i], handle, flags);
	}
	const RenderUniformData *uniforms = render_config_resource::uniforms(_resource);
	for (u32 i = mod.first_uniform; i < mod.first_uniform + mod.num_uniforms; ++i)
		bgfx::setUniform(_uniforms[i], &uniforms[i].value);
	const u32 index = u32(&mod - render_config_resource::modifiers(_resource));
	if (bgfx::isValid(_pixel_sizes[index])) {
		const Vector4 size = { 1.0f / output.width, 1.0f / output.height, 0.0f, 0.0f };
		bgfx::setUniform(_pixel_sizes[index], &size);
	}
	PIPELINE_ENSURE(mod.shader != StringId32(), "Fullscreen modifier needs a shader");
	const ShaderData shader = _shader_manager->shader(mod.shader); // Also observes shader hot reloads.
	bgfx::setState(shader.state);
	bgfx::setStencil(shader.stencil_front, shader.stencil_back);
	bgfx::submit(view, shader.program);
	return true;
}

namespace render_pipeline
{
	// Fullscreen triangle geometry adapted from bgfx's screenSpaceQuad helper.
	// Copyright 2011-2017 Branimir Karadzic. All rights reserved.
	// License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
	bool fullscreen_triangle(u16 width, u16 height)
	{
		CE_UNUSED_2(width, height); // No half-texel adjustment on current backends.
		struct Vertex { f32 x, y, z, u, v; };
		static bgfx::VertexLayout layout;
		if (!layout.m_hash) {
			layout.begin();
			layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
			layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
			layout.end();
		}
		if (bgfx::getAvailTransientVertexBuffer(3, layout) != 3) return false;
		bgfx::TransientVertexBuffer buffer;
		bgfx::allocTransientVertexBuffer(&buffer, 3, layout);
		Vertex *vertices = (Vertex *)buffer.data;
		const bool flip = bgfx::getCaps()->originBottomLeft;
		vertices[0] = {  1.0f, -1.0f, 0.0f,  1.0f, flip ? -1.0f : 2.0f };
		vertices[1] = {  1.0f,  1.0f, 0.0f,  1.0f, flip ?  1.0f : 0.0f };
		vertices[2] = { -1.0f,  1.0f, 0.0f, -1.0f, flip ?  1.0f : 0.0f };
		bgfx::setVertexBuffer(0, &buffer);
		return true;
	}
}
} // namespace crown

#undef PIPELINE_ENSURE
