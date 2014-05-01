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
#include "OsThread.h"
#include "OS.h"
#include "IdTable.h"

namespace crown
{

extern ShaderUniform::Enum name_to_stock_uniform(const char* uniform);
class RendererImplementation;

/// @defgroup Graphics Graphics

/// Renderer interface.
///
/// @ingroup Graphics
class Renderer
{
public:

	Renderer(Allocator& a);
	~Renderer();

	void init_impl();
	void shutdown_impl();
	void render_impl();

	void create_vertex_buffer_impl(VertexBufferId id, size_t size, const void* data, VertexFormat::Enum format);
	void create_dynamic_vertex_buffer_impl(VertexBufferId id, size_t size);
	void update_vertex_buffer_impl(VertexBufferId id, size_t offset, size_t size, const void* data);
	void destroy_vertex_buffer_impl(VertexBufferId id);

	void create_index_buffer_impl(IndexBufferId id, size_t size, const void* data);
	void create_dynamic_index_buffer_impl(IndexBufferId id, size_t size);
	void update_index_buffer_impl(IndexBufferId id, size_t offset, size_t size, const void* data);
	void destroy_index_buffer_impl(IndexBufferId id);

	void create_texture_impl(TextureId id, uint32_t width, uint32_t height, PixelFormat::Enum format, const void* data);
	void update_texture_impl(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data);
	void destroy_texture_impl(TextureId id);

	void create_shader_impl(ShaderId id, ShaderType::Enum type, const char* text);
	void destroy_shader_impl(ShaderId id);

	void create_gpu_program_impl(GPUProgramId id, ShaderId vertex, ShaderId pixel);
	void destroy_gpu_program_impl(GPUProgramId id);

	void create_uniform_impl(UniformId id, const char* name, UniformType::Enum type, uint8_t num);
	void update_uniform_impl(UniformId id, size_t size, const void* data);
	void destroy_uniform_impl(UniformId id);

	void create_render_target_impl(RenderTargetId id, uint16_t width, uint16_t height, PixelFormat::Enum format, uint32_t flags);
	void destroy_render_target_impl(RenderTargetId id);

	/// Initializes the renderer.
	/// Should be the first call to the renderer.
	void init()
	{
		m_should_run = true;
		// m_thread.start(render_thread, this);

		m_submit->m_commands.write(CommandType::INIT_RENDERER);
		frame();

		m_submit->m_transient_vb = create_transient_vertex_buffer(CE_TRANSIENT_VERTEX_BUFFER_SIZE);
		m_submit->m_transient_ib = create_transient_index_buffer(CE_TRANSIENT_INDEX_BUFFER_SIZE);
		frame();
		m_submit->m_transient_vb = create_transient_vertex_buffer(CE_TRANSIENT_VERTEX_BUFFER_SIZE);
		m_submit->m_transient_ib = create_transient_index_buffer(CE_TRANSIENT_INDEX_BUFFER_SIZE);
		frame();
	}

	/// Shutdowns the renderer.
	/// Should be the last call to the renderer.
	void shutdown()
	{
		if (m_should_run)
		{
			destroy_transient_index_buffer(m_submit->m_transient_ib);
			destroy_transient_vertex_buffer(m_submit->m_transient_vb);
			frame();

			destroy_transient_index_buffer(m_submit->m_transient_ib);
			destroy_transient_vertex_buffer(m_submit->m_transient_vb);
			frame();

			m_submit->m_commands.write(CommandType::SHUTDOWN_RENDERER);
			frame();

			//m_thread.stop();		
		}
	}

	/// Creates a new vertex buffer optimized for rendering static vertex data.
	/// @a data is the array containig @a size bytes of vertex data in the given @a format.
	VertexBufferId create_vertex_buffer(size_t size, const void* data, VertexFormat::Enum format)
	{
		const VertexBufferId id = m_vertex_buffers.create();

		m_submit->m_commands.write(CommandType::CREATE_VERTEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(size);
		m_submit->m_commands.write(data);
		m_submit->m_commands.write(format);

		return id;
	}

	/// Creates a new vertex buffer optimized for renderering dynamic vertex data.
	/// This function only allocates storage for @a size bytes of vertex data.
	/// use Renderer::update_vertex_buffer() to fill the buffer with actual data.
	VertexBufferId create_dynamic_vertex_buffer(size_t size)
	{
		const VertexBufferId id = m_vertex_buffers.create();

		m_submit->m_commands.write(CommandType::CREATE_DYNAMIC_VERTEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(size);

		return id;
	}

	/// Creates a new transient vertex buffer with storage for exactly @a size bytes.
	/// Transient vertex buffers are useful when you have to render highly dynamic vertex data, such as font glyphs.
	/// @note
	/// This call is tipically only performed by the backend to allocate a common large generic transient buffer
	/// that can be used to carve out smaller transient buffers by calling Renderer::reserve_transient_vertex_buffer().
	TransientVertexBuffer* create_transient_vertex_buffer(size_t size)
	{
		VertexBufferId vb = create_dynamic_vertex_buffer(size);
		TransientVertexBuffer* tvb = NULL;

		tvb = (TransientVertexBuffer*) default_allocator().allocate(sizeof(TransientVertexBuffer) + size);
		tvb->vb = vb;
		tvb->start_vertex = 0;
		tvb->data = (char*) &tvb[1]; // Nice trick
		tvb->size = size;

		return tvb;
	}

	/// Reserves a portion of the common transient vertex buffer for exactly @a num vertices
	/// of the given @a format.
	/// @note
	/// The returned @a tvb transient buffer is valid for one frame only!
	void reserve_transient_vertex_buffer(TransientVertexBuffer* tvb, uint32_t num, VertexFormat::Enum format)
	{
		CE_ASSERT(tvb != NULL, "Transient buffer must be != NULL");

		TransientVertexBuffer& tvb_shared = *m_submit->m_transient_vb;

		const uint32_t offset = m_submit->reserve_transient_vertex_buffer(num, format);

		tvb->vb = tvb_shared.vb;
		tvb->data = &tvb_shared.data[offset];
		tvb->start_vertex = offset / Vertex::bytes_per_vertex(format);
		tvb->size = Vertex::bytes_per_vertex(format) * num;
		tvb->format = format;
	}

	/// Updates the vertex buffer data of @a id with @a size bytes of vertex @a data starting
	/// at the given @a offset.
	void update_vertex_buffer(VertexBufferId id, size_t offset, size_t size, const void* data)
	{
		CE_ASSERT(m_vertex_buffers.has(id), "Vertex buffer does not exist");

		m_submit->m_commands.write(CommandType::UPDATE_VERTEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(offset);
		m_submit->m_commands.write(size);
		m_submit->m_commands.write(data);	
	}

	/// Destroys the given vertex buffer @a id.
	void destroy_vertex_buffer(VertexBufferId id)
	{
		CE_ASSERT(m_vertex_buffers.has(id), "Vertex buffer does not exist");

		m_submit->m_commands.write(CommandType::DESTROY_VERTEX_BUFFER);
		m_submit->m_commands.write(id);
	}

	/// Destroys the given @a tvb transient buffer
	void destroy_transient_vertex_buffer(TransientVertexBuffer* tvb)
	{
		CE_ASSERT(tvb != NULL, "Transient buffer must be != NULL");

		destroy_vertex_buffer(tvb->vb);
		default_allocator().deallocate(tvb);
	}

	/// Creates a new index buffer optimized for rendering static index buffers.
	/// @a data is the array containing @a size bytes of index data.
	IndexBufferId create_index_buffer(size_t size, const void* data)
	{
		const IndexBufferId id = m_index_buffers.create();

		m_submit->m_commands.write(CommandType::CREATE_INDEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(size);
		m_submit->m_commands.write(data);

		return id;
	}

	/// Creates a new index buffer optimized for rendering dynamic index buffers.
	/// This function only allocates storage for @a size bytes of index data.
	/// use Renderer::update_index_buffer() to fill the buffer with actual data.
	IndexBufferId create_dynamic_index_buffer(size_t size)
	{
		const IndexBufferId id = m_index_buffers.create();

		m_submit->m_commands.write(CommandType::CREATE_DYNAMIC_INDEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(size);

		return id;
	}

	/// Creates a new transient index buffer with storage for exactly @a size bytes.
	/// Transient index buffers are useful when you have to render highly dynamic index data, such as font glyphs.
	/// @note
	/// This call is tipically only performed by the backend to allocate a common large generic transient buffer
	/// that can be used to carve out smaller transient buffers by calling Renderer::reserve_transient_index_buffer().
	TransientIndexBuffer* create_transient_index_buffer(size_t size)
	{
		IndexBufferId ib = create_dynamic_index_buffer(size);
		TransientIndexBuffer* tib = NULL;

		tib = (TransientIndexBuffer*) default_allocator().allocate(sizeof(TransientIndexBuffer) + size);
		tib->ib = ib;
		tib->start_index = 0;
		tib->data = (char*) &tib[1]; // Same as before
		tib->size = size;

		return tib;
	}

	/// Reserves a portion of the common transient index buffer for exactly @a num indices.
	/// @note
	/// The returned @a tvb transient buffer is valid for one frame only!
	void reserve_transient_index_buffer(TransientIndexBuffer* tib, uint32_t num)
	{
		CE_ASSERT(tib != NULL, "Transient buffer must be != NULL");

		TransientIndexBuffer& tib_shared = *m_submit->m_transient_ib;

		const uint32_t offset = m_submit->reserve_transient_index_buffer(num);

		tib->ib = tib_shared.ib;
		tib->data = &tib_shared.data[offset];
		tib->start_index = offset / sizeof(uint16_t);
		tib->size = sizeof(uint16_t) * num;
	}

	/// Updates the index buffer data of @a id with @a size bytes of index @data starting
	/// at the given @a offset.
	void update_index_buffer(IndexBufferId id, size_t offset, size_t size, const void* data)
	{
		CE_ASSERT(m_index_buffers.has(id), "Index buffer does not exist");

		m_submit->m_commands.write(CommandType::UPDATE_INDEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(offset);
		m_submit->m_commands.write(size);
		m_submit->m_commands.write(data);
	}

	/// Destroys the @a id index buffer.
	void destroy_index_buffer(IndexBufferId id)
	{
		CE_ASSERT(m_index_buffers.has(id), "Index buffer does not exist");

		m_submit->m_commands.write(CommandType::DESTROY_INDEX_BUFFER);
		m_submit->m_commands.write(id);
	}

	/// Destroys the given @a tvi transient buffer
	void destroy_transient_index_buffer(TransientIndexBuffer* tib)
	{
		CE_ASSERT(tib != NULL, "Transient buffer must be != NULL");

		destroy_index_buffer(tib->ib);
		default_allocator().deallocate(tib);
	}

	/// Creates a new texture of size @a width and @height.
	/// The array @a data should contain @a width * @a height elements of the given @a format.
	TextureId create_texture(uint32_t width, uint32_t height, PixelFormat::Enum format, const void* data)
	{
		const TextureId id = m_textures.create();

		m_submit->m_commands.write(CommandType::CREATE_TEXTURE);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(width);
		m_submit->m_commands.write(height);
		m_submit->m_commands.write(format);
		m_submit->m_commands.write(data);

		return id;
	}

	/// Updates the pixels of texture @a id at region defined by @a x, @a y, @a width, @a height.
	/// The array @a data should contain @a width * @a height elements of the format originally specified
	/// to Renderer::create_texture()
	void update_texture(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
	{
		CE_ASSERT(m_textures.has(id), "Texture does not exist");

		m_submit->m_commands.write(CommandType::UPDATE_TEXTURE);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(x);
		m_submit->m_commands.write(y);
		m_submit->m_commands.write(width);
		m_submit->m_commands.write(height);
		m_submit->m_commands.write(data);
	}

	/// Destroys the texture @æ id.
	void destroy_texture(TextureId id)
	{
		CE_ASSERT(m_textures.has(id), "Texture does not exist");

		m_submit->m_commands.write(CommandType::DESTROY_TEXTURE);
		m_submit->m_commands.write(id);
	}

	/// Creates a new shader of the given @a type from the string @a text.
	ShaderId create_shader(ShaderType::Enum type, const char* text)
	{
		const ShaderId id = m_shaders.create();

		m_submit->m_commands.write(CommandType::CREATE_SHADER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(type);
		m_submit->m_commands.write(text);

		return id;
	}

	/// Destroy the shader @a id.
	void destroy_shader(ShaderId id)
	{
		CE_ASSERT(m_shaders.has(id), "Shader does not exist");

		m_submit->m_commands.write(CommandType::DESTROY_SHADER);
		m_submit->m_commands.write(id);
	}

	/// Creates a new gpu program from @a vertex shader and @a pixel shader.
	GPUProgramId create_gpu_program(ShaderId vertex, ShaderId pixel)
	{
		const GPUProgramId id = m_gpu_programs.create();

		m_submit->m_commands.write(CommandType::CREATE_GPU_PROGRAM);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(vertex);
		m_submit->m_commands.write(pixel);

		return id;
	}

	/// Destroys the gpu program @a id.
	void destroy_gpu_program(GPUProgramId id)
	{
		CE_ASSERT(m_gpu_programs.has(id), "GPU program does not exist");

		m_submit->m_commands.write(CommandType::DESTROY_GPU_PROGRAM);
		m_submit->m_commands.write(id);
	}

	/// Creates a new uniform with the given @a name, with storage for exactly
	/// @a num elements of the given @a type.
	UniformId create_uniform(const char* name, UniformType::Enum type, uint8_t num)
	{
		CE_ASSERT(name_to_stock_uniform(name) == ShaderUniform::COUNT, "Uniform name '%s' is a stock uniform.", name);

		const UniformId id = m_uniforms.create();
		size_t len = string::strlen(name);

		CE_ASSERT(len < CE_MAX_UNIFORM_NAME_LENGTH, "Max uniform name length is %d", CE_MAX_UNIFORM_NAME_LENGTH);

		m_submit->m_commands.write(CommandType::CREATE_UNIFORM);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(len);
		m_submit->m_commands.write(name, len);
		m_submit->m_commands.write(type);
		m_submit->m_commands.write(num);

		return id;
	}

	/// Destroys the uniform @a id.
	void destroy_uniform(UniformId id)
	{
		CE_ASSERT(m_uniforms.has(id), "Uniform does not exist");

		m_submit->m_commands.write(CommandType::DESTROY_UNIFORM);
		m_submit->m_commands.write(id);
	}

	/// Creates a new render target of size @a width and @a height with the given @a format.
	RenderTargetId create_render_target(uint16_t width, uint16_t height, PixelFormat::Enum format, uint32_t flags = 0)
	{
		const RenderTargetId id = m_render_targets.create();

		m_submit->m_commands.write(CommandType::CREATE_RENDER_TARGET);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(width);
		m_submit->m_commands.write(height);
		m_submit->m_commands.write(format);
		m_submit->m_commands.write(flags);

		return id;
	}

	/// Destroys the render target @a id.
	void destroy_render_target(RenderTargetId id)
	{
		CE_ASSERT(m_render_targets.has(id), "Render target does not exist");

		m_submit->m_commands.write(CommandType::DESTROY_RENDER_TARGET);
		m_submit->m_commands.write(id);
	}

	//-----------------------------------------------------------------------------
	void execute_commands(CommandBuffer& cmds)
	{
		bool end = false;

		do
		{
			CommandType::Enum command;
			cmds.read(command);

			switch (command)
			{
				case CommandType::INIT_RENDERER:
				{
					init_impl();
					m_is_initialized = true;
					break;
				}
				case CommandType::SHUTDOWN_RENDERER:
				{
					shutdown_impl();
					m_is_initialized = false;
					m_should_run = false;
					break;
				}
				case CommandType::CREATE_VERTEX_BUFFER:
				{
					VertexBufferId id;
					size_t size;
					const void* data;
					VertexFormat::Enum format;

					cmds.read(id);
					cmds.read(size);
					cmds.read(data);
					cmds.read(format);

					create_vertex_buffer_impl(id, size, data, format);
					break;
				}
				case CommandType::CREATE_DYNAMIC_VERTEX_BUFFER:
				{
					VertexBufferId id;
					size_t size;

					cmds.read(id);
					cmds.read(size);

					create_dynamic_vertex_buffer_impl(id, size);
					break;
				}
				case CommandType::UPDATE_VERTEX_BUFFER:
				{
					VertexBufferId id;
					size_t offset;
					size_t size;
					void* data;

					cmds.read(id);
					cmds.read(offset);
					cmds.read(size);
					cmds.read(data);

					update_vertex_buffer_impl(id, offset, size, data);			
					break;
				}
				case CommandType::DESTROY_VERTEX_BUFFER:
				{
					VertexBufferId id;
					cmds.read(id);

					destroy_vertex_buffer_impl(id);
					break;
				}
				case CommandType::CREATE_INDEX_BUFFER:
				{
					IndexBufferId id;
					size_t size;
					void* data;

					cmds.read(id);
					cmds.read(size);
					cmds.read(data);

					create_index_buffer_impl(id, size, data);
					break;
				}
				case CommandType::CREATE_DYNAMIC_INDEX_BUFFER:
				{
					IndexBufferId id;
					size_t size;

					cmds.read(id);
					cmds.read(size);

					create_dynamic_index_buffer_impl(id, size);
					break;
				}
				case CommandType::UPDATE_INDEX_BUFFER:
				{
					IndexBufferId id;
					size_t offset;
					size_t size;
					void* data;

					cmds.read(id);
					cmds.read(offset);
					cmds.read(size);
					cmds.read(data);

					update_index_buffer_impl(id, offset, size, data);
					break;
				}
				case CommandType::DESTROY_INDEX_BUFFER:
				{
					IndexBufferId id;
					cmds.read(id);

					destroy_index_buffer_impl(id);
					break;
				}
				case CommandType::CREATE_TEXTURE:
				{
					TextureId id;
					uint32_t width;
					uint32_t height;
					PixelFormat::Enum format;
					void* data;

					cmds.read(id);
					cmds.read(width);
					cmds.read(height);
					cmds.read(format);
					cmds.read(data);

					create_texture_impl(id, width, height, format, data);
					break;
				}
				case CommandType::UPDATE_TEXTURE:
				{
					TextureId id;
					uint32_t x;
					uint32_t y;
					uint32_t width;
					uint32_t height;
					void* data;

					cmds.read(id);
					cmds.read(x);
					cmds.read(y);
					cmds.read(width);
					cmds.read(height);
					cmds.read(data);

					update_texture_impl(id, x, y, width, height, data);
					break;
				}
				case CommandType::DESTROY_TEXTURE:
				{
					TextureId id;
					cmds.read(id);

					destroy_texture_impl(id);
					break;
				}
				case CommandType::CREATE_SHADER:
				{
					ShaderId id;
					ShaderType::Enum type;
					char* text;

					cmds.read(id);
					cmds.read(type);
					cmds.read(text);

					create_shader_impl(id, type, text);
					break;
				}
				case CommandType::DESTROY_SHADER:
				{
					ShaderId id;
					cmds.read(id);

					destroy_shader_impl(id);
					break;
				}
				case CommandType::CREATE_GPU_PROGRAM:
				{
					GPUProgramId id;
					ShaderId vertex;
					ShaderId pixel;

					cmds.read(id);
					cmds.read(vertex);
					cmds.read(pixel);

					create_gpu_program_impl(id, vertex, pixel);
					break;
				}
				case CommandType::DESTROY_GPU_PROGRAM:
				{
					GPUProgramId id;
					cmds.read(id);

					destroy_gpu_program_impl(id);
					break;
				}
				case CommandType::CREATE_UNIFORM:
				{
					UniformId id;
					size_t len;
					char name[CE_MAX_UNIFORM_NAME_LENGTH];
					UniformType::Enum type;
					uint8_t num;

					cmds.read(id);
					cmds.read(len);
					cmds.read(name, len);
					name[len] = '\0';
					cmds.read(type);
					cmds.read(num);

					create_uniform_impl(id, name, type, num);

					break;
				}
				case CommandType::DESTROY_UNIFORM:
				{
					UniformId id;
					cmds.read(id);

					destroy_uniform_impl(id);

					break;
				}
				case CommandType::CREATE_RENDER_TARGET:
				{
					RenderTargetId id;
					uint16_t width;
					uint16_t height;
					PixelFormat::Enum format;
					uint32_t flags;

					cmds.read(id);
					cmds.read(width);
					cmds.read(height);
					cmds.read(format);
					cmds.read(flags);

					create_render_target_impl(id, width, height, format, flags);
					break;
				}
				case CommandType::DESTROY_RENDER_TARGET:
				{
					RenderTargetId id;
					cmds.read(id);

					destroy_render_target_impl(id);
					break;
				}
				case CommandType::END:
				{
					end = true;
					break;
				}
				default:
				{
					CE_ASSERT(false, "Oops, unknown command");
					break;
				}
			}
		}
		while (!end);

		cmds.clear();
	}

	void update_uniforms(ConstantBuffer& cbuf, uint32_t begin, uint32_t end)
	{
		cbuf.reset(begin);

		while (cbuf.position() < end)
		{
			UniformType::Enum type = (UniformType::Enum) cbuf.read();

			if (type == UniformType::END)
				break;

			UniformId id;
			uint32_t size;

			cbuf.read(&id, sizeof(UniformId));
			cbuf.read(&size, sizeof(uint32_t));
			const void* data = cbuf.read(size);

			update_uniform_impl(id, size, data);
		}
	}

	void set_state(uint64_t flags)
	{
		m_submit->set_state(flags);
	}

	void set_pose(const Matrix4x4& pose)
	{
		m_submit->set_pose(pose);
	}

	void set_program(GPUProgramId id)
	{
		CE_ASSERT(m_gpu_programs.has(id), "GPU program does not exist");
		m_submit->set_program(id);
	}

	void set_vertex_buffer(VertexBufferId id, uint32_t num_vertices = 0xFFFFFFFF)
	{
		CE_ASSERT(m_vertex_buffers.has(id), "Vertex buffer does not exist");
		m_submit->set_vertex_buffer(id, num_vertices);
	}

	void set_vertex_buffer(const TransientVertexBuffer& tvb, uint32_t num_vertices = 0xFFFFFFFF)
	{
		m_submit->set_vertex_buffer(tvb, num_vertices);
	}

	void set_index_buffer(IndexBufferId id, uint32_t start_index = 0, uint32_t num_indices = 0xFFFFFFFF)
	{
		CE_ASSERT(m_index_buffers.has(id), "Index buffer does not exist");
		m_submit->set_index_buffer(id, start_index, num_indices);
	}

	void set_index_buffer(const TransientIndexBuffer& tib, uint32_t num_indices = 0xFFFFFFFF)
	{
		m_submit->set_index_buffer(tib, num_indices);
	}

	void set_uniform(UniformId id, UniformType::Enum type, const void* value, uint8_t num)
	{
		CE_ASSERT(m_uniforms.has(id), "Uniform does not exist");
		CE_ASSERT_NOT_NULL(value);
		m_submit->set_uniform(id, type, value, num);
	}

	void set_texture(uint8_t unit, UniformId sampler_uniform, TextureId texture, uint32_t flags)
	{
		CE_ASSERT(m_uniforms.has(sampler_uniform), "Uniform does not exist");
		CE_ASSERT(m_textures.has(texture), "Texture does not exist");
		m_submit->set_texture(unit, sampler_uniform, texture, flags);
	}

	void set_texture(uint8_t unit, UniformId sampler_uniform, RenderTargetId texture, uint8_t attachment, uint32_t texture_flags)
	{
		CE_ASSERT(m_uniforms.has(sampler_uniform), "Uniform does not exist");
		CE_ASSERT(m_render_targets.has(texture), "Render target does not exist");
		m_submit->set_texture(unit, sampler_uniform, texture, attachment, texture_flags);
	}

	void set_layer_render_target(uint8_t layer, RenderTargetId id)
	{
		CE_ASSERT(m_render_targets.has(id), "Render target does not exist");
		m_submit->set_layer_render_target(layer, id);
	}

	void set_layer_clear(uint8_t layer, uint8_t flags, const Color4& color, float depth)
	{
		m_submit->set_layer_clear(layer, flags, color, depth);
	}

	void set_layer_view(uint8_t layer, const Matrix4x4& view)
	{
		m_submit->set_layer_view(layer, view);
	}

	void set_layer_projection(uint8_t layer, const Matrix4x4& projection)
	{
		m_submit->set_layer_projection(layer, projection);
	}

	void set_layer_viewport(uint8_t layer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		m_submit->set_layer_viewport(layer, x, y, width, height);
	}

	void set_layer_scissor(uint8_t layer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		m_submit->set_layer_scissor(layer, x, y, width, height);
	}

	void commit(uint8_t layer)
	{
		m_submit->commit(layer);
	}

	static int32_t render_thread(void* thiz)
	{
		// Renderer* renderer = (Renderer*)thiz;
		// while (renderer->m_should_run)
		// {
		// 	renderer->render_all();
		// }

		return 0;
	}

	void swap_contexts()
	{
		// Ensure CommandType::END at the end of submit command buffer
		m_submit->push();

		RenderContext* temp = m_submit;
		m_submit = m_draw;
		m_draw = temp;
	}

	void frame()
	{
		// Signal main thread finished updating
		// m_render_wait.post();
		// m_main_wait.wait();
		render_all();
	}

	// Do all the processing needed to render a frame
	void render_all()
	{
		// Waits for main thread to finish update
		// m_render_wait.wait();

		swap_contexts();

		execute_commands(m_draw->m_commands);

		if (m_is_initialized)
		{
			render_impl();
		}

		// m_main_wait.post();
	}

protected:

	Allocator& m_allocator;
	RendererImplementation* m_impl;

	OsThread m_thread;
	Semaphore m_render_wait;
	Semaphore m_main_wait;

	RenderContext m_contexts[2];
	RenderContext* m_submit;
	RenderContext* m_draw;

	// Id tables
	IdTable<CE_MAX_VERTEX_BUFFERS> m_vertex_buffers;
	IdTable<CE_MAX_INDEX_BUFFERS> m_index_buffers;
	IdTable<CE_MAX_TEXTURES> m_textures;
	IdTable<CE_MAX_SHADERS> m_shaders;
	IdTable<CE_MAX_GPU_PROGRAMS> m_gpu_programs;
	IdTable<CE_MAX_UNIFORMS> m_uniforms;
	IdTable<CE_MAX_RENDER_TARGETS> m_render_targets;

	bool m_is_initialized;
	bool m_should_run;
};

} // namespace crown
