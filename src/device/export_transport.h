/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "device/export_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif
void *export_bind(const char *path);
int export_is_open(void *receiver);
int export_receive(CrownExportPacket *packet, void *source, void *receiver);
int export_send(const char *path, const CrownExportPacket *packet, int fd);
void export_close(void *receiver, const char *path);
#ifdef __cplusplus
}
#endif
