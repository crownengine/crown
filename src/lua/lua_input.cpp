/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "device.h"
#include "input_manager.h"
#include "input_device.h"

namespace crown
{

struct KeyboardButtonInfo
{
	const char* name;
	KeyboardButton::Enum button;
};

static KeyboardButtonInfo s_keyboard_button[] =
{
	{ "tab",          KeyboardButton::TAB             },
	{ "enter",        KeyboardButton::ENTER           },
	{ "escape",       KeyboardButton::ESCAPE          },
	{ "space",        KeyboardButton::SPACE           },
	{ "backspace",    KeyboardButton::BACKSPACE       },
	{ "num_lock",     KeyboardButton::NUM_LOCK        },
	{ "numpad_enter", KeyboardButton::NUMPAD_ENTER    },
	{ "numpad_.",     KeyboardButton::NUMPAD_DELETE   },
	{ "numpad_*",     KeyboardButton::NUMPAD_MULTIPLY },
	{ "numpad_+",     KeyboardButton::NUMPAD_ADD      },
	{ "numpad_-",     KeyboardButton::NUMPAD_SUBTRACT },
	{ "numpad_/",     KeyboardButton::NUMPAD_DIVIDE   },
	{ "numpad_0",     KeyboardButton::NUMPAD_0        },
	{ "numpad_1",     KeyboardButton::NUMPAD_1        },
	{ "numpad_2",     KeyboardButton::NUMPAD_2        },
	{ "numpad_3",     KeyboardButton::NUMPAD_3        },
	{ "numpad_4",     KeyboardButton::NUMPAD_4        },
	{ "numpad_5",     KeyboardButton::NUMPAD_5        },
	{ "numpad_6",     KeyboardButton::NUMPAD_6        },
	{ "numpad_7",     KeyboardButton::NUMPAD_7        },
	{ "numpad_8",     KeyboardButton::NUMPAD_8        },
	{ "numpad_9",     KeyboardButton::NUMPAD_9        },
	{ "f1",           KeyboardButton::F1              },
	{ "f2",           KeyboardButton::F2              },
	{ "f3",           KeyboardButton::F3              },
	{ "f4",           KeyboardButton::F4              },
	{ "f5",           KeyboardButton::F5              },
	{ "f6",           KeyboardButton::F6              },
	{ "f7",           KeyboardButton::F7              },
	{ "f8",           KeyboardButton::F8              },
	{ "f9",           KeyboardButton::F9              },
	{ "f10",          KeyboardButton::F10             },
	{ "f11",          KeyboardButton::F11             },
	{ "f12",          KeyboardButton::F12             },
	{ "home",         KeyboardButton::HOME            },
	{ "left",         KeyboardButton::LEFT            },
	{ "up",           KeyboardButton::UP              },
	{ "right",        KeyboardButton::RIGHT           },
	{ "down",         KeyboardButton::DOWN            },
	{ "page_up",      KeyboardButton::PAGE_UP         },
	{ "page_down",    KeyboardButton::PAGE_DOWN       },
	{ "delete",       KeyboardButton::DELETE          },
	{ "end",          KeyboardButton::END             },
	{ "left_ctrl",    KeyboardButton::LEFT_CTRL       },
	{ "right_ctrl",   KeyboardButton::RIGHT_CTRL      },
	{ "left_shift",   KeyboardButton::LEFT_SHIFT      },
	{ "right_shift",  KeyboardButton::RIGHT_SHIFT     },
	{ "caps_lock",    KeyboardButton::CAPS_LOCK       },
	{ "left_alt",     KeyboardButton::LEFT_ALT        },
	{ "right_alt",    KeyboardButton::RIGHT_ALT       },
	{ "left_super",   KeyboardButton::LEFT_SUPER      },
	{ "right_super",  KeyboardButton::RIGHT_SUPER     },
	{ "0",            KeyboardButton::NUMBER_0        },
	{ "1",            KeyboardButton::NUMBER_1        },
	{ "2",            KeyboardButton::NUMBER_2        },
	{ "3",            KeyboardButton::NUMBER_3        },
	{ "4",            KeyboardButton::NUMBER_4        },
	{ "5",            KeyboardButton::NUMBER_5        },
	{ "6",            KeyboardButton::NUMBER_6        },
	{ "7",            KeyboardButton::NUMBER_7        },
	{ "8",            KeyboardButton::NUMBER_8        },
	{ "9",            KeyboardButton::NUMBER_9        },
	{ "a",            KeyboardButton::A               },
	{ "b",            KeyboardButton::B               },
	{ "c",            KeyboardButton::C               },
	{ "d",            KeyboardButton::D               },
	{ "e",            KeyboardButton::E               },
	{ "f",            KeyboardButton::F               },
	{ "g",            KeyboardButton::G               },
	{ "h",            KeyboardButton::H               },
	{ "i",            KeyboardButton::I               },
	{ "j",            KeyboardButton::J               },
	{ "k",            KeyboardButton::K               },
	{ "l",            KeyboardButton::L               },
	{ "m",            KeyboardButton::M               },
	{ "n",            KeyboardButton::N               },
	{ "o",            KeyboardButton::O               },
	{ "p",            KeyboardButton::P               },
	{ "q",            KeyboardButton::Q               },
	{ "r",            KeyboardButton::R               },
	{ "s",            KeyboardButton::S               },
	{ "t",            KeyboardButton::T               },
	{ "u",            KeyboardButton::U               },
	{ "v",            KeyboardButton::V               },
	{ "w",            KeyboardButton::W               },
	{ "x",            KeyboardButton::X               },
	{ "y",            KeyboardButton::Y               },
	{ "z",            KeyboardButton::Z               }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_keyboard_button) == KeyboardButton::COUNT);

struct MouseButtonInfo
{
	const char* name;
	MouseButton::Enum button;
};

static MouseButtonInfo s_mouse_button[] =
{
	{ "left",    MouseButton::LEFT    },
	{ "middle",  MouseButton::MIDDLE  },
	{ "right",   MouseButton::RIGHT   },
	{ "extra_1", MouseButton::EXTRA_1 },
	{ "extra_2", MouseButton::EXTRA_2 }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_mouse_button) == MouseButton::COUNT);

struct MouseAxisInfo
{
	const char* name;
	MouseAxis::Enum button;
};

static MouseAxisInfo s_mouse_axis[] =
{
	{ "cursor",       MouseAxis::CURSOR       },
	{ "cursor_delta", MouseAxis::CURSOR_DELTA },
	{ "wheel",        MouseAxis::WHEEL        }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_mouse_axis) == MouseAxis::COUNT);

struct PadButtonInfo
{
	const char* name;
	JoypadButton::Enum button;
};

static PadButtonInfo s_pad_button[] =
{
	{ "up",             JoypadButton::UP             },
	{ "down",           JoypadButton::DOWN           },
	{ "left",           JoypadButton::LEFT           },
	{ "right",          JoypadButton::RIGHT          },
	{ "start",          JoypadButton::START          },
	{ "back",           JoypadButton::BACK           },
	{ "guide",          JoypadButton::GUIDE          },
	{ "left_thumb",     JoypadButton::LEFT_THUMB     },
	{ "right_thumb",    JoypadButton::RIGHT_THUMB    },
	{ "left_shoulder",  JoypadButton::LEFT_SHOULDER  },
	{ "right_shoulder", JoypadButton::RIGHT_SHOULDER },
	{ "a",              JoypadButton::A              },
	{ "b",              JoypadButton::B              },
	{ "x",              JoypadButton::X              },
	{ "y",              JoypadButton::Y              }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_pad_button) == JoypadButton::COUNT);

struct PadAxisInfo
{
	const char* name;
	JoypadAxis::Enum button;
};

static PadAxisInfo s_pad_axis[] =
{
	{ "left",  JoypadAxis::LEFT  },
	{ "right", JoypadAxis::RIGHT }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_pad_axis) == JoypadAxis::COUNT);

static KeyboardButton::Enum name_to_keyboard_button(LuaStack& stack, const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_keyboard_button); ++i)
	{
		if (strcmp(s_keyboard_button[i].name, name) == 0)
			return s_keyboard_button[i].button;
	}

	LUA_ASSERT(false, stack, "Unknown button: %s", name);
	return KeyboardButton::COUNT;
}

static MouseButton::Enum name_to_mouse_button(LuaStack& stack, const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_mouse_button); ++i)
	{
		if (strcmp(s_mouse_button[i].name, name) == 0)
			return s_mouse_button[i].button;
	}

	LUA_ASSERT(false, stack, "Unknown button: %s", name);
	return MouseButton::COUNT;
}

static MouseAxis::Enum name_to_mouse_axis(LuaStack& stack, const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_mouse_axis); ++i)
	{
		if (strcmp(s_mouse_axis[i].name, name) == 0)
			return s_mouse_axis[i].button;
	}

	LUA_ASSERT(false, stack, "Unknown axis: %s", name);
	return MouseAxis::COUNT;
}

static JoypadButton::Enum name_to_pad_button(LuaStack& stack, const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_pad_button); ++i)
	{
		if (strcmp(s_pad_button[i].name, name) == 0)
			return s_pad_button[i].button;
	}

	LUA_ASSERT(false, stack, "Unknown button: %s", name);
	return JoypadButton::COUNT;
}

static JoypadAxis::Enum name_to_pad_axis(LuaStack& stack, const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_pad_axis); ++i)
	{
		if (strcmp(s_pad_axis[i].name, name) == 0)
			return s_pad_axis[i].button;
	}

	LUA_ASSERT(false, stack, "Unknown button: %s", name);
	return JoypadAxis::COUNT;
}

static int input_device_name(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_string(id.name());
	return 1;
}

static int input_device_connected(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.connected());
	return 1;
}

static int input_device_num_buttons(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_int(id.num_buttons());
	return 1;
}

static int input_device_num_axes(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_int(id.num_axes());
	return 1;
}

static int input_device_pressed(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.pressed(stack.get_int(1)));
	return 1;
}

static int input_device_released(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.released(stack.get_int(1)));
	return 1;
}

static int input_device_any_pressed(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.any_pressed());
	return 1;
}

static int input_device_any_released(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.any_released());
	return 1;
}

static int input_device_axis(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_vector3(id.axis(stack.get_int(1)));
	return 1;
}

static int keyboard_button_id(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int(name_to_keyboard_button(stack, stack.get_string(1)));
	return 1;
}

static int mouse_button_id(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int(name_to_mouse_button(stack, stack.get_string(1)));
	return 1;
}

static int mouse_axis_id(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int(name_to_mouse_axis(stack, stack.get_string(1)));
	return 1;
}

static int pad_button_id(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int(name_to_pad_button(stack, stack.get_string(1)));
	return 1;
}

static int pad_axis_id(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int(name_to_pad_axis(stack, stack.get_string(1)));
	return 1;
}

#define KEYBOARD_FN(name) keyboard_##name
#define MOUSE_FN(name) mouse_##name
#define TOUCH_FN(name) touch_##name
#define JOYPAD_FN(index, name) joypad_##name##index

#define KEYBOARD(name) static int KEYBOARD_FN(name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->keyboard()); }
#define MOUSE(name) static int MOUSE_FN(name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->mouse()); }
#define TOUCH(name) static int TOUCH_FN(name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->touch()); }
#define JOYPAD(index, name) static int JOYPAD_FN(index, name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->joypad(index)); }

KEYBOARD(name)
KEYBOARD(connected)
KEYBOARD(num_buttons)
KEYBOARD(num_axes)
KEYBOARD(pressed)
KEYBOARD(released)
KEYBOARD(any_pressed)
KEYBOARD(any_released)
// KEYBOARD(axis) // Keyboard has no axis

MOUSE(name)
MOUSE(connected)
MOUSE(num_buttons)
MOUSE(num_axes)
MOUSE(pressed)
MOUSE(released)
MOUSE(any_pressed)
MOUSE(any_released)
MOUSE(axis)

TOUCH(name)
TOUCH(connected)
TOUCH(num_buttons)
TOUCH(num_axes)
TOUCH(pressed)
TOUCH(released)
TOUCH(any_pressed)
TOUCH(any_released)
TOUCH(axis)

JOYPAD(0, name)
JOYPAD(0, connected)
JOYPAD(0, num_buttons)
JOYPAD(0, num_axes)
JOYPAD(0, pressed)
JOYPAD(0, released)
JOYPAD(0, any_pressed)
JOYPAD(0, any_released)
JOYPAD(0, axis)

JOYPAD(1, name)
JOYPAD(1, connected)
JOYPAD(1, num_buttons)
JOYPAD(1, num_axes)
JOYPAD(1, pressed)
JOYPAD(1, released)
JOYPAD(1, any_pressed)
JOYPAD(1, any_released)
JOYPAD(1, axis)

JOYPAD(2, name)
JOYPAD(2, connected)
JOYPAD(2, num_buttons)
JOYPAD(2, num_axes)
JOYPAD(2, pressed)
JOYPAD(2, released)
JOYPAD(2, any_pressed)
JOYPAD(2, any_released)
JOYPAD(2, axis)

JOYPAD(3, name)
JOYPAD(3, connected)
JOYPAD(3, num_buttons)
JOYPAD(3, num_axes)
JOYPAD(3, pressed)
JOYPAD(3, released)
JOYPAD(3, any_pressed)
JOYPAD(3, any_released)
JOYPAD(3, axis)

void load_input(LuaEnvironment& env)
{
	env.load_module_function("Keyboard", "name",         KEYBOARD_FN(name));
	env.load_module_function("Keyboard", "connected",    KEYBOARD_FN(connected));
	env.load_module_function("Keyboard", "num_buttons",  KEYBOARD_FN(num_buttons));
	env.load_module_function("Keyboard", "num_axes",     KEYBOARD_FN(num_axes));
	env.load_module_function("Keyboard", "pressed",      KEYBOARD_FN(pressed));
	env.load_module_function("Keyboard", "released",     KEYBOARD_FN(released));
	env.load_module_function("Keyboard", "any_pressed",  KEYBOARD_FN(any_pressed));
	env.load_module_function("Keyboard", "any_released", KEYBOARD_FN(any_released));
	env.load_module_function("Keyboard", "button_id",    keyboard_button_id);

	env.load_module_function("Mouse", "name",         MOUSE_FN(name));
	env.load_module_function("Mouse", "connected",    MOUSE_FN(connected));
	env.load_module_function("Mouse", "num_buttons",  MOUSE_FN(num_buttons));
	env.load_module_function("Mouse", "num_axes",     MOUSE_FN(num_axes));
	env.load_module_function("Mouse", "pressed",      MOUSE_FN(pressed));
	env.load_module_function("Mouse", "released",     MOUSE_FN(released));
	env.load_module_function("Mouse", "any_pressed",  MOUSE_FN(any_pressed));
	env.load_module_function("Mouse", "any_released", MOUSE_FN(any_released));
	env.load_module_function("Mouse", "axis",         MOUSE_FN(axis));
	env.load_module_function("Mouse", "button_id",    mouse_button_id);
	env.load_module_function("Mouse", "axis_id",      mouse_axis_id);

	env.load_module_function("Touch", "name",         TOUCH_FN(name));
	env.load_module_function("Touch", "connected",    TOUCH_FN(connected));
	env.load_module_function("Touch", "num_buttons",  TOUCH_FN(num_buttons));
	env.load_module_function("Touch", "num_axes",     TOUCH_FN(num_axes));
	env.load_module_function("Touch", "pressed",      TOUCH_FN(pressed));
	env.load_module_function("Touch", "released",     TOUCH_FN(released));
	env.load_module_function("Touch", "any_pressed",  TOUCH_FN(any_pressed));
	env.load_module_function("Touch", "any_released", TOUCH_FN(any_released));
	env.load_module_function("Touch", "axis",         TOUCH_FN(axis));

	env.load_module_function("Pad1", "name",         JOYPAD_FN(0, name));
	env.load_module_function("Pad1", "connected",    JOYPAD_FN(0, connected));
	env.load_module_function("Pad1", "num_buttons",  JOYPAD_FN(0, num_buttons));
	env.load_module_function("Pad1", "num_axes",     JOYPAD_FN(0, num_axes));
	env.load_module_function("Pad1", "pressed",      JOYPAD_FN(0, pressed));
	env.load_module_function("Pad1", "released",     JOYPAD_FN(0, released));
	env.load_module_function("Pad1", "any_pressed",  JOYPAD_FN(0, any_pressed));
	env.load_module_function("Pad1", "any_released", JOYPAD_FN(0, any_released));
	env.load_module_function("Pad1", "axis",         JOYPAD_FN(0, axis));
	env.load_module_function("Pad1", "button_id",    pad_button_id);
	env.load_module_function("Pad1", "axis_id",      pad_axis_id);

	env.load_module_function("Pad2", "name",         JOYPAD_FN(1, name));
	env.load_module_function("Pad2", "connected",    JOYPAD_FN(1, connected));
	env.load_module_function("Pad2", "num_buttons",  JOYPAD_FN(1, num_buttons));
	env.load_module_function("Pad2", "num_axes",     JOYPAD_FN(1, num_axes));
	env.load_module_function("Pad2", "pressed",      JOYPAD_FN(1, pressed));
	env.load_module_function("Pad2", "released",     JOYPAD_FN(1, released));
	env.load_module_function("Pad2", "any_pressed",  JOYPAD_FN(1, any_pressed));
	env.load_module_function("Pad2", "any_released", JOYPAD_FN(1, any_released));
	env.load_module_function("Pad2", "axis",         JOYPAD_FN(1, axis));
	env.load_module_function("Pad2", "button_id",    pad_button_id);
	env.load_module_function("Pad2", "axis_id",      pad_axis_id);

	env.load_module_function("Pad3", "name",         JOYPAD_FN(2, name));
	env.load_module_function("Pad3", "connected",    JOYPAD_FN(2, connected));
	env.load_module_function("Pad3", "num_buttons",  JOYPAD_FN(2, num_buttons));
	env.load_module_function("Pad3", "num_axes",     JOYPAD_FN(2, num_axes));
	env.load_module_function("Pad3", "pressed",      JOYPAD_FN(2, pressed));
	env.load_module_function("Pad3", "released",     JOYPAD_FN(2, released));
	env.load_module_function("Pad3", "any_pressed",  JOYPAD_FN(2, any_pressed));
	env.load_module_function("Pad3", "any_released", JOYPAD_FN(2, any_released));
	env.load_module_function("Pad3", "axis",         JOYPAD_FN(2, axis));
	env.load_module_function("Pad3", "button_id",    pad_button_id);
	env.load_module_function("Pad3", "axis_id",      pad_axis_id);

	env.load_module_function("Pad4", "name",         JOYPAD_FN(3, name));
	env.load_module_function("Pad4", "connected",    JOYPAD_FN(3, connected));
	env.load_module_function("Pad4", "num_buttons",  JOYPAD_FN(3, num_buttons));
	env.load_module_function("Pad4", "num_axes",     JOYPAD_FN(3, num_axes));
	env.load_module_function("Pad4", "pressed",      JOYPAD_FN(3, pressed));
	env.load_module_function("Pad4", "released",     JOYPAD_FN(3, released));
	env.load_module_function("Pad4", "any_pressed",  JOYPAD_FN(3, any_pressed));
	env.load_module_function("Pad4", "any_released", JOYPAD_FN(3, any_released));
	env.load_module_function("Pad4", "axis",         JOYPAD_FN(3, axis));
	env.load_module_function("Pad4", "button_id",    pad_button_id);
	env.load_module_function("Pad4", "axis_id",      pad_axis_id);
}

} // namespace crown
