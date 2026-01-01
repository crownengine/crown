/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#if CROWN_PLATFORM_LINUX
extern uint gdk_x11_window_get_xid(Gdk.Window window);
#elif CROWN_PLATFORM_WINDOWS
extern uint gdk_win32_window_get_handle(Gdk.Window window);
#endif

namespace Crown
{
public class EditorView : Gtk.EventBox
{
	public const Gtk.TargetEntry[] dnd_targets =
	{
		{ "RESOURCE_PATH", Gtk.TargetFlags.SAME_APP, 0 },
	};

	// Data
	public RuntimeInstance _runtime;

	public Gtk.Allocation _allocation;
	public uint _resize_timer_id;

	public bool _mouse_left;
	public bool _mouse_middle;
	public bool _mouse_right;

	public uint _window_id;
	public uint _last_window_id;

	public Gee.HashMap<uint, bool> _keys;
	public bool _input_enabled;
	public bool _drag_enter;
	public uint _drag_last_time;
	public int64 _motion_last_time;
	public const int MOTION_EVENTS_RATE = 75;

	public GLib.StringBuilder _buffer;

	public Gtk.EventControllerKey _controller_key;
	public Gtk.GestureMultiPress _gesture_click;
	public Gtk.EventControllerMotion _controller_motion;
	public Gtk.EventControllerScroll _controller_scroll;

	// Signals
	public signal void native_window_ready(uint window_id, int width, int height);

	public string key_to_string(uint k)
	{
		switch (k) {
		case Gdk.Key.w:         return "w";
		case Gdk.Key.a:         return "a";
		case Gdk.Key.s:         return "s";
		case Gdk.Key.d:         return "d";
		case Gdk.Key.Control_L: return "ctrl_left";
		case Gdk.Key.Shift_L:   return "shift_left";
		case Gdk.Key.Alt_L:     return "alt_left";
		case Gdk.Key.Alt_R:     return "alt_right";
		default:                return "<unknown>";
		}
	}

	public bool camera_modifier_pressed()
	{
		return _keys[Gdk.Key.Alt_L]
			|| _keys[Gdk.Key.Alt_R]
			;
	}

	public void camera_modifier_reset()
	{
		_keys[Gdk.Key.Alt_L] = false;
		_keys[Gdk.Key.Alt_R] = false;
	}

	public EditorView(RuntimeInstance runtime, bool input_enabled = true)
	{
		_runtime = runtime;

		_allocation = { 0, 0, 0, 0 };
		_resize_timer_id = 0;

		_mouse_left   = false;
		_mouse_middle = false;
		_mouse_right  = false;

		_window_id = 0;
		_last_window_id = 0;

		_keys = new Gee.HashMap<uint, bool>();
		_keys[Gdk.Key.w] = false;
		_keys[Gdk.Key.a] = false;
		_keys[Gdk.Key.s] = false;
		_keys[Gdk.Key.d] = false;
		_keys[Gdk.Key.Control_L] = false;
		_keys[Gdk.Key.Shift_L] = false;
		_keys[Gdk.Key.Alt_L] = false;
		_keys[Gdk.Key.Alt_R] = false;

		_input_enabled = input_enabled;
		_drag_enter = false;
		_drag_last_time = 0;
		_motion_last_time = 0;

		_buffer = new GLib.StringBuilder();

		// Widgets
		this.can_focus = true;
		this.events |= Gdk.EventMask.POINTER_MOTION_MASK
			| Gdk.EventMask.KEY_PRESS_MASK
			| Gdk.EventMask.KEY_RELEASE_MASK
			| Gdk.EventMask.FOCUS_CHANGE_MASK
			| Gdk.EventMask.SCROLL_MASK
			;
		this.focus_out_event.connect(on_event_box_focus_out_event);
		this.size_allocate.connect(on_size_allocate);

		if (input_enabled) {
			_controller_key = new Gtk.EventControllerKey(this);
			_controller_key.key_pressed.connect(on_key_pressed);
			_controller_key.key_released.connect(on_key_released);

			_gesture_click = new Gtk.GestureMultiPress(this);
			_gesture_click.set_button(0);
			_gesture_click.pressed.connect(on_button_pressed);
			_gesture_click.released.connect(on_button_released);

			_controller_motion = new Gtk.EventControllerMotion(this);
			_controller_motion.enter.connect(on_enter);
			_controller_motion.motion.connect(on_motion);

			_controller_scroll = new Gtk.EventControllerScroll(this, Gtk.EventControllerScrollFlags.BOTH_AXES);
			_controller_scroll.scroll.connect(on_scroll);
		}

		this.realize.connect(on_event_box_realized);
		this.set_visual(Gdk.Screen.get_default().get_system_visual());
		this.events |= Gdk.EventMask.STRUCTURE_MASK; // map_event
		this.map_event.connect(() => {
				device_frame_delayed(16, _runtime);
				return Gdk.EVENT_PROPAGATE;
			});

		Gtk.drag_dest_set(this, Gtk.DestDefaults.MOTION, dnd_targets, Gdk.DragAction.COPY);
		this.drag_data_received.connect(on_drag_data_received);
		this.drag_motion.connect(on_drag_motion);
		this.drag_drop.connect(on_drag_drop);
		this.drag_leave.connect(on_drag_leave);
	}

	public void on_drag_data_received(Gdk.DragContext context, int x, int y, Gtk.SelectionData data, uint info, uint time_)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_data_received.html
		unowned uint8[] raw_data = data.get_data_with_length();
		if (raw_data.length == -1)
			return;

		string resource_path = (string)raw_data;
		string type = ResourceId.type(resource_path);
		string name = ResourceId.name(resource_path);

		if (type == OBJECT_TYPE_UNIT || type == OBJECT_TYPE_SOUND) {
			GLib.Application.get_default().activate_action("set-placeable", new GLib.Variant.tuple({ type, name }));

			int scale = this.get_scale_factor();
			_runtime.send_script(LevelEditorApi.mouse_down(x*scale, y*scale));
		}
	}

	public bool on_drag_motion(Gdk.DragContext context, int x, int y, uint _time)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_motion.html
		Gdk.Atom target;

		target = Gtk.drag_dest_find_target(this, context, null);
		if (target == Gdk.Atom.NONE) {
			Gdk.drag_status(context, 0, _time);
		} else {
			if (_drag_enter == false) {
				Gtk.drag_get_data(this, context, target, _time);
				_drag_enter = true;
			}

			if (_time - _drag_last_time >= 1000/MOTION_EVENTS_RATE) {
				// Drag motion events seem to fire at a very high frequency compared to regular
				// motion notify events. Limit them to 60 hz.
				_drag_last_time = _time;
				int scale = this.get_scale_factor();
				_runtime.send_script(LevelEditorApi.set_mouse_state(x*scale
					, y*scale
					, _mouse_left
					, _mouse_middle
					, _mouse_right
					));

				_runtime.send(DeviceApi.frame());
			}
		}

		return true;
	}

	public bool on_drag_drop(Gdk.DragContext context, int x, int y, uint time_)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_drop.html
		int scale = this.get_scale_factor();
		_runtime.send_script(LevelEditorApi.mouse_up(x*scale, y*scale));
		GLib.Application.get_default().activate_action("cancel-place", null);
		_runtime.send(DeviceApi.frame());
		Gtk.drag_finish(context, true, false, time_);
		return true;
	}

	public void on_drag_leave(Gdk.DragContext context, uint time_)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_leave.html
		_drag_enter = false;
	}

	public void on_button_released(int n_press, double x, double y)
	{
		uint button = _gesture_click.get_current_button();
		int scale = this.get_scale_factor();

		_mouse_left   = button == Gdk.BUTTON_PRIMARY   ? false : _mouse_left;
		_mouse_middle = button == Gdk.BUTTON_MIDDLE    ? false : _mouse_middle;
		_mouse_right  = button == Gdk.BUTTON_SECONDARY ? false : _mouse_right;

		_buffer.append(LevelEditorApi.set_mouse_state((int)x*scale
			, (int)y*scale
			, _mouse_left
			, _mouse_middle
			, _mouse_right
			));

		if (button == Gdk.BUTTON_PRIMARY)
			_buffer.append(LevelEditorApi.mouse_up((int)x*scale, (int)y*scale));

		if (camera_modifier_pressed()) {
			if (!_mouse_left || !_mouse_middle || !_mouse_right)
				_buffer.append("LevelEditor:camera_drag_start('idle')");
		} else if (!_mouse_middle) {
			_buffer.append("LevelEditor:camera_drag_start('idle')");
		}

		if (_buffer.len != 0) {
			_runtime.send_script(_buffer.str);
			_buffer.erase();
			_runtime.send(DeviceApi.frame());
		}
	}

	public void on_button_pressed(int n_press, double x, double y)
	{
		uint button = _gesture_click.get_current_button();
		int scale = this.get_scale_factor();

		this.grab_focus();

		_mouse_left   = button == Gdk.BUTTON_PRIMARY   ? true : _mouse_left;
		_mouse_middle = button == Gdk.BUTTON_MIDDLE    ? true : _mouse_middle;
		_mouse_right  = button == Gdk.BUTTON_SECONDARY ? true : _mouse_right;

		_buffer.append(LevelEditorApi.set_mouse_state((int)x*scale
			, (int)y*scale
			, _mouse_left
			, _mouse_middle
			, _mouse_right
			));

		if (camera_modifier_pressed()) {
			if (_mouse_left)
				_buffer.append("LevelEditor:camera_drag_start('tumble')");
			if (_mouse_middle)
				_buffer.append("LevelEditor:camera_drag_start('track')");
			if (_mouse_right)
				_buffer.append("LevelEditor:camera_drag_start('dolly')");
		} else if (_mouse_middle) {
			_buffer.append("LevelEditor:camera_drag_start('tumble')");
		}

		if (button == Gdk.BUTTON_PRIMARY)
			_buffer.append(LevelEditorApi.mouse_down((int)x*scale, (int)y*scale));

		if (_buffer.len != 0) {
			_runtime.send_script(_buffer.str);
			_buffer.erase();
			_runtime.send(DeviceApi.frame());
		}
	}

	public bool on_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval == Gdk.Key.Escape)
			GLib.Application.get_default().activate_action("cancel-place", null);

		if (keyval == Gdk.Key.Up)
			_buffer.append("LevelEditor:key_down(\"move_up\")");
		if (keyval == Gdk.Key.Down)
			_buffer.append("LevelEditor:key_down(\"move_down\")");
		if (keyval == Gdk.Key.Right)
			_buffer.append("LevelEditor:key_down(\"move_right\")");
		if (keyval == Gdk.Key.Left)
			_buffer.append("LevelEditor:key_down(\"move_left\")");

		if (_keys.has_key(keyval)) {
			if (!_keys[keyval])
				_buffer.append(LevelEditorApi.key_down(key_to_string(keyval)));

			_keys[keyval] = true;
		}

		if (_buffer.len != 0) {
			_runtime.send_script(_buffer.str);
			_buffer.erase();
			_runtime.send(DeviceApi.frame());
		}
		return Gdk.EVENT_PROPAGATE;
	}

	public void on_key_released(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (_keys.has_key(keyval)) {
			if (_keys[keyval])
				_buffer.append(LevelEditorApi.key_up(key_to_string(keyval)));

			_keys[keyval] = false;
		}

		if (_buffer.len != 0) {
			_runtime.send_script(_buffer.str);
			_buffer.erase();
			_runtime.send(DeviceApi.frame());
		}
	}

	public void on_motion(double x, double y)
	{
		int64 now = GLib.get_monotonic_time();

		if (now - _motion_last_time >= (1000*1000)/MOTION_EVENTS_RATE) {
			_motion_last_time = now;
			int scale = this.get_scale_factor();
			_runtime.send_script(LevelEditorApi.set_mouse_state((int)x*scale
				, (int)y*scale
				, _mouse_left
				, _mouse_middle
				, _mouse_right
				));
			_runtime.send(DeviceApi.frame());
		}
	}

	public void on_scroll(double dx, double dy)
	{
		if (camera_modifier_pressed()) {
			_runtime.send_script(LevelEditorApi.mouse_wheel(dy));
		} else {
			_runtime.send_script("LevelEditor:camera_drag_start_relative('dolly')");
			_runtime.send_script("LevelEditor._camera:update(1,0,%.17f,1,1)".printf(-dy * 32.0));
			_runtime.send_script("LevelEditor:camera_drag_start('idle')");
			_runtime.send(DeviceApi.frame());
		}
	}

	public bool on_event_box_focus_out_event(Gdk.EventFocus ev)
	{
		camera_modifier_reset();

		_keys[Gdk.Key.Control_L] = false;
		_keys[Gdk.Key.Shift_L] = false;
		_runtime.send_script(LevelEditorApi.key_up(key_to_string(Gdk.Key.Control_L)));
		_runtime.send_script(LevelEditorApi.key_up(key_to_string(Gdk.Key.Shift_L)));

		return Gdk.EVENT_PROPAGATE;
	}

	public void on_size_allocate(Gtk.Allocation ev)
	{
		int scale = this.get_scale_factor();

		if (_allocation.x == ev.x
			&& _allocation.y == ev.y
			&& _allocation.width == ev.width
			&& _allocation.height == ev.height
			)
			return;

		if (_last_window_id != _window_id) {
			_last_window_id = _window_id;
			native_window_ready(_window_id, ev.width*scale, ev.height*scale);
		}

		_allocation = ev;
		_runtime.send(DeviceApi.resize(_allocation.width*scale, _allocation.height*scale));

		// Ensure there is some delay between the last resize() and the last frame().
		if (_resize_timer_id == 0) {
			_resize_timer_id = GLib.Timeout.add_full(GLib.Priority.DEFAULT, 200, () => {
					_runtime.send(DeviceApi.frame());
					_resize_timer_id = 0;
					return GLib.Source.REMOVE;
				});
		}
	}

	public void on_event_box_realized()
	{
		this.get_window().ensure_native();
#if CROWN_PLATFORM_LINUX
		this.get_display().sync();
		_window_id = gdk_x11_window_get_xid(this.get_window());
#elif CROWN_PLATFORM_WINDOWS
		_window_id = gdk_win32_window_get_handle(this.get_window());
#endif
	}

	public void on_enter(double x, double y)
	{
		this.grab_focus();
	}
}

} /* namespace Crown */
