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

#define SAMPLER_LOG(...) g_printerr("infinite drag sampler: " __VA_ARGS__)

#if defined(__linux__)
static void initialize_xlib_threads(void) __attribute__((constructor));

static void initialize_xlib_threads(void)
{
	if (!XInitThreads()) {
//		SAMPLER_LOG("X11 thread initialization failed\n");
	}
}

#endif /* if defined(__linux__) */

typedef enum CrownInfiniteDragSamplerBackend
{
	CROWN_INFINITE_DRAG_SAMPLER_BACKEND_NONE,
	CROWN_INFINITE_DRAG_SAMPLER_BACKEND_X11,
	CROWN_INFINITE_DRAG_SAMPLER_BACKEND_WINDOWS,
} CrownInfiniteDragSamplerBackend;

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
	CrownInfiniteDragSamplerBackend backend;
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

static const char *backend_names[] = { "none", "x11", "windows" };

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
// 		SAMPLER_LOG("X11 anchor translation failed\n");
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
		if (warp_x11_pointer_to_anchor(display, grab)) {
// 			SAMPLER_LOG("X11 pointer restored to anchor=(%d,%d)\n", grab->anchor_x, grab->anchor_y);
		}

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
// 		SAMPLER_LOG("X11 setup failed: pointer grab status=%d\n", status);
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
/*
	SAMPLER_LOG("X11 pointer confined and XFixes-hidden window=0x%lx anchor=(%d,%d)\n"
		, window
		, anchor_x
		, anchor_y
		);
 */
	return TRUE;
}

static gpointer sample_pointer_x11(gpointer data)
{
	CrownInfiniteDragSampler *sampler = data;
	Display *display = XOpenDisplay(NULL);
	if (display == NULL) {
//		SAMPLER_LOG("X11 connection failed\n");
		return NULL;
	}
	int screen_number = DefaultScreen(display);
	Window root = RootWindow(display, screen_number);
//	SAMPLER_LOG("worker started backend=x11 mode=event-driven screen=%d\n", screen_number);

	int xinput_opcode;
	int xinput_event;
	int xinput_error;
	if (!XQueryExtension(display, "XInputExtension", &xinput_opcode, &xinput_event, &xinput_error)) {
//		SAMPLER_LOG("X11 setup failed: XInput extension unavailable\n");
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
//		SAMPLER_LOG("X11 setup failed: XInput 2.1 unavailable status=%d\n", status);
		XCloseDisplay(display);
		return NULL;
	}
/*
	SAMPLER_LOG("XInput version=%d.%d opcode=%d\n"
		, xinput_major
		, xinput_minor
		, xinput_opcode
		);
 */

	int xfixes_event;
	int xfixes_error;
	int xfixes_major = 4;
	int xfixes_minor = 0;
	if (!XFixesQueryExtension(display, &xfixes_event, &xfixes_error)
		|| !XFixesQueryVersion(display, &xfixes_major, &xfixes_minor)
		|| xfixes_major < 4
		) {
//		SAMPLER_LOG("X11 setup failed: XFixes 4 unavailable\n");
		XCloseDisplay(display);
		return NULL;
	}
/*
	SAMPLER_LOG("XFixes version=%d.%d\n"
		, xfixes_major
		, xfixes_minor
		);
 */

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
//		SAMPLER_LOG("X11 setup failed: select raw motion\n");
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

//	SAMPLER_LOG("X11 raw motion selected on root=0x%lx\n", root);

	while (g_atomic_int_get(&sampler->running)) {
		while (XPending(display) > 0) {
			XEvent event;
			XNextEvent(display, &event);
			if (event.type == ButtonRelease) {
				g_atomic_int_set(&sampler->released, TRUE);
				g_atomic_int_set(&sampler->running, FALSE);
//				SAMPLER_LOG("X11 button released detail=%u\n", event.xbutton.button);
			} else if (event.type == GenericEvent
				&& event.xcookie.extension == xinput_opcode
				&& XGetEventData(display, &event.xcookie)
				) {
				XIRawEvent *raw = event.xcookie.data;
				if (event.xcookie.evtype == XI_RawButtonRelease) {
					g_atomic_int_set(&sampler->released, TRUE);
					g_atomic_int_set(&sampler->running, FALSE);
//					SAMPLER_LOG("XInput raw button released detail=%u\n", raw->detail);
				} else if (event.xcookie.evtype == XI_RawButtonPress) {
					if (sampler->cancel_button != 0 && raw->detail == sampler->cancel_button) {
						g_atomic_int_set(&sampler->cancel_requested, TRUE);
						g_atomic_int_set(&sampler->running, FALSE);
//						SAMPLER_LOG("XInput raw button pressed detail=%u cancel-requested\n", raw->detail);
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
//			SAMPLER_LOG("X11 poll failed error=%d\n", errno);
			break;
		}
		if ((descriptors[1].revents & POLLIN) != 0) {
			uint64_t wake_value;
			while (read(sampler->x11_wake_fd, &wake_value, sizeof(wake_value)) < 0 && errno == EINTR) {
			}
		}
		if ((descriptors[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
//			SAMPLER_LOG("X11 socket poll error=0x%x\n", descriptors[0].revents);
			break;
		}
	}

	destroy_x11_pointer_grab(display, &pointer_grab);
	XCloseDisplay(display);
//	SAMPLER_LOG("worker stopped backend=x11\n");
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
		.dwFlags = target != NULL ? RIDEV_INPUTSINK : RIDEV_REMOVE,
		.hwndTarget = target,
	};
	if (RegisterRawInputDevices(&mouse, 1, sizeof(mouse)))
		return TRUE;

//	SAMPLER_LOG("Windows RegisterRawInputDevices failed error=%lu\n", GetLastError());
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

	if ((mouse->usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) != 0) {
		g_atomic_int_set(&sampler->released, TRUE);
		g_atomic_int_set(&sampler->running, FALSE);
		SetEvent(sampler->windows_stop_event);
//		SAMPLER_LOG("Windows raw button released\n");
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
		if (result == (UINT)-1) {
//			SAMPLER_LOG("Windows GetRawInputData failed error=%lu\n", GetLastError());
		} else if (input.header.dwType == RIM_TYPEMOUSE) {
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
//		SAMPLER_LOG("Windows RegisterClassEx failed error=%lu\n", GetLastError());
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
	if (window == NULL)
//		SAMPLER_LOG("Windows CreateWindowEx failed error=%lu\n", GetLastError());
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
//	SAMPLER_LOG("worker started backend=windows mode=raw-input\n");
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
//		SAMPLER_LOG("Windows ClipCursor failed error=%lu\n", GetLastError());
		goto setup_failed;
	}
//	SAMPLER_LOG("Windows raw input registered and cursor clipped anchor=(%d,%d)\n", sampler->anchor_x, sampler->anchor_y);
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
//			SAMPLER_LOG("Windows message wait failed error=%lu\n", GetLastError());
			break;
		}

		drain_windows_raw_input_messages();
	}

	/* GTK may observe the release just before this worker does. */
	drain_windows_raw_input_messages();

	/* Preserve the release invariant: restore, unconfine, then reveal in GTK. */
	if (!SetCursorPos(sampler->anchor_x, sampler->anchor_y))
//		SAMPLER_LOG("Windows cursor restore failed error=%lu\n", GetLastError());
	else
//		SAMPLER_LOG("Windows pointer restored to anchor=(%d,%d)\n", sampler->anchor_x, sampler->anchor_y);
	if (!ClipCursor(NULL))
//		SAMPLER_LOG("Windows ClipCursor release failed error=%lu\n", GetLastError());

	register_windows_raw_input(NULL);
	DestroyWindow(window);
//	SAMPLER_LOG("worker stopped backend=windows\n");
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

void *crown_infinite_drag_sampler_start(GdkDisplay *display, GdkWindow *window, GdkDevice *device, gint anchor_x, gint anchor_y, gint cancel_button)
{
	CrownInfiniteDragSampler *sampler = g_new0(CrownInfiniteDragSampler, 1);
	g_mutex_init(&sampler->mutex);
	#if defined(__linux__)
	sampler->x11_wake_fd = -1;
	#endif
	g_atomic_int_set(&sampler->running, TRUE);
	sampler->anchor_x = anchor_x;
	sampler->anchor_y = anchor_y;
	sampler->cancel_button = cancel_button;
	#if defined(__linux__)
	sampler->backend = CROWN_INFINITE_DRAG_SAMPLER_BACKEND_X11;
	sampler->x11_wake_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if (sampler->x11_wake_fd < 0) {
//		SAMPLER_LOG("X11 setup failed: eventfd error=%d\n", errno);
		g_mutex_clear(&sampler->mutex);
		g_free(sampler);
		return NULL;
	}
	GdkWindow *toplevel = gdk_window_get_toplevel(window);
	sampler->x11_window = gdk_x11_window_get_xid(toplevel);
//	SAMPLER_LOG("start backend=x11 anchor=(%d,%d)\n", anchor_x, anchor_y);
//	SAMPLER_LOG("X11 toplevel window=0x%lx\n", sampler->x11_window);
	GdkSeat *seat = gdk_device_get_seat(device);
	if (seat != NULL) {
		gdk_seat_ungrab(seat);
		gdk_display_sync(display);
//		SAMPLER_LOG("released GTK pointer grab for XInput worker\n");
	}
	#elif defined(_WIN32)
	(void)display;
	(void)window;
	(void)device;
	sampler->backend = CROWN_INFINITE_DRAG_SAMPLER_BACKEND_WINDOWS;
	g_cond_init(&sampler->windows_setup_cond);
	sampler->windows_stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (sampler->windows_stop_event == NULL) {
//		SAMPLER_LOG("Windows setup failed: stop event error=%lu\n", GetLastError());
		g_cond_clear(&sampler->windows_setup_cond);
		g_mutex_clear(&sampler->mutex);
		g_free(sampler);
		return NULL;
	}
//	SAMPLER_LOG("start backend=windows anchor=(%d,%d)\n", anchor_x, anchor_y);
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
//	SAMPLER_LOG("stop requested backend=%s\n", backend_names[sampler->backend]);
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
/*
	SAMPLER_LOG("stop complete backend=%s final-samples=%d final-delta=(%.3f,%.3f)\n"
		, backend_names[sampler->backend]
		, *samples
		, *delta_x
		, *delta_y
		);
 */
	g_free(sampler);
}
