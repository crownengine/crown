/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "resource/render_config_resource.h"
#include "world/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct RenderPipeline;
typedef void (*RenderModifierFunction)(RenderPipeline &, const RenderModifierData &, u16 view, void *user_data);

struct RenderModifierType
{
	StringId32 name;
	RenderModifierFunction function;
};

// Supplied for one synchronous frame execution. No frame pointers are kept by
// the pipeline, and no native callbacks are stored in the resource blob.
struct RenderDrawContext
{
	u32 layer;
	u32 index;
	u16 view;
};

typedef void (*RenderDrawFunction)(RenderPipeline &, const RenderDrawData &, const RenderDrawContext &, void *frame_data);

struct RenderDrawType
{
	StringId32 name;
	RenderDrawFunction function;
};

struct RenderFrameContext
{
	const RenderDrawType *types;
	u32 num_types;
	void *data;
};

struct RenderResourceSize
{
	StringId32 name;
	u16 width;
	u16 height;
};

struct RenderTextureState
{
	bgfx::TextureHandle handle;
	u16 width;
	u16 height;
	u32 msaa_quality;
};

struct RenderViewState
{
	bgfx::FrameBufferHandle frame_buffer;
	u16 width;
	u16 height;
	bool usable;
	u32 msaa_quality = 0;
};

/// Technique-independent executor. Descriptions are read-only; every GPU handle
/// and every mutable condition belongs to this runtime instance.
struct RenderPipeline
{
	const RenderConfigResource *_resource;
	ShaderManager *_shader_manager;
	void *_user_data;
	u32 _conditions;
	u32 _allocation_conditions;
	Array<u32> _resource_textures;
	Array<RenderTextureState> _textures;
	Array<RenderViewState> _views;
	Array<u16> _layer_views;
	Array<bgfx::FrameBufferHandle> _frame_buffers;
	Array<bgfx::UniformHandle> _samplers;
	Array<bgfx::UniformHandle> _uniforms;
	Array<bgfx::UniformHandle> _pixel_sizes;
	Array<bgfx::UniformHandle> _draw_object_ids;
	Array<RenderModifierFunction> _functions;
	HashMap<StringId32, u32> _resource_names;
	HashMap<StringId32, u32> _layer_names;

	RenderPipeline(Allocator &a, ShaderManager &sm);
	~RenderPipeline();
	RenderPipeline(const RenderPipeline &) = delete;
	RenderPipeline &operator=(const RenderPipeline &) = delete;

	void create(const RenderConfigResource *resource, u16 width, u16 height
		, u32 msaa_quality, u32 conditions
		, const RenderResourceSize *sizes, u32 num_sizes
		, const RenderModifierType *types, u32 num_types, void *user_data);
	void destroy();
	void render(const Matrix4x4 &view, const Matrix4x4 &projection, const RenderFrameContext *frame = NULL);

	/// Allocation conditions take effect on create(); execution conditions can
	/// change each frame. Consumers must also require their resources' conditions.
	void set_condition(StringId32 name, bool enabled);
	bool enabled(const RenderCondition &condition) const;

	/// Optional name lookups return RENDER_CONFIG_INVALID / invalid handles.
	/// Binary references from compiled data are still checked strictly.
	u32 resource_index(StringId32 name) const;
	const RenderTextureState &texture_state(RenderResourceRef ref) const;
	bgfx::TextureHandle texture(StringId32 name, u32 index = 0) const;
	u32 layer_index(StringId32 name) const;
	u16 layer_view(u32 layer, u32 index = 0) const;
	u16 view_id(StringId32 name, u32 index = 0) const;
	/// Scene producers cannot submit into resource-generator views.
	u16 geometry_view(StringId32 name, u32 index = 0) const;
	u32 geometry_view_count(StringId32 name) const;
	bool empty() const;
	bgfx::FrameBufferHandle frame_buffer(StringId32 layer, u32 index = 0) const;
	u16 shader_view(StringId32 shader, u16 fallback) const;
	const RenderShaderLayerData *shader_route(StringId32 shader, StringId32 context) const;

	/// Resolve declared operations, never semantic layer names.
	u32 draw_layer(StringId32 type, StringId32 source) const;
	u32 source_view_count(StringId32 source) const;
	u16 external_view(StringId32 source, u32 index = 0) const;

	/// Shared by fullscreen modifiers and scene draws. Fallbacks resolve from
	/// the current runtime allocation, not from cached native texture aliases.
	void bind_inputs(u32 first, u32 count) const;
	void bind_uniforms(u32 first, u32 count) const;

	/// Common primitive for both data-only modifiers and native callbacks.
	/// Returns false (and discards pending draw state) if transient memory runs out.
	bool draw_fullscreen(const RenderModifierData &modifier, u16 view);

private:
	RenderViewState create_target(const RenderTargetData &target, u16 width, u16 height);
	void setup_view(u16 view, const RenderTargetData &target, const char *name);
};

namespace render_pipeline
{
	/// The same fullscreen triangle is used by the light-cookie atlas producer.
	bool fullscreen_triangle(u16 width, u16 height);
	bool fullscreen_vertices(bgfx::TransientVertexBuffer &vertices);
}
} // namespace crown
