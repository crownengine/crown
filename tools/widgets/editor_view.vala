/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#if CROWN_GTK3 && CROWN_PLATFORM_LINUX
extern uint gdk_x11_window_get_xid(Gdk.Window window);
#elif CROWN_GTK3 && CROWN_PLATFORM_WINDOWS
extern uint gdk_win32_window_get_handle(Gdk.Window window);
#endif

namespace Crown
{
#if CROWN_GTK3
public class EditorView : Gtk.EventBox
#else
public class EditorView : Gtk.Box
#endif
{
#if CROWN_GTK3
	public const Gtk.TargetEntry[] DND_TARGETS =
	{
		{ "RESOURCE_PATH", Gtk.TargetFlags.SAME_APP, TargetInfo.RESOURCE_PATH },
	};
#endif
	// Data
	public RuntimeInstance _runtime;

	public Gtk.Allocation _allocation;
	public uint _resize_timer_id;
	public uint _enable_accels_id;
	public uint _tick_callback_id;

	public bool _mouse_left;
	public bool _mouse_middle;
	public bool _mouse_right;
	public double _flythrough_mouse_x;
	public double _flythrough_mouse_y;

	public uint _window_id;
	public uint _last_window_id;

	public GLib.HashTable<uint, bool> _keys;
	public bool _input_enabled;
	public bool _drag_enter;
	public int64 _drag_last_time;
	public int64 _motion_last_time;
	public const int MOTION_EVENTS_RATE_HZ = 75;

	public GLib.StringBuilder _buffer;

	public Gtk.EventControllerKey _controller_key;
	public Gtk.GestureSingle _gesture_click;
	public Gtk.EventControllerMotion _controller_motion;
	public Gtk.EventControllerScroll _controller_scroll;
#if !CROWN_GTK3
	public Gtk.EventControllerLegacy _controller_legacy;
	public Gtk.EventControllerFocus _controller_focus;
	public Gtk.DropTarget _drop_target;
#endif

	public InfiniteDragController _flythrough_drag;
	public bool _resetting_flythrough_gesture;

	// Signals
	public signal void native_window_ready(uint window_id, int width, int height);

	public string key_to_string(uint k)
	{
		switch (k) {
		case Gdk.Key.w:         return "w";
		case Gdk.Key.a:         return "a";
		case Gdk.Key.s:         return "s";
		case Gdk.Key.d:         return "d";
		case Gdk.Key.q:         return "q";
		case Gdk.Key.e:         return "e";
		case Gdk.Key.Up:        return "move_up";
		case Gdk.Key.Down:      return "move_down";
		case Gdk.Key.Right:     return "move_right";
		case Gdk.Key.Left:      return "move_left";
		case Gdk.Key.Control_L: return "ctrl_left";
		case Gdk.Key.Shift_L:   return "shift_left";
		case Gdk.Key.Shift_R:   return "shift_right";
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
		_enable_accels_id = 0;
		_tick_callback_id = 0;

		_mouse_left   = false;
		_mouse_middle = false;
		_mouse_right  = false;
		_flythrough_mouse_x = 0.0;
		_flythrough_mouse_y = 0.0;

		_window_id = 0;
		_last_window_id = 0;

		_keys = new GLib.HashTable<uint, bool>(GLib.direct_hash, GLib.direct_equal);
		_keys[Gdk.Key.w] = false;
		_keys[Gdk.Key.a] = false;
		_keys[Gdk.Key.s] = false;
		_keys[Gdk.Key.d] = false;
		_keys[Gdk.Key.q] = false;
		_keys[Gdk.Key.e] = false;
		_keys[Gdk.Key.Up] = false;
		_keys[Gdk.Key.Down] = false;
		_keys[Gdk.Key.Right] = false;
		_keys[Gdk.Key.Left] = false;
		_keys[Gdk.Key.Control_L] = false;
		_keys[Gdk.Key.Shift_L] = false;
		_keys[Gdk.Key.Shift_R] = false;
		_keys[Gdk.Key.Alt_L] = false;
		_keys[Gdk.Key.Alt_R] = false;

		_input_enabled = input_enabled;
		_drag_enter = false;
		_drag_last_time = 0;
		_motion_last_time = 0;

		_buffer = new GLib.StringBuilder();

		// Widgets
#if CROWN_GTK3
		this.can_focus = true;
		this.events |= 0
			| Gdk.EventMask.BUTTON_PRESS_MASK
			| Gdk.EventMask.BUTTON_RELEASE_MASK
			| Gdk.EventMask.POINTER_MOTION_MASK
			| Gdk.EventMask.KEY_PRESS_MASK
			| Gdk.EventMask.KEY_RELEASE_MASK
			| Gdk.EventMask.FOCUS_CHANGE_MASK
			| Gdk.EventMask.SCROLL_MASK
			;
		this.focus_out_event.connect(on_event_box_focus_out_event);
		this.size_allocate.connect(on_size_allocate);
#else
		this.focusable = true;

		_controller_focus = new Gtk.EventControllerFocus();
		_controller_focus.leave.connect(on_event_box_focus_leave);
		this.add_controller(_controller_focus);
#endif /* if CROWN_GTK3 */

		if (input_enabled) {
			_flythrough_drag = new InfiniteDragController(this);
			_flythrough_drag.axis_mode = InfiniteDragController.Axis.XY;
			_flythrough_drag.activation_margin = 0.0;
			_flythrough_drag.trigger_button = Gdk.BUTTON_SECONDARY; // must match the button start()
			_flythrough_drag.cancel_button = 0; // rmb itself drives flythrough; no separate abort button
			_flythrough_drag.preserve_legacy_events = true; // let GTK observe the matching rmb release

#if CROWN_GTK3
			this.button_press_event.connect(on_event_box_button_event);
			this.button_release_event.connect(on_event_box_button_event);
			_controller_key = new Gtk.EventControllerKey(this);
			_gesture_click = new Gtk.GestureMultiPress(this);
			((Gtk.GestureMultiPress)_gesture_click).pressed.connect(on_button_pressed);
			((Gtk.GestureMultiPress)_gesture_click).released.connect(on_button_released);
			_controller_motion = new Gtk.EventControllerMotion(this);
			_controller_scroll = new Gtk.EventControllerScroll(this, Gtk.EventControllerScrollFlags.BOTH_AXES);
			_flythrough_drag.drag_scroll.connect(on_scroll);
#else
			_controller_legacy = new Gtk.EventControllerLegacy();
			_controller_legacy.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
			_controller_legacy.event.connect(on_event_controller_legacy_event);
			this.add_controller(_controller_legacy);
			_controller_key = new Gtk.EventControllerKey();
			_gesture_click = new Gtk.GestureClick();
			((Gtk.GestureClick)_gesture_click).pressed.connect(on_button_pressed);
			((Gtk.GestureClick)_gesture_click).released.connect(on_button_released);
			_controller_motion = new Gtk.EventControllerMotion();
			_controller_scroll = new Gtk.EventControllerScroll(Gtk.EventControllerScrollFlags.BOTH_AXES);
			this.add_controller(_controller_key);
			this.add_controller(_gesture_click);
			this.add_controller(_controller_motion);
			this.add_controller(_controller_scroll);
			_flythrough_drag.drag_scroll.connect(on_flythrough_scroll);
#endif /* if CROWN_GTK3 */
			_controller_key.key_pressed.connect(on_key_pressed);
			_controller_key.key_released.connect(on_key_released);
			_gesture_click.set_button(0);
			_gesture_click.cancel.connect(on_gesture_cancelled);
			_flythrough_drag.drag_delta.connect(on_flythrough_drag_delta);
			_flythrough_drag.drag_finished.connect(on_flythrough_drag_finished);
			_flythrough_drag.release_detected_externally.connect(on_flythrough_release_detected_externally);
			_controller_motion.enter.connect(on_enter);
			_controller_motion.motion.connect(on_motion);
			_controller_scroll.scroll.connect(on_scroll);
		}

#if CROWN_GTK3
		this.realize.connect(on_event_box_realized);
		this.set_visual(Gdk.Screen.get_default().get_system_visual());
		this.events |= Gdk.EventMask.STRUCTURE_MASK; // map_event
		this.map_event.connect(() => {
				device_frame_delayed(16, _runtime);
				return Gdk.EVENT_PROPAGATE;
			});
		Gtk.drag_dest_set(this, Gtk.DestDefaults.MOTION, DND_TARGETS, Gdk.DragAction.COPY);
		this.drag_data_received.connect(on_drag_data_received);
		this.drag_motion.connect(on_drag_motion);
		this.drag_drop.connect(on_drag_drop);
		this.drag_leave.connect(on_drag_leave);
#else
		_drop_target = new Gtk.DropTarget(typeof(string), Gdk.DragAction.COPY);
		_drop_target.preload = true;
		_drop_target.accept.connect(on_drag_accept);
		_drop_target.enter.connect(on_drag_enter);
		_drop_target.motion.connect(on_drag_motion);
		_drop_target.drop.connect(on_drag_drop);
		_drop_target.leave.connect(on_drag_leave);
		this.add_controller(_drop_target);
		Gtk.Label placeholder = new Gtk.Label("EditorView");
		placeholder.hexpand = true;
		this.append(placeholder);
#endif /* if CROWN_GTK3 */
	}

	public void place_drag_resource_path(string resource_path, double x, double y)
	{
		string type = ResourceId.type(resource_path);
		string name = ResourceId.name(resource_path);
		if (type == OBJECT_TYPE_UNIT || type == OBJECT_TYPE_SOUND) {
			GLib.Application.get_default().activate_action("set-placeable", new GLib.Variant.tuple({ type, name }));

			int scale = this.get_scale_factor();
			_runtime.send_script(LevelEditorApi.mouse_down((int)x*scale, (int)y*scale));
		}
	}

#if CROWN_GTK3
	public void on_drag_data_received(Gdk.DragContext context, int x, int y, Gtk.SelectionData data, uint info, uint time_)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_data_received.html
		unowned uint8[] raw_data = data.get_data_with_length();
		if (raw_data.length == -1)
			return;

		place_drag_resource_path((string)raw_data, x, y);
	}
#endif

	public bool can_drop_resource_path(string resource_path)
	{
		string type = ResourceId.type(resource_path);
		return type == OBJECT_TYPE_UNIT || type == OBJECT_TYPE_SOUND;
	}

#if !CROWN_GTK3
	public bool has_drag_resource_path()
	{
		Gdk.Drop? drop = _drop_target.get_current_drop();
		if (drop == null)
			return false;

		Gdk.ContentFormats? formats = drop.get_formats();
		return formats != null && formats.contain_gtype(typeof(string));
	}
#endif

#if !CROWN_GTK3
	public string? drag_resource_path()
	{
		if (!has_drag_resource_path())
			return null;

		unowned GLib.Value? value = _drop_target.get_value();
		if (value == null)
			return null;

		if (value.type() != typeof(string))
			return null;

		return (string)value;
	}
#endif

#if !CROWN_GTK3
	public void update_drag_placeable(double x, double y)
	{
		string? resource_path = drag_resource_path();
		if (resource_path == null || !can_drop_resource_path(resource_path))
			return;

		place_drag_resource_path(resource_path, x, y);
	}
#endif

	public void update_drag_control(Gdk.ModifierType state)
	{
		bool control_pressed = (state & Gdk.ModifierType.CONTROL_MASK) != 0;
		if (_keys[Gdk.Key.Control_L] == control_pressed)
			return;

		if (control_pressed)
			on_key_pressed(Gdk.Key.Control_L, 0, state);
		else
			on_key_released(Gdk.Key.Control_L, 0, state);
	}

#if CROWN_GTK3
	public bool on_drag_motion(Gdk.DragContext context, int x, int y, uint time_)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_motion.html
		Gdk.Atom target = Gtk.drag_dest_find_target(this, context, null);
		if (target == Gdk.Atom.NONE) {
			Gdk.drag_status(context, 0, time_);
		} else {
			if (_drag_enter == false) {
				Gtk.drag_get_data(this, context, target, time_);
				_drag_enter = true;
			}

			Gdk.ModifierType state;
			context.get_device().get_state(this.get_window(), null, out state);
			update_drag_control(state);
			if (time_ - (uint)_drag_last_time >= 1000/MOTION_EVENTS_RATE_HZ) {
				// Drag motion events seem to fire at a very high frequency compared to regular
				// motion notify events. Limit them to MOTION_EVENTS_RATE_HZ.
				_drag_last_time = time_;
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
#endif /* if CROWN_GTK3 */

#if !CROWN_GTK3
	public bool on_drag_accept(Gdk.Drop drop)
	{
		return true;
	}
#endif

#if !CROWN_GTK3
	public Gdk.DragAction on_drag_enter(double x, double y)
	{
		_drag_enter = true;
		if (has_drag_resource_path())
			update_drag_placeable(x, y);
		_drag_last_time = 0;

		string? resource_path = drag_resource_path();
		return resource_path != null && can_drop_resource_path(resource_path) ? Gdk.DragAction.COPY : 0;
	}
#endif

#if !CROWN_GTK3
	public Gdk.DragAction on_drag_motion(double x, double y)
	{
		string? resource_path = drag_resource_path();
		if (resource_path == null || !can_drop_resource_path(resource_path))
			return 0;

		if (!_drag_enter) {
			_drag_enter = true;
			update_drag_placeable(x, y);
		}

		Gdk.Drop? drop = _drop_target.get_current_drop();
		if (drop != null)
			update_drag_control(drop.get_device().get_modifier_state());

		int64 time = GLib.get_monotonic_time();
		if (time - _drag_last_time >= 1000000 / MOTION_EVENTS_RATE_HZ) {
			_drag_last_time = time;
			int scale = this.get_scale_factor();
			_runtime.send_script(LevelEditorApi.set_mouse_state((int)x*scale
				, (int)y*scale
				, _mouse_left
				, _mouse_middle
				, _mouse_right
				));

			_runtime.send(DeviceApi.frame());
		}

		return Gdk.DragAction.COPY;
	}
#endif /* if !CROWN_GTK3 */

	public void finish_drag_placeable(double x, double y)
	{
		int scale = this.get_scale_factor();
		_runtime.send_script(LevelEditorApi.mouse_up((int)x*scale, (int)y*scale));
		GLib.Application.get_default().activate_action("cancel-place", null);
		_runtime.send(DeviceApi.frame());
	}

#if CROWN_GTK3
	public bool on_drag_drop(Gdk.DragContext context, int x, int y, uint time_)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_drop.html
		finish_drag_placeable(x, y);
		Gtk.drag_finish(context, true, false, time_);
		if (_keys[Gdk.Key.Control_L])
			on_key_released(Gdk.Key.Control_L, 0, 0);
		return true;
	}
#endif

#if !CROWN_GTK3
	public bool on_drag_drop(GLib.Value value, double x, double y)
	{
		string? resource_path = value.type() == typeof(string) ? (string)value : null;
		if (resource_path == null || !can_drop_resource_path(resource_path))
			return false;

		finish_drag_placeable(x, y);
		if (_keys[Gdk.Key.Control_L])
			on_key_released(Gdk.Key.Control_L, 0, 0);
		return true;
	}
#endif

#if CROWN_GTK3
	public void on_drag_leave(Gdk.DragContext context, uint time_)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_leave.html
		_drag_enter = false;
		if (_keys[Gdk.Key.Control_L])
			on_key_released(Gdk.Key.Control_L, 0, 0);
	}
#endif

#if !CROWN_GTK3
	public void on_drag_leave()
	{
		_drag_enter = false;
		GLib.Application.get_default().activate_action("cancel-place", null);
		if (_keys[Gdk.Key.Control_L])
			on_key_released(Gdk.Key.Control_L, 0, 0);
	}
#endif

	public bool on_button_event(uint button)
	{
		// Hack: GtkGestureSingle cancels the active button sequence when another mouse button
		// reaches it. Let the first handled button own the gesture until release, and swallow
		// competing button events while active.
		// See: https://gitlab.gnome.org/GNOME/gtk/-/work_items/7752

		if (button != Gdk.BUTTON_PRIMARY
			&& button != Gdk.BUTTON_MIDDLE
			&& button != Gdk.BUTTON_SECONDARY
			)
			return Gdk.EVENT_PROPAGATE;

		uint owner_button = 0;
		if (_mouse_left)
			owner_button = Gdk.BUTTON_PRIMARY;
		else if (_mouse_middle)
			owner_button = Gdk.BUTTON_MIDDLE;
		else if (_mouse_right)
			owner_button = Gdk.BUTTON_SECONDARY;

		return owner_button != 0 && button != owner_button
			? Gdk.EVENT_STOP
			: Gdk.EVENT_PROPAGATE
			;
	}

#if CROWN_GTK3
	public bool on_event_box_button_event(Gdk.EventButton ev)
	{
		return on_button_event(ev.button);
	}
#endif

#if !CROWN_GTK3
	public bool on_event_controller_legacy_event(Gdk.Event ev)
	{
		Gdk.EventType type = ev.get_event_type();
		if (type != Gdk.EventType.BUTTON_PRESS && type != Gdk.EventType.BUTTON_RELEASE)
			return Gdk.EVENT_PROPAGATE;

		return on_button_event(((Gdk.ButtonEvent)ev).get_button());
	}
#endif

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
		} else if (!_mouse_middle || !_mouse_right) {
			_buffer.append("LevelEditor:camera_drag_start('idle')");
		}

		if (!_mouse_right)
			_flythrough_drag.release();

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
		} else if (_mouse_right) {
			_buffer.append("LevelEditor:camera_drag_start('flythrough')");
			_flythrough_mouse_x = x;
			_flythrough_mouse_y = y;
			_flythrough_drag.start();

			if (_tick_callback_id == 0)
				_tick_callback_id = add_tick_callback(on_tick);

			if (_enable_accels_id > 0)
				GLib.Source.remove(_enable_accels_id);
			((LevelEditorApplication)GLib.Application.get_default()).set_conflicting_accels(false);
		}

		if (button == Gdk.BUTTON_PRIMARY)
			_buffer.append(LevelEditorApi.mouse_down((int)x*scale, (int)y*scale));

		if (_buffer.len != 0) {
			_runtime.send_script(_buffer.str);
			_buffer.erase();
			_runtime.send(DeviceApi.frame());
		}
	}

	public void on_gesture_cancelled(Gdk.EventSequence? sequence)
	{
		if (_resetting_flythrough_gesture)
			return;

		_flythrough_drag.cancel();
	}

	public void on_flythrough_release_detected_externally()
	{
		_resetting_flythrough_gesture = true;
		_gesture_click.reset();
		_resetting_flythrough_gesture = false;
	}

	public void on_flythrough_drag_finished(bool was_dragging)
	{
		if (_tick_callback_id == 0)
			return;

		_mouse_right = false;
		remove_tick_callback(_tick_callback_id);
		_tick_callback_id = 0;

		// Wait a little to prevent camera movement keys
		// from  activating unwanted accelerators.
		_enable_accels_id = GLib.Timeout.add_full(GLib.Priority.DEFAULT, 300, on_enable_accels);

		_buffer.append("LevelEditor:camera_drag_start('idle')");
		_runtime.send_script(_buffer.str);
		_buffer.erase();
		_runtime.send(DeviceApi.frame());
	}

	public bool on_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		uint key = Gdk.keyval_to_lower(keyval);
		bool stop_event = _tick_callback_id != 0
			&& (key == Gdk.Key.Up
			|| key == Gdk.Key.Down
			|| key == Gdk.Key.Right
			|| key == Gdk.Key.Left
			);

		if (keyval == Gdk.Key.Escape)
			GLib.Application.get_default().activate_action("cancel-place", null);

		if (_keys.contains(key)) {
			if (!_keys[key]) {
				_buffer.append(LevelEditorApi.key_down(key_to_string(key)));

				if (key == Gdk.Key.w || key == Gdk.Key.Up)
					_buffer.append("LevelEditor._camera.actions.forward = true;");
				if (key == Gdk.Key.s || key == Gdk.Key.Down)
					_buffer.append("LevelEditor._camera.actions.back = true;");
				if (key == Gdk.Key.a || key == Gdk.Key.Left)
					_buffer.append("LevelEditor._camera.actions.left = true;");
				if (key == Gdk.Key.d || key == Gdk.Key.Right)
					_buffer.append("LevelEditor._camera.actions.right = true;");
				if (key == Gdk.Key.q)
					_buffer.append("LevelEditor._camera.actions.up = true;");
				if (key == Gdk.Key.e)
					_buffer.append("LevelEditor._camera.actions.down = true;");
				if (key == Gdk.Key.Shift_L || key == Gdk.Key.Shift_R)
					_buffer.append("LevelEditor._camera.actions.fast = true;");
				if (key == Gdk.Key.Alt_L || key == Gdk.Key.Alt_R)
					_buffer.append("LevelEditor._camera.actions.slow = true;");
			}

			_keys[key] = true;
		}

		if (_buffer.len != 0) {
			_runtime.send_script(_buffer.str);
			_buffer.erase();
			_runtime.send(DeviceApi.frame());
		}
		return stop_event ? Gdk.EVENT_STOP : Gdk.EVENT_PROPAGATE;
	}

	public void on_key_released(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		uint key = Gdk.keyval_to_lower(keyval);

		if (_keys.contains(key)) {
			if (_keys[key]) {
				_buffer.append(LevelEditorApi.key_up(key_to_string(key)));

				if (key == Gdk.Key.w || key == Gdk.Key.Up)
					_buffer.append("LevelEditor._camera.actions.forward = false");
				if (key == Gdk.Key.s || key == Gdk.Key.Down)
					_buffer.append("LevelEditor._camera.actions.back = false");
				if (key == Gdk.Key.a || key == Gdk.Key.Left)
					_buffer.append("LevelEditor._camera.actions.left = false");
				if (key == Gdk.Key.d || key == Gdk.Key.Right)
					_buffer.append("LevelEditor._camera.actions.right = false");
				if (key == Gdk.Key.q)
					_buffer.append("LevelEditor._camera.actions.up = false");
				if (key == Gdk.Key.e)
					_buffer.append("LevelEditor._camera.actions.down = false");
				if (key == Gdk.Key.Shift_L || key == Gdk.Key.Shift_R)
					_buffer.append("LevelEditor._camera.actions.fast = false");
				if (key == Gdk.Key.Alt_L || key == Gdk.Key.Alt_R)
					_buffer.append("LevelEditor._camera.actions.slow = false");
			}

			_keys[key] = false;
		}

		if (_buffer.len != 0) {
			_runtime.send_script(_buffer.str);
			_buffer.erase();
			_runtime.send(DeviceApi.frame());
		}
	}

	public void on_motion(double x, double y)
	{
		if (_tick_callback_id != 0)
			return;

		int64 now = GLib.get_monotonic_time();

		if (now - _motion_last_time >= (1000*1000)/MOTION_EVENTS_RATE_HZ) {
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

	public void on_flythrough_drag_delta(double dx, double dy)
	{
		_flythrough_mouse_x += dx;
		_flythrough_mouse_y += dy;
	}

	public bool handle_scroll(double dx, double dy, Gdk.ModifierType state)
	{
		if (_tick_callback_id == 0
			&& (state & (Gdk.ModifierType.BUTTON1_MASK | Gdk.ModifierType.BUTTON2_MASK | Gdk.ModifierType.BUTTON3_MASK)) != 0
			)
			return false;

		if (_tick_callback_id != 0 || _keys[Gdk.Key.Shift_L] || _keys[Gdk.Key.Shift_R]) {
			_runtime.send_script(LevelEditorApi.mouse_wheel(-dy));
		} else {
			_runtime.send_script("LevelEditor:camera_drag_start_relative('dolly')");
			_runtime.send_script("LevelEditor._camera:update(1,0,%.17f,1,1)".printf(-dy * 32.0));
			_runtime.send_script("LevelEditor:camera_drag_start('idle')");
			_runtime.send(DeviceApi.frame());
		}
		return true;
	}

#if CROWN_GTK3
	public void on_scroll(double dx, double dy)
	{
		Gdk.ModifierType state = 0;
		if (!Gtk.get_current_event_state(out state))
			state = 0;
		handle_scroll(dx, dy, state);
	}
#endif

	public void on_flythrough_scroll(double dx, double dy)
	{
		on_scroll(dx, dy);
	}

#if !CROWN_GTK3
	public bool on_scroll(double dx, double dy)
	{
		return handle_scroll(dx, dy, _controller_scroll.get_current_event_state())
			? Gdk.EVENT_PROPAGATE
			: Gdk.EVENT_STOP
			;
	}
#endif

	public void handle_focus_leave()
	{
		camera_modifier_reset();

		_flythrough_drag.cancel();

		_keys[Gdk.Key.Control_L] = false;
		_keys[Gdk.Key.Shift_L] = false;
		_keys[Gdk.Key.Shift_R] = false;
		_runtime.send_script(LevelEditorApi.key_up(key_to_string(Gdk.Key.Control_L)));
		_runtime.send_script(LevelEditorApi.key_up(key_to_string(Gdk.Key.Shift_L)));
		_runtime.send_script(LevelEditorApi.key_up(key_to_string(Gdk.Key.Shift_R)));
	}

#if CROWN_GTK3
	public bool on_event_box_focus_out_event(Gdk.EventFocus ev)
	{
		handle_focus_leave();
		return Gdk.EVENT_PROPAGATE;
	}
#endif

	public void on_event_box_focus_leave()
	{
		handle_focus_leave();
	}

#if CROWN_GTK3
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
#endif /* if CROWN_GTK3 */

#if CROWN_GTK3
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
#endif

	public void on_enter(double x, double y)
	{
		this.grab_focus();
	}

	public bool on_tick(Gtk.Widget widget, Gdk.FrameClock frame_clock)
	{
		int scale = this.get_scale_factor();
		_buffer.append(LevelEditorApi.set_mouse_state((int)_flythrough_mouse_x*scale
			, (int)_flythrough_mouse_y*scale
			, _mouse_left
			, _mouse_middle
			, _mouse_right
			));
		_runtime.send_script(_buffer.str);
		_buffer.erase();

		_runtime.send(DeviceApi.frame());
		return GLib.Source.CONTINUE;
	}

	public bool on_enable_accels()
	{
		((LevelEditorApplication)GLib.Application.get_default()).set_conflicting_accels(true);
		_enable_accels_id = 0;
		return GLib.Source.REMOVE;
	}
}

} /* namespace Crown */
