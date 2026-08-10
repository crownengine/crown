/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <glib.h>
#include <gdk/gdk.h>
#include <stdint.h>

#if defined(__linux__)
#include <errno.h>
#include <gdk/gdkx.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xfixes.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif /* if defined(__linux__) */

#if defined(__linux__)
static void initialize_xlib_threads(void) __attribute__((constructor));

static void initialize_xlib_threads(void)
{
	XInitThreads();
}

#endif /* if defined(__linux__) */

typedef struct CrownInfiniteDragSampler
{
	GThread *thread;
	GMutex mutex;
	gint running;
	gint anchor_x;
	gint anchor_y;
	gint cancel_button;
	gdouble delta_x;
	gdouble delta_y;
	gint samples;
	gint released;
	gint cancel_requested;
	#if defined(__linux__)
	Window x11_window;
	gint x11_wake_fd;
	#elif defined(_WIN32)
	HANDLE windows_stop_event;
	GCond windows_setup_cond;
	gboolean windows_setup_complete;
	gboolean windows_setup_success;
	gint windows_absolute_x;
	gint windows_absolute_y;
	gboolean windows_absolute_initialized;
	#endif /* if defined(__linux__) */
} CrownInfiniteDragSampler;

static void store_delta(CrownInfiniteDragSampler *sampler, gdouble delta_x, gdouble delta_y)
{
	g_mutex_lock(&sampler->mutex);
	sampler->delta_x += delta_x;
	sampler->delta_y += delta_y;
	g_mutex_unlock(&sampler->mutex);
}

static void store_sample(CrownInfiniteDragSampler *sampler)
{
	g_mutex_lock(&sampler->mutex);
	sampler->samples += 1;
	g_mutex_unlock(&sampler->mutex);
}

#if defined(__linux__)
typedef struct CrownInfiniteDragX11Grab
{
	Window window;
	Window root;
	gint anchor_x;
	gint anchor_y;
	gboolean cursor_hidden;
	gboolean active;
} CrownInfiniteDragX11Grab;

/* Keep grab+XFixes-hide+XI2-raw-motion together: this combination is what activates Xwayland's relative-pointer device. */
static gboolean warp_x11_pointer_to_anchor(Display *display, CrownInfiniteDragX11Grab *grab)
{
	int window_x;
	int window_y;
	Window child;
	if (!XTranslateCoordinates(display
		, grab->root
		, grab->window
		, grab->anchor_x
		, grab->anchor_y
		, &window_x
		, &window_y
		, &child
		)) {
		return FALSE;
	}

	XWarpPointer(display
		, None
		, grab->window
		, 0
		, 0
		, 0
		, 0
		, window_x
		, window_y
		);
	XSync(display, False);
	return TRUE;
}

static void destroy_x11_pointer_grab(Display *display, CrownInfiniteDragX11Grab *grab)
{
	if (grab->active) {
		/* Commit the original position hint before revealing the pointer. */
		warp_x11_pointer_to_anchor(display, grab);

		/* Ungrab while hidden or Xwayland may discard the position hint. */
		XUngrabPointer(display, CurrentTime);
		XSync(display, False);
		grab->active = FALSE;
	}
	if (grab->cursor_hidden) {
		XFixesShowCursor(display, grab->window);
		XSync(display, False);
		grab->cursor_hidden = FALSE;
	}
}

static gboolean create_x11_pointer_grab(Display *display
	, Window root
	, Window window
	, gint anchor_x
	, gint anchor_y
	, CrownInfiniteDragX11Grab *grab
	)
{
	grab->window = window;
	grab->root = root;
	grab->anchor_x = anchor_x;
	grab->anchor_y = anchor_y;
	int status = XGrabPointer(display
		, window
		, False
		, ButtonReleaseMask
		, GrabModeAsync
		, GrabModeAsync
		, window
		, None
		, CurrentTime
		);
	if (status != GrabSuccess) {
		destroy_x11_pointer_grab(display, grab);
		return FALSE;
	}
	grab->active = TRUE;

	XFixesHideCursor(display, window);
	XSync(display, False);
	grab->cursor_hidden = TRUE;

	/* A single warp; repeated warps would be reported as synthetic raw motion. */
	if (!warp_x11_pointer_to_anchor(display, grab)) {
		destroy_x11_pointer_grab(display, grab);
		return FALSE;
	}
	return TRUE;
}

static gpointer sample_pointer_x11(gpointer data)
{
	CrownInfiniteDragSampler *sampler = data;
	Display *display = XOpenDisplay(NULL);
	if (display == NULL) {
		return NULL;
	}
	int screen_number = DefaultScreen(display);
	Window root = RootWindow(display, screen_number);

	int xinput_opcode;
	int xinput_event;
	int xinput_error;
	if (!XQueryExtension(display, "XInputExtension", &xinput_opcode, &xinput_event, &xinput_error)) {
		XCloseDisplay(display);
		return NULL;
	}

	int xinput_major = 2;
	int xinput_minor = 1;
	int status = XIQueryVersion(display, &xinput_major, &xinput_minor);
	if (status != Success
		|| xinput_major < 2
		|| (xinput_major == 2 && xinput_minor < 1)
		) {
		XCloseDisplay(display);
		return NULL;
	}

	int xfixes_event;
	int xfixes_error;
	int xfixes_major = 4;
	int xfixes_minor = 0;
	if (!XFixesQueryExtension(display, &xfixes_event, &xfixes_error)
		|| !XFixesQueryVersion(display, &xfixes_major, &xfixes_minor)
		|| xfixes_major < 4
		) {
		XCloseDisplay(display);
		return NULL;
	}

	unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 };
	XISetMask(mask, XI_RawMotion);
	if (sampler->cancel_button != 0)
		XISetMask(mask, XI_RawButtonPress);
	XISetMask(mask, XI_RawButtonRelease);
	XIEventMask event_mask =
	{
		.deviceid = XIAllMasterDevices,
		.mask_len = sizeof(mask),
		.mask = mask,
	};
	if (XISelectEvents(display, root, &event_mask, 1) != Success) {
		XCloseDisplay(display);
		return NULL;
	}
	XSync(display, False);

	CrownInfiniteDragX11Grab pointer_grab = { 0 };
	if (!create_x11_pointer_grab(display
		, root
		, sampler->x11_window
		, sampler->anchor_x
		, sampler->anchor_y
		, &pointer_grab
		)) {
		XCloseDisplay(display);
		return NULL;
	}

	while (g_atomic_int_get(&sampler->running)) {
		while (XPending(display) > 0) {
			XEvent event;
			XNextEvent(display, &event);
			if (event.type == ButtonRelease) {
				g_atomic_int_set(&sampler->released, TRUE);
				g_atomic_int_set(&sampler->running, FALSE);
			} else if (event.type == GenericEvent
				&& event.xcookie.extension == xinput_opcode
				&& XGetEventData(display, &event.xcookie)
				) {
				XIRawEvent *raw = event.xcookie.data;
				if (event.xcookie.evtype == XI_RawButtonRelease) {
					g_atomic_int_set(&sampler->released, TRUE);
					g_atomic_int_set(&sampler->running, FALSE);
				} else if (event.xcookie.evtype == XI_RawButtonPress) {
					if (sampler->cancel_button != 0 && raw->detail == sampler->cancel_button) {
						g_atomic_int_set(&sampler->cancel_requested, TRUE);
						g_atomic_int_set(&sampler->running, FALSE);
					}
				} else if (event.xcookie.evtype == XI_RawMotion) {
					int value_index = 0;
					gdouble delta_x = 0.0;
					gdouble delta_y = 0.0;
					for (int axis = 0; axis < raw->valuators.mask_len * 8; ++axis) {
						if (!XIMaskIsSet(raw->valuators.mask, axis))
							continue;
						gdouble value = raw->valuators.values[value_index];
						if (axis == 0)
							delta_x = value;
						else if (axis == 1)
							delta_y = value;
						value_index += 1;
					}
					store_delta(sampler, delta_x, delta_y);
					store_sample(sampler);
				}
				XFreeEventData(display, &event.xcookie);
			}
		}

		if (!g_atomic_int_get(&sampler->running))
			break;

		struct pollfd descriptors[] =
		{
			{ .fd = ConnectionNumber(display), .events = POLLIN },
			{ .fd = sampler->x11_wake_fd, .events = POLLIN },
		};
		int poll_result;
		do {
			poll_result = poll(descriptors, G_N_ELEMENTS(descriptors), -1);
		} while (poll_result < 0 && errno == EINTR);
		if (poll_result < 0) {
			break;
		}
		if ((descriptors[1].revents & POLLIN) != 0) {
			uint64_t wake_value;
			while (read(sampler->x11_wake_fd, &wake_value, sizeof(wake_value)) < 0 && errno == EINTR) {
			}
		}
		if ((descriptors[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
			break;
		}
	}

	destroy_x11_pointer_grab(display, &pointer_grab);
	XCloseDisplay(display);
	return NULL;
}

#elif defined(_WIN32)
static const wchar_t WINDOWS_RAW_INPUT_CLASS_NAME[] = L"CrownInfiniteDragRawInput";

/* Event-driven only: do not replace with cursor-position polling or repeated SetCursorPos calls. */

static void complete_windows_setup(CrownInfiniteDragSampler *sampler, gboolean success)
{
	g_mutex_lock(&sampler->mutex);
	sampler->windows_setup_success = success;
	sampler->windows_setup_complete = TRUE;
	g_cond_signal(&sampler->windows_setup_cond);
	g_mutex_unlock(&sampler->mutex);
}

static gboolean register_windows_raw_input(HWND target)
{
	RAWINPUTDEVICE mouse =
	{
		.usUsagePage = 0x01,
		.usUsage = 0x02,
		.dwFlags = target != NULL ? RIDEV_INPUTSINK | RIDEV_NOLEGACY : RIDEV_REMOVE,
		.hwndTarget = target,
	};
	if (RegisterRawInputDevices(&mouse, 1, sizeof(mouse)))
		return TRUE;

	return FALSE;
}

static void process_windows_raw_mouse(CrownInfiniteDragSampler *sampler, const RAWMOUSE *mouse)
{
	gint delta_x = 0;
	gint delta_y = 0;
	if ((mouse->usFlags & MOUSE_MOVE_ABSOLUTE) != 0) {
		gboolean virtual_desktop = (mouse->usFlags & MOUSE_VIRTUAL_DESKTOP) != 0;
		gint screen_x = virtual_desktop ? GetSystemMetrics(SM_XVIRTUALSCREEN) : 0;
		gint screen_y = virtual_desktop ? GetSystemMetrics(SM_YVIRTUALSCREEN) : 0;
		gint screen_width = virtual_desktop ? GetSystemMetrics(SM_CXVIRTUALSCREEN) : GetSystemMetrics(SM_CXSCREEN);
		gint screen_height = virtual_desktop ? GetSystemMetrics(SM_CYVIRTUALSCREEN) : GetSystemMetrics(SM_CYSCREEN);
		gint absolute_x = screen_x + MulDiv(mouse->lLastX, MAX(screen_width - 1, 1), 65535);
		gint absolute_y = screen_y + MulDiv(mouse->lLastY, MAX(screen_height - 1, 1), 65535);
		if (sampler->windows_absolute_initialized) {
			delta_x = absolute_x - sampler->windows_absolute_x;
			delta_y = absolute_y - sampler->windows_absolute_y;
		}
		sampler->windows_absolute_x = absolute_x;
		sampler->windows_absolute_y = absolute_y;
		sampler->windows_absolute_initialized = TRUE;
	} else {
		delta_x = mouse->lLastX;
		delta_y = mouse->lLastY;
		sampler->windows_absolute_initialized = FALSE;
	}

	if (delta_x != 0 || delta_y != 0) {
		store_delta(sampler, delta_x, delta_y);
		store_sample(sampler);
	}

	USHORT cancel_button_down = 0;
	switch (sampler->cancel_button) {
	case GDK_BUTTON_PRIMARY:
		cancel_button_down = RI_MOUSE_LEFT_BUTTON_DOWN;
		break;
	case GDK_BUTTON_MIDDLE:
		cancel_button_down = RI_MOUSE_MIDDLE_BUTTON_DOWN;
		break;
	case GDK_BUTTON_SECONDARY:
		cancel_button_down = RI_MOUSE_RIGHT_BUTTON_DOWN;
		break;
	}
	if (cancel_button_down != 0 && (mouse->usButtonFlags & cancel_button_down) != 0) {
		g_atomic_int_set(&sampler->cancel_requested, TRUE);
		g_atomic_int_set(&sampler->running, FALSE);
		SetEvent(sampler->windows_stop_event);
		return;
	}

	if ((mouse->usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) != 0) {
		g_atomic_int_set(&sampler->released, TRUE);
		g_atomic_int_set(&sampler->running, FALSE);
		SetEvent(sampler->windows_stop_event);
	}
}

static LRESULT CALLBACK windows_raw_input_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_NCCREATE) {
		CREATESTRUCTW *create = (CREATESTRUCTW *)lparam;
		SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)create->lpCreateParams);
		return TRUE;
	}

	CrownInfiniteDragSampler *sampler = (CrownInfiniteDragSampler *)GetWindowLongPtrW(window, GWLP_USERDATA);
	if (message == WM_INPUT && sampler != NULL) {
		RAWINPUT input;
		UINT input_size = sizeof(input);
		UINT result = GetRawInputData((HRAWINPUT)lparam
			, RID_INPUT
			, &input
			, &input_size
			, sizeof(RAWINPUTHEADER)
			);
		if (result != (UINT)-1 && input.header.dwType == RIM_TYPEMOUSE) {
			process_windows_raw_mouse(sampler, &input.data.mouse);
		}
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

static HWND create_windows_raw_input_window(CrownInfiniteDragSampler *sampler)
{
	HINSTANCE instance = GetModuleHandleW(NULL);
	WNDCLASSEXW window_class =
	{
		.cbSize = sizeof(window_class),
		.lpfnWndProc = windows_raw_input_window_proc,
		.hInstance = instance,
		.lpszClassName = WINDOWS_RAW_INPUT_CLASS_NAME,
	};
	if (RegisterClassExW(&window_class) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
		return NULL;
	}

	HWND window = CreateWindowExW(0
		, WINDOWS_RAW_INPUT_CLASS_NAME
		, L""
		, 0
		, 0
		, 0
		, 0
		, 0
		, HWND_MESSAGE
		, NULL
		, instance
		, sampler
		);
	return window;
}

static void drain_windows_raw_input_messages(void)
{
	MSG message;
	while (PeekMessageW(&message, NULL, WM_INPUT, WM_INPUT, PM_REMOVE))
		DispatchMessageW(&message);
}

static gpointer sample_pointer_windows(gpointer data)
{
	CrownInfiniteDragSampler *sampler = data;
	HWND window = create_windows_raw_input_window(sampler);
	gboolean registered = FALSE;
	if (window == NULL)
		goto setup_failed;

	if (!register_windows_raw_input(window))
		goto setup_failed;
	registered = TRUE;

	RECT clip =
	{
		.left = sampler->anchor_x,
		.top = sampler->anchor_y,
		.right = sampler->anchor_x + 1,
		.bottom = sampler->anchor_y + 1,
	};
	if (!ClipCursor(&clip)) {
		goto setup_failed;
	}
	complete_windows_setup(sampler, TRUE);

	while (g_atomic_int_get(&sampler->running)) {
		DWORD wait_result = MsgWaitForMultipleObjects(1
			, &sampler->windows_stop_event
			, FALSE
			, INFINITE
			, QS_RAWINPUT
			);
		if (wait_result == WAIT_OBJECT_0)
			break;
		if (wait_result == WAIT_FAILED) {
			break;
		}

		drain_windows_raw_input_messages();
	}

	/* GTK may observe the release just before this worker does. */
	drain_windows_raw_input_messages();

	/* Preserve the release invariant: restore, unconfine, then reveal in GTK. */
	SetCursorPos(sampler->anchor_x, sampler->anchor_y);
	ClipCursor(NULL);

	register_windows_raw_input(NULL);
	DestroyWindow(window);
	return NULL;

setup_failed:
	if (registered)
		register_windows_raw_input(NULL);
	if (window != NULL)
		DestroyWindow(window);
	g_atomic_int_set(&sampler->running, FALSE);
	complete_windows_setup(sampler, FALSE);
	return NULL;
}

#endif /* if defined(__linux__) */

static gpointer sample_pointer(gpointer data)
{
	#if defined(__linux__)
	return sample_pointer_x11(data);
	#elif defined(_WIN32)
	return sample_pointer_windows(data);
	#endif
	return NULL;
}

void *crown_infinite_drag_sampler_start(GdkDisplay *display, GdkWindow *window, GdkDevice *device, gint cancel_button)
{
	CrownInfiniteDragSampler *sampler = g_new0(CrownInfiniteDragSampler, 1);
	g_mutex_init(&sampler->mutex);
	#if defined(__linux__)
	sampler->x11_wake_fd = -1;
	#endif
	g_atomic_int_set(&sampler->running, TRUE);
	sampler->cancel_button = cancel_button;
	#if defined(__linux__)
	sampler->x11_wake_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if (sampler->x11_wake_fd < 0) {
		g_mutex_clear(&sampler->mutex);
		g_free(sampler);
		return NULL;
	}
	GdkWindow *toplevel = gdk_window_get_toplevel(window);
	sampler->x11_window = gdk_x11_window_get_xid(toplevel);
	Display *xdisplay = gdk_x11_display_get_xdisplay(display);
	Window query_root;
	Window query_child;
	int window_x;
	int window_y;
	unsigned int mask;
	if (!XQueryPointer(xdisplay
		, sampler->x11_window
		, &query_root
		, &query_child
		, &sampler->anchor_x
		, &sampler->anchor_y
		, &window_x
		, &window_y
		, &mask
		)) {
		close(sampler->x11_wake_fd);
		g_mutex_clear(&sampler->mutex);
		g_free(sampler);
		return NULL;
	}
	GdkSeat *seat = gdk_device_get_seat(device);
	if (seat != NULL) {
		gdk_seat_ungrab(seat);
		gdk_display_sync(display);
	}
	#elif defined(_WIN32)
	(void)display;
	(void)window;
	(void)device;
	POINT cursor_position;
	if (!GetCursorPos(&cursor_position)) {
		g_mutex_clear(&sampler->mutex);
		g_free(sampler);
		return NULL;
	}
	sampler->anchor_x = cursor_position.x;
	sampler->anchor_y = cursor_position.y;
	g_cond_init(&sampler->windows_setup_cond);
	sampler->windows_stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (sampler->windows_stop_event == NULL) {
		g_cond_clear(&sampler->windows_setup_cond);
		g_mutex_clear(&sampler->mutex);
		g_free(sampler);
		return NULL;
	}
	#else
	(void)display;
	(void)window;
	(void)device;
	#endif /* if defined(__linux__) */
	sampler->thread = g_thread_new("infinite-drag", sample_pointer, sampler);
	#if defined(_WIN32)
	g_mutex_lock(&sampler->mutex);
	while (!sampler->windows_setup_complete)
		g_cond_wait(&sampler->windows_setup_cond, &sampler->mutex);
	gboolean setup_success = sampler->windows_setup_success;
	g_mutex_unlock(&sampler->mutex);
	if (!setup_success) {
		g_thread_join(sampler->thread);
		CloseHandle(sampler->windows_stop_event);
		g_cond_clear(&sampler->windows_setup_cond);
		g_mutex_clear(&sampler->mutex);
		g_free(sampler);
		return NULL;
	}
	#endif
	return sampler;
}

void crown_infinite_drag_sampler_drain(void *data, gdouble *delta_x, gdouble *delta_y, gint *samples)
{
	CrownInfiniteDragSampler *sampler = data;
	g_mutex_lock(&sampler->mutex);
	*delta_x = sampler->delta_x;
	*delta_y = sampler->delta_y;
	*samples = sampler->samples;
	sampler->delta_x = 0;
	sampler->delta_y = 0;
	sampler->samples = 0;
	g_mutex_unlock(&sampler->mutex);
}

gboolean crown_infinite_drag_sampler_released(void *data)
{
	CrownInfiniteDragSampler *sampler = data;
	return g_atomic_int_get(&sampler->released);
}

gboolean crown_infinite_drag_sampler_cancel_requested(void *data)
{
	CrownInfiniteDragSampler *sampler = data;
	return g_atomic_int_get(&sampler->cancel_requested);
}

void crown_infinite_drag_sampler_stop(void *data, gdouble *delta_x, gdouble *delta_y, gint *samples)
{
	CrownInfiniteDragSampler *sampler = data;
	g_atomic_int_set(&sampler->running, FALSE);
	#if defined(__linux__)
	if (sampler->x11_wake_fd >= 0) {
		uint64_t wake_value = 1;
		while (write(sampler->x11_wake_fd, &wake_value, sizeof(wake_value)) < 0 && errno == EINTR) {
		}
	}
	#elif defined(_WIN32)
	SetEvent(sampler->windows_stop_event);
	#endif
	g_thread_join(sampler->thread);
	crown_infinite_drag_sampler_drain(sampler, delta_x, delta_y, samples);
	#if defined(__linux__)
	if (sampler->x11_wake_fd >= 0)
		close(sampler->x11_wake_fd);
	#elif defined(_WIN32)
	CloseHandle(sampler->windows_stop_event);
	g_cond_clear(&sampler->windows_setup_cond);
	#endif
	g_mutex_clear(&sampler->mutex);
	g_free(sampler);
}
