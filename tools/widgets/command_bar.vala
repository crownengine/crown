/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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

public class CommandBar : Gtk.Box
{
	// Data
	public EntryHistory _entry_history;
	public uint _distance;
	public uint _completion_request_id;
	public uint _completion_source_id;
	public bool _completion_updating_entry;
	public bool _completion_suppress_autofill_once;
	public string _completion_query_text;
	public string _completion_suffix_text;
	public int _completion_replace_start_byte;
	public bool _history_navigation_active;

	// Widgets
	public Gtk.MenuButton _runtime_button;
	public Gtk.Entry _entry;
	public Gtk.Popover _entry_suggestions_popover;
	public Gtk.ScrolledWindow _entry_suggestions_scroller;
	public Gtk.ListBox _entry_suggestions_list;
	public Gtk.EventControllerKey _entry_controller_key;

	public CommandBar()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);

		_entry_history = new EntryHistory(256);
		_distance = 0;
		_completion_query_text = "";
		_completion_suffix_text = "";

		_runtime_button = new Gtk.MenuButton();
		_runtime_button.set_tooltip_text(_("Runtime that will receive the message."));
		_runtime_button.set_label(_("Editor"));
		_runtime_button.set_size_request(50, -1);
		GLib.Menu runtime_menu = new GLib.Menu();
		GLib.MenuItem editor_item = new GLib.MenuItem(_("Editor"), null);
		editor_item.set_action_and_target_value("app.console-runtime", new GLib.Variant.int32((int)ConsoleRuntime.EDITOR));
		runtime_menu.append_item(editor_item);
		GLib.MenuItem game_item = new GLib.MenuItem(_("Game"), null);
		game_item.set_action_and_target_value("app.console-runtime", new GLib.Variant.int32((int)ConsoleRuntime.GAME));
		runtime_menu.append_item(game_item);
#if CROWN_GTK3
		_runtime_button.set_use_popover(true);
#endif
		_runtime_button.set_menu_model(runtime_menu);

		_entry = new Gtk.Entry();
		_entry.activate.connect(on_entry_activated);
		_entry.changed.connect(on_entry_changed);
#if CROWN_GTK3
		_entry.focus_in_event.connect(on_entry_focus_in);
		_entry.focus_out_event.connect(on_entry_focus_out);
#endif
		_entry.set_placeholder_text(_("Enter Command or Lua expression"));
		_entry.hexpand = true;
		_entry.width_chars = 1;
#if !CROWN_GTK3
		var entry_focus = new Gtk.EventControllerFocus();
		entry_focus.enter.connect(on_entry_focus_in);
		entry_focus.leave.connect(on_entry_focus_out);
		_entry.add_controller(entry_focus);
#endif

		_entry_suggestions_list = new Gtk.ListBox();
		_entry_suggestions_list.selection_mode = Gtk.SelectionMode.SINGLE;
		_entry_suggestions_list.set_sort_func(compare_suggestions);
		_entry_suggestions_list.row_activated.connect(on_suggestion_activated);

#if CROWN_GTK3
		_entry_suggestions_scroller = new Gtk.ScrolledWindow(null, null);
#else
		_entry_suggestions_scroller = new Gtk.ScrolledWindow();
#endif
		_entry_suggestions_scroller.hscrollbar_policy = Gtk.PolicyType.NEVER;
		_entry_suggestions_scroller.vscrollbar_policy = Gtk.PolicyType.AUTOMATIC;
		_entry_suggestions_scroller.set_min_content_height(-1);
		_entry_suggestions_scroller.set_max_content_height(500);
#if CROWN_GTK3
		_entry_suggestions_scroller.add(_entry_suggestions_list);

		_entry_controller_key = new Gtk.EventControllerKey(_entry);
#else
		_entry_suggestions_scroller.set_child(_entry_suggestions_list);

		_entry_controller_key = new Gtk.EventControllerKey();
#endif
		_entry_controller_key.key_pressed.connect(on_entry_key_pressed);
#if !CROWN_GTK3
		_entry.add_controller(_entry_controller_key);
#endif

#if CROWN_GTK3
		this.pack_start(_runtime_button, false, false);
		this.pack_start(_entry, true, true);

		_entry_suggestions_popover = new Gtk.Popover(_entry);
		_entry_suggestions_popover.set_modal(false);
#else
		this.append(_runtime_button);
		this.append(_entry);

		_entry_suggestions_popover = new Gtk.Popover();
		_entry_suggestions_popover.set_autohide(false);
#endif
		_entry_suggestions_popover.set_can_focus(false);
		_entry_suggestions_popover.set_position(Gtk.PositionType.TOP);
#if CROWN_GTK3
		_entry_suggestions_popover.add(_entry_suggestions_scroller);
		_entry_suggestions_popover.show_all();
#else
		_entry_suggestions_popover.set_parent(this);
		_entry_suggestions_popover.set_child(_entry_suggestions_scroller);
#endif
		_entry_suggestions_popover.popdown();

		this.unmap.connect(invalidate_lua_suggestions);
#if CROWN_GTK3
		this.get_style_context().add_class("command-bar");
#else
		this.add_css_class("command-bar");
#endif
	}

	public int compare_suggestions(Gtk.ListBoxRow row1, Gtk.ListBoxRow row2)
	{
		string a = row1.get_data<string>("suggestion");
		string b = row2.get_data<string>("suggestion");
		return strcasecmp(a, b);
	}

	public void on_suggestion_activated(Gtk.ListBoxRow row)
	{
		apply_suggestion_row(row);
		hide_lua_suggestions();
	}

	public void clear_lua_suggestions()
	{
#if CROWN_GTK3
		foreach (var child in _entry_suggestions_list.get_children()) {
			_entry_suggestions_list.remove(child);
		}
#else
		Gtk.Widget? child = _entry_suggestions_list.get_first_child();
		while (child != null) {
			Gtk.Widget? next = child.get_next_sibling();
			_entry_suggestions_list.remove(child);
			child = next;
		}
#endif
	}

	public void reset_lua_suggestions_state()
	{
		clear_lua_suggestions();
		invalidate_lua_suggestions();
		_completion_suppress_autofill_once = false;
	}

	public void hide_lua_suggestions()
	{
		_entry_suggestions_popover.popdown();
	}

	public void invalidate_lua_suggestions()
	{
		++_completion_request_id;
		if (_completion_source_id != 0) {
			GLib.Source.remove(_completion_source_id);
			_completion_source_id = 0;
		}
		hide_lua_suggestions();
	}

	public string text_from_suggestion(out int cursor_position, string suggestion)
	{
		string completed_prefix = _completion_query_text[0 : _completion_replace_start_byte] + suggestion;
		cursor_position = completed_prefix.char_count();
		return completed_prefix + _completion_suffix_text;
	}

	public void set_entry_from_suggestion(string suggestion)
	{
		int cursor_position;
		string text = text_from_suggestion(out cursor_position, suggestion);

		_completion_updating_entry = true;
		_entry.text = text;
		_entry.set_position(cursor_position);
		_completion_updating_entry = false;
	}

	public void apply_suggestion_row(Gtk.ListBoxRow row)
	{
		string? suggestion = row.get_data<string>("suggestion");
		if (suggestion == null)
			return;

		set_entry_from_suggestion(suggestion);
	}

	public void navigate_suggestions(int delta)
	{
#if CROWN_GTK3
		var children = _entry_suggestions_list.get_children();
		int count = (int)children.length();
#else
		int count = 0;
		for (Gtk.Widget? child = _entry_suggestions_list.get_first_child(); child != null; child = child.get_next_sibling())
			++count;
#endif
		if (count == 0)
			return;

		var selected = _entry_suggestions_list.get_selected_row();
		int index = selected != null? selected.get_index() : (delta > 0 ? -1 : count);
		int target_index = index + delta;
		if (target_index < 0)
			target_index = 0;

		if (target_index >= count)
			target_index = count - 1;

		var row = _entry_suggestions_list.get_row_at_index(target_index);
		if (row == null)
			return;

		_entry_suggestions_list.select_row(row);
		apply_suggestion_row(row);

		var adj = _entry_suggestions_scroller.get_vadjustment();
		if (adj != null) {
			Gtk.Allocation row_alloc;
			row.get_allocation(out row_alloc);

			double top = (double)row_alloc.y;
			double bottom = top + (double)row_alloc.height;
			double view_top = adj.value;
			double view_bottom = adj.value + adj.page_size;

			if (top < view_top)
				adj.set_value(top);
			else if (bottom > view_bottom)
				adj.set_value(bottom - adj.page_size);
		}
	}

	public bool has_prefix_case(string str, string prefix)
	{
		return str.down().has_prefix(prefix.down());
	}

	public int strcasecmp(string a, string b)
	{
		int cmp = strcmp(a.down(), b.down());
		return cmp != 0 ? cmp : strcmp(a, b);
	}

	public void current_completion_context(out string query_text, out string suffix_text)
	{
		string text = _entry.text;
		int query_end_position = _entry.get_position();
		int suffix_start_position = query_end_position;
		int start_pos;
		int end_pos;
		if (_entry.get_selection_bounds(out start_pos, out end_pos)) {
			query_end_position = start_pos < end_pos ? start_pos : end_pos;
			suffix_start_position = start_pos < end_pos ? end_pos : start_pos;
		}

		int query_end = text.index_of_nth_char(query_end_position);
		int suffix_start = text.index_of_nth_char(suffix_start_position);
		query_text = text[0 : query_end];
		suffix_text = text[suffix_start : text.length];
	}

	public void on_entry_changed()
	{
		if (_completion_updating_entry || _history_navigation_active)
			return;

		// Gtk.Entry emits changed before updating its cursor position.
		// Defer reading the completion context until the edit is finished.
		++_completion_request_id;
		if (_completion_source_id != 0)
			GLib.Source.remove(_completion_source_id);
		_completion_source_id = GLib.Idle.add(on_completion_idle);
	}

	public bool on_completion_idle()
	{
		_completion_source_id = 0;
		request_lua_suggestions(_completion_request_id);
		return GLib.Source.REMOVE;
	}

	public void request_lua_suggestions(uint request_id)
	{
		if (request_id != _completion_request_id || !_entry.get_mapped())
			return;

		string text;
		string suffix_text;
		current_completion_context(out text, out suffix_text);
		_completion_query_text = text;
		_completion_suffix_text = suffix_text;
		if (text.length == 0 || text[0] == ':') {
			reset_lua_suggestions_state();
			return;
		}

		var app = (LevelEditorApplication)GLib.Application.get_default();
		RuntimeInstance? runtime = app.current_selected_runtime();
		if (runtime == null || !runtime.is_connected()) {
			reset_lua_suggestions_state();
			return;
		}

		runtime.send(RuntimeApi.suggest(request_id, text));
	}

	public void set_lua_suggestions(uint request_id, uint replace_start_byte, GLib.GenericArray<Value?> items)
	{
		if (request_id != _completion_request_id || !_entry.get_mapped())
			return;
		if (replace_start_byte > (uint)_completion_query_text.length
			|| !_completion_query_text.validate((ssize_t)replace_start_byte)
			) {
			reset_lua_suggestions_state();
			return;
		}

		_completion_replace_start_byte = (int)replace_start_byte;

		clear_lua_suggestions();

		int suggestions_count = 0;
		for (int i = 0; i < items.length; ++i) {
			var item = (GLib.HashTable<string, Value?>)items[i];
			string s = (string)item["name"];

			Gtk.ListBoxRow row = new Gtk.ListBoxRow();
			Gtk.Box box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 8);
			Gtk.Label label = new Gtk.Label(s);
			label.set_xalign(0.0f);
			label.hexpand = true;
			Gtk.Label type_label = new Gtk.Label((string)item["type"]);
			type_label.set_xalign(1.0f);
#if CROWN_GTK3
			type_label.get_style_context().add_class(Gtk.STYLE_CLASS_DIM_LABEL);
			box.pack_start(label);
			box.pack_start(type_label, false, false, 0);
			row.add(box);
#else
			type_label.add_css_class("dim-label");
			box.append(label);
			box.append(type_label);
			row.set_child(box);
#endif
			row.set_data("suggestion", s);
#if CROWN_GTK3
			_entry_suggestions_list.add(row);
#else
			_entry_suggestions_list.append(row);
#endif
			++suggestions_count;
		}
		_entry_suggestions_list.invalidate_sort();
#if CROWN_GTK3
		_entry_suggestions_list.show_all();
#endif

		if (suggestions_count > 0) {
			Gtk.ListBoxRow? first_row = _entry_suggestions_list.get_row_at_index(0);
			string? first = first_row != null? first_row.get_data<string>("suggestion") : null;
			int prefix_len = _completion_query_text.char_count();
			int completion_end = 0;
			string? first_text = null;
			if (first != null)
				first_text = text_from_suggestion(out completion_end, first);

			if (first_text != null && !_completion_suppress_autofill_once && has_prefix_case(first_text, _completion_query_text)) {
				// Preserve the text the user typed and append only the suggested suffix.
				int suffix_start_byte = first_text.index_of_nth_char(prefix_len);
				string inline_text = _completion_query_text + first_text[suffix_start_byte : first_text.length];

				_completion_updating_entry = true;
				_entry.text = inline_text;
				_entry.set_position(prefix_len);
				_entry.select_region(prefix_len, completion_end);
				_completion_updating_entry = false;
			}

			int list_min_h = 0;
			int list_nat_h = 0;
#if CROWN_GTK3
			_entry_suggestions_list.get_preferred_height(out list_min_h, out list_nat_h);
#else
			int baseline_min = 0;
			int baseline_nat = 0;
			_entry_suggestions_list.measure(Gtk.Orientation.VERTICAL
				, -1
				, out list_min_h
				, out list_nat_h
				, out baseline_min
				, out baseline_nat
				);
#endif
			int desired_h = list_nat_h + 8;
			const int max_h = 500;
			if (desired_h > max_h)
				desired_h = max_h;
			_entry_suggestions_scroller.set_max_content_height(max_h);
			_entry_suggestions_scroller.set_min_content_height(desired_h);
			_entry_suggestions_popover.set_size_request(-1, desired_h);
			Gdk.Rectangle cursor_rect;
#if CROWN_GTK3
			Pango.Rectangle strong;
			Pango.Rectangle weak;
			int layout_index = _entry.text_index_to_layout_index(_entry.text.index_of_nth_char(prefix_len));
			_entry.get_layout().get_cursor_pos(layout_index, out strong, out weak);
			int layout_x;
			int layout_y;
			_entry.get_layout_offsets(out layout_x, out layout_y);
			cursor_rect = { layout_x + strong.x / Pango.SCALE, layout_y + strong.y / Pango.SCALE, 1, strong.height / Pango.SCALE };
#else
			Gtk.Text entry_text = (Gtk.Text)_entry.get_delegate();
			Graphene.Rect strong;
			Graphene.Rect weak;
			entry_text.compute_cursor_extents(prefix_len, out strong, out weak);
			double cursor_x;
			double cursor_y;
			entry_text.translate_coordinates(this, strong.origin.x, strong.origin.y, out cursor_x, out cursor_y);
			cursor_rect = { (int)cursor_x, (int)cursor_y, 1, (int)strong.size.height };
#endif /* if CROWN_GTK3 */
			_entry_suggestions_popover.set_pointing_to(cursor_rect);
			_entry_suggestions_popover.popup();
			if (first_row != null)
				_entry_suggestions_list.select_row(first_row);
			_entry.grab_focus_without_selecting();
		} else {
			hide_lua_suggestions();
		}

		_completion_suppress_autofill_once = false;
	}

	public void on_entry_activated()
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

	public bool has_entry_focus()
	{
#if CROWN_GTK3
		return _entry.has_focus;
#else
		return ((Gtk.Widget)_entry.get_delegate()).has_focus;
#endif
	}

#if CROWN_GTK3
	public bool on_entry_focus_in(Gdk.EventFocus ev)
#else
	public void on_entry_focus_in()
#endif
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(_entry);
#if CROWN_GTK3
		return Gdk.EVENT_PROPAGATE;
#endif
	}

#if CROWN_GTK3
	public bool on_entry_focus_out(Gdk.EventFocus ef)
#else
	public void on_entry_focus_out()
#endif
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_out(_entry);
		invalidate_lua_suggestions();
#if CROWN_GTK3
		return Gdk.EVENT_PROPAGATE;
#endif
	}

	public bool on_entry_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval == Gdk.Key.Tab && _entry_suggestions_popover.get_visible()) {
			Gtk.ListBoxRow? row = _entry_suggestions_list.get_selected_row();
			if (row != null) {
				apply_suggestion_row(row);
				hide_lua_suggestions();
				return Gdk.EVENT_STOP;
			}
		}

		if (keyval == Gdk.Key.Escape && _entry_suggestions_popover.get_visible()) {
			invalidate_lua_suggestions();
			return Gdk.EVENT_STOP;
		}

		if (keyval == Gdk.Key.BackSpace || keyval == Gdk.Key.Delete)
			_completion_suppress_autofill_once = true;

		if (keyval == Gdk.Key.Down) {
			if (_entry_suggestions_popover.get_visible()) {
				navigate_suggestions(+1);
				return Gdk.EVENT_STOP;
			}

			_history_navigation_active = true;
			if (_distance > 1) {
				--_distance;
				_entry.text = _entry_history.element(_distance);
			} else {
				_entry.text = "";
			}
			_history_navigation_active = false;

			_entry.set_position(_entry.text.length);
			return Gdk.EVENT_STOP;
		} else if (keyval == Gdk.Key.Up) {
			if (_entry_suggestions_popover.get_visible()) {
				navigate_suggestions(-1);
				return Gdk.EVENT_STOP;
			}

			_history_navigation_active = true;
			if (_distance < _entry_history._size) {
				++_distance;
				_entry.text = _entry_history.element(_distance);
			}
			_history_navigation_active = false;

			_entry.set_position(_entry.text.length);
			return Gdk.EVENT_STOP;
		}

		return Gdk.EVENT_PROPAGATE;
	}

	public override void dispose()
	{
		this.unmap.disconnect(invalidate_lua_suggestions);
		invalidate_lua_suggestions();
#if !CROWN_GTK3
		if (_entry_suggestions_popover.get_parent() != null)
			_entry_suggestions_popover.unparent();
#endif
		base.dispose();
	}
}

} /* namespace Crown */
