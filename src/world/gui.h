/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/list.h"
#include "core/math/types.h"
#include "device/pipeline.h"
#include "resource/shader_resource.h"
#include "resource/types.h"
#include "world/material.h"
#include "world/shader_manager.h"
#include "world/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct GuiBuffer
{
	ShaderManager *_shader_manager;
	u32 _num_vertices;
	u32 _num_indices;
	bgfx::VertexLayout _pos_tex_col;
	bgfx::TransientVertexBuffer _vertex_buffer;
	bgfx::TransientIndexBuffer _index_buffer;

	///
	explicit GuiBuffer(ShaderManager &sm);

	///
	void *vertex_buffer_end();

	///
	void *index_buffer_end();

	///
	void create();

	///
	void reset();

	///
	void submit(u32 num_vertices, u32 num_indices, const Matrix4x4 &world, ShaderData &shader, u8 view, u32 depth);

	///
	void submit_with_material(u32 num_vertices, u32 num_indices, const Matrix4x4 &world, u8 view, u32 depth, Material *material);
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
	GuiBuffer *_buffer;
	ResourceManager *_resource_manager;
	ShaderManager *_shader_manager;
	MaterialManager *_material_manager;
	Matrix4x4 _world;
	ShaderData *_gui_shader;
	u8 _view;
	ListNode _node;

	///
	Gui(GuiBuffer &gb
		, ResourceManager &rm
		, ShaderManager &sm
		, MaterialManager &mm
		, ShaderData *shader
		, u8 view
		);

	///
	~Gui();

	///
	void move(const Vector2 &pos);

	///
	void triangle_3d(const Matrix4x4 &local_pose, const Vector3 &a, const Vector3 &b, const Vector3 &c, const Color4 &color = COLOR4_WHITE, f32 depth = 0.0f);

	///
	void triangle(const Vector2 &a, const Vector2 &b, const Vector2 &c, const Color4 &color = COLOR4_WHITE, f32 depth = 0.0f);

	///
	void rect_3d(const Matrix4x4 &local_pose, const Vector3 &pos, const Vector2 &size, const Color4 &color = COLOR4_WHITE, f32 depth = 0.0f);

	///
	void rect(const Vector3 &pos, const Vector2 &size, const Color4 &color = COLOR4_WHITE);

	///
	void image_3d(const Matrix4x4 &local_pose, const Vector3 &pos, const Vector2 &size, StringId64 material, const Color4 &color = COLOR4_WHITE, f32 depth = 0.0f);

	///
	void image(const Vector3 &pos, const Vector2 &size, StringId64 material, const Color4 &color = COLOR4_WHITE);

	///
	void image_3d_uv(const Matrix4x4 &local_pose, const Vector3 &pos, const Vector2 &size, const Vector2 &uv0, const Vector2 &uv1, StringId64 material, const Color4 &color = COLOR4_WHITE, f32 depth = 0.0f);

	///
	void image_uv(const Vector3 &pos, const Vector2 &size, const Vector2 &uv0, const Vector2 &uv1, StringId64 material, const Color4 &color = COLOR4_WHITE);

	///
	void text_3d(const Matrix4x4 &local_pose, const Vector3 &pos, u32 font_size, const char *str, StringId64 font, StringId64 material, const Color4 &color = COLOR4_WHITE, f32 depth = 0.0f);

	///
	void text(const Vector3 &pos, u32 font_size, const char *str, StringId64 font, StringId64 material, const Color4 &color = COLOR4_WHITE);

	///
	Vector2 text_extents(const u32 font_size, const char *str, StringId64 font);

	/// Returns the material @a material_resource.
	Material *material(ResourceId material_resource);
};

namespace gui
{
	Gui *create_screen_gui(Allocator &allocator
		, GuiBuffer &buffer
		, ResourceManager &resource_manager
		, ShaderManager &shader_manager
		, MaterialManager &material_manager
		, ShaderData *shader
		);

	Gui *create_world_gui(Allocator &allocator
		, GuiBuffer &buffer
		, ResourceManager &resource_manager
		, ShaderManager &shader_manager
		, MaterialManager &material_manager
		, ShaderData *shader
		);

} // namespace gui

} // namespace crown
