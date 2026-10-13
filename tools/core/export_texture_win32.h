/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <gtk/gtk.h>

GdkTexture *export_d3d12_texture_import(uint32_t producer_pid
	, uint64_t resource_handle
	, uint64_t fence_handle
	, uint64_t fence_value
	, GDestroyNotify destroy
	, gpointer data
	, GError **error
	);
