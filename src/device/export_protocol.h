/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

#define CROWN_EXPORT_MAGIC UINT32_C(0x43564557) /* CVEW */
#define CROWN_EXPORT_VERSION 2
#define CROWN_EXPORT_FRAME 1
#define CROWN_EXPORT_ERROR 2

typedef struct _CrownExportPacket
{
	uint32_t magic;
	uint32_t version;
	uint32_t kind;
	uint32_t generation;
	uint32_t buffer_id;
	uint16_t width;
	uint16_t height;
	uint32_t stride;
	uint32_t offset;
	uint32_t size;
	uint32_t fourcc;
	uint64_t modifier;
	uint32_t error;
	uint32_t reserved;
	uint64_t resource_handle;
	uint64_t fence_handle;
	uint64_t fence_value;
} CrownExportPacket;

enum CrownExportError
{
	CROWN_EXPORT_UNSUPPORTED = 1,
	CROWN_EXPORT_FAILED      = 2
};
