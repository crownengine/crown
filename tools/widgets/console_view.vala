/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class EntryHistory
{
	public uint _capacity;
	public uint _size;
	public uint _index;
	public string[] _data;

	// Creates a new history with room for capacity records.
	public EntryHistory(uint capacity)
	{
		_capacity = capacity;
		_size = 0;
		_index = 0;
		_data = new string[capacity];
	}

	// Push a new string into the history.
	public void push(string text)
	{
		// Add command to history
		_data[_index] = text;
		_index = (_index + 1) % _capacity;

		if (_size < _capacity)
			++_size;
	}

	public void clear()
	{
		_size = 0;
		_index = 0;
	}

	// Returns the element at @a distance slots from the current index.
	// Distance must be in the [1; _size] range.
	public string element(uint distance)
	{
		if (distance < 1 || distance > _size)
			return "ERROR";

		if (_index >= distance)
			return _data[_index - distance];
		else
			return _data[_capacity - (distance - _index)];
	}

	public void save(string path)
	{
		FileStream fs = FileStream.open(path, "wb");
		if (fs == null)
			return;

		uint first_entry = _index + (_capacity - _size);
		for (uint ii = 0; ii < _size; ++ii)
			fs.printf("%s\n", _data[(first_entry + ii) % _capacity]);
	}

	public void load(string path)
	{
		FileStream fs = FileStream.open(path, "rb");
		if (fs == null)
			return;

		string? line = null;
		while ((line = fs.read_line()) != null)
			push(line);
	}
}

public class ConsoleView : Gtk.Box
{
	// Data
	public EntryHistory _entry_history;
	public uint _distance;
	public Project _project;
	public PreferencesDialog _preferences_dialog;

	// Widgets
	public Gtk.ScrolledWindow _scrolled_window;
	public Gtk.TextView _text_view;
	public EntryText _entry;
	public Gtk.Box _entry_hbox;

	public ConsoleView(Project project, Gtk.ComboBoxText combo, PreferencesDialog preferences_dialog)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		// Data
		_entry_history = new EntryHistory(256);
		_distance = 0;
		_project = project;
		_preferences_dialog = preferences_dialog;

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

		_entry = new EntryText();
		_entry.key_press_event.connect(on_entry_key_pressed);
		_entry.activate.connect(on_entry_activated);

		_entry_hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_entry_hbox.pack_start(_entry, true, true);
		_entry_hbox.pack_end(combo, false, false);

		this.pack_start(_scrolled_window, true, true, 0);
		this.pack_start(_entry_hbox, false, true, 0);

		this.show.connect(on_show);
		this.destroy.connect(on_destroy);

		this.get_style_context().add_class("console-view");

		_console_view_valid = true;
	}

	private void on_entry_activated()
	{
		string text = _entry.text;
		text = text.strip();

		if (text.length > 0)
		{
			_entry_history.push(text);
			_distance = 0;

			Gtk.Application app = ((Gtk.Window)this.get_toplevel()).application;
			ConsoleClient? client = ((LevelEditorApplication)app).current_selected_client();

			if (text[0] == ':')
			{
				string[] args = text[1:text.length].split(" ");
				if (args.length > 0)
				{
					if (client != null)
						client.send(DeviceApi.command(args));
				}
			}
			else
			{
				if (client != null)
					client.send_script(text);
			}
		}

		_entry.text = "";
	}

	private bool on_entry_key_pressed(Gdk.EventKey ev)
	{
		if (ev.keyval == Gdk.Key.Down)
		{
			if (_distance > 1)
			{
				--_distance;
				_entry.text = _entry_history.element(_distance);
			}
			else
			{
				_entry.text = "";
			}

			_entry.set_position(_entry.text.length);
			return Gdk.EVENT_STOP;
		}
		else if (ev.keyval == Gdk.Key.Up)
		{
			if (_distance < _entry_history._size)
			{
				++_distance;
				_entry.text = _entry_history.element(_distance);
			}

			_entry.set_position(_entry.text.length);
			return Gdk.EVENT_STOP;
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private void on_show()
	{
		_entry.grab_focus_without_selecting();
	}

	private void on_destroy()
	{
		_console_view_valid = false;
	}

	public void log(string severity, string message)
	{
		string line = message;

		// Replace IDs with human-readable names.
		int id_index = message.index_of("#ID(");
		if (id_index != -1)
		{
			string id = message.substring(id_index + 4, 16);
			string name = _project.id_to_name(id);
			line = message.replace("#ID(%s)".printf(id), "'%s'".printf(name));
		}

		Gtk.TextBuffer buffer = _text_view.buffer;

		// Limit number of lines recorded.
		int max_lines = (int)_preferences_dialog._console_max_lines.value;
		if (buffer.get_line_count()-1 >= max_lines)
		{
			Gtk.TextIter start_of_first_line;
			buffer.get_iter_at_line(out start_of_first_line, 0);
			Gtk.TextIter end_of_first_line = start_of_first_line;
			start_of_first_line.forward_line();
			buffer.delete(ref start_of_first_line, ref end_of_first_line);
		}

		// Append line.
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
