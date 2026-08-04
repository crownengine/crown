/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <gdk/gdk.h>

G_BEGIN_DECLS

typedef struct CrownInfiniteDragSampler CrownInfiniteDragSampler;

/// cancel_button: X11 button that cancels the drag instead of committing it; 0 disables it.
CrownInfiniteDragSampler *crown_infinite_drag_sampler_start(GdkDisplay
*display, GdkWindow *window, GdkDevice *device, gint anchor_x, gint
anchor_y, gint cancel_button);
void crown_infinite_drag_sampler_drain(CrownInfiniteDragSampler *sampler, gdouble *delta_x, gdouble *delta_y, gint *samples);
gboolean crown_infinite_drag_sampler_released(CrownInfiniteDragSampler
*sampler);
/// True once a cancel (secondary button) ended the session instead of a release.
gboolean crown_infinite_drag_sampler_cancel_requested(CrownInfiniteDragSampler
*sampler);
void crown_infinite_drag_sampler_stop(CrownInfiniteDragSampler *sampler, gdouble *delta_x, gdouble *delta_y, gint *samples);

G_END_DECLS
