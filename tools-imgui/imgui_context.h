/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include <bgfx/bgfx.h>
#include <imgui.h>
#include <iconfontheaders/icons_kenney.h>
#include <iconfontheaders/icons_font_awesome.h>

namespace crown
{
void imgui_create(float _fontSize = 18.0f, bx::AllocatorI* _allocator = NULL);
void imgui_destroy();
void imgui_begin_frame(uint8_t view_id, u16 width, u16 height);
void imgui_end_frame();

} // namespace crown

/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

namespace ImGui
{
#define IMGUI_FLAGS_NONE        UINT8_C(0x00)
#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)

	// Helper function for passing bgfx::TextureHandle to ImGui::Image.
	inline void Image(bgfx::TextureHandle _handle
		, uint8_t _flags
		, uint8_t _mip
		, const ImVec2& _size
		, const ImVec2& _uv0       = ImVec2(0.0f, 0.0f)
		, const ImVec2& _uv1       = ImVec2(1.0f, 1.0f)
		, const ImVec4& _tintCol   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
		, const ImVec4& _borderCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
		)
	{
		union { struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; ImTextureID ptr; } texture;
		texture.s.handle = _handle;
		texture.s.flags  = _flags;
		texture.s.mip    = _mip;
		Image(texture.ptr, _size, _uv0, _uv1, _tintCol, _borderCol);
	}

	// Helper function for passing bgfx::TextureHandle to ImGui::Image.
	inline void Image(bgfx::TextureHandle _handle
		, const ImVec2& _size
		, const ImVec2& _uv0       = ImVec2(0.0f, 0.0f)
		, const ImVec2& _uv1       = ImVec2(1.0f, 1.0f)
		, const ImVec4& _tintCol   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
		, const ImVec4& _borderCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
		)
	{
		Image(_handle, IMGUI_FLAGS_ALPHA_BLEND, 0, _size, _uv0, _uv1, _tintCol, _borderCol);
	}

	// Helper function for passing bgfx::TextureHandle to ImGui::ImageButton.
	inline bool ImageButton(bgfx::TextureHandle _handle
		, uint8_t _flags
		, uint8_t _mip
		, const ImVec2& _size
		, const ImVec2& _uv0     = ImVec2(0.0f, 0.0f)
		, const ImVec2& _uv1     = ImVec2(1.0f, 1.0f)
		, int _framePadding      = -1
		, const ImVec4& _bgCol   = ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
		, const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
		)
	{
		union { struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; ImTextureID ptr; } texture;
		texture.s.handle = _handle;
		texture.s.flags  = _flags;
		texture.s.mip    = _mip;
		return ImageButton(texture.ptr, _size, _uv0, _uv1, _framePadding, _bgCol, _tintCol);
	}

	// Helper function for passing bgfx::TextureHandle to ImGui::ImageButton.
	inline bool ImageButton(bgfx::TextureHandle _handle
		, const ImVec2& _size
		, const ImVec2& _uv0     = ImVec2(0.0f, 0.0f)
		, const ImVec2& _uv1     = ImVec2(1.0f, 1.0f)
		, int _framePadding      = -1
		, const ImVec4& _bgCol   = ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
		, const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
		)
	{
		return ImageButton(_handle, IMGUI_FLAGS_ALPHA_BLEND, 0, _size, _uv0, _uv1, _framePadding, _bgCol, _tintCol);
	}

	inline void NextLine()
	{
		SetCursorPosY(GetCursorPosY() + GetTextLineHeightWithSpacing() );
	}

	inline bool MouseOverArea()
	{
		return false
			|| ImGui::IsAnyItemHovered()
			|| ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)
			;
	}

} // namespace ImGui
