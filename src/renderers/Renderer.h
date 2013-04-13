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
#include "Point2.h"
#include "Material.h"
#include "Texture.h"

namespace crown
{

class Image;
class Rect;
class VertexBuffer;
class IndexBuffer;
class OcclusionQuery;
class TextureResource;

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

enum DrawMode
{
	DM_NO_DRAW		= 0,
	DM_BORDER		= 1,
	DM_FILL			= 2
};

class Renderer
{

public:

	static Renderer* CreateRenderer();
	static void	DestroyRenderer(Renderer* renderer);

	/**
		Constructor.
	*/
	Renderer() {}

	/**
		Destructor.
	*/
	virtual ~Renderer() {}

	/**
		Tasks to perform before a frame is rendered.
	*/
	virtual void begin_frame() = 0;

	/**
		Tasks to perform after a frame is rendered.
	*/
	virtual void end_frame() = 0;

	/**
		Sets the clearing color of the framebuffer.
	*/
	virtual void set_clear_color(const Color4& color) = 0;

	/**
		Sets the viewport.
	@note
		The vieport area is defined by a Rectangle containing
		window-space pixel coordinates for lower left corner and
		width and height of the viewport rectangle.
	@param absArea
		The viewport rectangle.
	*/
	virtual void set_viewport(const Rect& absArea) = 0;

	/**
		Specifies material parameters.
	@note
		This method specifies the values for ambient, diffuse,
		specular, emission and shininess parameters.
	@param ambient
		The ambient reflectance
	@param diffuse
		The diffuse reflectance
	@param specular
		The specular reflectance
	@param emission
		The emitted light int32_tensity
	@param shininess
		The specular exponent
	*/
	virtual void set_material_params(const Color4& ambient, const Color4& diffuse, const Color4& specular,
					const Color4& emission, int32_t shininess) = 0;

	/**
		Sets whether lighting is enabled.
	@param lighting
		Whether lighting is enabled or not
	*/
	virtual void set_lighting(bool lighting) = 0;

	/**
		Sets the global ambient light.
	@note
		The global ambient light defines the ambient
		int32_tensity for the entire scene.
	@param color
		The ambient int32_tensity
	*/
	virtual void set_ambient_light(const Color4& color) = 0;

	/**
		Set whether the given texture unit is enabled.
	@param unit
		The texture unit
	@param texturing
		Whether texturing is enabled or not
	*/
	virtual void set_texturing(uint32_t unit, bool texturing) = 0;


	/**
		Sets the texture mode for the given texture unit.
	@note
		The texture mode specifies how the texture
		values are int32_terpreted when a fragment is
		textured.
	@param unit
		The texture unit
	@param mode
		The texture mode
	@param
		The blend color. This parameter means only when
		mode == TM_BLEND
	*/
	virtual void set_texture_mode(uint32_t unit, TextureMode mode, const Color4& blendColor) = 0;

	/**
		Sets the texture wrap parameter for the given texture unit.
	@param unit
		The texture unit
	@param wrap
		The wrap parameter
	*/
	virtual void set_texture_wrap(uint32_t unit, TextureWrap wrap) = 0;

	/**
		Sets the filter for the given texture unit.
	@param unit
		The texture unit
	@filter
		The filter
	*/
	virtual void set_texture_filter(uint32_t unit, TextureFilter filter) = 0;

	virtual void set_light(uint32_t light, bool active) = 0;
	virtual void set_light_params(uint32_t light, LightType type, const Vec3& position) = 0;
	virtual void set_light_color(uint32_t light, const Color4& ambient, const Color4& diffuse, const Color4& specular) = 0;
	virtual void set_light_attenuation(uint32_t light, float constant, float linear, float quadratic) = 0;

	/**
		Sets whether backface-culling is enabled.
	@param culling
		Whether backface-culling is enabled or not
	*/
	virtual void set_backface_culling(bool culling) = 0;

	/**
		Sets whether separate specular color is enabled.	
	@param separate
		Whether separate specular color is enabled or not
	*/
	virtual void set_separate_specular_color(bool separate) = 0;

	/**
		Sets whether depth test is enabled.
	@param test
		Whether depth test is enabled or not
	*/
	virtual void set_depth_test(bool test) = 0;

	/**
		Sets whether writing to depth buffer is enabled.
	@param write
		Whether writing is enabled or not.
	*/
	virtual void set_depth_write(bool write) = 0;

	/**
		Sets the depth function to use when testing depth values.
	@param func
		The depth function
	*/
	virtual void set_depth_func(CompareFunction func) = 0;

	/**
		Sets whether normal vectors are scaled after transformations
		and before lighting.
	@note
		If the modelview matrix scales space uniformly, this
		has the effect of restoring the trasformed normal to unit length.
	@param rescale
		Whether rescaling is enabled or not
	*/
	virtual void set_rescale_normals(bool rescale) = 0;

	/**
		Sets whether blending is enabled.
	@param blending
		Whether blending is enabled or not
	*/
	virtual void set_blending(bool blending) = 0;

	/**
		Specifies blending parameters.
	@note
		This method specifies the blend equation, the blend function
		for source and destination pixel values and a const blend color.
	@param equation
		The blend equation
	@param src
		The blend function for source pixel
	@param dst
		The blend function for destination pixel
	@param color
		The constant color used when src or dst are equal to BF_CONSTANT_COLOR
	*/
	virtual void set_blending_params(BlendEquation equation, BlendFunction src, BlendFunction dst, const Color4& color) = 0;

	/**
		Sets whether writing to color buffer is enabled.
	@param write
		Whether writing is enabled or not
	*/
	virtual void set_color_write(bool write) = 0;

	/**
		Sets whether fog is enabled.
	@param fog
		Whether fog is enabled or not
	*/
	virtual void set_fog(bool fog) = 0;

	/**
		Specifies fog parameters.
	@note
		This method specifies mode, int32_tensity, start
		and end position and the color of fog.
	*/ // TODO 
	virtual void set_fog_params(FogMode mode, float density, float start, float end, const Color4& color) = 0;

	virtual void set_alpha_test(bool test) = 0;

	virtual void set_alpha_params(CompareFunction func, float ref) = 0;

	virtual void set_shading_type(ShadingType type) = 0;

	virtual void set_polygon_mode(PolygonMode mode) = 0;

	virtual void set_front_face(FrontFace face) = 0;

	virtual void set_viewport_params(int32_t x, int32_t y, int32_t width, int32_t height) = 0;

	virtual void set_scissor(bool scissor) = 0;

	virtual void set_scissor_params(int32_t x, int32_t y, int32_t width, int32_t height) = 0;

	virtual void set_point_sprite(bool sprite) = 0;

	virtual void set_point_size(float size) = 0;

	virtual void set_point_params(float min, float max) = 0;

	//! Sets the texture to use in the specified layer
	virtual void set_texture(uint32_t layer, TextureId texture) = 0;

	//! Returns the current matrix
	virtual Mat4 get_matrix(MatrixType type) const = 0;

	//! Loads the current matrix
	virtual void set_matrix(MatrixType type, const Mat4& matrix) = 0;

	//! Pushes the current model matrix
	virtual void push_matrix() = 0;

	//! Pops the previously pushed model matrix
	virtual void pop_matrix() = 0;

	//! Selects the active matrix
	virtual void select_matrix(MatrixType type) = 0;

	//! Sets the scissor box
	virtual void set_scissor_box(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

	//! Gets the scissor box params
	virtual void get_scissor_box(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) = 0;

	virtual void render_vertex_index_buffer(const VertexBuffer* vertices, const IndexBuffer* indices) = 0;

	virtual void render_point_buffer(const VertexBuffer* buffer) = 0;

	//! Draws a Rectangle
	virtual void draw_rectangle(const Point2& position, const Point2& dimensions, int32_t drawMode,
														 const Color4& borderColor = Color4::BLACK, const Color4& fillColor = Color4::WHITE) = 0;

	virtual void add_debug_line(const Vec3& start, const Vec3& end, const Color4& color) = 0;
	virtual void draw_debug_lines() = 0;

	virtual TextureId	load_texture(TextureResource* texture) = 0;
	virtual void		unload_texture(TextureResource* texture) = 0;
	virtual TextureId	reload_texture(TextureResource* old_texture, TextureResource* new_texture) = 0;
};

} // namespace crown
