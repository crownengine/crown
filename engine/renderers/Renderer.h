/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Config.h"
#include "Types.h"
#include "PixelFormat.h"
#include "VertexFormat.h"
#include "StringUtils.h"
#include "RenderContext.h"

namespace crown
{

enum RenderTargetFormat
{
	RTF_RGB_8,		///< RGB values, 8-bit each
	RTF_RGBA_8,		///< RGBA values, 8-bit each
	RTF_D24			///< Depth
};

enum ShaderType
{
	SHADER_VERTEX,
	SHADER_FRAGMENT
};

class Renderer
{
public:

	static Renderer*		create(Allocator& a);
	static void				destroy(Allocator& a, Renderer* renderer);

	virtual 				~Renderer() {};

	virtual void			init() = 0;
	virtual void			shutdown() = 0;

	/// Creates a new vertex buffer optimized for rendering static vertex data.
	/// @a vertices is the array containig @a count vertex data elements of the given @a format.
	virtual VertexBufferId	create_vertex_buffer(size_t count, VertexFormat format, const void* vertices) = 0;

	/// Updates the data associated with the given vertex buffer @a id.
	/// @a vertices is the array containig @a count vertex data elements of the format
	/// specified at the creation of the buffer.
	/// @note
	/// @a count and @a offset together do not have to exceed the number of elements specified
	/// at the creation of the buffer.	
	virtual void			update_vertex_buffer(VertexBufferId id, size_t offset, size_t count, const void* vertices) = 0;

	/// Destroys the @a id vertex buffer.
	virtual void			destroy_vertex_buffer(VertexBufferId id) = 0;

	/// Creates a new index buffer optimized for rendering static index buffers.
	/// @a indices is the array containing @a count index data elements.
	virtual IndexBufferId	create_index_buffer(size_t count, const void* indices) = 0;

	/// Destroys the @a id index buffer.
	virtual void			destroy_index_buffer(IndexBufferId id) = 0;

	virtual TextureId		create_texture(uint32_t width, uint32_t height, PixelFormat format, const void* data) = 0;
	virtual void			update_texture(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data) = 0;
	virtual void			destroy_texture(TextureId id) = 0;

	virtual ShaderId		create_shader(ShaderType type, const char* text) = 0;
	virtual void			destroy_shader(ShaderId id) = 0;

	virtual GPUProgramId	create_gpu_program(ShaderId vertex, ShaderId pixel) = 0;
	virtual void			destroy_gpu_program(GPUProgramId id) = 0;

	virtual UniformId		create_uniform(const char* name, UniformType type) = 0;
	virtual void			destroy_uniform(UniformId id) = 0;

	virtual RenderTargetId	create_render_target(uint16_t width, uint16_t height, RenderTargetFormat format) = 0;
	virtual void			destroy_render_target(RenderTargetId id) = 0;

	///	Tasks to perform before a frame is rendered.
	virtual void			frame() = 0;

	virtual void 			draw_lines(const float* vertices, const float* colors, uint32_t count) = 0;

	inline void set_state(uint64_t flags)
	{
		m_render_context.set_state(flags);
	}

	inline void set_pose(const Mat4& pose)
	{
		m_render_context.set_pose(pose);
	}

	inline void set_program(GPUProgramId program)
	{
		m_render_context.set_program(program);
	}

	inline void set_vertex_buffer(VertexBufferId vb)
	{
		m_render_context.set_vertex_buffer(vb);
	}

	inline void set_index_buffer(IndexBufferId ib)
	{
		m_render_context.set_index_buffer(ib);
	}

	inline void set_layer_render_target(uint8_t layer, RenderTargetId target)
	{
		m_render_context.set_layer_render_target(layer, target);
	}

	inline void set_layer_clear(uint8_t layer, uint8_t flags, const Color4& color, float depth)
	{
		m_render_context.set_layer_clear(layer, flags, color, depth);
	}

	inline void set_layer_view(uint8_t layer, const Mat4& view)
	{
		 m_render_context.set_layer_view(layer, view);
	}

	inline void set_layer_projection(uint8_t layer, const Mat4& projection)
	{
		m_render_context.set_layer_projection(layer, projection);
	}

	inline void set_layer_viewport(uint8_t layer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		m_render_context.set_layer_viewport(layer, x, y, width, height);
	}

	inline void set_layer_scissor(uint8_t layer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		m_render_context.set_layer_scissor(layer, x, y, width, height);
	}

	inline void commit(uint8_t layer)
	{
		m_render_context.commit(layer);
	}

protected:

	RenderContext m_render_context;
};

} // namespace crown
