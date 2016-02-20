/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "world_types.h"
#include "resource_types.h"
#include <bgfx/bgfx.h>

namespace crown
{
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

	ResourceManager* _resource_manager;
	ShaderManager* _shader_manager;
	MaterialManager* _material_manager;
	u16 _width;
	u16 _height;
	Matrix4x4 _projection;
	Matrix4x4 _world;
	bgfx::VertexDecl _pos_tex_col;

	Gui(ResourceManager& rm, ShaderManager& sm, MaterialManager& mm, u16 width, u16 height);

	Vector2 resolution() const;
	void move(const Vector2& pos);

	Vector2 screen_to_gui(const Vector2& pos);

	void triangle(const Vector3& a, const Vector3& b, const Vector3& c, StringId64 material, const Color4& color);

	void rect3d(const Vector3& pos, const Vector2& size, StringId64 material, const Color4& color);
	void rect(const Vector2& pos, const Vector2& size, StringId64 material, const Color4& color);
	void image_uv3d(const Vector3& pos, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color);
	void image_uv(const Vector2& pos, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color);
	void image3d(const Vector3& pos, const Vector2& size, StringId64 material, const Color4& color);
	void image(const Vector2& pos, const Vector2& size, StringId64 material, const Color4& color);
	void text3d(const Vector3& pos, u32 font_size, const char* str, StringId64 font, StringId64 material, const Color4& color);
	void text(const Vector2& pos, u32 font_size, const char* str, StringId64 font, StringId64 material, const Color4& color);
};

} // namespace crown
