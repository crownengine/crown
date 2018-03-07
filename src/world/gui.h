/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/types.h"
#include "resource/types.h"
#include "world/types.h"
#include <bgfx/bgfx.h>

#include "world/shader_manager.h"
#include "core/math/matrix4x4.h"
#include "world/material.h"
#include "device/pipeline.h"

namespace crown
{
struct GuiBuffer
{
	ShaderManager* _shader_manager;
	u32 _num_vertices;
	u32 _num_indices;
	bgfx::VertexDecl _pos_tex_col;
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;

	GuiBuffer(ShaderManager& sm)
		: _shader_manager(&sm)
		, _num_vertices(0)
		, _num_indices(0)
	{
	}

	void* vertex_buffer_end()
	{
		return tvb.data + _num_vertices*24;
	}

	void* index_buffer_end()
	{
		return tib.data + _num_indices*2;
	}

	void create()
	{
		_pos_tex_col.begin()
			.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true)
			.add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
			.end()
			;
	}

	void reset()
	{
		_num_vertices = 0;
		_num_indices = 0;

		bgfx::allocTransientVertexBuffer(&tvb, 4096, _pos_tex_col);
		bgfx::allocTransientIndexBuffer(&tib, 6144);
	}

	void submit(u32 num_vertices, u32 num_indices, const Matrix4x4& world)
	{
		bgfx::setVertexBuffer(0, &tvb, _num_vertices, num_vertices);
		bgfx::setIndexBuffer(&tib, _num_indices, num_indices);
		bgfx::setTransform(to_float_ptr(world));

		_shader_manager->submit(StringId32("gui"), VIEW_GUI);

		_num_vertices += num_vertices;
		_num_indices += num_indices;
	}

	void submit_with_material(u32 num_vertices, u32 num_indices, const Matrix4x4& world, ResourceManager& rm, Material* material)
	{
		bgfx::setVertexBuffer(0, &tvb, _num_vertices, num_vertices);
		bgfx::setIndexBuffer(&tib, _num_indices, num_indices);
		bgfx::setTransform(to_float_ptr(world));

		material->bind(rm, *_shader_manager, VIEW_GUI);

		_num_vertices += num_vertices;
		_num_indices += num_indices;
	}
};

/// Immediate mode Gui.
///
/// @ingroup World
struct Gui
{
	struct VertexData
	{
		Vector3 pos;
		Vector2 uv;
		u32 col;
	};

	struct IndexData
	{
		u16 a;
		u16 b;
	};

	u32 _marker;
	GuiBuffer* _buffer;
	ResourceManager* _resource_manager;
	ShaderManager* _shader_manager;
	MaterialManager* _material_manager;
	Matrix4x4 _world;

	///
	Gui(GuiBuffer& gb, ResourceManager& rm, ShaderManager& sm, MaterialManager& mm);

	///
	~Gui();

	///
	void move(const Vector2& pos);

	///
	void triangle_3d(const Vector3& a, const Vector3& b, const Vector3& c, const Color4& color);

	///
	void triangle(const Vector2& a, const Vector2& b, const Vector2& c, const Color4& color);

	///
	void rect_3d(const Vector3& pos, const Vector2& size, const Color4& color);

	///
	void rect(const Vector2& pos, const Vector2& size, const Color4& color);

	///
	void image_3d(const Vector3& pos, const Vector2& size, StringId64 material, const Color4& color);

	///
	void image(const Vector2& pos, const Vector2& size, StringId64 material, const Color4& color);

	///
	void image_uv_3d(const Vector3& pos, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color);

	///
	void image_uv(const Vector2& pos, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color);

	///
	void text_3d(const Vector3& pos, u32 font_size, const char* str, StringId64 font, StringId64 material, const Color4& color);

	///
	void text(const Vector2& pos, u32 font_size, const char* str, StringId64 font, StringId64 material, const Color4& color);
};

} // namespace crown
