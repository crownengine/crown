/*
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

#include "Types.h"
#include "Color4.h"
#include "Mat4.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderBuffer.h"
#include "Texture.h"
#include "Material.h"
#include "PixelFormat.h"
#include "VertexFormat.h"
#include "IdTable.h"

namespace crown
{

enum MatrixType
{
	MT_VIEW			= 0,
	MT_MODEL		= 1,
	MT_PROJECTION	= 2,
	MT_TEXTURE		= 3,
	MT_COLOR		= 4,
	MT_COUNT
};

enum LightType
{
	LT_POINT		= 0,
	LT_DIRECTION	= 1,
	LT_SPOT			= 2
};

enum ShaderAttrib
{
	SA_VERTEX			= 0,
	SA_COORDS			= 1,
	SA_NORMAL			= 2,
	SA_DIFFUSE_MAP		= 3,
	SA_DETAIL_MAP		= 4,
	SA_NORMAL_MAP		= 5,

	SA_COUNT
};

// Keep in sync with ShaderAttrib
const char* const SHADER_ATTRIB_NAMES[] =
{
	"vertex",			// 0
	"coords",			// 1
	"normal",			// 2
	"diffuse_map",		// 3
	"detail_map",		// 4
	"normal_map"		// 5
};

typedef Id VertexBufferId;
typedef Id IndexBufferId;
typedef Id RenderBufferId;
typedef Id TextureId;
typedef Id VertexShaderId;
typedef Id PixelShaderId;
typedef Id GPUProgramId;

class VertexBuffer;
class IndexBuffer;
class Vec2;
class Vec3;
class Vec4;
class Mat4;

class Renderer
{
public:

	virtual 				~Renderer() {};

	virtual void			init() = 0;
	virtual void			shutdown() = 0;

	/// Creates a new vertex buffer optimized for rendering static vertex data.
	/// @vertices is the array containig @count vertex data elements of the given @format.
	virtual VertexBufferId	create_vertex_buffer(size_t count, VertexFormat format, const void* vertices) = 0;

	/// Creates a new vertex buffer optimized for rendering dynamic vertex data.
	/// @vertices is the array containig @count vertex data elements of the given @format.
	virtual VertexBufferId	create_dynamic_vertex_buffer(size_t count, VertexFormat format, const void* vertices) = 0;

	/// Updates the data associated with the given vertex buffer @id.
	/// @vertices is the array containig @count vertex data elements of the format
	/// specified at the creation of the buffer.
	/// @note
	/// @count and @offset together do not have to exceed the number of elements specified
	/// at the creation of the buffer.	
	virtual void			update_vertex_buffer(VertexBufferId id, size_t offset, size_t count, const void* vertices) = 0;

	/// Destroys the @id vertex buffer.
	virtual void			destroy_vertex_buffer(VertexBufferId id) = 0;

	/// Creates a new index buffer optimized for rendering static index buffers.
	/// @indices is the array containing @count index data elements.
	virtual IndexBufferId	create_index_buffer(size_t count, const void* indices) = 0;

	/// Destroys the @id index buffer.
	virtual void			destroy_index_buffer(IndexBufferId id) = 0;

	virtual TextureId		create_texture(uint32_t width, uint32_t height, PixelFormat format, const void* data) = 0;
	virtual void			update_texture(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data) = 0;
	virtual void			destroy_texture(TextureId id) = 0;

	virtual VertexShaderId	create_vertex_shader(const char* program) = 0;
	virtual void			destroy_vertex_shader(VertexShaderId id) = 0;

	virtual PixelShaderId 	create_pixel_shader(const char* program) = 0;
	virtual void			destroy_pixel_shader(PixelShaderId id) = 0;

	virtual GPUProgramId	create_gpu_program(VertexShaderId vs, PixelShaderId ps) = 0;
	virtual void			destroy_gpu_program(GPUProgramId id) = 0;

	virtual void			set_gpu_program_bool_uniform(GPUProgramId id, const char* name, bool value) = 0;
	virtual void			set_gpu_program_int_uniform(GPUProgramId id, const char* name, int value) = 0;

	virtual void			set_gpu_program_vec2_uniform(GPUProgramId id, const char* name, const Vec2& value) = 0;
	virtual void			set_gpu_program_vec3_uniform(GPUProgramId id, const char* name, const Vec3& value) = 0;
	virtual void			set_gpu_program_vec4_uniform(GPUProgramId id, const char* name, const Vec4& value) = 0;

	virtual void			set_gpu_porgram_mat3_uniform(GPUProgramId id, const char* name, const Mat3& value) = 0;
	virtual void			set_gpu_program_mat4_uniform(GPUProgramId id, const char* name, const Mat4& value) = 0;

	virtual void			set_gpu_program_sampler_uniform(GPUProgramId id, const char* name, uint32_t value) = 0;

	virtual void			bind_gpu_program(GPUProgramId id) const = 0;

	//virtual RenderBufferId	create_render_buffer(uint32_t width, uint32_t height, PixelFormat format) = 0;
	//virtual void			destroy_render_buffer(RenderBufferId id) = 0;

	///	Tasks to perform before a frame is rendered.
	virtual void			begin_frame() = 0;

	/// Tasks to perform after a frame is rendered.
	virtual void			end_frame() = 0;

	/// Sets the clearing color of the framebuffer.
	virtual void			set_clear_color(const Color4& color) = 0;

	/// Sets the global ambient light @color.
	virtual void			set_ambient_light(const Color4& color) = 0;

	//! Sets the texture to use in the specified layer
	virtual void			bind_texture(uint32_t layer, TextureId texture) = 0;

	/// Set whether the given texture @unit is enabled.
	virtual void			set_texturing(uint32_t unit, bool texturing) = 0;

	/// Sets the texture @mode for the given texture @unit.
	virtual void			set_texture_mode(uint32_t unit, TextureMode mode, const Color4& blendColor) = 0;

	/// Sets the texture @wrap parameter for the given texture @unit.
	virtual void			set_texture_wrap(uint32_t unit, TextureWrap wrap) = 0;

	/// Sets the @filter for the given texture @unit.
	virtual void			set_texture_filter(uint32_t unit, TextureFilter filter) = 0;

	/// Sets whether backface-culling is enabled.
	virtual void			set_backface_culling(bool culling) = 0;

	/// Sets whether depth test is enabled.
	virtual void			set_depth_test(bool test) = 0;

	/// Sets whether writing to depth buffer is enabled.
	virtual void			set_depth_write(bool write) = 0;

	/// Sets the depth function to use when testing depth values.
	virtual void			set_depth_func(CompareFunction func) = 0;

	/// Sets whether blending is enabled.
	virtual void			set_blending(bool blending) = 0;

	///	Specifies blending parameters.
	/// @note
	///	This method specifies the blend equation, the blend function
	///	for source and destination pixel values and a const blend color.
	virtual void			set_blending_params(BlendEquation equation, BlendFunction src, BlendFunction dst, const Color4& color) = 0;

	/// Sets whether writing to color buffer is enabled.
	virtual void			set_color_write(bool write) = 0;

	virtual void			set_shading_type(ShadingType type) = 0;
				
	virtual void 			set_front_face(FrontFace face) = 0;
				
	virtual void 			set_viewport_params(int32_t x, int32_t y, int32_t width, int32_t height) = 0;
	virtual void 			get_viewport_params(int32_t& x, int32_t& y, int32_t& width, int32_t& height) = 0;
				
	virtual void 			set_scissor(bool scissor) = 0;
	virtual void 			set_scissor_params(int32_t x, int32_t y, int32_t width, int32_t height) = 0;
	virtual void 			get_scissor_params(int32_t& x, int32_t& y, int32_t& width, int32_t& height) = 0;
				
	virtual void 			set_point_sprite(bool sprite) = 0;
	virtual void 			set_point_size(float size) = 0;
	virtual void 			set_point_params(float min, float max) = 0;
				
	virtual Mat4 			get_matrix(MatrixType type) const = 0;
	virtual void 			set_matrix(MatrixType type, const Mat4& matrix) = 0;
				
	virtual void 			bind_vertex_buffer(VertexBufferId vb) const = 0;
	//virtual void bind_render_buffer(RenderBufferId id) const = 0;
				
	virtual void 			draw_triangles(IndexBufferId id) const = 0;
				
	virtual void 			draw_lines(const float* vertices, const float* colors, uint32_t count) = 0;
};

} // namespace crown
