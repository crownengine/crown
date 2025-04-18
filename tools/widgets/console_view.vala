/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class CounterLabel : Gtk.Label
{
	public CounterLabel()
	{
		this.get_style_context().add_class("counter-label");
		this.set_visible(true);
	}

	protected override void get_preferred_height(out int minimum_height, out int natural_height)
	{
		// FIXME: Find a proper way to position/size labels inside Gtk.TextView.
		// Make Gtk.Label think it only needs 16px vertical to show its text.
		minimum_height = 1;
		natural_height = 16;
	}
}
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
	public struct LastMsg
	{
		string text;
		int num_repetitions;
		Gtk.TextChildAnchor anchor;
	}

	// Data
	public EntryHistory _entry_history;
	public uint _distance;
	public Project _project;
	public PreferencesDialog _preferences_dialog;
	public LastMsg _last_message;

	// Widgets
	public Gdk.Cursor _text_cursor;
	public Gdk.Cursor _pointer_cursor;
	public bool _cursor_is_hovering_link;
	public Gtk.TextView _text_view;
	public Gtk.Overlay _text_view_overlay;
	public Gtk.ScrolledWindow _scrolled_window;
	public EntryText _entry;
	public Gtk.Box _entry_hbox;
	public Gtk.TextMark _scroll_mark;
	public Gtk.TextMark _time_mark;

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
		_text_view.can_focus = true;

		Gtk.Button clear_button = new Gtk.Button.from_icon_name("edit-clear");
		clear_button.margin_top = 8;
		clear_button.margin_end = 16;
		clear_button.valign = Gtk.Align.START;
		clear_button.halign = Gtk.Align.END;
		clear_button.clicked.connect(() => {
				reset();
			});

		// Create tags for color-formatted text.
		Gtk.TextBuffer tb = _text_view.buffer;
		tb.tag_table.add(new Gtk.TextTag("info"));
		tb.tag_table.add(new Gtk.TextTag("warning"));
		tb.tag_table.add(new Gtk.TextTag("error"));
		tb.tag_table.add(new Gtk.TextTag("time"));

		this.style_updated.connect(update_style);
		update_style();

		Gtk.TextIter end_iter;
		tb.get_end_iter(out end_iter);
		_scroll_mark = tb.create_mark("scroll", end_iter, true);
		_time_mark = tb.create_mark("time", end_iter, true);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.vscrollbar_policy = Gtk.PolicyType.ALWAYS;
		_scrolled_window.add(_text_view);

		_text_view_overlay = new Gtk.Overlay();
		_text_view_overlay.add(_scrolled_window);
		_text_view_overlay.add_overlay(clear_button);

		_entry = new EntryText();
		_entry.key_press_event.connect(on_entry_key_pressed);
		_entry.activate.connect(on_entry_activated);

		_entry_hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_entry_hbox.pack_start(combo, false, false);
		_entry_hbox.pack_start(_entry, true, true);

		Gtk.Box hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		hbox.pack_start(_entry_hbox, true, true, 0);

		this.pack_start(_text_view_overlay, true, true, 0);
		this.pack_start(hbox, false, true, 0);

		this.destroy.connect(on_destroy);

		_text_view.button_press_event.connect(on_button_pressed);
		_text_view.button_release_event.connect(on_button_released);
		_text_view.motion_notify_event.connect(on_motion_notify);

		this.get_style_context().add_class("console-view");

		_console_view_valid = true;
	}

	public void reset()
	{
		_text_view.buffer.set_text("");

		_last_message = LastMsg()
		{
			text = "",
			num_repetitions = 0,
			anchor = null
		};
	}

	private void on_entry_activated()
	{
		string text = _entry.text;
		text = text.strip();

		if (text.length > 0) {
			_entry_history.push(text);
			_distance = 0;

			var app = (LevelEditorApplication)GLib.Application.get_default();
			RuntimeInstance? runtime = app.current_selected_runtime();

			if (text[0] == ':') {
				string[] args = text[1 : text.length].split(" ");
				if (args.length > 0) {
					if (runtime != null) {
						runtime.send(DeviceApi.command(args));
						runtime.send(DeviceApi.frame());
					}
				}
			} else {
				if (runtime != null) {
					logi("> %s".printf(text));
					runtime.send_script(text);
					runtime.send(DeviceApi.frame());
				}
			}
		}

		_entry.text = "";
	}

	private bool on_entry_key_pressed(Gdk.EventKey ev)
	{
		if (ev.keyval == Gdk.Key.Down) {
			if (_distance > 1) {
				--_distance;
				_entry.text = _entry_history.element(_distance);
			} else {
				_entry.text = "";
			}

			_entry.set_position(_entry.text.length);
			return Gdk.EVENT_STOP;
		} else if (ev.keyval == Gdk.Key.Up) {
			if (_distance < _entry_history._size) {
				++_distance;
				_entry.text = _entry_history.element(_distance);
			}

			_entry.set_position(_entry.text.length);
			return Gdk.EVENT_STOP;
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private void on_destroy()
	{
		_console_view_valid = false;
	}

	private bool on_button_pressed(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_SECONDARY) {
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
			if (_text_view.get_iter_at_location(out iter, buffer_x, buffer_y)) {
				// Check whether the text under the mouse pointer has a link tag.
				GLib.SList<unowned TextTag> tags = iter.get_tags();
				foreach (var item in tags) {
					string item_data;
					if ((item_data = item.get_data<string>("uri")) == null)
						continue;

					if (item_data.has_prefix("resource_id:")) {
						Gtk.Menu menu = new Gtk.Menu();
						Gtk.MenuItem mi;

						mi = new Gtk.MenuItem.with_label("Reveal in Project Browser");
						mi.activate.connect(() => {
								string resource_path  = item_data[12 : item_data.length];
								string? resource_type = ResourceId.type(resource_path);
								string? resource_name = ResourceId.name(resource_path);

								if (resource_type == null || resource_name == null)
									return;

								GLib.Variant paramz[] = { resource_type, resource_name };
								GLib.Application.get_default().activate_action("reveal-resource", new GLib.Variant.tuple(paramz));
							});
						menu.add(mi);

						mi = new Gtk.MenuItem.with_label("Open Containing Folder...");
						mi.activate.connect(() => {
								string resource_path = item_data[12 : item_data.length];
								GLib.Application.get_default().activate_action("open-containing", new GLib.Variant.string(resource_path));
							});
						menu.add(mi);

						menu.show_all();
						menu.popup_at_pointer(ev);

						return Gdk.EVENT_STOP;
					}
				}
			}
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_button_released(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_PRIMARY) {
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
			if (_text_view.get_iter_at_location(out iter, buffer_x, buffer_y)) {
				// Check whether the text under the mouse pointer has a link tag.
				GLib.SList<unowned TextTag> tags = iter.get_tags();
				foreach (var item in tags) {
					string item_data;
					if ((item_data = item.get_data<string>("uri")) != null) {
						if (item_data.has_prefix("resource_id:")) {
							GLib.Application.get_default().activate_action("open-resource", new GLib.Variant.string(item_data[12 : item_data.length]));
						} else if (item_data.has_prefix("file:")) {
							open_directory(item_data[5 : item_data.length]);
						} else {
							try {
								GLib.AppInfo.launch_default_for_uri(item_data, null);
							} catch (GLib.Error e) {
								loge(e.message);
							}
						}
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
		if (_text_view.get_iter_at_location(out iter, buffer_x, buffer_y)) {
			// Check whether the text under the mouse pointer has a link tag.
			GLib.SList<unowned TextTag> tags = iter.get_tags();
			foreach (var item in tags) {
				if (item.get_data<string>("uri") != null)
					hovering = true;
			}
		}

		if (_cursor_is_hovering_link != hovering) {
			_cursor_is_hovering_link = hovering;

			if (_cursor_is_hovering_link)
				_text_view.get_window(Gtk.TextWindowType.TEXT).set_cursor(_pointer_cursor);
			else
				_text_view.get_window(Gtk.TextWindowType.TEXT).set_cursor(_text_cursor);
		}

		return Gdk.EVENT_PROPAGATE;
	}

	public void log(string time, string severity, string message)
	{
		Gtk.TextBuffer buffer = _text_view.buffer;

		// Limit number of lines recorded.
		int max_lines = (int)_preferences_dialog._console_max_lines.value;
		if (buffer.get_line_count() - 1 >= max_lines) {
			Gtk.TextIter start_of_first_line;
			buffer.get_iter_at_line(out start_of_first_line, 0);
			Gtk.TextIter end_of_first_line = start_of_first_line;
			start_of_first_line.forward_line();
			buffer.delete(ref start_of_first_line, ref end_of_first_line);
		}

		Gtk.TextIter end_iter;
		buffer.get_end_iter(out end_iter);

		// Avoid showing duplicated messages. Insert a little counter
		// at the end of each line occurring twice or more.
		if (_last_message.text == message) {
			// Replace the current time with the latest one.
			Gtk.TextIter time_start;
			Gtk.TextIter time_end;
			buffer.get_iter_at_mark(out time_start, _time_mark);
			time_end = time_start;
			time_end.forward_chars(time.length);
			buffer.delete(ref time_start, ref time_end);

			buffer.get_iter_at_mark(out time_start, _time_mark);
			buffer.insert_with_tags(ref time_start
				, time
				, time.length
				, buffer.tag_table.lookup("time")
				, null
				);

			if (_last_message.num_repetitions == 0) {
				// Create a new anchor at the end of the line.
				buffer.get_end_iter(out end_iter);
				end_iter.backward_char();
				_last_message.anchor = buffer.create_child_anchor(end_iter);
				_text_view.add_child_at_anchor(new CounterLabel(), _last_message.anchor);
				scroll_to_bottom();
			}

			++_last_message.num_repetitions;

			const int MAX_REPETITIONS = 1000;
			if (_last_message.num_repetitions < MAX_REPETITIONS) {
				List<unowned Widget> widgets = _last_message.anchor.get_widgets();
				unowned var label_widget = widgets.first();
				var cl = (CounterLabel)label_widget.data;

				if (_last_message.num_repetitions == MAX_REPETITIONS - 1)
					cl.set_markup("%d+".printf(_last_message.num_repetitions));
				else
					cl.set_markup("%d".printf(_last_message.num_repetitions + 1));
			}

			return;
		} else {
			_last_message.text = message;
			_last_message.num_repetitions = 0;
		}

		buffer.move_mark(_time_mark, end_iter);
		buffer.insert_with_tags(ref end_iter
			, time
			, time.length
			, buffer.tag_table.lookup("time")
			, null
			);

		// Replace all IDs with corresponding human-readable names.
		int id_index = 0;
		do {
			// Search for occurrences of the ID string.
			int id_index_orig = id_index;
			if ((id_index = message.index_of("#ID(", id_index_orig)) != -1) {
				// If an occurrenct is found, insert the preceding text as usual.
				string line_chunk = message.substring(id_index_orig, id_index - id_index_orig);
				buffer.insert_with_tags(ref end_iter
					, line_chunk
					, line_chunk.length
					, buffer.tag_table.lookup(severity)
					, null
					);

				// Try to extract the resource ID from #ID() argument.
				int id_closing_parentheses = message.index_of(")", id_index + 4);
				if (id_closing_parentheses == -1) {
					// Syntax error, insert the whole line as-is.
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
				string resource_id = message.substring(id_index + 4, id_closing_parentheses - (id_index + 4));
				_project.resource_id_to_name(out resource_name, resource_id);

				// Create a tag for link.
				Gtk.TextTag link = null;
				link = buffer.create_tag(null, "underline", Pango.Underline.SINGLE, null);
				link.set_data("uri", "resource_id:%s".printf(resource_name));

				buffer.insert_with_tags(ref end_iter
					, resource_name
					, -1
					, buffer.tag_table.lookup(severity)
					, link
					, null
					);
				id_index += 4 + resource_id.length;
				continue;
			} else if ((id_index = message.index_of("#FILE(", id_index_orig)) != -1) {
				// If an occurrenct is found, insert the preceding text as usual.
				string line_chunk = message.substring(id_index_orig, id_index - id_index_orig);
				buffer.insert_with_tags(ref end_iter
					, line_chunk
					, line_chunk.length
					, buffer.tag_table.lookup(severity)
					, null
					);

				// Try to extract the path from #FILE() argument.
				int id_closing_parentheses = message.index_of(")", id_index + 6);
				if (id_closing_parentheses == -1) {
					// Syntax error, insert the whole line as-is.
					buffer.insert_with_tags(ref end_iter
						, message.substring(id_index)
						, -1
						, buffer.tag_table.lookup(severity)
						, null
						);
					break;
				}

				string file_path = message.substring(id_index + 6, id_closing_parentheses - (id_index + 6));

				// Create a tag for link.
				Gtk.TextTag link = null;
				link = buffer.create_tag(null, "underline", Pango.Underline.SINGLE, null);
				link.set_data("uri", "file:%s".printf(file_path));

				buffer.insert_with_tags(ref end_iter
					, file_path
					, -1
					, buffer.tag_table.lookup(severity)
					, link
					, null
					);
				id_index += 6 + file_path.length;
				continue;
			} else {
				buffer.insert_with_tags(ref end_iter
					, message.substring(id_index_orig)
					, -1
					, buffer.tag_table.lookup(severity)
					, null
					);
			}
		} while (id_index++ >= 0);

		scroll_to_bottom();
	}

	private void scroll_to_bottom()
	{
		// Line height is computed in an idle handler, wait a bit before scrolling to bottom.
		// See: https://valadoc.org/gtk+-3.0/Gtk.TextView.scroll_to_iter.html
		GLib.Idle.add(() => {
				Gtk.TextIter end_iter;
				_text_view.buffer.get_end_iter(out end_iter);

				// Scroll to bottom.
				// See: gtk3-demo "Automatic Scrolling".
				end_iter.set_line_offset(0);
				_text_view.buffer.move_mark(_scroll_mark, end_iter);
				_text_view.scroll_mark_onscreen(_scroll_mark);

				return GLib.Source.REMOVE;
			});
	}

	private void update_style()
	{
		Gtk.TextBuffer tb = _text_view.buffer;
		Gtk.TextTag tag_warning = tb.tag_table.lookup("warning");
		Gtk.TextTag tag_error = tb.tag_table.lookup("error");
		Gtk.TextTag tag_info = tb.tag_table.lookup("info");
		Gtk.TextTag tag_time = tb.tag_table.lookup("time");

		Gdk.RGBA col;
		get_style_context().lookup_color("warning_color", out col);
		tag_warning.foreground_rgba = col;
		get_style_context().lookup_color("error_color", out col);
		tag_error.foreground_rgba = col;
		get_style_context().lookup_color("theme_fg_color", out col);
		tag_info.foreground_rgba = col;
		get_style_context().lookup_color("success_color", out col);
		tag_time.foreground_rgba = col;
	}
}

} /* namespace Crown */
