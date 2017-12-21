#pragma once

#include <bgfx/bgfx.h>
#include <iconfontheaders/icons_kenney.h>
#include <iconfontheaders/icons_font_awesome.h>

#define IMGUI_MBUT_LEFT   0x01
#define IMGUI_MBUT_RIGHT  0x02
#define IMGUI_MBUT_MIDDLE 0x04

#define IMGUI_FLAGS_NONE        UINT8_C(0x00)
#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)

namespace crown
{
void imgui_create(float _fontSize = 18.0f, bx::AllocatorI* _allocator = NULL);
void imgui_destroy();
void imgui_begin_frame(uint8_t view_id, u16 width, u16 height);
void imgui_end_frame();

} // namespace crown
