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

#include "Types.h"
#include "TextureResource.h"
#include "Resource.h"
#include "Vec3.h"
#include "Color4.h"
#include "Material.h"
#include "Texture.h"

namespace crown
{

/// Max texture layers supported by the material.
/// @note The maximum number of usable layers depends on the graphic card/Renderer config.
const uint32_t MAX_TEXTURE_LAYERS = 8;

class Bundle;
class Allocator;

/// A material describes the visual properties of a surface.
/// It is primarly intended for rendering purposes but can
/// also be used to drive other types of systems such as sounds or physics.
class MaterialResource
{
public:

	static void*		load(Allocator& allocator, Bundle& bundle, ResourceId id);
	static void			unload(Allocator& allocator, void* resource);
	static void			online(void* resource);
	static void			offline(void* resource);

private:

	Color4			m_ambient;
	Color4			m_diffuse;
	Color4			m_specular;
	Color4			m_emission;
	int32_t			m_shininess;

	bool			m_lighting					: 1; // Lighting enabled
	bool			m_texturing					: 1; // Texturing enabled
	bool			m_backface_culling			: 1; // Backface-culling enabled
	bool			m_separate_specular_color 	: 1; // Separate specular color enabled
	bool			m_depth_test				: 1; // Depth test enabled
	bool			m_depth_write				: 1; // Depth write enabled
	bool			m_rescale_normals			: 1; // Auto normal rescaling enabled
	bool			m_blending					: 1; // Blending enabled
	bool			m_color_write				: 1; // Writing into color buffer enabled
	bool			m_fog						: 1; // Fog enabled
	bool			m_alpha_test				: 1; // Alpha test enabled
	bool			m_point_sprite				: 1; // Point sprite enabled

	ShadingType		m_shading_type;
	PolygonMode		m_polygon_mode;
	FrontFace		m_front_face;

	CompareFunction	m_depth_func;

	FogMode 		m_fog_mode;
	float			m_fog_density;
	float			m_fog_start;
	float			m_fog_end;
	Color4			m_fog_color;

	CompareFunction	m_alpha_func;
	float			m_alpha_ref;

	float			m_point_size;
	float			m_point_size_min;
	float			m_point_size_max;

	BlendEquation	m_blend_equation;
	BlendFunction	m_blend_src;
	BlendFunction	m_blend_dst;
	Color4			m_blend_color;

	// A material can contain up to MAX_TEXTURE_LAYERS texture layers.
	// However, the maximum number of texture layers actually usable is Renderer-dependent.
	ResourceId		m_textures[MAX_TEXTURE_LAYERS];
};

} // namespace crown

