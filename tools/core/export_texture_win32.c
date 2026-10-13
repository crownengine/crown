/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#if defined(_WIN32) && !defined(CROWN_GTK3)

#define COBJMACROS
#include "export_texture_win32.h"
#include <d3d12.h>
#include <gdk/win32/gdkwin32.h>
#include <windows.h>
#include <stdlib.h>

static ID3D12Device *s_export_device;

static void export_device_close(void)
{
	if (s_export_device != NULL) {
		ID3D12Device_Release(s_export_device);
		s_export_device = NULL;
	}
}

static gboolean export_device_open(GError **error)
{
	if (s_export_device != NULL)
		return TRUE;
	HRESULT hr = D3D12CreateDevice(NULL
		, D3D_FEATURE_LEVEL_11_0
		, &IID_ID3D12Device
		, (void **)&s_export_device
		);
	if (FAILED(hr)) {
		g_set_error(error
			, G_IO_ERROR
			, G_IO_ERROR_FAILED
			, "Cannot create D3D12 import device (0x%08lx)"
			, (unsigned long)hr
			);
		return FALSE;
	}
	atexit(export_device_close);
	return TRUE;
}

static HANDLE duplicate_export_handle(HANDLE process, uint64_t value)
{
	HANDLE result = NULL;
	if (!DuplicateHandle(process
		, (HANDLE)(uintptr_t)value
		, GetCurrentProcess()
		, &result
		, 0
		, FALSE
		, DUPLICATE_SAME_ACCESS
		))
		return NULL;
	return result;
}

GdkTexture *export_d3d12_texture_import(uint32_t producer_pid
	, uint64_t resource_handle
	, uint64_t fence_handle
	, uint64_t fence_value
	, GDestroyNotify destroy
	, gpointer data
	, GError **error
	)
{
	if (!export_device_open(error))
		return NULL;
	HANDLE process = OpenProcess(PROCESS_DUP_HANDLE, FALSE, producer_pid);
	if (process == NULL) {
		g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Cannot open viewport renderer process");
		return NULL;
	}
	HANDLE resource_nt = duplicate_export_handle(process, resource_handle);
	HANDLE fence_nt = duplicate_export_handle(process, fence_handle);
	CloseHandle(process);
	if (resource_nt == NULL || fence_nt == NULL) {
		if (resource_nt != NULL)
			CloseHandle(resource_nt);
		if (fence_nt != NULL)
			CloseHandle(fence_nt);
		g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Cannot duplicate viewport GPU handles");
		return NULL;
	}
	ID3D12Resource *resource = NULL;
	ID3D12Fence *fence = NULL;
	HRESULT hr = ID3D12Device_OpenSharedHandle(s_export_device
		, resource_nt
		, &IID_ID3D12Resource
		, (void **)&resource
		);
	if (SUCCEEDED(hr))
		hr = ID3D12Device_OpenSharedHandle(s_export_device
			, fence_nt
			, &IID_ID3D12Fence
			, (void **)&fence
			);
	CloseHandle(resource_nt);
	CloseHandle(fence_nt);
	if (FAILED(hr)) {
		if (resource != NULL)
			ID3D12Resource_Release(resource);
		if (fence != NULL)
			ID3D12Fence_Release(fence);
		g_set_error(error
			, G_IO_ERROR
			, G_IO_ERROR_FAILED
			, "Cannot open viewport D3D12 resource or fence (0x%08lx)"
			, (unsigned long)hr
			);
		return NULL;
	}
	GdkD3D12TextureBuilder *builder = gdk_d3d12_texture_builder_new();
	gdk_d3d12_texture_builder_set_resource(builder, resource);
	gdk_d3d12_texture_builder_set_fence(builder, fence);
	gdk_d3d12_texture_builder_set_fence_wait(builder, fence_value);
	gdk_d3d12_texture_builder_set_premultiplied(builder, FALSE);
	GdkTexture *texture = gdk_d3d12_texture_builder_build(builder, destroy, data, error);
	g_object_unref(builder);
	ID3D12Resource_Release(resource);
	ID3D12Fence_Release(fence);
	return texture;
}

#endif /* if defined(_WIN32) && !defined(CROWN_GTK3) */
