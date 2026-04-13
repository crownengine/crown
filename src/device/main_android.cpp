/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PLATFORM_ANDROID
#include "core/error/error.inl"
#include "core/guid.h"
#include "core/memory/globals.h"
#include "core/memory/memory.inl"
#include "core/profiler.h"
#include "core/thread/spsc_queue.inl"
#include "core/thread/thread.h"
#include "device/device.h"
#include "device/device_event_queue.inl"
#include <android/sensor.h>
#include <android/window.h>
#include <jni.h>
#include <stdlib.h>
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED
#include <stb_sprintf.h>

extern "C"
{
#include <android_native_app_glue.c>
}

namespace crown
{
extern PlatformData g_platform_data;

static KeyboardButton::Enum android_translate_key(s32 keycode)
{
#ifndef AKEYCODE_SCROLL_LOCK
#define AKEYCODE_SCROLL_LOCK 116
#endif
#ifndef AKEYCODE_SYSRQ
#define AKEYCODE_SYSRQ 120
#endif
#ifndef AKEYCODE_BREAK
#define AKEYCODE_BREAK 121
#endif

	switch (keycode) {
	case AKEYCODE_TAB:                return KeyboardButton::TAB;
	case AKEYCODE_ENTER:              return KeyboardButton::ENTER;
	case AKEYCODE_ESCAPE:             return KeyboardButton::ESCAPE;
	case AKEYCODE_SPACE:              return KeyboardButton::SPACE;
	case AKEYCODE_DEL:                return KeyboardButton::BACKSPACE;
	case AKEYCODE_NUM_LOCK:           return KeyboardButton::NUM_LOCK;
	case AKEYCODE_NUMPAD_ENTER:       return KeyboardButton::NUMPAD_ENTER;
	case AKEYCODE_NUMPAD_DOT:         return KeyboardButton::NUMPAD_DELETE;
	case AKEYCODE_NUMPAD_MULTIPLY:    return KeyboardButton::NUMPAD_MULTIPLY;
	case AKEYCODE_NUMPAD_ADD:         return KeyboardButton::NUMPAD_ADD;
	case AKEYCODE_NUMPAD_SUBTRACT:    return KeyboardButton::NUMPAD_SUBTRACT;
	case AKEYCODE_NUMPAD_DIVIDE:      return KeyboardButton::NUMPAD_DIVIDE;
	case AKEYCODE_NUMPAD_0:           return KeyboardButton::NUMPAD_0;
	case AKEYCODE_NUMPAD_1:           return KeyboardButton::NUMPAD_1;
	case AKEYCODE_NUMPAD_2:           return KeyboardButton::NUMPAD_2;
	case AKEYCODE_NUMPAD_3:           return KeyboardButton::NUMPAD_3;
	case AKEYCODE_NUMPAD_4:           return KeyboardButton::NUMPAD_4;
	case AKEYCODE_NUMPAD_5:           return KeyboardButton::NUMPAD_5;
	case AKEYCODE_NUMPAD_6:           return KeyboardButton::NUMPAD_6;
	case AKEYCODE_NUMPAD_7:           return KeyboardButton::NUMPAD_7;
	case AKEYCODE_NUMPAD_8:           return KeyboardButton::NUMPAD_8;
	case AKEYCODE_NUMPAD_9:           return KeyboardButton::NUMPAD_9;
	case AKEYCODE_F1:                 return KeyboardButton::F1;
	case AKEYCODE_F2:                 return KeyboardButton::F2;
	case AKEYCODE_F3:                 return KeyboardButton::F3;
	case AKEYCODE_F4:                 return KeyboardButton::F4;
	case AKEYCODE_F5:                 return KeyboardButton::F5;
	case AKEYCODE_F6:                 return KeyboardButton::F6;
	case AKEYCODE_F7:                 return KeyboardButton::F7;
	case AKEYCODE_F8:                 return KeyboardButton::F8;
	case AKEYCODE_F9:                 return KeyboardButton::F9;
	case AKEYCODE_F10:                return KeyboardButton::F10;
	case AKEYCODE_F11:                return KeyboardButton::F11;
	case AKEYCODE_F12:                return KeyboardButton::F12;
	case AKEYCODE_MOVE_HOME:          return KeyboardButton::HOME;
	case AKEYCODE_DPAD_LEFT:          return KeyboardButton::LEFT;
	case AKEYCODE_DPAD_UP:            return KeyboardButton::UP;
	case AKEYCODE_DPAD_RIGHT:         return KeyboardButton::RIGHT;
	case AKEYCODE_DPAD_DOWN:          return KeyboardButton::DOWN;
	case AKEYCODE_PAGE_UP:            return KeyboardButton::PAGE_UP;
	case AKEYCODE_PAGE_DOWN:          return KeyboardButton::PAGE_DOWN;
	case AKEYCODE_INSERT:             return KeyboardButton::INS;
	case AKEYCODE_FORWARD_DEL:        return KeyboardButton::DEL;
	case AKEYCODE_MOVE_END:           return KeyboardButton::END;
	case AKEYCODE_SYSRQ:              return KeyboardButton::PRINT_SCREEN;
	case AKEYCODE_SCROLL_LOCK:        return KeyboardButton::SCROLL_LOCK;
	case AKEYCODE_BREAK:              return KeyboardButton::BREAK;
	case AKEYCODE_CTRL_LEFT:          return KeyboardButton::CTRL_LEFT;
	case AKEYCODE_CTRL_RIGHT:         return KeyboardButton::CTRL_RIGHT;
	case AKEYCODE_SHIFT_LEFT:         return KeyboardButton::SHIFT_LEFT;
	case AKEYCODE_SHIFT_RIGHT:        return KeyboardButton::SHIFT_RIGHT;
	case AKEYCODE_CAPS_LOCK:          return KeyboardButton::CAPS_LOCK;
	case AKEYCODE_ALT_LEFT:           return KeyboardButton::ALT_LEFT;
	case AKEYCODE_ALT_RIGHT:          return KeyboardButton::ALT_RIGHT;
	case AKEYCODE_META_LEFT:          return KeyboardButton::SUPER_LEFT;
	case AKEYCODE_META_RIGHT:         return KeyboardButton::SUPER_RIGHT;
	case AKEYCODE_0:                  return KeyboardButton::NUMBER_0;
	case AKEYCODE_1:                  return KeyboardButton::NUMBER_1;
	case AKEYCODE_2:                  return KeyboardButton::NUMBER_2;
	case AKEYCODE_3:                  return KeyboardButton::NUMBER_3;
	case AKEYCODE_4:                  return KeyboardButton::NUMBER_4;
	case AKEYCODE_5:                  return KeyboardButton::NUMBER_5;
	case AKEYCODE_6:                  return KeyboardButton::NUMBER_6;
	case AKEYCODE_7:                  return KeyboardButton::NUMBER_7;
	case AKEYCODE_8:                  return KeyboardButton::NUMBER_8;
	case AKEYCODE_9:                  return KeyboardButton::NUMBER_9;
	case AKEYCODE_A:                  return KeyboardButton::A;
	case AKEYCODE_B:                  return KeyboardButton::B;
	case AKEYCODE_C:                  return KeyboardButton::C;
	case AKEYCODE_D:                  return KeyboardButton::D;
	case AKEYCODE_E:                  return KeyboardButton::E;
	case AKEYCODE_F:                  return KeyboardButton::F;
	case AKEYCODE_G:                  return KeyboardButton::G;
	case AKEYCODE_H:                  return KeyboardButton::H;
	case AKEYCODE_I:                  return KeyboardButton::I;
	case AKEYCODE_J:                  return KeyboardButton::J;
	case AKEYCODE_K:                  return KeyboardButton::K;
	case AKEYCODE_L:                  return KeyboardButton::L;
	case AKEYCODE_M:                  return KeyboardButton::M;
	case AKEYCODE_N:                  return KeyboardButton::N;
	case AKEYCODE_O:                  return KeyboardButton::O;
	case AKEYCODE_P:                  return KeyboardButton::P;
	case AKEYCODE_Q:                  return KeyboardButton::Q;
	case AKEYCODE_R:                  return KeyboardButton::R;
	case AKEYCODE_S:                  return KeyboardButton::S;
	case AKEYCODE_T:                  return KeyboardButton::T;
	case AKEYCODE_U:                  return KeyboardButton::U;
	case AKEYCODE_V:                  return KeyboardButton::V;
	case AKEYCODE_W:                  return KeyboardButton::W;
	case AKEYCODE_X:                  return KeyboardButton::X;
	case AKEYCODE_Y:                  return KeyboardButton::Y;
	case AKEYCODE_Z:                  return KeyboardButton::Z;
	case AKEYCODE_GRAVE:              return KeyboardButton::BACKTICK;
	case AKEYCODE_MINUS:              return KeyboardButton::MINUS;
	case AKEYCODE_EQUALS:             return KeyboardButton::EQUAL;
	case AKEYCODE_LEFT_BRACKET:       return KeyboardButton::OPEN_BRACKET;
	case AKEYCODE_RIGHT_BRACKET:      return KeyboardButton::CLOSE_BRACKET;
	case AKEYCODE_BACKSLASH:          return KeyboardButton::BACKSLASH;
	case AKEYCODE_SEMICOLON:          return KeyboardButton::SEMICOLON;
	case AKEYCODE_APOSTROPHE:         return KeyboardButton::QUOTE;
	case AKEYCODE_COMMA:              return KeyboardButton::COMMA;
	case AKEYCODE_PERIOD:             return KeyboardButton::PERIOD;
	case AKEYCODE_SLASH:              return KeyboardButton::SLASH;
	default:                          return KeyboardButton::COUNT;
	}
}

static MouseButton::Enum mouse_button(s32 button_mask)
{
	switch (button_mask) {
	case AMOTION_EVENT_BUTTON_PRIMARY:   return MouseButton::LEFT;
	case AMOTION_EVENT_BUTTON_SECONDARY: return MouseButton::RIGHT;
	case AMOTION_EVENT_BUTTON_TERTIARY:  return MouseButton::MIDDLE;
	case AMOTION_EVENT_BUTTON_BACK:      return MouseButton::EXTRA_1;
	case AMOTION_EVENT_BUTTON_FORWARD:   return MouseButton::EXTRA_2;
	default:                             return MouseButton::COUNT;
	}
}

#ifndef AINPUT_SOURCE_GAMEPAD
#define AINPUT_SOURCE_GAMEPAD 0x00000401
#endif
#ifndef AINPUT_SOURCE_JOYSTICK
#define AINPUT_SOURCE_JOYSTICK 0x01000010
#endif
#ifndef AINPUT_SOURCE_DPAD
#define AINPUT_SOURCE_DPAD 0x00000201
#endif
#ifndef AMOTION_EVENT_AXIS_HAT_X
#define AMOTION_EVENT_AXIS_HAT_X 15
#endif
#ifndef AMOTION_EVENT_AXIS_HAT_Y
#define AMOTION_EVENT_AXIS_HAT_Y 16
#endif
#ifndef AMOTION_EVENT_AXIS_Z
#define AMOTION_EVENT_AXIS_Z 11
#endif
#ifndef AMOTION_EVENT_AXIS_RZ
#define AMOTION_EVENT_AXIS_RZ 14
#endif
#ifndef AMOTION_EVENT_AXIS_LTRIGGER
#define AMOTION_EVENT_AXIS_LTRIGGER 17
#endif
#ifndef AMOTION_EVENT_AXIS_RTRIGGER
#define AMOTION_EVENT_AXIS_RTRIGGER 18
#endif

static JoypadButton::Enum joypad_button(s32 keycode)
{
#ifndef AKEYCODE_BUTTON_A
#define AKEYCODE_BUTTON_A 96
#endif
#ifndef AKEYCODE_BUTTON_B
#define AKEYCODE_BUTTON_B 97
#endif
#ifndef AKEYCODE_BUTTON_X
#define AKEYCODE_BUTTON_X 99
#endif
#ifndef AKEYCODE_BUTTON_Y
#define AKEYCODE_BUTTON_Y 100
#endif
#ifndef AKEYCODE_BUTTON_L1
#define AKEYCODE_BUTTON_L1 102
#endif
#ifndef AKEYCODE_BUTTON_R1
#define AKEYCODE_BUTTON_R1 103
#endif
#ifndef AKEYCODE_BUTTON_THUMBL
#define AKEYCODE_BUTTON_THUMBL 106
#endif
#ifndef AKEYCODE_BUTTON_THUMBR
#define AKEYCODE_BUTTON_THUMBR 107
#endif
#ifndef AKEYCODE_BUTTON_START
#define AKEYCODE_BUTTON_START 108
#endif
#ifndef AKEYCODE_BUTTON_SELECT
#define AKEYCODE_BUTTON_SELECT 109
#endif
#ifndef AKEYCODE_BUTTON_MODE
#define AKEYCODE_BUTTON_MODE 110
#endif

	switch (keycode) {
	case AKEYCODE_DPAD_UP:       return JoypadButton::UP;
	case AKEYCODE_DPAD_DOWN:     return JoypadButton::DOWN;
	case AKEYCODE_DPAD_LEFT:     return JoypadButton::LEFT;
	case AKEYCODE_DPAD_RIGHT:    return JoypadButton::RIGHT;
	case AKEYCODE_BUTTON_START:  return JoypadButton::START;
	case AKEYCODE_BUTTON_SELECT: return JoypadButton::BACK;
	case AKEYCODE_BUTTON_MODE:   return JoypadButton::GUIDE;
	case AKEYCODE_BUTTON_THUMBL: return JoypadButton::THUMB_LEFT;
	case AKEYCODE_BUTTON_THUMBR: return JoypadButton::THUMB_RIGHT;
	case AKEYCODE_BUTTON_L1:     return JoypadButton::SHOULDER_LEFT;
	case AKEYCODE_BUTTON_R1:     return JoypadButton::SHOULDER_RIGHT;
	case AKEYCODE_BUTTON_A:      return JoypadButton::A;
	case AKEYCODE_BUTTON_B:      return JoypadButton::B;
	case AKEYCODE_BUTTON_X:      return JoypadButton::X;
	case AKEYCODE_BUTTON_Y:      return JoypadButton::Y;
	default:                     return JoypadButton::COUNT;
	}
}

static s16 joypad_axis(f32 value)
{
	const f32 v = clamp(value, -1.0f, 1.0f);
	return (s16)(v * INT16_MAX);
}

static s16 joypad_trigger(f32 value)
{
	const f32 v = clamp(value, 0.0f, 1.0f);
	return (s16)(v * INT16_MAX);
}

static s8 joypad_hat_value(f32 value)
{
	if (value <= -0.5f)
		return -1;
	if (value >= 0.5f)
		return 1;

	return 0;
}

static bool push_event(const OsEvent &ev);

struct AndroidDevice
{
	struct Joypad
	{
		struct AxisState
		{
			s16 lx, ly, rx, ry, lz, rz;
		};

		struct State
		{
			s32 device_id;
			u16 button_state;
			s8 hat_x;
			s8 hat_y;
			AxisState axis;
		};

		State state[CROWN_MAX_JOYPADS];
	};

	SPSCQueue<OsEvent, CROWN_MAX_OS_EVENTS> _events;
	DeviceEventQueue _queue;
	Thread _main_thread;
	android_app *_app;
	DeviceOptions *_opts;
	ANativeWindow *_window;
	CursorMode::Enum _cursor_mode;
	s32 _mouse_button_state;
	JavaVM *_jni_vm;
	jobject _pointer_capture_view;
	jmethodID _request_pointer_capture;
	jmethodID _release_pointer_capture;
	Joypad _joypad;

	explicit AndroidDevice(Allocator &a)
		: _events(a)
		, _queue(push_event)
		, _app(NULL)
		, _opts(NULL)
		, _cursor_mode(CursorMode::NORMAL)
		, _mouse_button_state(0)
		, _jni_vm(NULL)
		, _pointer_capture_view(NULL)
		, _request_pointer_capture(NULL)
		, _release_pointer_capture(NULL)
	{
		for (u8 ii = 0; ii < CROWN_MAX_JOYPADS; ++ii) {
			_joypad.state[ii].device_id = -1;
			_joypad.state[ii].button_state = 0;
			_joypad.state[ii].hat_x = 0;
			_joypad.state[ii].hat_y = 0;
			memset(&_joypad.state[ii].axis, 0, sizeof(_joypad.state[ii].axis));
		}
	}

	s8 joypad_id(s32 device_id, bool create)
	{
		for (s8 ii = 0; ii < CROWN_MAX_JOYPADS; ++ii) {
			if (_joypad.state[ii].device_id == device_id)
				return ii;
		}

		if (!create)
			return -1;

		for (s8 ii = 0; ii < CROWN_MAX_JOYPADS; ++ii) {
			if (_joypad.state[ii].device_id != -1)
				continue;

			_joypad.state[ii].device_id = device_id;
			_joypad.state[ii].button_state = 0;
			_joypad.state[ii].hat_x = 0;
			_joypad.state[ii].hat_y = 0;
			memset(&_joypad.state[ii].axis, 0, sizeof(_joypad.state[ii].axis));
			_queue.push_status_event(InputDeviceType::JOYPAD, ii, true);
			return ii;
		}

		return -1;
	}

	void init_pointer_capture_api()
	{
		if (_app == NULL || _app->activity == NULL || _app->activity->vm == NULL || _app->activity->clazz == NULL)
			return;
		if (_pointer_capture_view != NULL && _request_pointer_capture != NULL && _release_pointer_capture != NULL)
			return;

		JavaVM *vm = _app->activity->vm;
		JNIEnv *env = NULL;
		bool detach_thread = false;
		const s32 get_env = vm->GetEnv((void **)&env, JNI_VERSION_1_6);
		if (get_env == JNI_EDETACHED) {
			if (vm->AttachCurrentThread(&env, NULL) != JNI_OK)
				return;
			detach_thread = true;
		} else if (get_env != JNI_OK) {
			return;
		}

		_jni_vm = vm;

		jclass activity_class = NULL;
		jobject window = NULL;
		jclass window_class = NULL;
		jobject decor_view = NULL;
		jclass view_class = NULL;
		jmethodID get_window = NULL;
		jmethodID get_decor_view = NULL;
		jmethodID request_pointer_capture = NULL;
		jmethodID release_pointer_capture = NULL;

		activity_class = env->GetObjectClass(_app->activity->clazz);
		if (activity_class == NULL)
			goto fail_detach;

		get_window = env->GetMethodID(activity_class, "getWindow", "()Landroid/view/Window;");
		if (get_window == NULL)
			goto fail_clear_exception;

		window = env->CallObjectMethod(_app->activity->clazz, get_window);
		if (window == NULL || env->ExceptionCheck())
			goto fail_clear_exception;

		window_class = env->GetObjectClass(window);
		if (window_class == NULL)
			goto fail_clear_exception;

		get_decor_view = env->GetMethodID(window_class, "getDecorView", "()Landroid/view/View;");
		if (get_decor_view == NULL)
			goto fail_clear_exception;

		decor_view = env->CallObjectMethod(window, get_decor_view);
		if (decor_view == NULL || env->ExceptionCheck())
			goto fail_clear_exception;

		view_class = env->GetObjectClass(decor_view);
		if (view_class == NULL)
			goto fail_clear_exception;

		request_pointer_capture = env->GetMethodID(view_class, "requestPointerCapture", "()V");
		release_pointer_capture = env->GetMethodID(view_class, "releasePointerCapture", "()V");
		if (request_pointer_capture == NULL || release_pointer_capture == NULL)
			goto fail_clear_exception;

		_pointer_capture_view = env->NewGlobalRef(decor_view);
		if (_pointer_capture_view == NULL)
			goto fail_clear_exception;

		_request_pointer_capture = request_pointer_capture;
		_release_pointer_capture = release_pointer_capture;
		goto cleanup;

	fail_clear_exception:
		if (env->ExceptionCheck())
			env->ExceptionClear();
	fail_detach:
		_request_pointer_capture = NULL;
		_release_pointer_capture = NULL;
		_jni_vm = NULL;
	cleanup:
		if (view_class != NULL)
			env->DeleteLocalRef(view_class);
		if (decor_view != NULL)
			env->DeleteLocalRef(decor_view);
		if (window_class != NULL)
			env->DeleteLocalRef(window_class);
		if (window != NULL)
			env->DeleteLocalRef(window);
		if (activity_class != NULL)
			env->DeleteLocalRef(activity_class);
		if (detach_thread)
			vm->DetachCurrentThread();
		return;
	}

	void shutdown_pointer_capture_api()
	{
		if (_jni_vm == NULL || _pointer_capture_view == NULL)
			return;
		JavaVM *vm = _jni_vm;

		JNIEnv *env = NULL;
		bool detach_thread = false;
		const s32 get_env = vm->GetEnv((void **)&env, JNI_VERSION_1_6);
		if (get_env == JNI_EDETACHED) {
			if (vm->AttachCurrentThread(&env, NULL) != JNI_OK)
				return;
			detach_thread = true;
		} else if (get_env != JNI_OK) {
			return;
		}

		env->DeleteGlobalRef(_pointer_capture_view);
		_pointer_capture_view = NULL;
		_request_pointer_capture = NULL;
		_release_pointer_capture = NULL;
		_jni_vm = NULL;

		if (detach_thread)
			vm->DetachCurrentThread();
	}

	bool set_pointer_capture(bool enable)
	{
		if (_pointer_capture_view == NULL || _request_pointer_capture == NULL || _release_pointer_capture == NULL || _jni_vm == NULL)
			return false;

		JNIEnv *env = NULL;
		bool detach_thread = false;
		const s32 get_env = _jni_vm->GetEnv((void **)&env, JNI_VERSION_1_6);
		if (get_env == JNI_EDETACHED) {
			if (_jni_vm->AttachCurrentThread(&env, NULL) != JNI_OK)
				return false;
			detach_thread = true;
		} else if (get_env != JNI_OK) {
			return false;
		}

		env->CallVoidMethod(_pointer_capture_view
			, enable ? _request_pointer_capture : _release_pointer_capture
			);
		const bool ok = !env->ExceptionCheck();
		if (!ok)
			env->ExceptionClear();

		if (detach_thread)
			_jni_vm->DetachCurrentThread();

		return ok;
	}

	void run(struct android_app *app, DeviceOptions &opts)
	{
		_app = app;
		_opts = &opts;

		app->userData = this;
		app->onAppCmd = crown::AndroidDevice::on_app_cmd;
		app->onInputEvent = crown::AndroidDevice::on_input_event;
		ANativeActivity_setWindowFlags(app->activity
			, AWINDOW_FLAG_FULLSCREEN | AWINDOW_FLAG_KEEP_SCREEN_ON
			, 0
			);

		while (app->destroyRequested == 0) {
			s32 num;
			android_poll_source *source;
			ALooper_pollOnce(-1, NULL, &num, (void **)&source);

			if (source != NULL)
				source->process(app, source);
		}

		_main_thread.stop();
		shutdown_pointer_capture_api();
	}

	void process_command(struct android_app *app, s32 cmd)
	{
		switch (cmd) {
		case APP_CMD_SAVE_STATE:
			break;

		case APP_CMD_INIT_WINDOW: {
			CE_ASSERT(app->window != NULL, "Android window is NULL");
			_window = app->window;
			init_pointer_capture_api();

			// Push metrics here since Android does not trigger APP_CMD_WINDOW_RESIZED
			const s32 width  = ANativeWindow_getWidth(app->window);
			const s32 height = ANativeWindow_getHeight(app->window);
			_queue.push_resolution_event(width, height);

			if (!_main_thread.is_running()) {
				_main_thread.start([](void *user_data) {
						return crown::main_runtime(*((DeviceOptions *)user_data));
					}
					, _opts
					);
			}
			break;
		}

		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			break;

		case APP_CMD_WINDOW_RESIZED:
			// Not triggered by Android
			break;

		case APP_CMD_GAINED_FOCUS:
			break;

		case APP_CMD_LOST_FOCUS:
			break;

		case APP_CMD_DESTROY:
			_queue.push_exit_event();
			break;
		}
	}

#ifndef AMOTION_EVENT_AXIS_RELATIVE_X
#define AMOTION_EVENT_AXIS_RELATIVE_X 27
#endif
#ifndef AMOTION_EVENT_AXIS_RELATIVE_Y
#define AMOTION_EVENT_AXIS_RELATIVE_Y 28
#endif
#ifndef AINPUT_SOURCE_MOUSE_RELATIVE
#define AINPUT_SOURCE_MOUSE_RELATIVE 0x00020004
#endif

	s32 process_input(struct android_app *app, AInputEvent *event)
	{
		CE_UNUSED(app);
		const s32 type = AInputEvent_getType(event);
		if (type == AINPUT_EVENT_TYPE_MOTION) {
			const s32 source = AInputEvent_getSource(event);
			const s32 action_raw = AMotionEvent_getAction(event);
			const s32 action = action_raw & AMOTION_EVENT_ACTION_MASK;
			const bool is_joypad =
				((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK)
				|| ((source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD)
				;

			if (is_joypad) {
				if (action == AMOTION_EVENT_ACTION_MOVE || action == AMOTION_EVENT_ACTION_HOVER_MOVE) {
					const s8 joypad_num = joypad_id(AInputEvent_getDeviceId(event), true);
					if (joypad_num == -1)
						return 1;
					Joypad::State &jp = _joypad.state[joypad_num];

					const s16 lx =  joypad_axis(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X, 0));
					const s16 ly = -joypad_axis(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y, 0));
					const s16 rx =  joypad_axis(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Z, 0));
					const s16 ry = -joypad_axis(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RZ, 0));
					const s16 lz =  joypad_trigger(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_LTRIGGER, 0));
					const s16 rz =  joypad_trigger(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RTRIGGER, 0));

					if (lx != jp.axis.lx || ly != jp.axis.ly) {
						jp.axis.lx = lx;
						jp.axis.ly = ly;
						_queue.push_axis_event(InputDeviceType::JOYPAD
							, joypad_num
							, JoypadAxis::LEFT
							, lx
							, ly
							, 0
							);
					}

					if (rx != jp.axis.rx || ry != jp.axis.ry) {
						jp.axis.rx = rx;
						jp.axis.ry = ry;
						_queue.push_axis_event(InputDeviceType::JOYPAD
							, joypad_num
							, JoypadAxis::RIGHT
							, rx
							, ry
							, 0
							);
					}

					if (lz != jp.axis.lz) {
						jp.axis.lz = lz;
						_queue.push_axis_event(InputDeviceType::JOYPAD
							, joypad_num
							, JoypadAxis::TRIGGER_LEFT
							, 0
							, 0
							, lz
							);
					}

					if (rz != jp.axis.rz) {
						jp.axis.rz = rz;
						_queue.push_axis_event(InputDeviceType::JOYPAD
							, joypad_num
							, JoypadAxis::TRIGGER_RIGHT
							, 0
							, 0
							, rz
							);
					}

					const s8 hat_x = joypad_hat_value(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_X, 0));
					const s8 hat_y = joypad_hat_value(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_Y, 0));
					if (hat_x != jp.hat_x) {
						const bool left_pressed = hat_x < 0;
						const bool right_pressed = hat_x > 0;
						const u16 left_mask = 1u << JoypadButton::LEFT;
						const u16 right_mask = 1u << JoypadButton::RIGHT;
						const bool left_old = (jp.button_state & left_mask) != 0;
						const bool right_old = (jp.button_state & right_mask) != 0;

						if (left_pressed != left_old) {
							jp.button_state = (jp.button_state & ~left_mask) | (left_pressed ? left_mask : 0);

							_queue.push_button_event(InputDeviceType::JOYPAD
								, joypad_num
								, JoypadButton::LEFT
								, left_pressed
								);
						}

						if (right_pressed != right_old) {
							jp.button_state = (jp.button_state & ~right_mask) | (right_pressed ? right_mask : 0);

							_queue.push_button_event(InputDeviceType::JOYPAD
								, joypad_num
								, JoypadButton::RIGHT
								, right_pressed
								);
						}

						jp.hat_x = hat_x;
					}

					if (hat_y != jp.hat_y) {
						const bool up_pressed = hat_y < 0;
						const bool down_pressed = hat_y > 0;
						const u16 up_mask = 1u << JoypadButton::UP;
						const u16 down_mask = 1u << JoypadButton::DOWN;
						const bool up_old = (jp.button_state & up_mask) != 0;
						const bool down_old = (jp.button_state & down_mask) != 0;

						if (up_pressed != up_old) {
							jp.button_state = (jp.button_state & ~up_mask) | (up_pressed ? up_mask : 0);

							_queue.push_button_event(InputDeviceType::JOYPAD
								, joypad_num
								, JoypadButton::UP
								, up_pressed
								);
						}

						if (down_pressed != down_old) {
							jp.button_state = (jp.button_state & ~down_mask) | (down_pressed ? down_mask : 0);

							_queue.push_button_event(InputDeviceType::JOYPAD
								, joypad_num
								, JoypadButton::DOWN
								, down_pressed
								);
						}

						jp.hat_y = hat_y;
					}
				}

				return 1;
			}

			const bool is_mouse =
				((source & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE)
				|| ((source & AINPUT_SOURCE_MOUSE_RELATIVE) == AINPUT_SOURCE_MOUSE_RELATIVE)
				|| (AMotionEvent_getToolType(event, 0) == AMOTION_EVENT_TOOL_TYPE_MOUSE)
				;

			if (is_mouse) {
				if (action == AMOTION_EVENT_ACTION_MOVE
					|| action == AMOTION_EVENT_ACTION_HOVER_MOVE
					|| action == AMOTION_EVENT_ACTION_DOWN
					|| action == AMOTION_EVENT_ACTION_UP
					|| action == AMOTION_EVENT_ACTION_BUTTON_PRESS
					|| action == AMOTION_EVENT_ACTION_BUTTON_RELEASE) {
					_queue.push_axis_event(InputDeviceType::MOUSE
						, 0
						, MouseAxis::CURSOR_DELTA
						, AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RELATIVE_X, 0)
						, AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RELATIVE_Y, 0)
						, 0
						);

					if (_cursor_mode != CursorMode::DISABLED) {
						_queue.push_axis_event(InputDeviceType::MOUSE
							, 0
							, MouseAxis::CURSOR
							, (s16)AMotionEvent_getX(event, 0)
							, (s16)AMotionEvent_getY(event, 0)
							, 0
							);
					}
				}

				if (action == AMOTION_EVENT_ACTION_SCROLL) {
					_queue.push_axis_event(InputDeviceType::MOUSE
						, 0
						, MouseAxis::WHEEL
						, AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HSCROLL, 0)
						, AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, 0)
						, 0
						);
				}

				if (action == AMOTION_EVENT_ACTION_BUTTON_PRESS
					|| action == AMOTION_EVENT_ACTION_BUTTON_RELEASE
					|| action == AMOTION_EVENT_ACTION_DOWN
					|| action == AMOTION_EVENT_ACTION_UP) {
					const s32 new_button_state = AMotionEvent_getButtonState(event);
					const s32 changed_button_state = _mouse_button_state ^ new_button_state;
					const s32 button_masks[] = {
						AMOTION_EVENT_BUTTON_PRIMARY,
						AMOTION_EVENT_BUTTON_SECONDARY,
						AMOTION_EVENT_BUTTON_TERTIARY,
						AMOTION_EVENT_BUTTON_BACK,
						AMOTION_EVENT_BUTTON_FORWARD
					};

					for (u32 i = 0; i < countof(button_masks); ++i) {
						const s32 mask = button_masks[i];
						if ((changed_button_state & mask) == 0)
							continue;

						const MouseButton::Enum mb = mouse_button(mask);
						if (mb != MouseButton::COUNT)
							_queue.push_button_event(InputDeviceType::MOUSE, 0, mb, (new_button_state & mask) != 0);
					}

					_mouse_button_state = new_button_state;
				}

				return 1;
			}

			const s32 pointer_index = (action_raw & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			const s32 pointer_id = AMotionEvent_getPointerId(event, pointer_index);
			const f32 pointer_x = AMotionEvent_getX(event, pointer_index);
			const f32 pointer_y = AMotionEvent_getY(event, pointer_index);

			switch (action) {
			case AMOTION_EVENT_ACTION_DOWN:
			case AMOTION_EVENT_ACTION_POINTER_DOWN:
				if (pointer_id < TouchButton::COUNT)
					_queue.push_button_event(InputDeviceType::TOUCHSCREEN, 0, pointer_id, true);
				if (pointer_id < TouchAxis::COUNT)
					_queue.push_axis_event(InputDeviceType::TOUCHSCREEN, 0, pointer_id, pointer_x, pointer_y, 0);
				break;

			case AMOTION_EVENT_ACTION_UP:
			case AMOTION_EVENT_ACTION_POINTER_UP:
				if (pointer_id < TouchAxis::COUNT)
					_queue.push_axis_event(InputDeviceType::TOUCHSCREEN, 0, pointer_id, pointer_x, pointer_y, 0);
				if (pointer_id < TouchButton::COUNT)
					_queue.push_button_event(InputDeviceType::TOUCHSCREEN, 0, pointer_id, false);
				break;

			case AMOTION_EVENT_ACTION_OUTSIDE:
			case AMOTION_EVENT_ACTION_CANCEL:
				if (pointer_id < TouchButton::COUNT)
					_queue.push_button_event(InputDeviceType::TOUCHSCREEN, 0, pointer_id, false);
				break;

			case AMOTION_EVENT_ACTION_MOVE: {
				const s32 pointer_count = AMotionEvent_getPointerCount(event);
				for (s32 index = 0; index < pointer_count; ++index) {
					const s32 id = AMotionEvent_getPointerId(event, index);
					if (id < TouchAxis::COUNT)
						_queue.push_axis_event(InputDeviceType::TOUCHSCREEN, 0, id, AMotionEvent_getX(event, index), AMotionEvent_getY(event, index), 0);
				}
				break;
			}
			}

			return 1;
		}

		if (type == AINPUT_EVENT_TYPE_KEY) {
			const s32 keycode   = AKeyEvent_getKeyCode(event);
			const s32 keyaction = AKeyEvent_getAction(event);
			const s32 source = AInputEvent_getSource(event);
			const bool is_joypad =
				((source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD)
				|| ((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK)
				|| ((source & AINPUT_SOURCE_DPAD) == AINPUT_SOURCE_DPAD)
				;

			if (is_joypad) {
				const JoypadButton::Enum jb = joypad_button(keycode);
				if (jb != JoypadButton::COUNT) {
					const s8 joypad_num = joypad_id(AInputEvent_getDeviceId(event), true);
					if (joypad_num != -1 && (keyaction == AKEY_EVENT_ACTION_DOWN || keyaction == AKEY_EVENT_ACTION_UP)) {
						Joypad::State &jp = _joypad.state[joypad_num];
						const bool pressed = keyaction == AKEY_EVENT_ACTION_DOWN;
						const u16 mask = 1u << jb;
						const bool old_state = (jp.button_state & mask) != 0;

						if (pressed != old_state) {
							jp.button_state = (jp.button_state & ~mask) | (pressed ? mask : 0);

							_queue.push_button_event(InputDeviceType::JOYPAD
								, joypad_num
								, jb
								, pressed
								);
						}
					}
				}

				return 1;
			}

			const KeyboardButton::Enum kb = keycode == AKEYCODE_BACK
				? KeyboardButton::ESCAPE
				: android_translate_key(keycode)
				;

			if (kb != KeyboardButton::COUNT) {
				_queue.push_button_event(InputDeviceType::KEYBOARD
					, 0
					, kb
					, keyaction == AKEY_EVENT_ACTION_DOWN
					);
			}

			return 1;
		}

		return 0;
	}

	static s32 on_input_event(struct android_app *app, AInputEvent *event)
	{
		return static_cast<AndroidDevice *>(app->userData)->process_input(app, event);
	}

	static void on_app_cmd(struct android_app *app, s32 cmd)
	{
		static_cast<AndroidDevice *>(app->userData)->process_command(app, cmd);
	}
};

static AndroidDevice *s_android_device;

struct WindowAndroid : public Window
{
	WindowAndroid()
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
		if (mode == s_android_device->_cursor_mode)
			return true;

		if (mode == CursorMode::DISABLED) {
			if (!s_android_device->set_pointer_capture(true))
				return false;
		} else if (mode == CursorMode::NORMAL) {
			if (!s_android_device->set_pointer_capture(false))
				return false;
		}

		s_android_device->_cursor_mode = mode;
		return true;
	}

	void *native_handle() override
	{
		return s_android_device->_window;
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
		return CE_NEW(a, WindowAndroid)();
	}

	void destroy(Allocator &a, Window &w)
	{
		CE_DELETE(a, &w);
	}

} // namespace window

struct DisplayAndroid : public Display
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
		return CE_NEW(a, DisplayAndroid)();
	}

	void destroy(Allocator &a, Display &d)
	{
		CE_DELETE(a, &d);
	}

} // namespace display

static bool push_event(const OsEvent &ev)
{
	return s_android_device->_events.push(ev);
}

bool next_event(OsEvent &ev)
{
	return s_android_device->_events.pop(ev);
}

} // namespace crown

void android_main(struct android_app *app)
{
	using namespace crown;

	memory_globals::init();
	profiler_globals::init();
	guid_globals::init();

	DeviceOptions opts(default_allocator(), 0, NULL);
	g_platform_data._android_asset_manager = app->activity->assetManager;
	g_platform_data._android_internal_data_path = (void *)app->activity->internalDataPath;
	g_platform_data._android_obb_path = (void *)app->activity->obbPath;

	s_android_device = CE_NEW(default_allocator(), AndroidDevice)(default_allocator());
	s_android_device->run(app, opts);
	CE_DELETE(default_allocator(), s_android_device);

	guid_globals::shutdown();
	profiler_globals::shutdown();
	memory_globals::shutdown();
}

#endif // if CROWN_PLATFORM_ANDROID
