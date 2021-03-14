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
	public Gdk.Cursor _text_cursor;
	public Gdk.Cursor _pointer_cursor;
	public bool _cursor_is_hovering_link;
	public Gtk.TextView _text_view;
	public Gtk.ScrolledWindow _scrolled_window;
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
		_text_cursor = new Gdk.Cursor.from_name(this.get_display(), "text");
		_pointer_cursor = new Gdk.Cursor.from_name(this.get_display(), "pointer");
		_cursor_is_hovering_link = false;

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

		Gtk.TextIter end_iter;
		tb.get_end_iter(out end_iter);
		tb.create_mark("scroll", end_iter, true);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.vscrollbar_policy = Gtk.PolicyType.ALWAYS;
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
		this.button_release_event.connect(on_button_released);
		this.motion_notify_event.connect(on_motion_notify);

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

	private bool on_button_released(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_PRIMARY)
		{
			// Do not handle click if some text is selected.
			Gtk.TextIter dummy_iter;
			if (_text_view.buffer.get_selection_bounds(out dummy_iter, out dummy_iter))
				return Gdk.EVENT_PROPAGATE;

			int buffer_x;
			int buffer_y;
			_text_view.window_to_buffer_coords(Gtk.TextWindowType.WIDGET
				, (int)ev.x
				, (int)ev.y
				, out buffer_x
				, out buffer_y
				);

			Gtk.TextIter iter;
			if (_text_view.get_iter_at_location(out iter, buffer_x, buffer_y))
			{
				// Check whether the text under the mouse pointer has a link tag.
				GLib.SList<unowned TextTag> tags = iter.get_tags();
				foreach (var item in tags)
				{
					string resource_name = item.get_data<string>("resource_name");
					if (resource_name != null)
					{
						Gtk.Application app = ((Gtk.Window)this.get_toplevel()).application;
						app.activate_action("open-resource", new GLib.Variant.string(resource_name));
					}
				}
			}
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_motion_notify(Gdk.EventMotion ev)
	{
		bool hovering = false;

		int buffer_x;
		int buffer_y;
		_text_view.window_to_buffer_coords(TextWindowType.WIDGET
			, (int)ev.x
			, (int)ev.y
			, out buffer_x
			, out buffer_y
			);

		Gtk.TextIter iter;
		if (_text_view.get_iter_at_location(out iter, buffer_x, buffer_y))
		{
			// Check whether the text under the mouse pointer has a link tag.
			GLib.SList<unowned TextTag> tags = iter.get_tags();
			foreach (var item in tags)
			{
				string resource_name = item.get_data<string>("resource_name");
				if (resource_name != null)
				{
					hovering = true;
				}
			}
		}

		if (_cursor_is_hovering_link != hovering)
		{
			_cursor_is_hovering_link = hovering;

			if (_cursor_is_hovering_link)
				_text_view.get_window(Gtk.TextWindowType.TEXT).set_cursor(_pointer_cursor);
			else
				_text_view.get_window(Gtk.TextWindowType.TEXT).set_cursor(_text_cursor);
		}

		return Gdk.EVENT_PROPAGATE;
	}

	public void log(string severity, string message)
	{
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

		Gtk.TextIter end_iter;
		buffer.get_end_iter(out end_iter);

		// Replace all IDs with corresponding human-readable names.
		int id_index = 0;
		do
		{
			// Search for occurrences of the ID string.
			int id_index_orig = id_index;
			id_index = message.index_of("#ID(", id_index);
			if (id_index != -1)
			{
				// If an occurrenct is found, insert the preceding text as usual.
				string line_chunk = message.substring(id_index_orig, id_index-id_index_orig);
				buffer.insert_with_tags(ref end_iter
					, line_chunk
					, line_chunk.length
					, buffer.tag_table.lookup(severity)
					, null
					);

				// Try to extract the ID from the ID string.
				int id_closing_parentheses = message.index_of(")", id_index+4);
				if (id_closing_parentheses == -1)
				{
					// If the ID is malformed, insert the whole line as-is.
					buffer.insert_with_tags(ref end_iter
						, message.substring(id_index)
						, -1
						, buffer.tag_table.lookup(severity)
						, null
						);
					break;
				}

				// Convert the resource ID to human-readable resource name.
				string resource_name;
				string resource_id = message.substring(id_index+4, id_closing_parentheses-(id_index+4));
				_project.resource_id_to_name(out resource_name, resource_id);
				// Create a tag for hyperlink.
				Gtk.TextTag hyperlink = null;
				hyperlink = buffer.create_tag(null, "underline", Pango.Underline.SINGLE, null);
				hyperlink.set_data("resource_name", resource_name);

				buffer.insert_with_tags(ref end_iter
					, resource_name
					, -1
					, buffer.tag_table.lookup(severity)
					, hyperlink
					, null
					);
				id_index += 4 + resource_id.length;
				continue;
			}
			else
			{
				buffer.insert_with_tags(ref end_iter
					, message.substring(id_index_orig)
					, -1
					, buffer.tag_table.lookup(severity)
					, null
					);
			}
		}
		while (id_index++ >= 0);

		// Scroll to bottom.
		// See: gtk3-demo "Automatic Scrolling".
		end_iter.set_line_offset(0);
		Gtk.TextMark mark = buffer.get_mark("scroll");
		buffer.move_mark(mark, end_iter);
		_text_view.scroll_mark_onscreen(mark);
	}
}

}
