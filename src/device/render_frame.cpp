/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/error/error.h"
#include "core/math/matrix4x4.inl"
#include "core/strings/string_id.inl"
#include "device/render_frame.h"
#include "resource/material_resource.h"
#include "resource/render_config_resource.inl"
#include "world/material.h"
#include "world/shader_manager.h"
#include <string.h>

#define FRAME_ENSURE(condition, ...) \
	do { if (!(condition)) crown::error::abort(__VA_ARGS__); } while (0)

namespace crown
{
RenderBatch::RenderBatch()
	: material(NULL)
	, vertex_buffer(BGFX_INVALID_HANDLE)
	, index_buffer(BGFX_INVALID_HANDLE)
	, transient_vertices()
	, transient_indices()
	, first_vertex(0), num_vertices(UINT32_MAX), first_index(0), num_indices(UINT32_MAX)
	, transform(UINT32_MAX), num_transforms(1), skinned(false), transient(false)
	, type(RenderDrawTypes::PROCEDURAL), flags(0), group(0), depth(0), object_id(0)
	, first_texture(0), num_textures(0), first_uniform(0), num_uniforms(0)
	, override_state(false), override_stencil(false)
	, state(0), stencil_front(0), stencil_back(0)
{
}

RenderBatchView::RenderBatchView(StringId32 source_, u32 index_)
	: source(source_), index(index_), first_batch(0), num_batches(0)
	, view(MATRIX4X4_IDENTITY), projection(MATRIX4X4_IDENTITY), rect(VECTOR4_ZERO)
	, camera(false), viewport(false)
{
}

RenderFrame::RenderFrame(Allocator &a)
	: sources(a), views(a), batches(a), textures(a), uniforms(a), uniform_data(a), uploads(a), upload_data(a)
{
}

void RenderFrame::clear()
{
	array::clear(sources);
	array::clear(views);
	array::clear(batches);
	array::clear(textures);
	array::clear(uniforms);
	array::clear(uniform_data);
	array::clear(uploads);
	array::clear(upload_data);
}

void RenderFrame::add_source(StringId32 source)
{
	for (u32 i = 0; i < array::size(sources); ++i) {
		if (sources[i] == source)
			return;
	}
	array::push_back(sources, source);
}

u32 RenderFrame::add_view(const RenderBatchView &view)
{
	add_source(view.source);
	for (u32 i = 0; i < array::size(views); ++i)
		FRAME_ENSURE(views[i].source != view.source || views[i].index != view.index, "Duplicate prepared source view");
	RenderBatchView copy = view;
	copy.first_batch = array::size(batches);
	copy.num_batches = 0;
	array::push_back(views, copy);
	return array::size(views) - 1;
}

void RenderFrame::add_batch(u32 view, const RenderBatch &batch)
{
	FRAME_ENSURE(view < array::size(views), "Invalid prepared view");
	RenderBatchView &v = views[view];
	FRAME_ENSURE(v.first_batch + v.num_batches == array::size(batches), "Prepared view batch ranges must be contiguous");
	array::push_back(batches, batch);
	++v.num_batches;
}

void RenderFrame::add_uniform(StringId32 set, bgfx::UniformHandle handle, const void *data, u32 size, u32 count)
{
	FRAME_ENSURE(data != NULL && size != 0 && size % sizeof(Vector4) == 0 && count > 0 && count <= UINT16_MAX, "Invalid frame uniform");
	const u32 offset = array::size(uniform_data);
	FRAME_ENSURE(size / sizeof(Vector4) <= UINT32_MAX - offset, "Frame uniform storage overflow");
	array::resize(uniform_data, offset + size / sizeof(Vector4));
	memcpy(array::begin(uniform_data) + offset, data, size);
	RenderFrameUniform uniform = { set, handle, offset, count };
	array::push_back(uniforms, uniform);
}

void RenderFrame::add_upload(StringId32 source, u32 format, u32 width, u32 height, const void *data, u32 size)
{
	for (u32 i = 0; i < array::size(uploads); ++i)
		FRAME_ENSURE(uploads[i].source != source, "Duplicate frame upload source");
	RenderTextureUpload upload = { source, format, width, height, array::size(upload_data), size };
	if (size != 0) {
		FRAME_ENSURE(data != NULL && size <= UINT32_MAX - upload.offset, "Invalid upload storage");
		array::resize(upload_data, upload.offset + size);
		memcpy(array::begin(upload_data) + upload.offset, data, size);
	}
	array::push_back(uploads, upload);
}

namespace render_frame
{
	static void bind_parameters(const RenderFrame &frame, StringId32 set)
	{
		if (set == StringId32())
			return;
		bool found = false;
		for (u32 i = 0; i < array::size(frame.uniforms); ++i) {
			const RenderFrameUniform &uniform = frame.uniforms[i];
			if (uniform.set != set)
				continue;
			found = true;
			bgfx::setUniform(uniform.handle, array::begin(frame.uniform_data) + uniform.first_vector, (u16)uniform.count);
		}
		FRAME_ENSURE(found, "Unknown frame parameter set 0x%08x", set._id);
	}

	static const RenderBatchView *source_view(const RenderFrame &frame, StringId32 source, u32 index)
	{
		bool found = false;
		for (u32 i = 0; i < array::size(frame.sources); ++i)
			found = found || frame.sources[i] == source;
		FRAME_ENSURE(found, "Unknown prepared batch source 0x%08x", source._id);
		for (u32 i = 0; i < array::size(frame.views); ++i) {
			if (frame.views[i].source == source && frame.views[i].index == index)
				return &frame.views[i];
		}
		return NULL; // A registered source can legitimately have no visible views.
	}

	static bool accepts(const RenderBatch &batch, const RenderDrawData &draw)
	{
		if (!(batch.type & draw.types))
			return false;
		if (draw.filter == RenderDrawFilter::SELECTED && !(batch.flags & RenderBatchFlags::SELECTED))
			return false;
		if (draw.filter == RenderDrawFilter::SHADOW_CASTERS && !(batch.flags & RenderBatchFlags::SHADOW_CASTER))
			return false;
		return true;
	}

	static void bind_geometry(const RenderBatch &batch)
	{
		if (batch.transient) {
			bgfx::setVertexBuffer(0, &batch.transient_vertices, batch.first_vertex, batch.num_vertices);
			if (batch.num_indices != 0)
				bgfx::setIndexBuffer(&batch.transient_indices, batch.first_index, batch.num_indices);
		} else {
			bgfx::setVertexBuffer(0, batch.vertex_buffer, batch.first_vertex, batch.num_vertices);
			if (batch.num_indices != 0)
				bgfx::setIndexBuffer(batch.index_buffer, batch.first_index, batch.num_indices);
		}
		if (batch.transform == UINT32_MAX)
			bgfx::setTransform(to_float_ptr(MATRIX4X4_IDENTITY));
		else
			bgfx::setTransform(batch.transform, batch.num_transforms);
	}

	void draw_visible(RenderPipeline &runtime, const RenderDrawData &draw, const RenderDrawContext &context, void *frame_data)
	{
		FRAME_ENSURE(frame_data != NULL && runtime._resource != NULL
			&& context.layer < runtime._resource->num_layers && context.view < array::size(runtime._views)
			, "Invalid prepared draw context");
		const RenderFrame &frame = *(const RenderFrame *)frame_data;
		const u32 index = draw.source_index + (draw.view_mode == RenderDrawViewMode::VIEWS ? context.index : 0);
		const RenderBatchView *view = source_view(frame, draw.source, index);
		if (view == NULL)
			return;
		const RenderViewState &output = runtime._views[context.view];
		if (view->camera)
			bgfx::setViewTransform(context.view, to_float_ptr(view->view), to_float_ptr(view->projection));
		if (view->viewport) {
			FRAME_ENSURE(view->rect.x >= 0 && view->rect.y >= 0 && view->rect.z >= 1 && view->rect.w >= 1
				&& view->rect.x + view->rect.z <= output.width && view->rect.y + view->rect.w <= output.height
				, "Prepared viewport exceeds its configured destination");
			bgfx::setViewRect(context.view, (u16)view->rect.x, (u16)view->rect.y, (u16)view->rect.z, (u16)view->rect.w);
		}
		FRAME_ENSURE(view->first_batch <= array::size(frame.batches) && view->num_batches <= array::size(frame.batches) - view->first_batch, "Invalid prepared batch range");
		for (u32 i = view->first_batch; i < view->first_batch + view->num_batches; ++i) {
			const RenderBatch &batch = frame.batches[i];
			if (!accepts(batch, draw))
				continue;
			const RenderShaderLayerData *route = batch.material != NULL
				? runtime.shader_route(batch.material->_resource->shader, draw.context) : NULL;
			if (route != NULL) {
				if (route->layer != context.layer || route->index != context.index)
					continue;
			} else if (draw.view_mode == RenderDrawViewMode::GROUPS && batch.group != context.index) {
				continue;
			}
			StringId32 program = draw.shader;
			StringId32 skinned_program = draw.skinned_shader;
			if (route != NULL && route->program != StringId32()) {
				program = route->program;
				skinned_program = route->skinned_program;
			}
			FRAME_ENSURE(program != StringId32() || batch.material != NULL, "Procedural batches require a configured shader");
			FRAME_ENSURE(!batch.skinned || program == StringId32() || skinned_program != StringId32(), "Skinned batch needs skinned_shader/skinned_program for this override");
			const ShaderData shader = program == StringId32() ? batch.material->_shader
				: runtime._shader_manager->shader(batch.skinned ? skinned_program : program);
			FRAME_ENSURE(bgfx::isValid(shader.program), "Unavailable draw shader");
			bgfx::discard(); // Each prepared batch supplies its complete draw state.
			bind_geometry(batch);
			if (batch.material != NULL)
				batch.material->bind_parameters(shader);
			bind_parameters(frame, draw.parameters);
			FRAME_ENSURE(batch.first_uniform <= array::size(frame.uniforms)
				&& batch.num_uniforms <= array::size(frame.uniforms) - batch.first_uniform, "Invalid batch uniform range");
			for (u32 k = batch.first_uniform; k < batch.first_uniform + batch.num_uniforms; ++k) {
				const RenderFrameUniform &uniform = frame.uniforms[k];
				bgfx::setUniform(uniform.handle, array::begin(frame.uniform_data) + uniform.first_vector, (u16)uniform.count);
			}
			FRAME_ENSURE(batch.first_texture <= array::size(frame.textures) && batch.num_textures <= array::size(frame.textures) - batch.first_texture, "Invalid batch texture range");
			for (u32 k = batch.first_texture; k < batch.first_texture + batch.num_textures; ++k) {
				const RenderBatchTexture &texture = frame.textures[k];
				FRAME_ENSURE(texture.stage < bgfx::getCaps()->limits.maxTextureSamplers
					&& bgfx::isValid(texture.sampler) && bgfx::isValid(texture.texture), "Invalid batch texture binding");
				bgfx::setTexture((u8)texture.stage, texture.sampler, texture.texture, texture.flags);
			}
			runtime.bind_inputs(draw.first_input, draw.num_inputs);
			runtime.bind_uniforms(draw.first_uniform, draw.num_uniforms);
			if (bgfx::isValid(runtime._draw_object_ids[context.layer])) {
				Vector4 id = VECTOR4_ZERO;
				memcpy(&id.x, &batch.object_id, sizeof(batch.object_id));
				bgfx::setUniform(runtime._draw_object_ids[context.layer], &id);
			}
			u64 state = (batch.override_state ? batch.state : shader.state) & ~BGFX_STATE_MSAA;
			if (output.msaa_quality != 0)
				state |= BGFX_STATE_MSAA;
			bgfx::setState(state);
			bgfx::setStencil(batch.override_stencil ? batch.stencil_front : shader.stencil_front
				, batch.override_stencil ? batch.stencil_back : shader.stencil_back);
			bgfx::submit(context.view, shader.program, batch.depth);
		}
	}

	void upload(RenderPipeline &runtime, const RenderDrawData &draw, const RenderDrawContext &context, void *frame_data)
	{
		CE_UNUSED(context);
		const RenderFrame &frame = *(const RenderFrame *)frame_data;
		FRAME_ENSURE(draw.resource.resource != RENDER_CONFIG_INVALID, "Upload needs a destination");
		const RenderTextureState &texture = runtime.texture_state(draw.resource);
		const RenderResourceData &resource = render_config_resource::resources(runtime._resource)[draw.resource.resource];
		FRAME_ENSURE(bgfx::isValid(texture.handle) && !(resource.flags & RenderResourceFlags::RENDER_TARGET), "Upload destination must be an allocated ordinary texture");
		for (u32 i = 0; i < array::size(frame.uploads); ++i) {
			const RenderTextureUpload &upload = frame.uploads[i];
			if (upload.source != draw.source)
				continue;
			FRAME_ENSURE(upload.format == resource.format && upload.width <= texture.width && upload.height <= texture.height, "Upload does not match the configured texture");
			FRAME_ENSURE(upload.offset <= array::size(frame.upload_data) && upload.size <= array::size(frame.upload_data) - upload.offset, "Invalid upload range");
			static const u32 pixel_sizes[] = { 4, 4, 8, 16, 4 }; // Color formats through R32U.
			FRAME_ENSURE(upload.format < countof(pixel_sizes)
				&& u64(upload.width) * upload.height * pixel_sizes[upload.format] == upload.size
				, "Upload byte count does not match its tightly packed color pixels");
			if (upload.size != 0) {
				FRAME_ENSURE(upload.width > 0 && upload.height > 0, "Empty upload dimensions");
				bgfx::updateTexture2D(texture.handle, 0, 0, 0, 0, (u16)upload.width, (u16)upload.height
					, bgfx::copy(array::begin(frame.upload_data) + upload.offset, upload.size));
			}
			return;
		}
		FRAME_ENSURE(false, "Unknown frame upload source 0x%08x", draw.source._id);
	}

	RenderFrameContext context(RenderFrame &frame)
	{
		static const RenderDrawType types[] = {
			{ StringId32("draw_visible"), draw_visible },
			{ StringId32("upload"), upload }
		};
		return { types, countof(types), &frame };
	}
}
} // namespace crown

#undef FRAME_ENSURE
