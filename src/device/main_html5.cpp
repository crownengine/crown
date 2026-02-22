/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PLATFORM_EMSCRIPTEN
#include "core/command_line.h"
#include "core/containers/array.inl"
#include "core/guid.h"
#include "core/memory/globals.h"
#include "core/memory/memory.inl"
#include "core/option.inl"
#include "core/profiler.h"
#include "core/thread/spsc_queue.inl"
#include "core/unit_tests.h"
#include "core/unit_tests.h"
#include "device/device.h"
#include "device/device_event_queue.inl"
#include "device/device_options.h"
#include "device/display.h"
#include "device/types.h"
#include "device/window.h"
#include <stdlib.h> // EXIT_SUCCESS etc.
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED
#include <stb_sprintf.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#ifndef CROWN_HTML5_CANVAS_NAME
	#define CROWN_HTML5_CANVAS_NAME "canvas"
#endif

// See: https://emscripten.org/docs/api_reference/html5.h.html#callback-functions
#define EM_EVENT_STOP      true  // The handler consumed the event (calls .preventDefault()).
#define EM_EVENT_PROPAGATE false // Event not consumed (carries default browser event action).

namespace crown
{
// code-format off
EM_JS(void, crown_js_request_pointer_lock, (void), {
	if (Module.canvas.requestPointerLock) {
		Module.canvas.requestPointerLock({ unadjustedMovement: true });
	}
});

EM_JS(void, crown_js_request_pointer_lock_fallback, (void), {
	if (Module.canvas.requestPointerLock) {
		Module.canvas.requestPointerLock({ unadjustedMovement: false });
	}
});

EM_JS(void, crown_js_exit_pointer_lock, (void), {
	if (document.exitPointerLock) {
		document.exitPointerLock();
	}
});
// code-format on

struct WindowEmscripten : public Window
{
	WindowEmscripten()
	{
	}

	void open(u16 x, u16 y, u16 width, u16 height, u32 parent) override
	{
		CE_UNUSED_5(x, y, width, height, parent);
	}

	void close() override
	{
	}

	void show() override
	{
	}

	void hide() override
	{
	}

	void resize(u16 width, u16 height) override
	{
		CE_UNUSED_2(width, height);
	}

	void move(u16 x, u16 y) override
	{
		CE_UNUSED_2(x, y);
	}

	void minimize() override
	{
	}

	void maximize() override
	{
	}

	void restore() override
	{
	}

	const char *title() override
	{
		return NULL;
	}

	void set_title(const char *title) override
	{
		CE_UNUSED(title);
	}

	void show_cursor(bool show) override
	{
		CE_UNUSED(show);
	}

	void set_fullscreen(bool fullscreen) override
	{
		CE_UNUSED(fullscreen);
	}

	void set_cursor(MouseCursor::Enum cursor) override
	{
		CE_UNUSED(cursor);
	}

	bool set_cursor_mode(CursorMode::Enum mode) override
	{
		if (mode == CursorMode::DISABLED) {
			crown_js_request_pointer_lock();
		} else if (mode == CursorMode::NORMAL) {
			crown_js_exit_pointer_lock();
		}

		return true;
	}

	void *native_handle() override
	{
		return (void *)"#" CROWN_HTML5_CANVAS_NAME;
	}

	void *native_handle_type() override
	{
		return (void *)(uintptr_t)bgfx::NativeWindowHandleType::Default;
	}

	void *native_display() override
	{
		return NULL;
	}
};

namespace window
{
	Window *create(Allocator &a)
	{
		return CE_NEW(a, WindowEmscripten)();
	}

	void destroy(Allocator &a, Window &w)
	{
		CE_DELETE(a, &w);
	}

} // namespace window

struct DisplayEmscripten : public Display
{
	void modes(Array<DisplayMode> &modes) override
	{
		CE_UNUSED(modes);
	}

	void set_mode(u32 id) override
	{
		CE_UNUSED(id);
	}
};

namespace display
{
	Display *create(Allocator &a)
	{
		return CE_NEW(a, DisplayEmscripten)();
	}

	void destroy(Allocator &a, Display &d)
	{
		CE_DELETE(a, &d);
	}

} // namespace display

struct EmKeyInfo
{
	const char *key;
	KeyboardButton::Enum kb;
};

static const EmKeyInfo em_key_info[] =
{
	{ "Backspace",      KeyboardButton::BACKSPACE       },
	{ "Tab",            KeyboardButton::TAB             },
	{ "Space",          KeyboardButton::SPACE           },
	{ "Escape",         KeyboardButton::ESCAPE          },
	{ "Enter",          KeyboardButton::ENTER           },
	{ "F1",             KeyboardButton::F1              },
	{ "F2",             KeyboardButton::F2              },
	{ "F3",             KeyboardButton::F3              },
	{ "F4",             KeyboardButton::F4              },
	{ "F5",             KeyboardButton::F5              },
	{ "F6",             KeyboardButton::F6              },
	{ "F7",             KeyboardButton::F7              },
	{ "F8",             KeyboardButton::F8              },
	{ "F9",             KeyboardButton::F9              },
	{ "F10",            KeyboardButton::F10             },
	{ "F11",            KeyboardButton::F11             },
	{ "F12",            KeyboardButton::F12             },
	{ "Home",           KeyboardButton::HOME            },
	{ "ArrowLeft",      KeyboardButton::LEFT            },
	{ "ArrowUp",        KeyboardButton::UP              },
	{ "ArrowRight",     KeyboardButton::RIGHT           },
	{ "ArrowDown",      KeyboardButton::DOWN            },
	{ "PageUp",         KeyboardButton::PAGE_UP         },
	{ "PageDown",       KeyboardButton::PAGE_DOWN       },
	{ "Insert",         KeyboardButton::INS             },
	{ "Delete",         KeyboardButton::DEL             },
	{ "End",            KeyboardButton::END             },
	{ "ShiftLeft",      KeyboardButton::SHIFT_LEFT      },
	{ "ShiftRight",     KeyboardButton::SHIFT_RIGHT     },
	{ "ControlLeft",    KeyboardButton::CTRL_LEFT       },
	{ "ControlRight",   KeyboardButton::CTRL_RIGHT      },
	{ "CapsLock",       KeyboardButton::CAPS_LOCK       },
	{ "AltLeft",        KeyboardButton::ALT_LEFT        },
	{ "AltRight",       KeyboardButton::ALT_RIGHT       },
	{ "OSLeft",         KeyboardButton::SUPER_LEFT      },
	{ "OSRight",        KeyboardButton::SUPER_RIGHT     },
	{ "NumLock",        KeyboardButton::NUM_LOCK        },
	{ "NumpadEnter",    KeyboardButton::NUMPAD_ENTER    },
	{ "NumpadDelete",   KeyboardButton::NUMPAD_DELETE   },
	{ "NumpadMultiply", KeyboardButton::NUMPAD_MULTIPLY },
	{ "NumpadAdd",      KeyboardButton::NUMPAD_ADD      },
	{ "NumpadSubtract", KeyboardButton::NUMPAD_SUBTRACT },
	{ "NumpadDivide",   KeyboardButton::NUMPAD_DIVIDE   },
	{ "Numpad0",        KeyboardButton::NUMPAD_0        },
	{ "Numpad1",        KeyboardButton::NUMPAD_1        },
	{ "Numpad2",        KeyboardButton::NUMPAD_2        },
	{ "Numpad3",        KeyboardButton::NUMPAD_3        },
	{ "Numpad4",        KeyboardButton::NUMPAD_4        },
	{ "Numpad5",        KeyboardButton::NUMPAD_5        },
	{ "Numpad6",        KeyboardButton::NUMPAD_6        },
	{ "Numpad7",        KeyboardButton::NUMPAD_7        },
	{ "Numpad8",        KeyboardButton::NUMPAD_8        },
	{ "Numpad9",        KeyboardButton::NUMPAD_9        },
	{ "Digit0",         KeyboardButton::NUMBER_0        },
	{ "Digit1",         KeyboardButton::NUMBER_1        },
	{ "Digit2",         KeyboardButton::NUMBER_2        },
	{ "Digit3",         KeyboardButton::NUMBER_3        },
	{ "Digit4",         KeyboardButton::NUMBER_4        },
	{ "Digit5",         KeyboardButton::NUMBER_5        },
	{ "Digit6",         KeyboardButton::NUMBER_6        },
	{ "Digit7",         KeyboardButton::NUMBER_7        },
	{ "Digit8",         KeyboardButton::NUMBER_8        },
	{ "Digit9",         KeyboardButton::NUMBER_9        },
	{ "KeyA",           KeyboardButton::A               },
	{ "KeyB",           KeyboardButton::B               },
	{ "KeyC",           KeyboardButton::C               },
	{ "KeyD",           KeyboardButton::D               },
	{ "KeyE",           KeyboardButton::E               },
	{ "KeyF",           KeyboardButton::F               },
	{ "KeyG",           KeyboardButton::G               },
	{ "KeyH",           KeyboardButton::H               },
	{ "KeyI",           KeyboardButton::I               },
	{ "KeyJ",           KeyboardButton::J               },
	{ "KeyK",           KeyboardButton::K               },
	{ "KeyL",           KeyboardButton::L               },
	{ "KeyM",           KeyboardButton::M               },
	{ "KeyN",           KeyboardButton::N               },
	{ "KeyO",           KeyboardButton::O               },
	{ "KeyP",           KeyboardButton::P               },
	{ "KeyQ",           KeyboardButton::Q               },
	{ "KeyR",           KeyboardButton::R               },
	{ "KeyS",           KeyboardButton::S               },
	{ "KeyT",           KeyboardButton::T               },
	{ "KeyU",           KeyboardButton::U               },
	{ "KeyV",           KeyboardButton::V               },
	{ "KeyW",           KeyboardButton::W               },
	{ "KeyX",           KeyboardButton::X               },
	{ "KeyY",           KeyboardButton::Y               },
	{ "KeyZ",           KeyboardButton::Z               }
};

static bool push_event(const OsEvent &ev);

struct EmscriptenDevice
{
	SPSCQueue<OsEvent, CROWN_MAX_OS_EVENTS> _events;
	DeviceEventQueue _queue;
	bool _pointer_locked;

	explicit EmscriptenDevice(Allocator &a)
		: _events(a)
		, _queue(push_event)
		, _pointer_locked(false)
	{
	}

	static EM_BOOL key_callback(int event_type, const EmscriptenKeyboardEvent *key_event, void *user_data)
	{
		EmscriptenDevice *ed = (EmscriptenDevice *)user_data;

		KeyboardButton::Enum kb = KeyboardButton::COUNT;
		for (u32 ii = 0; ii < countof(em_key_info); ++ii) {
			if (strcmp(em_key_info[ii].key, key_event->code) == 0) {
				kb = em_key_info[ii].kb;
				break;
			}
		}

		if (kb == KeyboardButton::COUNT)
			return EM_EVENT_PROPAGATE;

		switch (event_type) {
		case EMSCRIPTEN_EVENT_KEYPRESS:
		case EMSCRIPTEN_EVENT_KEYDOWN:
		case EMSCRIPTEN_EVENT_KEYUP:
			ed->_queue.push_button_event(InputDeviceType::KEYBOARD
				, 0
				, kb
				, event_type != EMSCRIPTEN_EVENT_KEYUP
				);
			return EM_EVENT_STOP;
		}

		return EM_EVENT_PROPAGATE;
	}

	static EM_BOOL mouse_callback(int event_type, const EmscriptenMouseEvent *event, void *user_data)
	{
		EmscriptenDevice *ed = (EmscriptenDevice *)user_data;

		switch (event_type) {
		case EMSCRIPTEN_EVENT_MOUSEDOWN:
		case EMSCRIPTEN_EVENT_MOUSEUP: {
			MouseButton::Enum mb;
			switch (event->button) {
			case 0: mb = MouseButton::LEFT; break;
			case 1: mb = MouseButton::MIDDLE; break;
			case 2: mb = MouseButton::RIGHT; break;
			default: mb = MouseButton::COUNT; break;
			}

			ed->_queue.push_button_event(InputDeviceType::MOUSE
				, 0
				, mb
				, event_type == EMSCRIPTEN_EVENT_MOUSEDOWN
				);
			return EM_EVENT_STOP;
		}

		case EMSCRIPTEN_EVENT_MOUSEMOVE:
			if (!ed->_pointer_locked) {
				ed->_queue.push_axis_event(InputDeviceType::MOUSE
					, 0
					, MouseAxis::CURSOR
					, (s16)event->targetX
					, (s16)event->targetY
					, 0
					);
			}

			ed->_queue.push_axis_event(InputDeviceType::MOUSE
				, 0
				, MouseAxis::CURSOR_DELTA
				, (s16)event->movementX
				, (s16)event->movementY
				, 0
				);
			return EM_EVENT_STOP;
		}

		return EM_EVENT_PROPAGATE;
	}

	static EM_BOOL wheel_callback(int event_type, const EmscriptenWheelEvent *event, void *user_data)
	{
		EmscriptenDevice *ed = (EmscriptenDevice *)user_data;

		if (event_type == EMSCRIPTEN_EVENT_WHEEL) {
			ed->_queue.push_axis_event(InputDeviceType::MOUSE
				, 0
				, MouseAxis::WHEEL
				, event->deltaX > 0.0 ? -1.0f : event->deltaX != 0.0 ? 1.0f : 0.0f
				, event->deltaY > 0.0 ? -1.0f : event->deltaY != 0.0 ? 1.0f : 0.0f
				, event->deltaZ > 0.0 ? -1.0f : event->deltaZ != 0.0 ? 1.0f : 0.0f
				);
			return EM_EVENT_STOP;
		}

		return EM_EVENT_PROPAGATE;
	}

	static EM_BOOL touch_callback(int event_type, const EmscriptenTouchEvent *event, void *user_data)
	{
		EmscriptenDevice *ed = (EmscriptenDevice *)user_data;

		switch (event_type) {
		case EMSCRIPTEN_EVENT_TOUCHSTART:
			for (int i = 0; i < event->numTouches; ++i) {
				const EmscriptenTouchPoint *touch = &event->touches[i];

				if (touch->identifier < TouchButton::COUNT) {
					ed->_queue.push_button_event(InputDeviceType::TOUCHSCREEN
						, 0
						, touch->identifier
						, true
						);
				}

				if (touch->identifier < TouchAxis::COUNT) {
					ed->_queue.push_axis_event(InputDeviceType::TOUCHSCREEN
						, 0
						, touch->identifier
						, touch->targetX
						, touch->targetY
						, 0
						);
				}
			}
			return EM_EVENT_STOP;

		case EMSCRIPTEN_EVENT_TOUCHEND:
			for (int i = 0; i < event->numTouches; ++i) {
				const EmscriptenTouchPoint *touch = &event->touches[i];

				if (touch->identifier < TouchAxis::COUNT) {
					ed->_queue.push_axis_event(InputDeviceType::TOUCHSCREEN
						, 0
						, touch->identifier
						, touch->targetX
						, touch->targetY
						, 0
						);
				}

				if (touch->identifier < TouchButton::COUNT) {
					ed->_queue.push_button_event(InputDeviceType::TOUCHSCREEN
						, 0
						, touch->identifier
						, false
						);
				}
			}
			return EM_EVENT_STOP;

		case EMSCRIPTEN_EVENT_TOUCHMOVE:
			for (int i = 0; i < event->numTouches; ++i) {
				const EmscriptenTouchPoint *touch = &event->touches[i];

				if (touch->identifier < TouchAxis::COUNT) {
					ed->_queue.push_axis_event(InputDeviceType::TOUCHSCREEN
						, 0
						, touch->identifier
						, touch->targetX
						, touch->targetY
						, 0
						);
				}
			}
			return EM_EVENT_STOP;

		case EMSCRIPTEN_EVENT_TOUCHCANCEL:
			for (int i = 0; i < event->numTouches; ++i) {
				const EmscriptenTouchPoint *touch = &event->touches[i];

				if (touch->identifier < TouchButton::COUNT) {
					ed->_queue.push_button_event(InputDeviceType::TOUCHSCREEN
						, 0
						, touch->identifier
						, false
						);
				}
			}
			return EM_EVENT_STOP;
		}

		return EM_EVENT_PROPAGATE;
	}

	static EM_BOOL pointerlockchange_callback(int event_type, const EmscriptenPointerlockChangeEvent *event, void *user_data)
	{
		EmscriptenDevice *ed = (EmscriptenDevice *)user_data;

		if (event_type == EMSCRIPTEN_EVENT_POINTERLOCKCHANGE) {
			ed->_pointer_locked = event->isActive;
			return EM_EVENT_STOP;
		}

		return EM_EVENT_PROPAGATE;
	}

	static EM_BOOL pointerlockerror_callback(int event_type, const void *reserved, void *user_data)
	{
		CE_UNUSED_3(event_type, reserved, user_data);
		crown_js_request_pointer_lock_fallback();
		return EM_EVENT_STOP;
	}

	static EM_BOOL resize_callback(int event_type, const EmscriptenUiEvent *event, void *user_data)
	{
		EmscriptenDevice *ed = (EmscriptenDevice *)user_data;

		if (event_type == EMSCRIPTEN_EVENT_RESIZE) {
			double width;
			double height;
			emscripten_get_element_css_size("#" CROWN_HTML5_CANVAS_NAME, &width, &height);
			ed->_queue.push_resolution_event(width, height);
			return EM_EVENT_STOP;
		}

		return EM_EVENT_PROPAGATE;
	}

	int run(DeviceOptions *opts)
	{
		emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, EmscriptenDevice::key_callback);
		emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, EmscriptenDevice::key_callback);
		emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, EmscriptenDevice::key_callback);

		emscripten_set_mousedown_callback(CROWN_HTML5_CANVAS_NAME, this, true, EmscriptenDevice::mouse_callback);
		emscripten_set_mouseup_callback(CROWN_HTML5_CANVAS_NAME, this, true, EmscriptenDevice::mouse_callback);
		emscripten_set_mousemove_callback(CROWN_HTML5_CANVAS_NAME, this, true, EmscriptenDevice::mouse_callback);
		emscripten_set_wheel_callback(CROWN_HTML5_CANVAS_NAME, this, true, EmscriptenDevice::wheel_callback);

		emscripten_set_touchstart_callback(CROWN_HTML5_CANVAS_NAME, this, true, EmscriptenDevice::touch_callback);
		emscripten_set_touchend_callback(CROWN_HTML5_CANVAS_NAME, this, true, EmscriptenDevice::touch_callback);
		emscripten_set_touchmove_callback(CROWN_HTML5_CANVAS_NAME, this, true, EmscriptenDevice::touch_callback);
		emscripten_set_touchcancel_callback(CROWN_HTML5_CANVAS_NAME, this, true, EmscriptenDevice::touch_callback);

		emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, EmscriptenDevice::pointerlockchange_callback);
		emscripten_set_pointerlockerror_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, EmscriptenDevice::pointerlockerror_callback);

		emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, EmscriptenDevice::resize_callback);

		double width;
		double height;
		emscripten_get_element_css_size("#" CROWN_HTML5_CANVAS_NAME, &width, &height);
		_queue.push_resolution_event(width, height);

		return crown::main_runtime(*opts);
	}
};

static EmscriptenDevice *s_emscripten_device;

static bool push_event(const OsEvent &ev)
{
	return s_emscripten_device->_events.push(ev);
}

bool next_event(OsEvent &ev)
{
	return s_emscripten_device->_events.pop(ev);
}

struct InitGlobals
{
	InitGlobals()
	{
		memory_globals::init();
		profiler_globals::init();
		guid_globals::init();
	}

	~InitGlobals()
	{
		guid_globals::shutdown();
		profiler_globals::shutdown();
		memory_globals::shutdown();
	}
};

} // namespace crown

int main(int argc, char **argv)
{
	using namespace crown;

#if CROWN_BUILD_UNIT_TESTS
	CommandLine cl(argc, (const char **)argv);
	if (cl.has_option("run-unit-tests")) {
		return main_unit_tests();
	}
#endif

	InitGlobals m;
	CE_UNUSED(m);

	DeviceOptions opts(default_allocator(), argc, (const char **)argv);
	bool quit = false;
	int ec = opts.parse(&quit);

	if (quit)
		return ec;

	if (ec == EXIT_SUCCESS) {
		s_emscripten_device = CE_NEW(default_allocator(), EmscriptenDevice)(default_allocator());
		ec = s_emscripten_device->run(&opts);
		CE_DELETE(default_allocator(), s_emscripten_device);
	}

	return ec;
}

#endif // if CROWN_PLATFORM_LINUX
