/*
 * Copyright(c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;

namespace Crown
{
	public class EntryHistory
	{
		private uint _max_records;
		private uint _size;
		private uint _current;
		private string[] _history;

		// Creates a new hisory with room for max_records records.
		public EntryHistory(uint max_records)
		{
			_max_records = max_records;
			_size = 0;
			_current = 0;
			_history = new string[max_records];
		}

		// Push a new string into the history.
		public void push(string text)
		{
			// Add command to history
			_history[_size] = text;
			_size = (uint)Math.fmin((double)_size + 1, (double)_max_records - 1);
			_current = _size;
		}

		// Returns the previous entry in the history.
		public string previous()
		{
			_current = _current > 0 ? _current -1 : 0;
			return _history[_current];
		}

		// Returns the next entry in the history.
		public string next()
		{
			_current = (uint)Math.fmin((double)_current + 1, (double)_size - 1);
			return _history[_current];
		}
	}

	public class ConsoleView : Gtk.Box
	{
		// Data
		private EntryHistory _entry_history;
		private ConsoleClient _console_client;

		// Widgets
		private Gtk.ScrolledWindow _scrolled_window;
		private Gtk.TextView _text_view;
		private Gtk.Entry _entry;

		public ConsoleView(ConsoleClient client)
		{
			Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

			// Data
			_entry_history = new EntryHistory(256);
			_console_client = client;

			// Widgets
			Pango.FontDescription fd = new Pango.FontDescription();
			fd.set_family("Monospace");

			_text_view = new Gtk.TextView();
			_text_view.editable = false;
			_text_view.can_focus = false;
			_text_view.override_font(fd);

			// // Create tags for color-formatted text
			Gtk.TextTag tag_info = new Gtk.TextTag("info");
			tag_info.foreground_rgba = { 1.0, 1.0, 1.0, 1.0 };
			Gtk.TextTag tag_warning = new Gtk.TextTag("warning");
			tag_warning.foreground_rgba = { 1.0, 1.0, 0.4, 1.0 };
			Gtk.TextTag tag_error = new Gtk.TextTag("error");
			tag_error.foreground_rgba = { 1.0, 0.4, 0.4, 1.0 };
			Gtk.TextTag tag_debug = new Gtk.TextTag("debug");
			tag_debug.foreground_rgba = { 0.8, 0.8, 0.8, 1.0 };

			Gtk.TextBuffer tb = _text_view.buffer;
			tb.tag_table.add(tag_info);
			tb.tag_table.add(tag_warning);
			tb.tag_table.add(tag_error);
			tb.tag_table.add(tag_debug);

			_scrolled_window = new Gtk.ScrolledWindow(null, null);
			_scrolled_window.add(_text_view);

			_entry = new Gtk.Entry();
			_entry.key_press_event.connect(on_entry_key_pressed);
			_entry.activate.connect(on_entry_activated);

			pack_start(_scrolled_window, true, true, 0);
			pack_start(_entry, false, true, 0);

			set_size_request(700, 150);
			show_all();
		}

		private void do_command(string cmd)
		{
			string[] words = cmd.split(" ");

			if (words[0] == "reload")
			{
				if (words.length == 3)
					_console_client.send(EngineAPI.reload(words[1], words[2]));
				else
					log("Hint reload <type> <name>", "error");
			}
			else
			{
				log("Unknown command: '%s'".printf(words[0]), "error");
			}
		}

		private void do_stuff(string text)
		{
			if (text[0] == ':')
				do_command(text[1:text.length]);
			else
				_console_client.send_script(text);
		}

		private void on_entry_activated()
		{
			string text = _entry.text;
			text = text.strip();

			if (text.length > 0)
			{
				_entry_history.push(text);
				log("> " + text, "info");

				do_stuff(text);
			}

			_entry.text = "";
		}

		private bool on_entry_key_pressed(Gdk.EventKey ev)
		{
			if (ev.keyval == Gdk.Key.Down)
				_entry.text = _entry_history.next();
			else if (ev.keyval == Gdk.Key.Up)
				_entry.text = _entry_history.previous();
			else
				return false;

			return true;
		}

		public void log(string text, string severity)
		{
			string line = text + "\n";
			Gtk.TextBuffer buffer = _text_view.buffer;
			Gtk.TextIter end_iter;
			buffer.get_end_iter(out end_iter);
			buffer.insert(ref end_iter, line, line.length);
			end_iter.backward_chars(line.length);
			Gtk.TextIter start_iter = end_iter;
			buffer.get_end_iter(out end_iter);
			buffer.apply_tag(buffer.tag_table.lookup(severity), start_iter, end_iter);
			_text_view.scroll_to_mark(buffer.create_mark("bottom", end_iter, false), 0, true, 0.0, 1.0);
		}
	}
}
