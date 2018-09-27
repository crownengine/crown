/*
 * Copyright(c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE-GPLv2
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
		private Project _project;

		// Widgets
		private Gtk.ScrolledWindow _scrolled_window;
		private Gtk.TextView _text_view;
		private Gtk.Entry _entry;

		public ConsoleView(ConsoleClient client, Project project)
		{
			Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

			// Data
			_entry_history = new EntryHistory(256);
			_console_client = client;
			_project = project;

			// Widgets
			_text_view = new Gtk.TextView();
			_text_view.editable = false;
			_text_view.can_focus = false;

			// // Create tags for color-formatted text
			Gtk.TextTag tag_info = new Gtk.TextTag("info");
			tag_info.foreground_rgba = { 0.7, 0.7, 0.7, 1.0 };
			Gtk.TextTag tag_warning = new Gtk.TextTag("warning");
			tag_warning.foreground_rgba = { 1.0, 1.0, 0.4, 1.0 };
			Gtk.TextTag tag_error = new Gtk.TextTag("error");
			tag_error.foreground_rgba = { 1.0, 0.4, 0.4, 1.0 };

			Gtk.TextBuffer tb = _text_view.buffer;
			tb.tag_table.add(tag_info);
			tb.tag_table.add(tag_warning);
			tb.tag_table.add(tag_error);

			_scrolled_window = new Gtk.ScrolledWindow(null, null);
			_scrolled_window.add(_text_view);

			_entry = new Gtk.Entry();
			_entry.key_press_event.connect(on_entry_key_pressed);
			_entry.activate.connect(on_entry_activated);

			this.pack_start(_scrolled_window, true, true, 0);
			this.pack_start(_entry, false, true, 0);

			this.get_style_context().add_class("console-view");
			this.show_all();
		}

		private void on_entry_activated()
		{
			string text = _entry.text;
			text = text.strip();

			if (text.length > 0)
			{
				_entry_history.push(text);

				if (text[0] == ':')
				{
					string[] args = text[1:text.length].split(" ");
					_console_client.send(DeviceApi.command(args));
				}
				else
				{
					_console_client.send_script(text);
				}
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

			_entry.set_position(_entry.text.length);

			return true;
		}

		public void logi(string system, string text)
		{
			log(system, text, "info");
		}

		public void logw(string system, string text)
		{
			log(system, text, "warning");
		}

		public void loge(string system, string text)
		{
			log(system, text, "error");
		}

		public void log(string system, string text, string severity)
		{
			string msg = text;

			// Replace IDs with human-readable names
			int id_index = text.index_of("#ID(");
			if (id_index != -1)
			{
				string id = text.substring(id_index + 4, 16);
				string name = _project.id_to_name(id);
				msg = text.replace("#ID(%s)".printf(id), "'%s'".printf(name));
			}

			string line = system + ": " + msg + "\n";
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
