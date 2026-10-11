/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <glib.h>
#include <gdk/gdk.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#elif defined(__linux__)
#   include <errno.h>
#   include <dlfcn.h>
#if defined(CROWN_GTK3)
#   include <gdk/gdkx.h>
#else
#   include <gdk/x11/gdkx.h>
#endif
#   include <linux/input-event-codes.h>
#   include <poll.h>
#   include <sys/eventfd.h>
#   include <unistd.h>
#   include <X11/X.h>
#   include <X11/Xlib.h>
#   include <X11/extensions/XInput2.h>
#   include <X11/extensions/Xfixes.h>
#endif /* if defined(_WIN32) */

#if defined(__linux__)
#include <wayland-client-core.h>

#define WAYLAND_CLIENT_FUNCTIONS(X)      \
	X(wl_display_create_queue)           \
	X(wl_display_roundtrip_queue)        \
	X(wl_display_flush)                  \
	X(wl_display_prepare_read_queue)     \
	X(wl_display_dispatch_queue_pending) \
	X(wl_display_cancel_read)            \
	X(wl_display_get_fd)                 \
	X(wl_display_read_events)            \
	X(wl_event_queue_destroy)            \
	X(wl_proxy_create_wrapper)           \
	X(wl_proxy_set_queue)                \
	X(wl_proxy_wrapper_destroy)          \
	X(wl_proxy_add_dispatcher)           \
	X(wl_proxy_destroy)                  \
	X(wl_proxy_get_version)              \
	X(wl_proxy_marshal_flags)

#define DECLARE_WAYLAND_FUNCTION(name) static __typeof__(&name) crown_##name;
WAYLAND_CLIENT_FUNCTIONS(DECLARE_WAYLAND_FUNCTION)
#undef DECLARE_WAYLAND_FUNCTION

#define wl_display_create_queue (*crown_wl_display_create_queue)
#define wl_display_roundtrip_queue (*crown_wl_display_roundtrip_queue)
#define wl_display_flush (*crown_wl_display_flush)
#define wl_display_prepare_read_queue (*crown_wl_display_prepare_read_queue)
#define wl_display_dispatch_queue_pending (*crown_wl_display_dispatch_queue_pending)
#define wl_display_cancel_read (*crown_wl_display_cancel_read)
#define wl_display_get_fd (*crown_wl_display_get_fd)
#define wl_display_read_events (*crown_wl_display_read_events)
#define wl_event_queue_destroy (*crown_wl_event_queue_destroy)
#define wl_proxy_create_wrapper (*crown_wl_proxy_create_wrapper)
#define wl_proxy_set_queue (*crown_wl_proxy_set_queue)
#define wl_proxy_wrapper_destroy (*crown_wl_proxy_wrapper_destroy)
#define wl_proxy_add_dispatcher (*crown_wl_proxy_add_dispatcher)
#define wl_proxy_destroy (*crown_wl_proxy_destroy)
#define wl_proxy_get_version (*crown_wl_proxy_get_version)
#define wl_proxy_marshal_flags (*crown_wl_proxy_marshal_flags)

#if defined(CROWN_GTK3)
#include <gdk/gdkwayland.h>
#else
#include <gdk/wayland/gdkwayland.h>
#endif
#include <wayland-client.h>
#include "pointer-constraints-unstable-v1-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"
/* Keep protocol interfaces local; only the functions need runtime lookup. */
#include "wayland-client-protocol.c"
#include "pointer-constraints-unstable-v1-client-protocol.c"
#include "relative-pointer-unstable-v1-client-protocol.c"

static gboolean load_wayland_client(void)
{
	/* The resolved functions stay valid for every later Wayland drag. */
	static gsize initialized = 0;
	if (g_once_init_enter(&initialized)) {
		void *library = dlopen("libwayland-client.so.0", RTLD_NOW | RTLD_LOCAL);
		gboolean loaded = library != NULL;
		if (loaded) {
#define LOAD_WAYLAND_FUNCTION(name)                                         \
			crown_##name = (__typeof__(crown_##name))dlsym(library, #name); \
			if (crown_##name == NULL) loaded = FALSE;
			WAYLAND_CLIENT_FUNCTIONS(LOAD_WAYLAND_FUNCTION)
#undef LOAD_WAYLAND_FUNCTION
			if (!loaded)
				dlclose(library);
		}
		g_once_init_leave(&initialized, loaded ? 1 : 2);
	}
	return initialized == 1;
}

#define X11_FUNCTIONS(X)     \
	X(XCloseDisplay)         \
	X(XFreeEventData)        \
	X(XGetEventData)         \
	X(XGrabPointer)          \
	X(XNextEvent)            \
	X(XOpenDisplay)          \
	X(XPending)              \
	X(XQueryExtension)       \
	X(XQueryPointer)         \
	X(XSync)                 \
	X(XTranslateCoordinates) \
	X(XUngrabPointer)        \
	X(XWarpPointer)

#define XINPUT_FUNCTIONS(X) \
	X(XIQueryVersion)       \
	X(XISelectEvents)

#define XFIXES_FUNCTIONS(X) \
	X(XFixesHideCursor)     \
	X(XFixesQueryVersion)   \
	X(XFixesShowCursor)

#define DECLARE_X11_FUNCTION(name) static __typeof__(&name) crown_##name;
X11_FUNCTIONS(DECLARE_X11_FUNCTION)
XINPUT_FUNCTIONS(DECLARE_X11_FUNCTION)
XFIXES_FUNCTIONS(DECLARE_X11_FUNCTION)
#undef DECLARE_X11_FUNCTION

#define XCloseDisplay (*crown_XCloseDisplay)
#define XFreeEventData (*crown_XFreeEventData)
#define XGetEventData (*crown_XGetEventData)
#define XGrabPointer (*crown_XGrabPointer)
#define XNextEvent (*crown_XNextEvent)
#define XOpenDisplay (*crown_XOpenDisplay)
#define XPending (*crown_XPending)
#define XQueryExtension (*crown_XQueryExtension)
#define XQueryPointer (*crown_XQueryPointer)
#define XSync (*crown_XSync)
#define XTranslateCoordinates (*crown_XTranslateCoordinates)
#define XUngrabPointer (*crown_XUngrabPointer)
#define XWarpPointer (*crown_XWarpPointer)
#define XIQueryVersion (*crown_XIQueryVersion)
#define XISelectEvents (*crown_XISelectEvents)
#define XFixesHideCursor (*crown_XFixesHideCursor)
#define XFixesQueryVersion (*crown_XFixesQueryVersion)
#define XFixesShowCursor (*crown_XFixesShowCursor)

static gboolean load_x11_clients(void)
{
	static gsize initialized = 0;
	if (g_once_init_enter(&initialized)) {
		void *x11 = dlopen("libX11.so.6", RTLD_NOW | RTLD_LOCAL);
		void *xinput = x11 != NULL ? dlopen("libXi.so.6", RTLD_NOW | RTLD_LOCAL) : NULL;
		void *xfixes = xinput != NULL ? dlopen("libXfixes.so.3", RTLD_NOW | RTLD_LOCAL) : NULL;
		gboolean loaded = xfixes != NULL;
		if (loaded) {
#define LOAD_X11_FUNCTION(name)                                         \
			crown_##name = (__typeof__(crown_##name))dlsym(x11, #name); \
			if (crown_##name == NULL) loaded = FALSE;
#define LOAD_XINPUT_FUNCTION(name)                                         \
			crown_##name = (__typeof__(crown_##name))dlsym(xinput, #name); \
			if (crown_##name == NULL) loaded = FALSE;
#define LOAD_XFIXES_FUNCTION(name)                                         \
			crown_##name = (__typeof__(crown_##name))dlsym(xfixes, #name); \
			if (crown_##name == NULL) loaded = FALSE;
			X11_FUNCTIONS(LOAD_X11_FUNCTION)
			XINPUT_FUNCTIONS(LOAD_XINPUT_FUNCTION)
			XFIXES_FUNCTIONS(LOAD_XFIXES_FUNCTION)
#undef LOAD_XFIXES_FUNCTION
#undef LOAD_XINPUT_FUNCTION
#undef LOAD_X11_FUNCTION
		}
		if (!loaded) {
			if (xfixes != NULL)
				dlclose(xfixes);
			if (xinput != NULL)
				dlclose(xinput);
			if (x11 != NULL)
				dlclose(x11);
		}
		g_once_init_leave(&initialized, loaded ? 1 : 2);
	}
	return initialized == 1;
}

static void initialize_xlib_threads(void) __attribute__((constructor));

static void initialize_xlib_threads(void)
{
	/* XInitThreads must run before GTK opens its X11 display. */
	void *main_program = dlopen(NULL, RTLD_NOW);
	if (main_program != NULL) {
		__typeof__(&XInitThreads) init_threads = (__typeof__(&XInitThreads))dlsym(main_program, "XInitThreads");
		if (init_threads != NULL)
			init_threads();
		dlclose(main_program);
	}
}

#endif /* if defined(__linux__) */

#if defined(__linux__)
enum CrownInfiniteDragBackend
{
	CROWN_INFINITE_DRAG_X11,
	CROWN_INFINITE_DRAG_WAYLAND,
};
#endif

struct CrownInfiniteDragSampler
{
	GThread *thread;
	GMutex mutex;
	gint running;
	gint anchor_x;
	gint anchor_y;
	gint trigger_button;
	gint cancel_button;
	gboolean preserve_legacy_events;
	gdouble delta_x;
	gdouble delta_y;
	gdouble wheel_dx;
	gdouble wheel_dy;
	gint samples;
	gint released;
	gint cancel_requested;
#if defined(_WIN32)
	HANDLE windows_stop_event;
	gint windows_absolute_x;
	gint windows_absolute_y;
	gboolean windows_absolute_initialized;
#elif defined(__linux__)
	enum CrownInfiniteDragBackend backend;
	Window x11_window;
	gint wake_fd;
	struct wl_display *wayland_display;
	struct wl_event_queue *wayland_queue;
	struct wl_registry *wayland_registry;
	struct wl_pointer *wayland_pointer;
	struct zwp_pointer_constraints_v1 *pointer_constraints;
	struct zwp_locked_pointer_v1 *locked_pointer;
	struct zwp_relative_pointer_manager_v1 *relative_pointer_manager;
	struct zwp_relative_pointer_v1 *relative_pointer;
	gint wayland_locked;
	GdkCursor *wayland_original_cursor;
#if defined(CROWN_GTK3)
	GdkWindow *wayland_surface;
#else
	GdkSurface *wayland_surface;
#endif
#endif /* if defined(_WIN32) */
};

static void store_delta(struct CrownInfiniteDragSampler *sampler, gdouble delta_x, gdouble delta_y)
{
	g_mutex_lock(&sampler->mutex);
	sampler->delta_x += delta_x;
	sampler->delta_y += delta_y;
	g_mutex_unlock(&sampler->mutex);
}

static void store_sample(struct CrownInfiniteDragSampler *sampler)
{
	g_mutex_lock(&sampler->mutex);
	sampler->samples += 1;
	g_mutex_unlock(&sampler->mutex);
}

static void store_wheel(struct CrownInfiniteDragSampler *sampler, gdouble wheel_dx, gdouble wheel_dy)
{
	g_mutex_lock(&sampler->mutex);
	sampler->wheel_dx += wheel_dx;
	sampler->wheel_dy += wheel_dy;
	g_mutex_unlock(&sampler->mutex);
}

#if defined(_WIN32)
#define WINDOWS_KERNEL_FUNCTIONS(X) \
	X(CloseHandle)                  \
	X(CreateEventW)                 \
	X(SetEvent)

#define WINDOWS_USER_FUNCTIONS(X) \
	X(ClipCursor)                 \
	X(CreateWindowExW)            \
	X(DefWindowProcW)             \
	X(DestroyWindow)              \
	X(GetCursorPos)               \
	X(GetRawInputData)            \
	X(GetSystemMetrics)           \
	X(MsgWaitForMultipleObjects)  \
	X(PeekMessageW)               \
	X(RegisterRawInputDevices)    \
	X(SetCursorPos)               \
	X(ShowCursor)

#define DECLARE_WINDOWS_FUNCTION(name) static __typeof__(&name) crown_##name;
WINDOWS_KERNEL_FUNCTIONS(DECLARE_WINDOWS_FUNCTION)
WINDOWS_USER_FUNCTIONS(DECLARE_WINDOWS_FUNCTION)
#undef DECLARE_WINDOWS_FUNCTION

#define CloseHandle (*crown_CloseHandle)
#define CreateEventW (*crown_CreateEventW)
#define SetEvent (*crown_SetEvent)
#define ClipCursor (*crown_ClipCursor)
#define CreateWindowExW (*crown_CreateWindowExW)
#define DefWindowProcW (*crown_DefWindowProcW)
#define DestroyWindow (*crown_DestroyWindow)
#define GetCursorPos (*crown_GetCursorPos)
#define GetRawInputData (*crown_GetRawInputData)
#define GetSystemMetrics (*crown_GetSystemMetrics)
#define MsgWaitForMultipleObjects (*crown_MsgWaitForMultipleObjects)
#define PeekMessageW (*crown_PeekMessageW)
#define RegisterRawInputDevices (*crown_RegisterRawInputDevices)
#define SetCursorPos (*crown_SetCursorPos)
#define ShowCursor (*crown_ShowCursor)

static gboolean load_windows_clients(void)
{
	/* The resolved functions stay valid for every later Windows drag. */
	static gsize initialized = 0;
	if (g_once_init_enter(&initialized)) {
		HMODULE kernel32 = LoadLibraryW(L"kernel32.dll");
		HMODULE user32 = kernel32 != NULL ? LoadLibraryW(L"user32.dll") : NULL;
		gboolean loaded = user32 != NULL;
		if (loaded) {
#define LOAD_KERNEL_FUNCTION(name)                                                    \
			crown_##name = (__typeof__(crown_##name))GetProcAddress(kernel32, #name); \
			if (crown_##name == NULL) loaded = FALSE;
#define LOAD_USER_FUNCTION(name)                                                    \
			crown_##name = (__typeof__(crown_##name))GetProcAddress(user32, #name); \
			if (crown_##name == NULL) loaded = FALSE;
			WINDOWS_KERNEL_FUNCTIONS(LOAD_KERNEL_FUNCTION)
			WINDOWS_USER_FUNCTIONS(LOAD_USER_FUNCTION)
#undef LOAD_USER_FUNCTION
#undef LOAD_KERNEL_FUNCTION
		}
		if (!loaded) {
			if (user32 != NULL)
				FreeLibrary(user32);
			if (kernel32 != NULL)
				FreeLibrary(kernel32);
		}
		g_once_init_leave(&initialized, loaded ? 1 : 2);
	}
	return initialized == 1;
}

/* Event-driven only: do not replace with cursor-position polling or repeated SetCursorPos calls. */

static gboolean register_windows_raw_input(HWND target, gboolean preserve_legacy_events)
{
	RAWINPUTDEVICE mouse =
	{
		.usUsagePage = 0x01,
		.usUsage = 0x02,
		.dwFlags = target != NULL
			? RIDEV_INPUTSINK | (preserve_legacy_events ? 0 : RIDEV_NOLEGACY)
			: RIDEV_REMOVE,
		.hwndTarget = target,
	};
	return RegisterRawInputDevices(&mouse, 1, sizeof(mouse)) != FALSE;
}

static gint scale_windows_absolute_coordinate(LONG coordinate, gint size)
{
	return ((int64_t)coordinate * MAX(size - 1, 1) + 32767)/65535;
}

static void process_windows_raw_mouse(struct CrownInfiniteDragSampler *sampler, const RAWMOUSE *mouse)
{
	gint delta_x = 0;
	gint delta_y = 0;
	if ((mouse->usFlags & MOUSE_MOVE_ABSOLUTE) != 0) {
		gboolean virtual_desktop = (mouse->usFlags & MOUSE_VIRTUAL_DESKTOP) != 0;
		gint screen_x = virtual_desktop ? GetSystemMetrics(SM_XVIRTUALSCREEN) : 0;
		gint screen_y = virtual_desktop ? GetSystemMetrics(SM_YVIRTUALSCREEN) : 0;
		gint screen_width = virtual_desktop ? GetSystemMetrics(SM_CXVIRTUALSCREEN) : GetSystemMetrics(SM_CXSCREEN);
		gint screen_height = virtual_desktop ? GetSystemMetrics(SM_CYVIRTUALSCREEN) : GetSystemMetrics(SM_CYSCREEN);
		gint absolute_x = screen_x + scale_windows_absolute_coordinate(mouse->lLastX, screen_width);
		gint absolute_y = screen_y + scale_windows_absolute_coordinate(mouse->lLastY, screen_height);
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

	if ((mouse->usButtonFlags & RI_MOUSE_WHEEL) != 0) {
		SHORT wheel_delta = (SHORT)mouse->usButtonData;
		store_wheel(sampler, 0.0, -(gdouble)wheel_delta/120.0);
	}
	if ((mouse->usButtonFlags & RI_MOUSE_HWHEEL) != 0) {
		SHORT wheel_delta = (SHORT)mouse->usButtonData;
		store_wheel(sampler, (gdouble)wheel_delta/120.0, 0.0);
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
		return;
	}

	USHORT trigger_button_up = 0;
	switch (sampler->trigger_button) {
	case GDK_BUTTON_PRIMARY:
		trigger_button_up = RI_MOUSE_LEFT_BUTTON_UP;
		break;
	case GDK_BUTTON_MIDDLE:
		trigger_button_up = RI_MOUSE_MIDDLE_BUTTON_UP;
		break;
	case GDK_BUTTON_SECONDARY:
		trigger_button_up = RI_MOUSE_RIGHT_BUTTON_UP;
		break;
	}
	if (trigger_button_up != 0 && (mouse->usButtonFlags & trigger_button_up) != 0) {
		g_atomic_int_set(&sampler->released, TRUE);
		g_atomic_int_set(&sampler->running, FALSE);
	}
}

static void drain_windows_raw_input_messages(struct CrownInfiniteDragSampler *sampler)
{
	MSG message;
	while (PeekMessageW(&message, NULL, WM_INPUT, WM_INPUT, PM_REMOVE)) {
		RAWINPUT input;
		UINT input_size = sizeof(input);
		UINT result = GetRawInputData((HRAWINPUT)message.lParam
			, RID_INPUT
			, &input
			, &input_size
			, sizeof(RAWINPUTHEADER)
			);
		if (result != (UINT)-1 && input.header.dwType == RIM_TYPEMOUSE) {
			process_windows_raw_mouse(sampler, &input.data.mouse);
		}
		/* Release the raw input handle after reading it. */
		DefWindowProcW(message.hwnd, message.message, message.wParam, message.lParam);
	}
}

static gpointer sample_pointer_windows(gpointer data)
{
	struct CrownInfiniteDragSampler *sampler = data;
	/* Only a raw input target is needed; the worker reads WM_INPUT directly. */
	HWND window = CreateWindowExW(0
		, L"STATIC"
		, L""
		, 0
		, 0
		, 0
		, 0
		, 0
		, HWND_MESSAGE
		, NULL
		, NULL
		, NULL
		);
	gboolean registered = FALSE;
	if (window == NULL)
		goto setup_failed;

	if (!register_windows_raw_input(window, sampler->preserve_legacy_events))
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

		drain_windows_raw_input_messages(sampler);
	}

	/* GTK may observe the release just before this worker does. */
	drain_windows_raw_input_messages(sampler);

	/* Restore and unconfine before the GTK thread reveals the cursor after joining. */
	SetCursorPos(sampler->anchor_x, sampler->anchor_y);
	ClipCursor(NULL);

	register_windows_raw_input(NULL, FALSE);
	DestroyWindow(window);
	return NULL;

setup_failed:
	if (registered)
		register_windows_raw_input(NULL, FALSE);
	if (window != NULL)
		DestroyWindow(window);
	g_atomic_int_set(&sampler->running, FALSE);
	return NULL;
}

#elif defined(__linux__)
static int wayland_registry_event(const void *data, void *proxy, uint32_t opcode
	, const struct wl_message *message, union wl_argument *args)
{
	(void)opcode;
	if (strcmp(message->name, "global") != 0)
		return 0;

	struct CrownInfiniteDragSampler *sampler = (struct CrownInfiniteDragSampler *)data;
	struct wl_registry *registry = proxy;
	/* wl_registry.global(name, interface, version). */
	uint32_t name = args[0].u;
	const char *interface = args[1].s;
	uint32_t version = args[2].u;
	if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0) {
		sampler->pointer_constraints = wl_registry_bind(registry, name
			, &zwp_pointer_constraints_v1_interface, MIN(version, 1u));
	} else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0) {
		sampler->relative_pointer_manager = wl_registry_bind(registry, name
			, &zwp_relative_pointer_manager_v1_interface, MIN(version, 1u));
	}
	return 0;
}

static gint gdk_button_from_wayland(uint32_t button)
{
	switch (button) {
	case BTN_LEFT:
		return GDK_BUTTON_PRIMARY;
	case BTN_MIDDLE:
		return GDK_BUTTON_MIDDLE;
	case BTN_RIGHT:
		return GDK_BUTTON_SECONDARY;
	default:
		return 0;
	}
}

static int wayland_pointer_event(const void *data, void *proxy, uint32_t opcode
	, const struct wl_message *message, union wl_argument *args)
{
	(void)proxy;
	(void)opcode;
	/* GTK handles the other pointer events, including scrolling. */
	if (strcmp(message->name, "button") != 0)
		return 0;

	struct CrownInfiniteDragSampler *sampler = (struct CrownInfiniteDragSampler *)data;
	/* wl_pointer.button(serial, time, button, state). */
	gint gdk_button = gdk_button_from_wayland(args[2].u);
	uint32_t state = args[3].u;
	if (state == WL_POINTER_BUTTON_STATE_PRESSED
		&& sampler->cancel_button != 0
		&& gdk_button == sampler->cancel_button
		) {
		g_atomic_int_set(&sampler->cancel_requested, TRUE);
		g_atomic_int_set(&sampler->running, FALSE);
	} else if (state == WL_POINTER_BUTTON_STATE_RELEASED && gdk_button == sampler->trigger_button) {
		g_atomic_int_set(&sampler->released, TRUE);
		g_atomic_int_set(&sampler->running, FALSE);
	}
	return 0;
}

static int wayland_relative_pointer_event(const void *data, void *proxy, uint32_t opcode
	, const struct wl_message *message, union wl_argument *args)
{
	(void)proxy;
	(void)opcode;
	(void)message;
	struct CrownInfiniteDragSampler *sampler = (struct CrownInfiniteDragSampler *)data;
	/* relative_motion(time_hi, time_lo, dx, dy, dx_unaccelerated, dy_unaccelerated). */
	wl_fixed_t dx = args[2].f;
	wl_fixed_t dy = args[3].f;
	if (g_atomic_int_get(&sampler->wayland_locked)) {
		store_delta(sampler, wl_fixed_to_double(dx), wl_fixed_to_double(dy));
		store_sample(sampler);
	}
	return 0;
}

static int wayland_locked_pointer_event(const void *data, void *proxy, uint32_t opcode
	, const struct wl_message *message, union wl_argument *args)
{
	(void)proxy;
	(void)opcode;
	(void)args;
	struct CrownInfiniteDragSampler *sampler = (struct CrownInfiniteDragSampler *)data;
	if (strcmp(message->name, "locked") == 0) {
		g_atomic_int_set(&sampler->wayland_locked, TRUE);
	} else if (strcmp(message->name, "unlocked") == 0) {
		g_atomic_int_set(&sampler->wayland_locked, FALSE);
		g_atomic_int_set(&sampler->cancel_requested, TRUE);
		g_atomic_int_set(&sampler->running, FALSE);
	}
	return 0;
}

static gboolean create_wayland_pointer_lock(struct CrownInfiniteDragSampler *sampler
	, GdkDisplay *display
	, struct wl_surface *surface
	, struct wl_seat *seat
	)
{
	if (!load_wayland_client())
		return FALSE;
	sampler->wayland_display = gdk_wayland_display_get_wl_display(display);
	if (sampler->wayland_display == NULL || surface == NULL || seat == NULL)
		return FALSE;

	sampler->wayland_queue = wl_display_create_queue(sampler->wayland_display);
	if (sampler->wayland_queue == NULL)
		return FALSE;

	/* The wrapper assigns the registry and its children to our queue from creation. */
	struct wl_display *wrapped_display = wl_proxy_create_wrapper(sampler->wayland_display);
	if (wrapped_display == NULL)
		return FALSE;
	wl_proxy_set_queue((struct wl_proxy *)wrapped_display, sampler->wayland_queue);
	sampler->wayland_registry = wl_display_get_registry(wrapped_display);
	wl_proxy_wrapper_destroy(wrapped_display);
	if (sampler->wayland_registry == NULL)
		return FALSE;
	wl_proxy_add_dispatcher((struct wl_proxy *)sampler->wayland_registry
		, wayland_registry_event, sampler, NULL);
	if (wl_display_roundtrip_queue(sampler->wayland_display, sampler->wayland_queue) < 0
		|| sampler->pointer_constraints == NULL
		|| sampler->relative_pointer_manager == NULL
		) {
		return FALSE;
	}
	struct wl_seat *wrapped_seat = wl_proxy_create_wrapper(seat);
	if (wrapped_seat == NULL)
		return FALSE;
	wl_proxy_set_queue((struct wl_proxy *)wrapped_seat, sampler->wayland_queue);
	sampler->wayland_pointer = wl_seat_get_pointer(wrapped_seat);
	wl_proxy_wrapper_destroy(wrapped_seat);
	if (sampler->wayland_pointer == NULL)
		return FALSE;
	wl_proxy_add_dispatcher((struct wl_proxy *)sampler->wayland_pointer
		, wayland_pointer_event, sampler, NULL);

	sampler->locked_pointer = zwp_pointer_constraints_v1_lock_pointer(sampler->pointer_constraints
		, surface
		, sampler->wayland_pointer
		, NULL
		, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT
		);
	sampler->relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(sampler->relative_pointer_manager, sampler->wayland_pointer);
	if (sampler->locked_pointer == NULL || sampler->relative_pointer == NULL)
		return FALSE;

	wl_proxy_add_dispatcher((struct wl_proxy *)sampler->locked_pointer
		, wayland_locked_pointer_event, sampler, NULL);
	wl_proxy_add_dispatcher((struct wl_proxy *)sampler->relative_pointer
		, wayland_relative_pointer_event, sampler, NULL);
	if (wl_display_flush(sampler->wayland_display) < 0 && errno != EAGAIN)
		return FALSE;
	return TRUE;
}

static void destroy_wayland_pointer_lock(struct CrownInfiniteDragSampler *sampler)
{
	if (sampler->relative_pointer != NULL)
		zwp_relative_pointer_v1_destroy(sampler->relative_pointer);
	if (sampler->locked_pointer != NULL)
		zwp_locked_pointer_v1_destroy(sampler->locked_pointer);
	if (sampler->wayland_pointer != NULL) {
		if (wl_pointer_get_version(sampler->wayland_pointer) >= WL_POINTER_RELEASE_SINCE_VERSION)
			wl_pointer_release(sampler->wayland_pointer);
		else
			wl_pointer_destroy(sampler->wayland_pointer);
	}
	if (sampler->relative_pointer_manager != NULL)
		zwp_relative_pointer_manager_v1_destroy(sampler->relative_pointer_manager);
	if (sampler->pointer_constraints != NULL)
		zwp_pointer_constraints_v1_destroy(sampler->pointer_constraints);
	if (sampler->wayland_registry != NULL)
		wl_registry_destroy(sampler->wayland_registry);
	if (sampler->wayland_display != NULL)
		wl_display_flush(sampler->wayland_display);
	if (sampler->wayland_queue != NULL)
		wl_event_queue_destroy(sampler->wayland_queue);
}

#if defined(CROWN_GTK3)
static void hide_wayland_cursor(struct CrownInfiniteDragSampler *sampler, GdkDisplay *display, GdkWindow *surface)
#else
static void hide_wayland_cursor(struct CrownInfiniteDragSampler *sampler, GdkDisplay *display, GdkSurface *surface)
#endif
{
#if defined(CROWN_GTK3)
	GdkCursor *hidden_cursor = gdk_cursor_new_from_name(display, "none");
#else
	(void)display;
	GdkCursor *hidden_cursor = gdk_cursor_new_from_name("none", NULL);
#endif
	if (hidden_cursor == NULL)
		return;
	sampler->wayland_surface = g_object_ref(surface);
#if defined(CROWN_GTK3)
	sampler->wayland_original_cursor = gdk_window_get_cursor(surface);
#else
	sampler->wayland_original_cursor = gdk_surface_get_cursor(surface);
#endif
	if (sampler->wayland_original_cursor != NULL)
		g_object_ref(sampler->wayland_original_cursor);
#if defined(CROWN_GTK3)
	gdk_window_set_cursor(surface, hidden_cursor);
#else
	gdk_surface_set_cursor(surface, hidden_cursor);
#endif
	g_object_unref(hidden_cursor);
}

static void restore_wayland_cursor(struct CrownInfiniteDragSampler *sampler)
{
	if (sampler->wayland_surface == NULL)
		return;
#if defined(CROWN_GTK3)
	gdk_window_set_cursor(sampler->wayland_surface, sampler->wayland_original_cursor);
#else
	gdk_surface_set_cursor(sampler->wayland_surface, sampler->wayland_original_cursor);
#endif
	if (sampler->wayland_original_cursor != NULL)
		g_object_unref(sampler->wayland_original_cursor);
	g_object_unref(sampler->wayland_surface);
}

static gpointer sample_pointer_wayland(gpointer data)
{
	struct CrownInfiniteDragSampler *sampler = data;
	while (g_atomic_int_get(&sampler->running)) {
		while (wl_display_prepare_read_queue(sampler->wayland_display, sampler->wayland_queue) != 0) {
			if (wl_display_dispatch_queue_pending(sampler->wayland_display, sampler->wayland_queue) < 0)
				goto dispatch_failed;
		}
		if (!g_atomic_int_get(&sampler->running)) {
			wl_display_cancel_read(sampler->wayland_display);
			break;
		}

		int flush_result = wl_display_flush(sampler->wayland_display);
		if (flush_result < 0 && errno != EAGAIN) {
			wl_display_cancel_read(sampler->wayland_display);
			goto dispatch_failed;
		}
		struct pollfd descriptors[] =
		{
			{ .fd = wl_display_get_fd(sampler->wayland_display), .events = POLLIN | (flush_result < 0 ? POLLOUT : 0) },
			{ .fd = sampler->wake_fd, .events = POLLIN },
		};
		int poll_result;
		do {
			poll_result = poll(descriptors, G_N_ELEMENTS(descriptors), -1);
		} while (poll_result < 0 && errno == EINTR);
		if (poll_result < 0) {
			wl_display_cancel_read(sampler->wayland_display);
			goto dispatch_failed;
		}
		if (!g_atomic_int_get(&sampler->running) || descriptors[1].revents != 0) {
			wl_display_cancel_read(sampler->wayland_display);
			break;
		}
		if ((descriptors[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
			wl_display_cancel_read(sampler->wayland_display);
			goto dispatch_failed;
		}
		if ((descriptors[0].revents & POLLIN) == 0) {
			wl_display_cancel_read(sampler->wayland_display);
			continue;
		}
		if (wl_display_read_events(sampler->wayland_display) < 0
			|| wl_display_dispatch_queue_pending(sampler->wayland_display, sampler->wayland_queue) < 0
			) {
			goto dispatch_failed;
		}
		/* Reading may also queue button and scroll events for GDK. */
		g_main_context_wakeup(NULL);
	}
	return NULL;

dispatch_failed:
	g_atomic_int_set(&sampler->cancel_requested, TRUE);
	g_atomic_int_set(&sampler->running, FALSE);
	g_main_context_wakeup(NULL);
	return NULL;
}

/* Keep grab+XFixes-hide+XI2-raw-motion together: this combination is what activates Xwayland's relative-pointer device. */
static gboolean warp_x11_pointer_to_anchor(Display *display, Window root, struct CrownInfiniteDragSampler *sampler)
{
	int window_x;
	int window_y;
	Window child;
	if (!XTranslateCoordinates(display
		, root
		, sampler->x11_window
		, sampler->anchor_x
		, sampler->anchor_y
		, &window_x
		, &window_y
		, &child
		)) {
		return FALSE;
	}

	XWarpPointer(display
		, None
		, sampler->x11_window
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

static void destroy_x11_pointer_grab(Display *display, Window root, struct CrownInfiniteDragSampler *sampler)
{
	/* Commit the original position hint before revealing the pointer. */
	warp_x11_pointer_to_anchor(display, root, sampler);

	/* Ungrab while hidden or Xwayland may discard the position hint. */
	XUngrabPointer(display, CurrentTime);
	XSync(display, False);
	XFixesShowCursor(display, sampler->x11_window);
	XSync(display, False);
}

static gpointer x11_sampler_failed(struct CrownInfiniteDragSampler *sampler, Display *display)
{
	if (display != NULL)
		XCloseDisplay(display);
	g_atomic_int_set(&sampler->cancel_requested, TRUE);
	g_atomic_int_set(&sampler->running, FALSE);
	return NULL;
}

static gpointer sample_pointer_x11(gpointer data)
{
	struct CrownInfiniteDragSampler *sampler = data;
	Display *display = XOpenDisplay(NULL);
	if (display == NULL)
		return x11_sampler_failed(sampler, NULL);
	int screen_number = DefaultScreen(display);
	Window root = RootWindow(display, screen_number);

	int xinput_opcode;
	int xinput_event;
	int xinput_error;
	if (!XQueryExtension(display, "XInputExtension", &xinput_opcode, &xinput_event, &xinput_error)) {
		return x11_sampler_failed(sampler, display);
	}

	int xinput_major = 2;
	int xinput_minor = 1;
	int status = XIQueryVersion(display, &xinput_major, &xinput_minor);
	if (status != Success
		|| xinput_major < 2
		|| (xinput_major == 2 && xinput_minor < 1)
		) {
		return x11_sampler_failed(sampler, display);
	}

	int xfixes_major = 4;
	int xfixes_minor = 0;
	if (!XFixesQueryVersion(display, &xfixes_major, &xfixes_minor)
		|| xfixes_major < 4
		) {
		return x11_sampler_failed(sampler, display);
	}

	unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 };
	XISetMask(mask, XI_RawMotion);
	XISetMask(mask, XI_RawButtonPress);
	XISetMask(mask, XI_RawButtonRelease);
	XIEventMask event_mask =
	{
		.deviceid = XIAllMasterDevices,
		.mask_len = sizeof(mask),
		.mask = mask,
	};
	if (XISelectEvents(display, root, &event_mask, 1) != Success) {
		return x11_sampler_failed(sampler, display);
	}
	XSync(display, False);

	if (XGrabPointer(display
		, sampler->x11_window
		, False
		, ButtonReleaseMask
		, GrabModeAsync
		, GrabModeAsync
		, sampler->x11_window
		, None
		, CurrentTime
		) != GrabSuccess) {
		return x11_sampler_failed(sampler, display);
	}
	XFixesHideCursor(display, sampler->x11_window);
	XSync(display, False);

	/* A single warp; repeated warps would be reported as synthetic raw motion. */
	if (!warp_x11_pointer_to_anchor(display, root, sampler)) {
		destroy_x11_pointer_grab(display, root, sampler);
		return x11_sampler_failed(sampler, display);
	}

	while (g_atomic_int_get(&sampler->running)) {
		while (XPending(display) > 0) {
			XEvent event;
			XNextEvent(display, &event);
			if (event.type == GenericEvent
				&& event.xcookie.extension == xinput_opcode
				&& XGetEventData(display, &event.xcookie)
				) {
				XIRawEvent *raw = event.xcookie.data;
				if (event.xcookie.evtype == XI_RawButtonRelease) {
					if ((gint)raw->detail == sampler->trigger_button) {
						g_atomic_int_set(&sampler->released, TRUE);
						g_atomic_int_set(&sampler->running, FALSE);
					}
				} else if (event.xcookie.evtype == XI_RawButtonPress) {
					if (sampler->cancel_button != 0 && raw->detail == sampler->cancel_button) {
						g_atomic_int_set(&sampler->cancel_requested, TRUE);
						g_atomic_int_set(&sampler->running, FALSE);
					} else if (raw->detail == 4) {
						store_wheel(sampler, 0.0, -1.0);
					} else if (raw->detail == 5) {
						store_wheel(sampler, 0.0, 1.0);
					} else if (raw->detail == 6) {
						store_wheel(sampler, -1.0, 0.0);
					} else if (raw->detail == 7) {
						store_wheel(sampler, 1.0, 0.0);
					}
				} else if (event.xcookie.evtype == XI_RawMotion) {
					gdouble delta_x = 0.0;
					gdouble delta_y = 0.0;
					if (raw->valuators.mask_len > 0) {
						int value_index = 0;
						if (XIMaskIsSet(raw->valuators.mask, 0))
							delta_x = raw->valuators.values[value_index++];
						if (XIMaskIsSet(raw->valuators.mask, 1))
							delta_y = raw->valuators.values[value_index];
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
			{ .fd = sampler->wake_fd, .events = POLLIN },
		};
		int poll_result;
		do {
			poll_result = poll(descriptors, G_N_ELEMENTS(descriptors), -1);
		} while (poll_result < 0 && errno == EINTR);
		if (poll_result < 0) {
			break;
		}
		if ((descriptors[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
			break;
		}
	}

	if (g_atomic_int_get(&sampler->running)) {
		g_atomic_int_set(&sampler->cancel_requested, TRUE);
		g_atomic_int_set(&sampler->running, FALSE);
	}
	destroy_x11_pointer_grab(display, root, sampler);
	XCloseDisplay(display);
	return NULL;
}

#endif /* if defined(_WIN32) */

static gpointer sample_pointer(gpointer data)
{
#if defined(_WIN32)
	return sample_pointer_windows(data);
#elif defined(__linux__)
	struct CrownInfiniteDragSampler *sampler = data;
	return sampler->backend == CROWN_INFINITE_DRAG_WAYLAND
		? sample_pointer_wayland(data)
		: sample_pointer_x11(data);
#endif
	return NULL;
}

#if defined(CROWN_GTK3)
void *crown_infinite_drag_sampler_start(GdkDisplay *display, GdkWindow *window, GdkDevice *device, gint trigger_button, gint cancel_button, gboolean preserve_legacy_events)
#else
void *crown_infinite_drag_sampler_start(GdkDisplay *display, GdkSurface *surface, GdkDevice *device, gint trigger_button, gint cancel_button, gboolean preserve_legacy_events)
#endif
{
	struct CrownInfiniteDragSampler *sampler = g_new0(struct CrownInfiniteDragSampler, 1);
	g_mutex_init(&sampler->mutex);
#if defined(__linux__)
	sampler->wake_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if (sampler->wake_fd < 0)
		goto setup_failed;
#endif
	g_atomic_int_set(&sampler->running, TRUE);
	sampler->trigger_button = trigger_button;
	sampler->cancel_button = cancel_button;
	sampler->preserve_legacy_events = preserve_legacy_events;
#if defined(_WIN32)
	(void)display;
#if defined(CROWN_GTK3)
	(void)window;
#else
	(void)surface;
#endif
	(void)device;
	if (!load_windows_clients())
		goto setup_failed;

	POINT cursor_position;
	if (!GetCursorPos(&cursor_position))
		goto setup_failed;
	sampler->anchor_x = cursor_position.x;
	sampler->anchor_y = cursor_position.y;
	sampler->windows_stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (sampler->windows_stop_event == NULL)
		goto setup_failed;
	/* ShowCursor's counter is thread-local, so change it on GTK's UI thread. */
	ShowCursor(FALSE);
#elif defined(__linux__)
#if defined(CROWN_GTK3)
	GdkWindow *surface = gdk_window_get_toplevel(window);
#endif
	if (GDK_IS_WAYLAND_DISPLAY(display)) {
		sampler->backend = CROWN_INFINITE_DRAG_WAYLAND;
#if defined(CROWN_GTK3)
		struct wl_surface *wl_surface = gdk_wayland_window_get_wl_surface(surface);
#else
		struct wl_surface *wl_surface = gdk_wayland_surface_get_wl_surface(surface);
#endif
		struct wl_seat *wl_seat = gdk_wayland_device_get_wl_seat(device);
		if (!create_wayland_pointer_lock(sampler, display, wl_surface, wl_seat))
			goto setup_failed;
		hide_wayland_cursor(sampler, display, surface);
	} else {
		sampler->backend = CROWN_INFINITE_DRAG_X11;
		if (!load_x11_clients())
			goto setup_failed;
#if defined(CROWN_GTK3)
		sampler->x11_window = gdk_x11_window_get_xid(surface);
#else
		sampler->x11_window = gdk_x11_surface_get_xid(surface);
#endif
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
			goto setup_failed;
		}
#if defined(CROWN_GTK3)
		GdkSeat *seat = gdk_device_get_seat(device);
		if (seat != NULL) {
			gdk_seat_ungrab(seat);
			gdk_display_sync(display);
		}
#else
		/* GTK4's button press owns an implicit X grab; release it before the sampler grabs. */
		XUngrabPointer(xdisplay, CurrentTime);
		XSync(xdisplay, False);
#endif
	}
#else
	(void)display;
#if defined(CROWN_GTK3)
	(void)window;
#else
	(void)surface;
#endif
	(void)device;
#endif /* if defined(_WIN32) */
	sampler->thread = g_thread_new("infinite-drag", sample_pointer, sampler);
	return sampler;

setup_failed:
#if defined(__linux__)
	if (sampler->backend == CROWN_INFINITE_DRAG_WAYLAND)
		destroy_wayland_pointer_lock(sampler);
	if (sampler->wake_fd >= 0)
		close(sampler->wake_fd);
#endif
	g_mutex_clear(&sampler->mutex);
	g_free(sampler);
	return NULL;
}

void crown_infinite_drag_sampler_drain(void *data, gdouble *delta_x, gdouble *delta_y, gdouble *wheel_dx, gdouble *wheel_dy, gint *samples)
{
	struct CrownInfiniteDragSampler *sampler = data;
	g_mutex_lock(&sampler->mutex);
	*delta_x = sampler->delta_x;
	*delta_y = sampler->delta_y;
	*wheel_dx = sampler->wheel_dx;
	*wheel_dy = sampler->wheel_dy;
	*samples = sampler->samples;
	sampler->delta_x = 0;
	sampler->delta_y = 0;
	sampler->wheel_dx = 0;
	sampler->wheel_dy = 0;
	sampler->samples = 0;
	g_mutex_unlock(&sampler->mutex);
}

gboolean crown_infinite_drag_sampler_released(void *data)
{
	struct CrownInfiniteDragSampler *sampler = data;
	return g_atomic_int_get(&sampler->released);
}

gboolean crown_infinite_drag_sampler_cancel_requested(void *data)
{
	struct CrownInfiniteDragSampler *sampler = data;
	return g_atomic_int_get(&sampler->cancel_requested);
}

void crown_infinite_drag_sampler_stop(void *data, gdouble *delta_x, gdouble *delta_y, gdouble *wheel_dx, gdouble *wheel_dy, gint *samples)
{
	struct CrownInfiniteDragSampler *sampler = data;
	g_atomic_int_set(&sampler->running, FALSE);
#if defined(_WIN32)
	SetEvent(sampler->windows_stop_event);
#elif defined(__linux__)
	if (sampler->wake_fd >= 0) {
		uint64_t wake_value = 1;
		while (write(sampler->wake_fd, &wake_value, sizeof(wake_value)) < 0 && errno == EINTR) {
		}
	}
#endif
	g_thread_join(sampler->thread);
#if defined(_WIN32)
	ShowCursor(TRUE);
#endif
	crown_infinite_drag_sampler_drain(sampler, delta_x, delta_y, wheel_dx, wheel_dy, samples);
#if defined(_WIN32)
	CloseHandle(sampler->windows_stop_event);
#elif defined(__linux__)
	if (sampler->backend == CROWN_INFINITE_DRAG_WAYLAND) {
		restore_wayland_cursor(sampler);
		destroy_wayland_pointer_lock(sampler);
	}
	if (sampler->wake_fd >= 0)
		close(sampler->wake_fd);
#endif
	g_mutex_clear(&sampler->mutex);
	g_free(sampler);
}
