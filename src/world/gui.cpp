/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"
#include "core/memory/memory.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/utf8.h"
#include "resource/font_resource.h"
#include "resource/material_resource.h"
#include "resource/resource_manager.h"
#include "world/gui.h"
#include "world/material_manager.h"
#include "world/shader_manager.h"
#include <bgfx/bgfx.h>

namespace crown
{
static inline u32 depth_u32(f32 depth)
{
	return u32(depth * 100.0f * 1000.0f);
}

GuiBuffer::GuiBuffer(ShaderManager &sm)
	: _shader_manager(&sm)
	, _num_vertices(0)
	, _num_indices(0)
	, _vertex_buffer()
	, _index_buffer()
{
}

void *GuiBuffer::vertex_buffer_end()
{
	return _vertex_buffer.data + _num_vertices*24;
}

void *GuiBuffer::index_buffer_end()
{
	return _index_buffer.data + _num_indices*2;
}

void GuiBuffer::create()
{
	_pos_tex_col.begin();
	_pos_tex_col.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float);
	_pos_tex_col.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true);
	_pos_tex_col.add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true);
	_pos_tex_col.end();
}

void GuiBuffer::reset()
{
	_num_vertices = 0;
	_num_indices = 0;

	bgfx::allocTransientVertexBuffer(&_vertex_buffer, 4096, _pos_tex_col);
	bgfx::allocTransientIndexBuffer(&_index_buffer, 6144);
}

void GuiBuffer::submit(u32 num_vertices, u32 num_indices, const Matrix4x4 &world, ShaderData &shader, u8 view, u32 depth)
{
	bgfx::setVertexBuffer(0, &_vertex_buffer, _num_vertices, num_vertices);
	bgfx::setIndexBuffer(&_index_buffer, _num_indices, num_indices);
	bgfx::setTransform(to_float_ptr(world));

	bgfx::setState(shader.state);
	bgfx::submit(view, shader.program, depth);

	_num_vertices += num_vertices;
	_num_indices += num_indices;
}

void GuiBuffer::submit_with_material(u32 num_vertices, u32 num_indices, const Matrix4x4 &world, u8 view, u32 depth, Material *material)
{
	bgfx::setVertexBuffer(0, &_vertex_buffer, _num_vertices, num_vertices);
	bgfx::setIndexBuffer(&_index_buffer, _num_indices, num_indices);
	bgfx::setTransform(to_float_ptr(world));

	material->bind(view, depth);

	_num_vertices += num_vertices;
	_num_indices += num_indices;
}

Gui::Gui(GuiBuffer &gb
	, ResourceManager &rm
	, ShaderManager &sm
	, MaterialManager &mm
	, ShaderData *shader
	, u8 view
	)
	: _marker(DEBUG_GUI_MARKER)
	, _buffer(&gb)
	, _resource_manager(&rm)
	, _shader_manager(&sm)
	, _material_manager(&mm)
	, _world(MATRIX4X4_IDENTITY)
	, _gui_shader(shader)
	, _view(view)
{
	_node.next = NULL;
	_node.prev = NULL;
}

Gui::~Gui()
{
	_marker = 0;
}

void Gui::move(const Vector2 &pos)
{
	set_translation(_world, vector3(pos.x, pos.y, 0));
}

void Gui::triangle_3d(const Matrix4x4 &local_pose, const Vector3 &a, const Vector3 &b, const Vector3 &c, const Color4 &color, f32 depth)
{
	VertexData *vd = (VertexData *)_buffer->vertex_buffer_end();
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

	u16 *inds = (u16 *)_buffer->index_buffer_end();
	inds[0] = 0;
	inds[1] = 1;
	inds[2] = 2;

	_buffer->submit(3, 3, local_pose*_world, *_gui_shader, _view, depth_u32(depth));
}

void Gui::triangle(const Vector2 &a, const Vector2 &b, const Vector2 &c, const Color4 &color, f32 depth)
{
	triangle_3d(MATRIX4X4_IDENTITY
		, vector3(a.x, a.y, 0.0f)
		, vector3(b.x, b.y, 0.0f)
		, vector3(c.x, c.y, 0.0f)
		, color
		, depth
		);
}

void Gui::rect_3d(const Matrix4x4 &local_pose, const Vector3 &pos, const Vector2 &size, const Color4 &color, f32 depth)
{
	VertexData *vd = (VertexData *)_buffer->vertex_buffer_end();
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

	u16 *inds = (u16 *)_buffer->index_buffer_end();
	inds[0] = 0;
	inds[1] = 1;
	inds[2] = 2;
	inds[3] = 0;
	inds[4] = 2;
	inds[5] = 3;

	_buffer->submit(4, 6, local_pose*_world, *_gui_shader, _view, depth_u32(depth));
}

void Gui::rect(const Vector3 &pos, const Vector2 &size, const Color4 &color)
{
	rect_3d(MATRIX4X4_IDENTITY
		, vector3(pos.x, pos.y, 0.0f)
		, size
		, color
		, pos.z
		);
}

void Gui::image_3d_uv(const Matrix4x4 &local_pose, const Vector3 &pos, const Vector2 &size, const Vector2 &uv0, const Vector2 &uv1, StringId64 material, const Color4 &color, f32 depth)
{
	VertexData *vd = (VertexData *)_buffer->vertex_buffer_end();
	vd[0].pos.x = pos.x;
	vd[0].pos.y = pos.y;
	vd[0].pos.z = pos.z;
	vd[0].uv.x  = uv0.x;
	vd[0].uv.y  = uv1.y;
	vd[0].col   = to_abgr(color);

	vd[1].pos.x = pos.x + size.x;
	vd[1].pos.y = pos.y;
	vd[1].pos.z = pos.z;
	vd[1].uv.x  = uv1.x;
	vd[1].uv.y  = uv1.y;
	vd[1].col   = to_abgr(color);

	vd[2].pos.x = pos.x + size.x;
	vd[2].pos.y = pos.y + size.y;
	vd[2].pos.z = pos.z;
	vd[2].uv.x  = uv1.x;
	vd[2].uv.y  = uv0.y;
	vd[2].col   = to_abgr(color);

	vd[3].pos.x = pos.x;
	vd[3].pos.y = pos.y + size.y;
	vd[3].pos.z = pos.z;
	vd[3].uv.x  = uv0.x;
	vd[3].uv.y  = uv0.y;
	vd[3].col   = to_abgr(color);

	u16 *inds = (u16 *)_buffer->index_buffer_end();
	inds[0] = 0;
	inds[1] = 1;
	inds[2] = 2;
	inds[3] = 0;
	inds[4] = 2;
	inds[5] = 3;

	const MaterialResource *mr = (MaterialResource *)_resource_manager->get(RESOURCE_TYPE_MATERIAL, material);
	_material_manager->create_material(mr);
	_buffer->submit_with_material(4
		, 6
		, local_pose*_world
		, _view
		, depth_u32(depth)
		, _material_manager->get(mr)
		);
}

void Gui::image_uv(const Vector3 &pos, const Vector2 &size, const Vector2 &uv0, const Vector2 &uv1, StringId64 material, const Color4 &color)
{
	image_3d_uv(MATRIX4X4_IDENTITY
		, vector3(pos.x, pos.y, 0.0f)
		, size
		, uv0
		, uv1
		, material
		, color
		, pos.z
		);
}

void Gui::image_3d(const Matrix4x4 &local_pose, const Vector3 &pos, const Vector2 &size, StringId64 material, const Color4 &color, f32 depth)
{
	image_3d_uv(local_pose
		, pos
		, size
		, VECTOR2_ZERO
		, VECTOR2_ONE
		, material
		, color
		, depth
		);
}

void Gui::image(const Vector3 &pos, const Vector2 &size, StringId64 material, const Color4 &color)
{
	image_3d(MATRIX4X4_IDENTITY
		, vector3(pos.x, pos.y, 0.0f)
		, size
		, material
		, color
		, pos.z
		);
}

void Gui::text_3d(const Matrix4x4 &local_pose, const Vector3 &pos, u32 font_size, const char *str, StringId64 font, StringId64 material, const Color4 &color, f32 depth)
{
	const MaterialResource *mr = (MaterialResource *)_resource_manager->get(RESOURCE_TYPE_MATERIAL, material);
	_material_manager->create_material(mr);

	const FontResource *fr = (FontResource *)_resource_manager->get(RESOURCE_TYPE_FONT, font);
	const f32 scale = (f32)font_size / (f32)fr->font_size;

	f32 pen_x;
	f32 pen_y;
	f32 pen_advance_x = 0.0f;
	f32 pen_advance_y = 0.0f;

	VertexData *vd = (VertexData *)_buffer->vertex_buffer_end();
	u16 *id = (u16 *)_buffer->index_buffer_end();

	const u32 len = strlen32(str);
	u32 num_vertices = 0;
	u32 num_indices = 0;
	for (u32 i = 0; i < len; ++i) {
		switch (str[i]) {
		case '\n':
			pen_advance_x = 0.0f;
			pen_advance_y -= scale*fr->font_size;
			continue;

		case '\t':
			pen_advance_x += scale*font_size*4;
			continue;
		}

		u32 state = 0;
		u32 code_point = 0;
		if (utf8::decode(&state, &code_point, str[i]) == UTF8_ACCEPT) {
			const GlyphData deffault_glyph = {};
			const GlyphData *glyph = font_resource::glyph(fr, code_point, &deffault_glyph);

			const f32 baseline = glyph->height - glyph->y_offset;

			// Set pen position
			pen_x = pos.x + scale*glyph->x_offset;
			pen_y = pos.y - scale*baseline;

			// Position coords
			const f32 x0 = (pen_x + pen_advance_x);
			const f32 y0 = (pen_y + pen_advance_y);
			const f32 x1 = (pen_x + pen_advance_x + scale*glyph->width);
			const f32 y1 = (pen_y + pen_advance_y + scale*glyph->height);

			// Texture coords
			const f32 u0 = glyph->x / fr->texture_size;
			const f32 v1 = glyph->y / fr->texture_size; // Upper-left char corner
			const f32 u1 = glyph->width  / fr->texture_size + u0;
			const f32 v0 = glyph->height / fr->texture_size + v1; // Bottom-left char corner

			// Fill vertex buffer
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
			id[0] = num_vertices + 0;
			id[1] = num_vertices + 1;
			id[2] = num_vertices + 2;
			id[3] = num_vertices + 0;
			id[4] = num_vertices + 2;
			id[5] = num_vertices + 3;

			// Advance pen position
			pen_advance_x += scale*glyph->x_advance;

			vd += 4;
			id += 6;
			num_vertices += 4;
			num_indices  += 6;
		}
	}

	_buffer->submit_with_material(num_vertices
		, num_indices
		, local_pose*_world
		, _view
		, depth_u32(depth)
		, _material_manager->get(mr)
		);
}

void Gui::text(const Vector3 &pos, u32 font_size, const char *str, StringId64 font, StringId64 material, const Color4 &color)
{
	text_3d(MATRIX4X4_IDENTITY
		, vector3(pos.x, pos.y, 0.0f)
		, font_size
		, str
		, font
		, material
		, color
		, pos.z
		);
}

Material *Gui::material(ResourceId material_resource)
{
	const MaterialResource *mr = (MaterialResource *)_resource_manager->get(RESOURCE_TYPE_MATERIAL, material_resource);
	return _material_manager->get(mr);
}

namespace gui
{
	Gui *create_screen_gui(Allocator &allocator
		, GuiBuffer &buffer
		, ResourceManager &resource_manager
		, ShaderManager &shader_manager
		, MaterialManager &material_manager
		, ShaderData *shader
		)
	{
		Gui *gui = CE_NEW(allocator, Gui)(buffer
			, resource_manager
			, shader_manager
			, material_manager
			, shader
			, VIEW_SCREEN_GUI
			);
		return gui;
	}

	Gui *create_world_gui(Allocator &allocator
		, GuiBuffer &buffer
		, ResourceManager &resource_manager
		, ShaderManager &shader_manager
		, MaterialManager &material_manager
		, ShaderData *shader
		)
	{
		Gui *gui = CE_NEW(allocator, Gui)(buffer
			, resource_manager
			, shader_manager
			, material_manager
			, shader
			, VIEW_WORLD_GUI
			);
		return gui;
	}

} // namespace gui

} // namespace crown
