/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <bx/allocator.h>
#include <bx/math.h>
#include <bx/timer.h>
#include <imgui.h>

#include "data/roboto_regular.ttf.h"
#include "data/robotomono_regular.ttf.h"
#include "data/icons_kenney.ttf.h"
#include "data/icons_font_awesome.ttf.h"

#include "core/strings/string_id.h"
#include "core/strings/string_id.inl"
#include "device/device.h"
#include "device/input_device.h"
#include "device/input_types.h"
#include "device/pipeline.h"
#include "device/window.h"
#include "imgui_context.h"
#include "world/shader_manager.h"

using namespace crown;

// From bgfx_utils.h
inline bool checkAvailTransientBuffers(uint32_t _numVertices, const bgfx::VertexLayout& _layout, uint32_t _numIndices)
{
	return _numVertices == bgfx::getAvailTransientVertexBuffer(_numVertices, _layout)
		&& _numIndices  == bgfx::getAvailTransientIndexBuffer(_numIndices)
		;
}

struct FontRangeMerge
{
	const void* data;
	size_t      size;
	ImWchar     ranges[3];
};

static FontRangeMerge s_fontRangeMerge[] =
{
	{ s_iconsKenneyTtf,      sizeof(s_iconsKenneyTtf),      { ICON_MIN_KI, ICON_MAX_KI, 0 } },
	{ s_iconsFontAwesomeTtf, sizeof(s_iconsFontAwesomeTtf), { ICON_MIN_FA, ICON_MAX_FA, 0 } },
};

static int g_MouseCursors[ImGuiMouseCursor_COUNT] = { MouseCursor::ARROW /* Which is == 0 */ };

static void* memAlloc(size_t _size, void* _userData);
static void memFree(void* _ptr, void* _userData);

struct ImGuiContext
{
	void render(ImDrawData* _drawData)
	{
		const ImGuiIO& io = ImGui::GetIO();

		const float width  = io.DisplaySize.x;
		const float height = io.DisplaySize.y;

		bgfx::setViewName(m_viewId, "ImGui");
		bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);

		const bgfx::Caps* caps = bgfx::getCaps();
		float ortho[16];
		bx::mtxOrtho(ortho, 0.0f, width, height, 0.0f, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
		bgfx::setViewTransform(m_viewId, NULL, ortho);
		bgfx::setViewRect(m_viewId, 0, 0, uint16_t(width), uint16_t(height) );

		// Render command lists
		for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii)
		{
			bgfx::TransientVertexBuffer tvb;
			bgfx::TransientIndexBuffer tib;

			const ImDrawList* drawList = _drawData->CmdLists[ii];
			uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
			uint32_t numIndices  = (uint32_t)drawList->IdxBuffer.size();

			if (!checkAvailTransientBuffers(numVertices, m_layout, numIndices) )
			{
				// not enough space in transient buffer just quit drawing the rest...
				break;
			}

			bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_layout);
			bgfx::allocTransientIndexBuffer(&tib, numIndices);

			ImDrawVert* verts = (ImDrawVert*)tvb.data;
			bx::memCopy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert) );

			ImDrawIdx* indices = (ImDrawIdx*)tib.data;
			bx::memCopy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx) );

			uint32_t offset = 0;
			for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
			{
				if (cmd->UserCallback)
				{
					cmd->UserCallback(drawList, cmd);
				}
				else if (0 != cmd->ElemCount)
				{
					uint64_t state = 0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_A
						| BGFX_STATE_MSAA
						;

					bgfx::TextureHandle th = m_texture;
					StringId32 program = STRING_ID_32("ocornut_imgui", 0xde1bd0ed);

					if (NULL != cmd->TextureId)
					{
						union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
						state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
							? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
							: BGFX_STATE_NONE
							;
						th = texture.s.handle;
						if (0 != texture.s.mip)
						{
							const float lodEnabled[4] = { float(texture.s.mip), 1.0f, 0.0f, 0.0f };
							bgfx::setUniform(u_imageLodEnabled, lodEnabled);
							program = STRING_ID_32("imgui_image", 0xe2ec5960);
						}
					}
					else
					{
						state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
					}

					const uint16_t xx = uint16_t(bx::max(cmd->ClipRect.x, 0.0f) );
					const uint16_t yy = uint16_t(bx::max(cmd->ClipRect.y, 0.0f) );
					bgfx::setScissor(xx, yy
							, uint16_t(bx::min(cmd->ClipRect.z, 65535.0f)-xx)
							, uint16_t(bx::min(cmd->ClipRect.w, 65535.0f)-yy)
							);

					bgfx::setState(state);
					bgfx::setTexture(0, s_tex, th);
					bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
					bgfx::setIndexBuffer(&tib, offset, cmd->ElemCount);
					device()->_shader_manager->submit(program, VIEW_IMGUI, 0, state);
				}

				offset += cmd->ElemCount;
			}
		}
	}

	void create(float _fontSize, bx::AllocatorI* _allocator)
	{
		m_allocator = _allocator;

		if (NULL == _allocator)
		{
			static bx::DefaultAllocator allocator;
			m_allocator = &allocator;
		}

		m_viewId = VIEW_IMGUI;

		ImGui::SetAllocatorFunctions(memAlloc, memFree, NULL);

		m_imgui = ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2(1280.0f, 720.0f);
		io.DeltaTime   = 1.0f / 60.0f;
		io.IniFilename = NULL;

		setupStyle(true);

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
		io.KeyMap[ImGuiKey_Tab]        = KeyboardButton::TAB;
		io.KeyMap[ImGuiKey_LeftArrow]  = KeyboardButton::LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = KeyboardButton::RIGHT;
		io.KeyMap[ImGuiKey_UpArrow]    = KeyboardButton::UP;
		io.KeyMap[ImGuiKey_DownArrow]  = KeyboardButton::DOWN;
		io.KeyMap[ImGuiKey_PageUp]     = KeyboardButton::PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown]   = KeyboardButton::PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home]       = KeyboardButton::HOME;
		io.KeyMap[ImGuiKey_End]        = KeyboardButton::END;
		io.KeyMap[ImGuiKey_Insert]     = KeyboardButton::INS;
		io.KeyMap[ImGuiKey_Delete]     = KeyboardButton::DEL;
		io.KeyMap[ImGuiKey_Backspace]  = KeyboardButton::BACKSPACE;
		io.KeyMap[ImGuiKey_Space]      = KeyboardButton::SPACE;
		io.KeyMap[ImGuiKey_Enter]      = KeyboardButton::ENTER;
		io.KeyMap[ImGuiKey_Escape]     = KeyboardButton::ESCAPE;
		io.KeyMap[ImGuiKey_A]          = KeyboardButton::A;
		io.KeyMap[ImGuiKey_C]          = KeyboardButton::C;
		io.KeyMap[ImGuiKey_V]          = KeyboardButton::V;
		io.KeyMap[ImGuiKey_X]          = KeyboardButton::X;
		io.KeyMap[ImGuiKey_Y]          = KeyboardButton::Y;
		io.KeyMap[ImGuiKey_Z]          = KeyboardButton::Z;

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.IniFilename = "imgui.ini";

		u_imageLodEnabled = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);

		m_layout
			.begin()
			.add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
			.end();

		s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

		uint8_t* data;
		int32_t width;
		int32_t height;
		{
			ImFontConfig config;
			config.FontDataOwnedByAtlas = false;
			config.MergeMode = false;
//			config.MergeGlyphCenterV = true;

			const ImWchar* ranges = io.Fonts->GetGlyphRangesCyrillic();
			m_font[ImGui::Font::Regular] = io.Fonts->AddFontFromMemoryTTF( (void*)s_robotoRegularTtf,     sizeof(s_robotoRegularTtf),     _fontSize,      &config, ranges);
			m_font[ImGui::Font::Mono   ] = io.Fonts->AddFontFromMemoryTTF( (void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf), _fontSize-3.0f, &config, ranges);

			config.MergeMode = true;
			config.DstFont   = m_font[ImGui::Font::Regular];

			for (uint32_t ii = 0; ii < BX_COUNTOF(s_fontRangeMerge); ++ii)
			{
				const FontRangeMerge& frm = s_fontRangeMerge[ii];

				io.Fonts->AddFontFromMemoryTTF( (void*)frm.data
						, (int)frm.size
						, _fontSize-3.0f
						, &config
						, frm.ranges
						);
			}
		}

		io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

		m_texture = bgfx::createTexture2D(
			  (uint16_t)width
			, (uint16_t)height
			, false
			, 1
			, bgfx::TextureFormat::BGRA8
			, 0
			, bgfx::copy(data, width*height*4)
			);
	}

	void destroy()
	{
		ImGui::DestroyContext(m_imgui);

		bgfx::destroy(s_tex);
		bgfx::destroy(m_texture);

		bgfx::destroy(u_imageLodEnabled);

		m_allocator = NULL;
	}

	void setupStyle(bool _dark)
	{
		// Doug Binks' darl color scheme
		// https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9
		ImGuiStyle& style = ImGui::GetStyle();
		if (_dark)
		{
			ImGui::StyleColorsDark(&style);
		}
		else
		{
			ImGui::StyleColorsLight(&style);
		}

		style.FrameRounding = 2.0f;
		style.ScrollbarRounding = 2.0f;
		style.ScrollbarSize = 13.0f;
		style.WindowBorderSize = 0.0f;
		style.WindowPadding = ImVec2(4.0f, 4.0f);
		style.WindowRounding = 2.0f;
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

		g_MouseCursors[ImGuiMouseCursor_Arrow] = MouseCursor::ARROW;
		g_MouseCursors[ImGuiMouseCursor_TextInput] = MouseCursor::TEXT_INPUT;
		g_MouseCursors[ImGuiMouseCursor_ResizeAll] = MouseCursor::ARROW;
		g_MouseCursors[ImGuiMouseCursor_ResizeNS] = MouseCursor::SIZE_VERTICAL;
		g_MouseCursors[ImGuiMouseCursor_ResizeEW] = MouseCursor::SIZE_HORIZONTAL;
		g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = MouseCursor::CORNER_TOP_RIGHT;
		g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = MouseCursor::CORNER_TOP_LEFT;
		g_MouseCursors[ImGuiMouseCursor_Hand] = MouseCursor::HAND;
	}

	void updateMouseCursor()
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
			return;

		ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		for (int n = 0; n < platform_io.Viewports.Size; n++)
		{
			if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
			{
				// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
				device()->_window->show_cursor(false);
			}
			else
			{
				// Show OS mouse cursor
				device()->_window->set_cursor((MouseCursor::Enum)g_MouseCursors[imgui_cursor]);
			}
		}
	}

	void beginFrame(uint8_t view_id, uint16_t width, uint16_t height)
	{
		m_viewId = view_id;

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(width, height);
		io.DeltaTime   = 1.0f / 60.0f;

		updateMouseCursor();
		ImGui::NewFrame();
	}

	void endFrame()
	{
		ImGui::Render();
		render(ImGui::GetDrawData());
	}

	ImGuiContext*       m_imgui;
	bx::AllocatorI*     m_allocator;
	bgfx::VertexLayout  m_layout;
	bgfx::TextureHandle m_texture;
	bgfx::UniformHandle s_tex;
	bgfx::UniformHandle u_imageLodEnabled;
	ImFont* m_font[ImGui::Font::Count];
	bgfx::ViewId m_viewId;
};

static ImGuiContext s_ctx;

static void* memAlloc(size_t _size, void* _userData)
{
	BX_UNUSED(_userData);
	return BX_ALLOC(s_ctx.m_allocator, _size);
}

static void memFree(void* _ptr, void* _userData)
{
	BX_UNUSED(_userData);
	BX_FREE(s_ctx.m_allocator, _ptr);
}

namespace ImGui
{
	void PushFont(Font::Enum _font)
	{
		PushFont(s_ctx.m_font[_font]);
	}
} // namespace ImGui


namespace crown
{

void imgui_create(f32 _fontSize, bx::AllocatorI* _allocator)
{
	s_ctx.create(_fontSize, _allocator);
}

void imgui_destroy()
{
	s_ctx.destroy();
}

void imgui_begin_frame(uint8_t view_id, u16 width, u16 height)
{
	s_ctx.beginFrame(view_id, width, height);
}

void imgui_end_frame()
{
	s_ctx.endFrame();
}

} // namespace crown

BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4505); // error C4505: '' : unreferenced local function has been removed
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-function"); // warning: ‘int rect_width_compare(const void*, const void*)’ defined but not used
BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunknown-pragmas")
//BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-but-set-variable"); // warning: variable ‘L1’ set but not used
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wtype-limits"); // warning: comparison is always true due to limited range of data type
#define STBTT_malloc(_size, _userData) memAlloc(_size, _userData)
#define STBTT_free(_ptr, _userData) memFree(_ptr, _userData)
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
BX_PRAGMA_DIAGNOSTIC_POP();
