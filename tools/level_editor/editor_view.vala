/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gdk;
using Gee;
using Gtk;

#if CROWN_PLATFORM_WINDOWS
extern uint gdk_win32_window_get_handle (Gdk.Window window);
#endif

namespace Crown
{
public class EditorView : Gtk.Alignment
{
	// Data
	private ConsoleClient _client;

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

	// Widgets
#if CROWN_PLATFORM_LINUX
	private Gtk.Socket _socket;
#endif
	public Gtk.EventBox _event_box;

	// Signals
	public signal void realized();

	private string key_to_string(uint k)
	{
		switch (k)
		{
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
		this.xalign = 0;
		this.yalign = 0;
		this.xscale = 1;
		this.yscale = 1;

		_client = client;

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
#if CROWN_PLATFORM_LINUX
		_socket = new Gtk.Socket();
		_socket.set_visual(Gdk.Screen.get_default().get_system_visual());
		_socket.realize.connect(on_socket_realized);
		_socket.plug_removed.connect(on_socket_plug_removed);
		_socket.set_size_request(128, 128);
#endif
		_event_box = new Gtk.EventBox();
		_event_box.can_focus = true;
		_event_box.events |= Gdk.EventMask.POINTER_MOTION_MASK
			| Gdk.EventMask.KEY_PRESS_MASK
			| Gdk.EventMask.KEY_RELEASE_MASK
			| Gdk.EventMask.FOCUS_CHANGE_MASK
			| Gdk.EventMask.SCROLL_MASK
			;
		_event_box.focus_out_event.connect(on_event_box_focus_out_event);
#if CROWN_PLATFORM_WINDOWS
		_event_box.realize.connect(on_event_box_realized);
		_event_box.size_allocate.connect(on_size_allocate);
#endif

		if (input_enabled)
		{
			_event_box.button_release_event.connect(on_button_release);
			_event_box.button_press_event.connect(on_button_press);
			_event_box.key_press_event.connect(on_key_press);
			_event_box.key_release_event.connect(on_key_release);
			_event_box.motion_notify_event.connect(on_motion_notify);
			_event_box.scroll_event.connect(on_scroll);
		}

#if CROWN_PLATFORM_LINUX
		_event_box.add(_socket);
#endif
		add(_event_box);
	}

	private bool on_button_release(Gdk.EventButton ev)
	{
		_mouse_left   = ev.button == Gdk.BUTTON_PRIMARY   ? false : _mouse_left;
		_mouse_middle = ev.button == Gdk.BUTTON_MIDDLE    ? false : _mouse_middle;
		_mouse_right  = ev.button == Gdk.BUTTON_SECONDARY ? false : _mouse_right;

		string s = LevelEditorApi.set_mouse_state(_mouse_curr_x
			, _mouse_curr_y
			, _mouse_left
			, _mouse_middle
			, _mouse_right
			);

		if (camera_modifier_pressed())
		{
			if (!_mouse_left || !_mouse_middle || !_mouse_right)
				s += "LevelEditor:camera_drag_start('idle')";
		}
		else
		{
			if (ev.button == Gdk.BUTTON_PRIMARY)
				s += LevelEditorApi.mouse_up((int)ev.x, (int)ev.y);
		}

		_client.send_script(s);
		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_button_press(Gdk.EventButton ev)
	{
		// Grab keyboard focus
		_event_box.grab_focus();

		_mouse_left   = ev.button == Gdk.BUTTON_PRIMARY   ? true : _mouse_left;
		_mouse_middle = ev.button == Gdk.BUTTON_MIDDLE    ? true : _mouse_middle;
		_mouse_right  = ev.button == Gdk.BUTTON_SECONDARY ? true : _mouse_right;

		string s = LevelEditorApi.set_mouse_state(_mouse_curr_x
			, _mouse_curr_y
			, _mouse_left
			, _mouse_middle
			, _mouse_right
			);

		if (camera_modifier_pressed())
		{
			if (_mouse_left)
				s += "LevelEditor:camera_drag_start('tumble')";
			if (_mouse_middle)
				s += "LevelEditor:camera_drag_start('track')";
			if (_mouse_right)
				s += "LevelEditor:camera_drag_start('dolly')";
		}
		else
		{
			if (ev.button == Gdk.BUTTON_PRIMARY)
				s += LevelEditorApi.mouse_down((int)ev.x, (int)ev.y);
		}

		_client.send_script(s);
		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_key_press(Gdk.EventKey ev)
	{
		if (ev.keyval == Gdk.Key.Up)
			_client.send_script("LevelEditor:key_down(\"move_up\")");
		if (ev.keyval == Gdk.Key.Down)
			_client.send_script("LevelEditor:key_down(\"move_down\")");
		if (ev.keyval == Gdk.Key.Right)
			_client.send_script("LevelEditor:key_down(\"move_right\")");
		if (ev.keyval == Gdk.Key.Left)
			_client.send_script("LevelEditor:key_down(\"move_left\")");

		if (_keys.has_key(ev.keyval))
		{
			if (!_keys[ev.keyval])
				_client.send_script(LevelEditorApi.key_down(key_to_string(ev.keyval)));

			_keys[ev.keyval] = true;
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_key_release(Gdk.EventKey ev)
	{
		if ((ev.keyval == Gdk.Key.Alt_L || ev.keyval == Gdk.Key.Alt_R))
			_client.send_script("LevelEditor:camera_drag_start('idle')");

		if (_keys.has_key(ev.keyval))
		{
			if (_keys[ev.keyval])
				_client.send_script(LevelEditorApi.key_up(key_to_string(ev.keyval)));

			_keys[ev.keyval] = false;
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

#if CROWN_PLATFORM_LINUX
	private void on_socket_realized()
	{
		// We do not have window XID until socket is realized...
		_window_id = (uint)_socket.get_id();
		realized();
	}

	private bool on_socket_plug_removed()
	{
		// Prevent the default handler from destroying the Socket.
		return Gdk.EVENT_STOP;
	}

#elif CROWN_PLATFORM_WINDOWS
	private void on_event_box_realized()
	{
		_event_box.get_window().ensure_native();
		_window_id = gdk_win32_window_get_handle(_event_box.get_window());
		realized();
	}

	private void on_size_allocate(Gtk.Allocation ev)
	{
		_client.send(DeviceApi.resize(ev.width, ev.height));
	}
#endif
}

}
