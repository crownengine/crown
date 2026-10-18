/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

GInputStream *crown_fifo_input_stream_new(const char *path, GError **error);
void crown_fifo_input_stream_connect_async(GInputStream *stream, int priority, GCancellable *cancellable, GAsyncReadyCallback callback, void *user_data);
gboolean crown_fifo_input_stream_connect_finish(GAsyncResult *result, GError **error);

G_END_DECLS
