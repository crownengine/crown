/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/math/types.h"
#include "device/render_pipeline.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct Material;

struct RenderBatchFlags
{
	enum Enum { SELECTED = 1, SHADOW_CASTER = 2 };
};

// Frame-local geometry, not a reference to a mutable culling result or manager
// slot. Material parameters remain owned by Material and are read synchronously.
struct RenderBatch
{
	Material *material;
	bgfx::VertexBufferHandle vertex_buffer;
	bgfx::IndexBufferHandle index_buffer;
	bgfx::TransientVertexBuffer transient_vertices;
	bgfx::TransientIndexBuffer transient_indices;
	u32 first_vertex;
	u32 num_vertices;
	u32 first_index;
	u32 num_indices;
	u32 transform;
	u16 num_transforms;
	bool skinned;
	bool transient;
	u32 type;
	u32 flags;
	u32 group;
	u32 depth;
	u32 object_id;
	u32 first_texture;
	u32 num_textures;
	// Per-batch uniform snapshots, e.g. a skinning palette's atlas row.
	u32 first_uniform;
	u32 num_uniforms;
	bool override_state;
	bool override_stencil;
	u64 state;
	u32 stencil_front;
	u32 stencil_back;

	RenderBatch();
};

struct RenderBatchView
{
	StringId32 source;
	u32 index;
	u32 first_batch;
	u32 num_batches;
	Matrix4x4 view;
	Matrix4x4 projection;
	Vector4 rect;
	bool camera;
	bool viewport;

	RenderBatchView(StringId32 source, u32 index);
};

struct RenderBatchTexture
{
	bgfx::UniformHandle sampler;
	bgfx::TextureHandle texture;
	u32 stage;
	u32 flags;
};

struct RenderFrameUniform
{
	StringId32 set;
	bgfx::UniformHandle handle;
	u32 first_vector;
	u32 count;
};

struct RenderTextureUpload
{
	StringId32 source;
	u32 format;
	u32 width;
	u32 height;
	u32 offset;
	u32 size;
};

// Cleared once per world/camera preparation; capacities survive across frames.
// Batches and source cameras are immutable throughout layer execution. Each
// shadow view owns a batch range, so a later cull cannot overwrite an earlier one.
struct RenderFrame
{
	Array<StringId32> sources;
	Array<RenderBatchView> views;
	Array<RenderBatch> batches;
	Array<RenderBatchTexture> textures;
	Array<RenderFrameUniform> uniforms;
	Array<Vector4> uniform_data;
	Array<RenderTextureUpload> uploads;
	Array<u8> upload_data;

	explicit RenderFrame(Allocator &a);
	RenderFrame(const RenderFrame &) = delete;
	RenderFrame &operator=(const RenderFrame &) = delete;

	void clear();
	void add_source(StringId32 source);
	u32 add_view(const RenderBatchView &view);
	void add_batch(u32 view, const RenderBatch &batch);
	void add_uniform(StringId32 set, bgfx::UniformHandle handle, const void *data, u32 size, u32 count = 1);
	void add_upload(StringId32 source, u32 format, u32 width, u32 height, const void *data, u32 size);
};

namespace render_frame
{
	RenderFrameContext context(RenderFrame &frame);

	// Also callable by native modifiers: the primitive has no special layer names.
	void draw_visible(RenderPipeline &runtime, const RenderDrawData &draw, const RenderDrawContext &context, void *frame);
	void upload(RenderPipeline &runtime, const RenderDrawData &draw, const RenderDrawContext &context, void *frame);
}
} // namespace crown
