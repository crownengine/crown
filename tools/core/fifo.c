/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#ifdef _WIN32

#include "tools/core/fifo.h"
#include <gio/gwin32inputstream.h>
#include <windows.h>

struct FifoConnect
{
	HANDLE handle;
	OVERLAPPED overlapped;
};

static void cancel_connect(GCancellable *cancellable, void *user_data)
{
	(void)cancellable;
	struct FifoConnect *connect = user_data;
	CancelIoEx(connect->handle, &connect->overlapped);
}

static void connect_thread(GTask *task, void *source_object, void *task_data, GCancellable *cancellable)
{
	(void)task_data;
	if (g_task_return_error_if_cancelled(task))
		return;
	struct FifoConnect connect = { 0 };
	connect.handle = g_win32_input_stream_get_handle(G_WIN32_INPUT_STREAM(source_object));
	connect.overlapped.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
	if (connect.overlapped.hEvent == NULL) {
		g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_FAILED, "CreateEvent failed (%lu)", (gulong)GetLastError());
		return;
	}
	BOOL connected = ConnectNamedPipe(connect.handle, &connect.overlapped);
	DWORD error = connected ? ERROR_SUCCESS : GetLastError();
	if (error == ERROR_PIPE_CONNECTED) {
		connected = TRUE;
	} else if (error == ERROR_IO_PENDING) {
		gulong handler = cancellable != NULL
			? g_cancellable_connect(cancellable, G_CALLBACK(cancel_connect), &connect, NULL)
			: 0;
		DWORD size;
		connected = GetOverlappedResult(connect.handle, &connect.overlapped, &size, TRUE);
		error = connected ? ERROR_SUCCESS : GetLastError();
		if (handler != 0)
			g_cancellable_disconnect(cancellable, handler);
	}
	CloseHandle(connect.overlapped.hEvent);
	if (g_task_return_error_if_cancelled(task))
		return;
	if (connected)
		g_task_return_boolean(task, TRUE);
	else
		g_task_return_new_error(task, G_IO_ERROR, g_io_error_from_win32_error(error), "ConnectNamedPipe failed (%lu)", (gulong)error);
}

GInputStream *crown_fifo_input_stream_new(const char *path, GError **error)
{
	HANDLE handle = CreateNamedPipeA(path
		, PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE
		, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS
		, 1, 0, 64, 0, NULL
		);
	if (handle == INVALID_HANDLE_VALUE) {
		DWORD code = GetLastError();
		g_set_error(error, G_IO_ERROR, g_io_error_from_win32_error(code), "CreateNamedPipe failed (%lu)", (gulong)code);
		return NULL;
	}
	return g_win32_input_stream_new(handle, TRUE);
}

void crown_fifo_input_stream_connect_async(GInputStream *stream, int priority, GCancellable *cancellable, GAsyncReadyCallback callback, void *user_data)
{
	GTask *task = g_task_new(stream, cancellable, callback, user_data);
	g_task_set_priority(task, priority);
	g_task_run_in_thread(task, connect_thread);
	g_object_unref(task);
}

gboolean crown_fifo_input_stream_connect_finish(GAsyncResult *result, GError **error)
{
	return g_task_propagate_boolean(G_TASK(result), error);
}

#endif /* ifdef _WIN32 */
