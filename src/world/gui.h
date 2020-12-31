/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/list.h"
#include "core/math/types.h"
#include "device/pipeline.h"
#include "resource/types.h"
#include "world/material.h"
#include "world/shader_manager.h"
#include "world/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct GuiBuffer
{
	ShaderManager* _shader_manager;
	u32 _num_vertices;
	u32 _num_indices;
	bgfx::VertexLayout _pos_tex_col;
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;

	///
	GuiBuffer(ShaderManager& sm);

	///
	void* vertex_buffer_end();

	///
	void* index_buffer_end();

	///
	void create();

	///
	void reset();

	///
	void submit(u32 num_vertices, u32 num_indices, const Matrix4x4& world);

	///
	void submit_with_material(u32 num_vertices, u32 num_indices, const Matrix4x4& world, ResourceManager& rm, Material* material);
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
	ListNode _node;

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
