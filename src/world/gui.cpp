/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "color4.h"
#include "font_resource.h"
#include "gui.h"
#include "material_manager.h"
#include "material_resource.h"
#include "matrix4x4.h"
#include "resource_manager.h"
#include "string_utils.h"
#include "utf8.h"
#include "vector2.h"
#include "vector3.h"
#include <bgfx/bgfx.h>

namespace crown
{
Gui::Gui(ResourceManager& rm, ShaderManager& sm, MaterialManager& mm, u16 width, u16 height)
	: _resource_manager(&rm)
	, _shader_manager(&sm)
	, _material_manager(&mm)
	, _width(width)
	, _height(height)
	, _projection(MATRIX4X4_IDENTITY)
	, _world(MATRIX4X4_IDENTITY)
{
	orthographic(_projection, 0, width, 0, height, -0.01f, 100.0f);

	_pos_tex_col
		.begin()
		.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true)
		.add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
		.end();
}

Vector2 Gui::resolution() const
{
	return vector2(_width, _height);
}

void Gui::move(const Vector2& pos)
{
	set_identity(_world);
	set_translation(_world, vector3(pos.x, pos.y, 0));
}

Vector2 Gui::screen_to_gui(const Vector2& pos)
{
	return vector2(pos.x, _height - pos.y);
}

void Gui::triangle(const Vector3& a, const Vector3& b, const Vector3& c, StringId64 material, const Color4& color)
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	bgfx::allocTransientVertexBuffer(&tvb, 3, _pos_tex_col);
	bgfx::allocTransientIndexBuffer(&tib, 3);

	VertexData* vd = (VertexData*)tvb.data;
	vd[0].pos.x = a.x;
	vd[0].pos.y = a.y;
	vd[0].pos.z = a.z;
	vd[0].uv.x  = 0.0f;
	vd[0].uv.y  = 0.0f;
	vd[0].col   = to_abgr(color);

	vd[1].pos.x = b.x;
	vd[1].pos.y = b.y;
	vd[1].pos.z = b.z;
	vd[1].uv.x  = 1.0f;
	vd[1].uv.y  = 0.0f;
	vd[1].col   = to_abgr(color);

	vd[2].pos.x = c.x;
	vd[2].pos.y = c.y;
	vd[2].pos.z = c.z;
	vd[2].uv.x  = 1.0f;
	vd[2].uv.y  = 1.0f;
	vd[2].col   = to_abgr(color);

	u16* inds = (u16*)tib.data;
	inds[0] = 0;
	inds[1] = 1;
	inds[2] = 2;
}

void Gui::rect3d(const Vector3& pos, const Vector2& size, StringId64 material, const Color4& color)
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	bgfx::allocTransientVertexBuffer(&tvb, 4, _pos_tex_col);
	bgfx::allocTransientIndexBuffer(&tib, 6);

	VertexData* vd = (VertexData*)tvb.data;
	vd[0].pos.x = pos.x;
	vd[0].pos.y = pos.y;
	vd[0].pos.z = pos.z;
	vd[0].uv.x  = 0.0f;
	vd[0].uv.y  = 1.0f;
	vd[0].col   = to_abgr(color);

	vd[1].pos.x = pos.x + size.x;
	vd[1].pos.y = pos.y;
	vd[1].pos.z = pos.z;
	vd[1].uv.x  = 1.0f;
	vd[1].uv.y  = 1.0f;
	vd[1].col   = to_abgr(color);

	vd[2].pos.x = pos.x + size.x;
	vd[2].pos.y = pos.y + size.y;
	vd[2].pos.z = pos.z;
	vd[2].uv.x  = 1.0f;
	vd[2].uv.y  = 0.0f;
	vd[2].col   = to_abgr(color);

	vd[3].pos.x = pos.x;
	vd[3].pos.y = pos.y + size.y;
	vd[3].pos.z = pos.z;
	vd[3].uv.x  = 0.0f;
	vd[3].uv.y  = 0.0f;
	vd[3].col   = to_abgr(color);

	u16* inds = (u16*)tib.data;
	inds[0] = 0;
	inds[1] = 1;
	inds[2] = 2;
	inds[3] = 0;
	inds[4] = 2;
	inds[5] = 3;

	bgfx::setVertexBuffer(&tvb);
	bgfx::setIndexBuffer(&tib);
	bgfx::setTransform(to_float_ptr(_projection));
	_material_manager->create_material(material);
	_material_manager->get(material)->bind(*_resource_manager, *_shader_manager, 2);
}

void Gui::rect(const Vector2& pos, const Vector2& size, StringId64 material, const Color4& color)
{
	rect3d(vector3(pos.x, pos.y, 0.0f), size, material, color);
}

void Gui::image_uv3d(const Vector3& pos, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color)
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	bgfx::allocTransientVertexBuffer(&tvb, 4, _pos_tex_col);
	bgfx::allocTransientIndexBuffer(&tib, 6);

	VertexData* vd = (VertexData*)tvb.data;
	vd[0].pos.x = pos.x;
	vd[0].pos.y = pos.y;
	vd[0].pos.z = pos.z;
	vd[0].uv.x  = 0.0f;
	vd[0].uv.y  = 1.0f;
	vd[0].col   = to_abgr(color);

	vd[1].pos.x = pos.x + size.x;
	vd[1].pos.y = pos.y;
	vd[1].pos.z = pos.z;
	vd[1].uv.x  = 1.0f;
	vd[1].uv.y  = 1.0f;
	vd[1].col   = to_abgr(color);

	vd[2].pos.x = pos.x + size.x;
	vd[2].pos.y = pos.y + size.y;
	vd[2].pos.z = pos.z;
	vd[2].uv.x  = 1.0f;
	vd[2].uv.y  = 0.0f;
	vd[2].col   = to_abgr(color);

	vd[3].pos.x = pos.x;
	vd[3].pos.y = pos.y + size.y;
	vd[3].pos.z = pos.z;
	vd[3].uv.x  = 0.0f;
	vd[3].uv.y  = 0.0f;
	vd[3].col   = to_abgr(color);

	u16* inds = (u16*)tib.data;
	inds[0] = 0;
	inds[1] = 1;
	inds[2] = 2;
	inds[3] = 0;
	inds[4] = 2;
	inds[5] = 3;

	bgfx::setVertexBuffer(&tvb);
	bgfx::setIndexBuffer(&tib);
}

void Gui::image_uv(const Vector2& pos, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color)
{
	image_uv3d(vector3(pos.x, pos.y, 0.0f), size, uv0, uv1, material, color);
}

void Gui::image3d(const Vector3& pos, const Vector2& size, StringId64 material, const Color4& color)
{
	image_uv3d(pos, size, VECTOR2_ZERO, VECTOR2_ONE, material, color);
}

void Gui::image(const Vector2& pos, const Vector2& size, StringId64 material, const Color4& color)
{
	image3d(vector3(pos.x, pos.y, 0.0f), size, material, color);
}

void Gui::text3d(const Vector3& pos, u32 font_size, const char* str, StringId64 font, StringId64 material, const Color4& color)
{
	const FontResource* fr = (FontResource*)_resource_manager->get(RESOURCE_TYPE_FONT, font);
	const f32 scale = (f32)font_size / (f32)fr->font_size;
	const u32 len = strlen32(str);

	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	bgfx::allocTransientVertexBuffer(&tvb, 4 * len, _pos_tex_col);
	bgfx::allocTransientIndexBuffer(&tib, 6 * len);

	u16 index = 0;
	f32 x_pen_advance = 0.0f;
	f32 y_pen_advance = 0.0f;

	Vector2 m_pen;

	u32 state = 0;
	u32 code_point = 0;
	for (u32 i = 0; i < len; i++)
	{
		switch (str[i])
		{
			case '\n':
			{
				x_pen_advance = 0.0f;
				y_pen_advance -= fr->font_size;
				continue;
			}
			case '\t':
			{
				x_pen_advance += font_size * 4;
				continue;
			}
		}

		if (utf8::decode(&state, &code_point, str[i]) == UTF8_ACCEPT)
		{
			const GlyphData& g = *font_resource::get_glyph(fr, code_point);

			const f32 baseline = g.height - g.y_offset;

			// Set pen position
			m_pen.x = pos.x + g.x_offset;
			m_pen.y = pos.y - baseline;

			// Position coords
			const f32 x0 = (m_pen.x + x_pen_advance) * scale;
			const f32 y0 = (m_pen.y + y_pen_advance) * scale;
			const f32 x1 = (m_pen.x + g.width + x_pen_advance) * scale;
			const f32 y1 = (m_pen.y + g.height + y_pen_advance) * scale;

			// Texture coords
			const f32 u0 = g.x / fr->texture_size;
			const f32 v1 = g.y / fr->texture_size; // Upper-left char corner
			const f32 u1 = u0 + g.width  / fr->texture_size;
			const f32 v0 = v1 + g.height / fr->texture_size; // Bottom-left char corner

			// Fill vertex buffer
			VertexData* vd = (VertexData*)&tvb.data[i*4*sizeof(VertexData)];
			vd[0].pos.x = x0;
			vd[0].pos.y = y0;
			vd[0].pos.z = pos.z;
			vd[0].uv.x  = u0;
			vd[0].uv.y  = v0;
			vd[0].col   = to_abgr(color);

			vd[1].pos.x = x1;
			vd[1].pos.y = y0;
			vd[1].pos.z = pos.z;
			vd[1].uv.x  = u1;
			vd[1].uv.y  = v0;
			vd[1].col   = to_abgr(color);

			vd[2].pos.x = x1;
			vd[2].pos.y = y1;
			vd[2].pos.z = pos.z;
			vd[2].uv.x  = u1;
			vd[2].uv.y  = v1;
			vd[2].col   = to_abgr(color);

			vd[3].pos.x = x0;
			vd[3].pos.y = y1;
			vd[3].pos.z = pos.z;
			vd[3].uv.x  = u0;
			vd[3].uv.y  = v1;
			vd[3].col   = to_abgr(color);

			// Fill index buffer
			u16* inds = (u16*)&tib.data[i*6*sizeof(u16)];
			inds[0] = index + 0;
			inds[1] = index + 1;
			inds[2] = index + 2;
			inds[3] = index + 0;
			inds[4] = index + 2;
			inds[5] = index + 3;

			// Advance pen position
			x_pen_advance += g.x_advance;

			index += 4;
		}
	}

	bgfx::setVertexBuffer(&tvb);
	bgfx::setIndexBuffer(&tib);
	bgfx::setTransform(to_float_ptr(_projection));
	_material_manager->create_material(material);
	_material_manager->get(material)->bind(*_resource_manager, *_shader_manager, 2);
}

void Gui::text(const Vector2& pos, u32 font_size, const char* str, StringId64 font, StringId64 material, const Color4& color)
{
	text3d(vector3(pos.x, pos.y, 0.0f), font_size, str, font, material, color);
}

} // namespace crown
