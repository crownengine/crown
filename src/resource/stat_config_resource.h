/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/strings/string_id.h"
#include "core/types.h"

namespace crown
{
struct StatPanelLayout
{
	f32 margin_left;
	f32 margin_right;
	f32 margin_bottom;
	f32 margin_top;
	f32 padding_left;
	f32 padding_right;
	f32 padding_bottom;
	f32 padding_top;
};

struct StatCounter
{
	char label[32];
	u32 num_counters;
	u32 counters[8];
};

struct StatPanel
{
	StringId64 font;
	StringId64 material;
	u32 font_size;
	StatPanelLayout layout;
	u32 num_counters;
	StatCounter counters[16];
	char label[32];
	char name[32];
};

struct StatConfigResource
{
	u32 version;
	u32 num_panels;
};

} // namespace crown
