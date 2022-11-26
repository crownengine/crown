/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
 * License: https://github.com/crownengine/crown/blob/master/LICENSE
 */

using Gdk;
using Gee;
using Gtk;

#if CROWN_PLATFORM_LINUX
extern uint gdk_x11_window_get_xid(Gdk.Window window);
#elif CROWN_PLATFORM_WINDOWS
extern uint gdk_win32_window_get_handle(Gdk.Window window);
#endif

namespace Crown
{
public class EditorView : Gtk.EventBox
{
	// Data
	private ConsoleClient _client;

	private Gtk.Allocation _allocation;
	private uint _resize_timer_id;

	private int _mouse_curr_x;
	private int _mouse_curr_y;

	private bool _mouse_left;
	private bool _mouse_middle;
	private bool _mouse_right;

	private uint _window_id;

	public uint window_id
	{
		get { return (uint)_window_id; }
	}

	private HashMap<uint, bool> _keys;

	// Signals
	public signal void realized();

	private string key_to_string(uint k)
	{
		switch (k) {
		case Gdk.Key.w:     return "w";
		case Gdk.Key.a:     return "a";
		case Gdk.Key.s:     return "s";
		case Gdk.Key.d:     return "d";
		case Gdk.Key.Alt_L: return "alt_left";
		case Gdk.Key.Alt_R: return "alt_right";
		default:            return "<unknown>";
		}
	}

	private bool camera_modifier_pressed()
	{
		return _keys[Gdk.Key.Alt_L]
			|| _keys[Gdk.Key.Alt_R]
			;
	}

	private void camera_modifier_reset()
	{
		_keys[Gdk.Key.Alt_L] = false;
		_keys[Gdk.Key.Alt_R] = false;
	}

	public EditorView(ConsoleClient client, bool input_enabled = true)
	{
		_client = client;

		_allocation = { 0, 0, 0, 0 };
		_resize_timer_id = 0;

		_mouse_curr_x = 0;
		_mouse_curr_y = 0;

		_mouse_left   = false;
		_mouse_middle = false;
		_mouse_right  = false;

		_window_id = 0;

		_keys = new HashMap<uint, bool>();
		_keys[Gdk.Key.w] = false;
		_keys[Gdk.Key.a] = false;
		_keys[Gdk.Key.s] = false;
		_keys[Gdk.Key.d] = false;
		_keys[Gdk.Key.Alt_L] = false;
		_keys[Gdk.Key.Alt_R] = false;

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
			this.button_release_event.connect(on_button_release);
			this.button_press_event.connect(on_button_press);
			this.key_press_event.connect(on_key_press);
			this.key_release_event.connect(on_key_release);
			this.motion_notify_event.connect(on_motion_notify);
			this.scroll_event.connect(on_scroll);
		}

		this.realize.connect(on_event_box_realized);
		this.set_visual(Gdk.Screen.get_default().get_system_visual());
		this.set_size_request(128, 128);
		this.events |= Gdk.EventMask.STRUCTURE_MASK; // map_event
		this.map_event.connect(() => {
				device_frame_delayed(16, _client);
				return Gdk.EVENT_PROPAGATE;
			});
	}

	private bool on_button_release(Gdk.EventButton ev)
	{
		_mouse_left   = ev.button == Gdk.BUTTON_PRIMARY   ? false : _mouse_left;
		_mouse_middle = ev.button == Gdk.BUTTON_MIDDLE    ? false : _mouse_middle;
		_mouse_right  = ev.button == Gdk.BUTTON_SECONDARY ? false : _mouse_right;

		string str = LevelEditorApi.set_mouse_state(_mouse_curr_x
			, _mouse_curr_y
			, _mouse_left
			, _mouse_middle
			, _mouse_right
			);

		if (ev.button == Gdk.BUTTON_PRIMARY)
			str += LevelEditorApi.mouse_up((int)ev.x, (int)ev.y);

		if (camera_modifier_pressed()) {
			if (!_mouse_left || !_mouse_middle || !_mouse_right)
				str += "LevelEditor:camera_drag_start('idle')";
		}

		if (str.length != 0) {
			_client.send_script(str);
			_client.send(DeviceApi.frame());
		}
		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_button_press(Gdk.EventButton ev)
	{
		// Grab keyboard focus
		this.grab_focus();

		_mouse_left   = ev.button == Gdk.BUTTON_PRIMARY   ? true : _mouse_left;
		_mouse_middle = ev.button == Gdk.BUTTON_MIDDLE    ? true : _mouse_middle;
		_mouse_right  = ev.button == Gdk.BUTTON_SECONDARY ? true : _mouse_right;

		string str = LevelEditorApi.set_mouse_state(_mouse_curr_x
			, _mouse_curr_y
			, _mouse_left
			, _mouse_middle
			, _mouse_right
			);

		if (camera_modifier_pressed()) {
			if (_mouse_left)
				str += "LevelEditor:camera_drag_start('tumble')";
			if (_mouse_middle)
				str += "LevelEditor:camera_drag_start('track')";
			if (_mouse_right)
				str += "LevelEditor:camera_drag_start('dolly')";
		}

		if (ev.button == Gdk.BUTTON_PRIMARY)
			str += LevelEditorApi.mouse_down((int)ev.x, (int)ev.y);

		if (str.length != 0) {
			_client.send_script(str);
			_client.send(DeviceApi.frame());
		}
		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_key_press(Gdk.EventKey ev)
	{
		string str = "";

		if (ev.keyval == Gdk.Key.Escape) {
			LevelEditorApplication app = (LevelEditorApplication)((Gtk.Window)this.get_toplevel()).application;
			app.activate_last_tool_before_place();
		}

		if (ev.keyval == Gdk.Key.Up)
			str += "LevelEditor:key_down(\"move_up\")";
		if (ev.keyval == Gdk.Key.Down)
			str += "LevelEditor:key_down(\"move_down\")";
		if (ev.keyval == Gdk.Key.Right)
			str += "LevelEditor:key_down(\"move_right\")";
		if (ev.keyval == Gdk.Key.Left)
			str += "LevelEditor:key_down(\"move_left\")";

		if (_keys.has_key(ev.keyval)) {
			if (!_keys[ev.keyval])
				str += LevelEditorApi.key_down(key_to_string(ev.keyval));

			_keys[ev.keyval] = true;
		}

		if (str.length != 0) {
			_client.send_script(str);
			_client.send(DeviceApi.frame());
		}
		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_key_release(Gdk.EventKey ev)
	{
		string str = "";

		if ((ev.keyval == Gdk.Key.Alt_L || ev.keyval == Gdk.Key.Alt_R))
			str += "LevelEditor:camera_drag_start('idle')";

		if (_keys.has_key(ev.keyval)) {
			if (_keys[ev.keyval])
				str += LevelEditorApi.key_up(key_to_string(ev.keyval));

			_keys[ev.keyval] = false;
		}

		if (str.length != 0) {
			_client.send_script(str);
			_client.send(DeviceApi.frame());
		}
		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_motion_notify(Gdk.EventMotion ev)
	{
		_mouse_curr_x = (int)ev.x;
		_mouse_curr_y = (int)ev.y;

		_client.send_script(LevelEditorApi.set_mouse_state(_mouse_curr_x
			, _mouse_curr_y
			, _mouse_left
			, _mouse_middle
			, _mouse_right
			));

		_client.send(DeviceApi.frame());
		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_scroll(Gdk.EventScroll ev)
	{
		_client.send_script(LevelEditorApi.mouse_wheel(ev.direction == Gdk.ScrollDirection.UP ? 1.0 : -1.0));
		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_event_box_focus_out_event(Gdk.EventFocus ev)
	{
		camera_modifier_reset();
		return Gdk.EVENT_PROPAGATE;
	}

	private void on_size_allocate(Gtk.Allocation ev)
	{
		if (_allocation.x == ev.x
			&& _allocation.y == ev.y
			&& _allocation.width == ev.width
			&& _allocation.height == ev.height
			)
			return;

		_allocation = ev;
		_client.send(DeviceApi.resize(_allocation.width, _allocation.height));

		// Ensure there is some delay between the last resize() and the last frame().
		if (_resize_timer_id == 0) {
			_resize_timer_id = GLib.Timeout.add_full(GLib.Priority.DEFAULT, 200, () => {
					_client.send(DeviceApi.frame());
					_resize_timer_id = 0;
					return GLib.Source.REMOVE;
				});
		}
	}

	private void on_event_box_realized()
	{
		this.get_window().ensure_native();
#if CROWN_PLATFORM_LINUX
		this.get_display().sync();
		_window_id = gdk_x11_window_get_xid(this.get_window());
#elif CROWN_PLATFORM_WINDOWS
		_window_id = gdk_win32_window_get_handle(this.get_window());
#endif
		realized();
	}
}

} /* namespace Crown */
