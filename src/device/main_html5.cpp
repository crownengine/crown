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
#include <string.h> // memset, strcmp
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

EM_JS(int, crown_js_is_firefox, (void), {
	if (typeof navigator === 'undefined' || !navigator.userAgent) {
		return 0;
	}
	return navigator.userAgent.indexOf('Firefox/') !== -1 ? 1 : 0;
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
	{ "PrintScreen",    KeyboardButton::PRINT_SCREEN    },
	{ "ScrollLock",     KeyboardButton::SCROLL_LOCK     },
	{ "Pause",          KeyboardButton::BREAK           },
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
	{ "Backquote",      KeyboardButton::BACKTICK        },
	{ "Minus",          KeyboardButton::MINUS           },
	{ "Equal",          KeyboardButton::EQUAL           },
	{ "BracketLeft",    KeyboardButton::OPEN_BRACKET    },
	{ "BracketRight",   KeyboardButton::CLOSE_BRACKET   },
	{ "Backslash",      KeyboardButton::BACKSLASH       },
	{ "Semicolon",      KeyboardButton::SEMICOLON       },
	{ "Quote",          KeyboardButton::QUOTE           },
	{ "Comma",          KeyboardButton::COMMA           },
	{ "Period",         KeyboardButton::PERIOD          },
	{ "Slash",          KeyboardButton::SLASH           },
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

struct EmJoypadButtonInfo
{
	u8 gb;
	JoypadButton::Enum jb;
};

static const EmJoypadButtonInfo em_joypad_button_info[] =
{
	{ 12, JoypadButton::UP             },
	{ 13, JoypadButton::DOWN           },
	{ 14, JoypadButton::LEFT           },
	{ 15, JoypadButton::RIGHT          },
	{ 9,  JoypadButton::START          },
	{ 8,  JoypadButton::BACK           },
	{ 16, JoypadButton::GUIDE          },
	{ 10, JoypadButton::THUMB_LEFT     },
	{ 11, JoypadButton::THUMB_RIGHT    },
	{ 4,  JoypadButton::SHOULDER_LEFT  },
	{ 5,  JoypadButton::SHOULDER_RIGHT },
	{ 0,  JoypadButton::A              },
	{ 1,  JoypadButton::B              }
};

static bool push_event(const OsEvent &ev);
struct EmscriptenDevice;

struct Joypad
{
	struct State
	{
		bool connected;
		u16 button_state;
		s16 lx, ly, lz;
		s16 rx, ry, rz;
	};

	DeviceEventQueue *_queue;
	EmscriptenDevice *_device;
	State _state[CROWN_MAX_JOYPADS];

	explicit Joypad(DeviceEventQueue &queue, EmscriptenDevice &device);

	static s16 axis(f32 value)
	{
		const f32 clamped = clamp(value, -1.0f, 1.0f);
		return (s16)(clamped * (f32)INT16_MAX);
	}

	static f32 trigger_normalize(f32 value)
	{
		// Triggers are exposed on [0; 1] or [-1; 1] depending on browser/controller mapping.
		return clamp(value < 0.0f ? (value + 1.0f) * 0.5f : value, 0.0f, 1.0f);
	}

	static s16 trigger(f32 value)
	{
		const f32 clamped = clamp(value, 0.0f, 1.0f);
		return (s16)(clamped * (f32)INT16_MAX);
	}

	static bool button_pressed(const EmscriptenGamepadEvent &event, u8 button)
	{
		return button < event.numButtons
			&& (event.digitalButton[button] || event.analogButton[button] > 0.5);
	}

	static f32 trigger_value(const EmscriptenGamepadEvent &event, u8 axis_0, u8 axis_1, u8 button)
	{
		const bool has_axis_0 = axis_0 < event.numAxes;
		const bool has_axis_1 = axis_1 < event.numAxes;
		if (has_axis_0 || has_axis_1) {
			const f32 axis_0_value = has_axis_0 ? trigger_normalize((f32)event.axis[axis_0]) : 0.0f;
			const f32 axis_1_value = has_axis_1 ? trigger_normalize((f32)event.axis[axis_1]) : 0.0f;
			return max(axis_0_value, axis_1_value);
		}

		if (button < event.numButtons) {
			const f32 analog = clamp((f32)event.analogButton[button], 0.0f, 1.0f);
			if (analog > 0.0f || !event.digitalButton[button])
				return analog;
			return 1.0f;
		}

		return 0.0f;
	}

	void xy_buttons(u8 &x_button, u8 &y_button) const;

	static EM_BOOL callback(int event_type, const EmscriptenGamepadEvent *event, void *user_data)
	{
		Joypad *joypad = (Joypad *)user_data;
		const int ii = event->index;
		if (ii < 0 || ii >= CROWN_MAX_JOYPADS)
			return EM_EVENT_PROPAGATE;

		if (event_type == EMSCRIPTEN_EVENT_GAMEPADCONNECTED) {
			if (!joypad->_state[ii].connected) {
				joypad->_state[ii].connected = true;
				joypad->_queue->push_status_event(InputDeviceType::JOYPAD, ii, true);
			}
			return EM_EVENT_STOP;
		}

		if (event_type == EMSCRIPTEN_EVENT_GAMEPADDISCONNECTED) {
			if (joypad->_state[ii].connected) {
				memset(&joypad->_state[ii], 0, sizeof(joypad->_state[ii]));
				joypad->_queue->push_status_event(InputDeviceType::JOYPAD, ii, false);
			}
			return EM_EVENT_STOP;
		}

		return EM_EVENT_PROPAGATE;
	}

	void poll()
	{
		emscripten_sample_gamepad_data();

		for (s8 ii = 0; ii < CROWN_MAX_JOYPADS; ++ii) {
			EmscriptenGamepadEvent event;
			memset(&event, 0, sizeof(event));

			const EMSCRIPTEN_RESULT result = emscripten_get_gamepad_status(ii, &event);
			const bool connected = result == EMSCRIPTEN_RESULT_SUCCESS && event.connected;
			State &jp = _state[ii];
			if (!connected) {
				if (jp.connected) {
					memset(&jp, 0, sizeof(jp));
					_queue->push_status_event(InputDeviceType::JOYPAD, ii, false);
				}
				continue;
			}

			if (!jp.connected) {
				jp.connected = true;
				_queue->push_status_event(InputDeviceType::JOYPAD, ii, true);
			}

			u16 button_state = 0;
			for (u32 ii = 0; ii < countof(em_joypad_button_info); ++ii) {
				const EmJoypadButtonInfo &gb = em_joypad_button_info[ii];
				if (button_pressed(event, gb.gb))
					button_state |= (u16)1 << gb.jb;
			}

			u8 x_button;
			u8 y_button;
			xy_buttons(x_button, y_button);
			if (button_pressed(event, x_button))
				button_state |= (u16)1 << JoypadButton::X;
			if (button_pressed(event, y_button))
				button_state |= (u16)1 << JoypadButton::Y;

			const u16 button_diff = button_state ^ jp.button_state;
			for (u8 jb = 0; jb < JoypadButton::COUNT; ++jb) {
				const u16 mask = (u16)1 << jb;
				if (button_diff & mask) {
					_queue->push_button_event(InputDeviceType::JOYPAD
						, ii
						, jb
						, (button_state & mask) != 0
						);
				}
			}
			jp.button_state = button_state;

			const u8 stick_axis_x[] = { 0, 2 };
			const u8 stick_axis_y[] = { 1, 3 };
			const JoypadAxis::Enum stick_out_axis[] = { JoypadAxis::LEFT, JoypadAxis::RIGHT };
			s16 *stick_state_x[] = { &jp.lx, &jp.rx };
			s16 *stick_state_y[] = { &jp.ly, &jp.ry };
			for (u8 stick = 0; stick < 2; ++stick) {
				const s16 x = stick_axis_x[stick] < event.numAxes
						? axis((f32)event.axis[stick_axis_x[stick]])
						: 0;
				const s16 y = stick_axis_y[stick] < event.numAxes
						? -axis((f32)event.axis[stick_axis_y[stick]])
						: 0;
				if (x != *stick_state_x[stick] || y != *stick_state_y[stick]) {
					*stick_state_x[stick] = x;
					*stick_state_y[stick] = y;
					_queue->push_axis_event(InputDeviceType::JOYPAD
						, ii
						, stick_out_axis[stick]
						, x
						, y
						, 0
						);
				}
			}

			const u8 trigger_axis_0[] = { 4, 5 };
			const u8 trigger_axis_1[] = { 6, 7 };
			const u8 trigger_button[] = { 6, 7 };
			const JoypadAxis::Enum trigger_out_axis[] = { JoypadAxis::TRIGGER_LEFT, JoypadAxis::TRIGGER_RIGHT };
			s16 *trigger_state[] = { &jp.lz, &jp.rz };
			for (u8 trigger_id = 0; trigger_id < 2; ++trigger_id) {
				const s16 z = trigger(trigger_value(event
					, trigger_axis_0[trigger_id]
					, trigger_axis_1[trigger_id]
					, trigger_button[trigger_id]
					));
				if (z != *trigger_state[trigger_id]) {
					*trigger_state[trigger_id] = z;
					_queue->push_axis_event(InputDeviceType::JOYPAD
						, ii
						, trigger_out_axis[trigger_id]
						, 0
						, 0
						, z
						);
				}
			}
		}
	}
};

struct EmscriptenDevice
{
	SPSCQueue<OsEvent, CROWN_MAX_OS_EVENTS> _events;
	DeviceEventQueue _queue;
	bool _pointer_locked;
	bool _is_firefox;
	bool _joypad_polled;
	Joypad _joypad;

	explicit EmscriptenDevice(Allocator &a)
		: _events(a)
		, _queue(push_event)
		, _pointer_locked(false)
		, _is_firefox(crown_js_is_firefox() != 0)
		, _joypad_polled(false)
		, _joypad(_queue, *this)
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

		emscripten_set_gamepadconnected_callback(&_joypad, true, Joypad::callback);
		emscripten_set_gamepaddisconnected_callback(&_joypad, true, Joypad::callback);

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

Joypad::Joypad(DeviceEventQueue &queue, EmscriptenDevice &device)
	: _queue(&queue)
	, _device(&device)
{
	memset(&_state, 0, sizeof(_state));
}

void Joypad::xy_buttons(u8 &x_button, u8 &y_button) const
{
	// Firefox reports swapped X/Y compared to Chromium.
	x_button = _device->_is_firefox ? 3 : 2;
	y_button = _device->_is_firefox ? 2 : 3;
}

static EmscriptenDevice *s_emscripten_device;

static bool push_event(const OsEvent &ev)
{
	return s_emscripten_device->_events.push(ev);
}

bool next_event(OsEvent &ev)
{
	if (!s_emscripten_device->_joypad_polled) {
		s_emscripten_device->_joypad.poll();
		s_emscripten_device->_joypad_polled = true;
	}

	if (s_emscripten_device->_events.pop(ev))
		return true;

	s_emscripten_device->_joypad_polled = false;
	return false;
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
