/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gdk;
using Gee;
using Gtk;

namespace Crown
{
	public class EngineView : Gtk.Alignment
	{
		// Data
		private ConsoleClient _client;

		private int _mouse_curr_x;
		private int _mouse_curr_y;

		private bool _mouse_left;
		private bool _mouse_middle;
		private bool _mouse_right;

		private int _even;
		private X.Window _window_id;

		public uint window_id
		{
			get { return (uint)_window_id; }
		}

		private HashMap<int, bool> _keys;

		// Widgets
		private Gtk.Socket _socket;
		public Gtk.EventBox _event_box;

		// Signals
		public signal void realized();

		private string key_to_string(int k)
		{
			switch ((uint)k)
			{
			case Gdk.Key.w: return "w";
			case Gdk.Key.a: return "a";
			case Gdk.Key.s: return "s";
			case Gdk.Key.d: return "d";
			default:        return "<unknown>";
			}
		}

		public EngineView(ConsoleClient client, bool input_enabled = true)
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

			_even = 0;

			_window_id = 0;

			_keys = new HashMap<int, bool>();
			_keys[(int)Gdk.Key.w] = false;
			_keys[(int)Gdk.Key.a] = false;
			_keys[(int)Gdk.Key.s] = false;
			_keys[(int)Gdk.Key.d] = false;

			// Widgets
			_socket = new Gtk.Socket();
			_socket.set_visual(Gdk.Visual.get_best_with_type(VisualType.TRUE_COLOR));
			_socket.realize.connect(on_socket_realized);
			_socket.plug_removed.connect(on_socket_plug_removed);
			_socket.set_size_request(300, 300);

			_event_box = new Gtk.EventBox();
			_event_box.can_focus = true;
			_event_box.events |= Gdk.EventMask.POINTER_MOTION_MASK
				| Gdk.EventMask.KEY_PRESS_MASK
				| Gdk.EventMask.KEY_RELEASE_MASK
				| Gdk.EventMask.FOCUS_CHANGE_MASK
				| Gdk.EventMask.SCROLL_MASK
				;

			if (input_enabled)
			{
				_event_box.button_release_event.connect(on_button_release);
				_event_box.button_press_event.connect(on_button_press);
				_event_box.key_press_event.connect(on_key_press);
				_event_box.key_release_event.connect(on_key_release);
				_event_box.motion_notify_event.connect(on_motion_notify);
				_event_box.scroll_event.connect(on_scroll);
			}

			_event_box.add(_socket);

			add(_event_box);
			show_all();
		}

		private bool on_button_release(Gdk.EventButton ev)
		{
			_mouse_left   = ev.button == 1 ? false : _mouse_left;
			_mouse_middle = ev.button == 2 ? false : _mouse_middle;
			_mouse_right  = ev.button == 3 ? false : _mouse_right;

			string s = LevelEditorApi.set_mouse_state(_mouse_curr_x
				, _mouse_curr_y
				, _mouse_left
				, _mouse_middle
				, _mouse_right
				);

			if (ev.button == 1)
				s += LevelEditorApi.mouse_up((int)ev.x, (int)ev.y);

			_client.send_script(s);
			return false;
		}

		private bool on_button_press(Gdk.EventButton ev)
		{
			// Grab keyboard focus
			_event_box.grab_focus();

			_mouse_left   = ev.button == 1 ? true : _mouse_left;
			_mouse_middle = ev.button == 2 ? true : _mouse_middle;
			_mouse_right  = ev.button == 3 ? true : _mouse_right;

			string s = LevelEditorApi.set_mouse_state(_mouse_curr_x
				, _mouse_curr_y
				, _mouse_left
				, _mouse_middle
				, _mouse_right
				);

			if (ev.button == 1)
				s += LevelEditorApi.mouse_down((int)ev.x, (int)ev.y);

			_client.send_script(s);
			return false;
		}

		private bool on_key_press(Gdk.EventKey ev)
		{
			if ((int)ev.keyval == Gdk.Key.Up)
				_client.send_script("LevelEditor:key_down(\"move_up\")");
			if ((int)ev.keyval == Gdk.Key.Down)
				_client.send_script("LevelEditor:key_down(\"move_down\")");
			if ((int)ev.keyval == Gdk.Key.Right)
				_client.send_script("LevelEditor:key_down(\"move_right\")");
			if ((int)ev.keyval == Gdk.Key.Left)
				_client.send_script("LevelEditor:key_down(\"move_left\")");

			if (!_keys.has_key((int)ev.keyval))
				return true;

			if (!_keys[(int)ev.keyval])
				_client.send_script(LevelEditorApi.key_down(key_to_string((int)ev.keyval)));

			_keys[(int)ev.keyval] = true;

			return false;
		}

		private bool on_key_release(Gdk.EventKey ev)
		{
			if (!_keys.has_key((int)ev.keyval))
				return false;

			if (_keys[(int)ev.keyval])
				_client.send_script(LevelEditorApi.key_up(key_to_string((int)ev.keyval)));

			_keys[(int)ev.keyval] = false;

			return false;
		}

		private bool on_motion_notify(Gdk.EventMotion ev)
		{
			_mouse_curr_x = (int)ev.x;
			_mouse_curr_y = (int)ev.y;

			_client.send_script(LevelEditorApi.mouse_move(_mouse_curr_x, _mouse_curr_y));

			return false;
		}

		private bool on_scroll(Gdk.EventScroll ev)
		{
			_client.send_script(LevelEditorApi.mouse_wheel(ev.direction == Gdk.ScrollDirection.UP ? 1.0 : -1.0));
			return false;
		}

		private void on_socket_realized()
		{
			// We do not have window XID until socket is realized...
			_window_id = _socket.get_id();

			realized();
		}

		private bool on_socket_plug_removed()
		{
			// Prevent the default handler from destroying the Socket.
			return true;
		}
	}
}
