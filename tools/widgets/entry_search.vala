/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class EntrySearch : Gtk.Box
{
	public Gtk.SearchEntry _entry;
	public uint _search_timeout_id;

	public signal void search_changed();

	public EntrySearch()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL);

		_entry = new Gtk.SearchEntry();
		_entry.set_width_chars(0);
		_search_timeout_id = 0;

		_entry.focus_in_event.connect(on_focus_in);
		_entry.focus_out_event.connect(on_focus_out);
		_entry.changed.connect(on_entry_changed);

		this.pack_start(_entry);
	}

	public string text {
		get { return _entry.text; }
		set { _entry.text = value; }
	}

	public void set_placeholder_text(string text)
	{
		_entry.set_placeholder_text(text);
	}

	public void on_entry_changed()
	{
		if (_search_timeout_id != 0)
			GLib.Source.remove(_search_timeout_id);

		const uint[] SEARCH_DELAYS_MS = { 150, 75, 38, 16 };
		uint length = _entry.get_buffer().length;
		if (length == 0)
			on_search_timeout();
		else if (length <= SEARCH_DELAYS_MS.length)
			_search_timeout_id = GLib.Timeout.add(SEARCH_DELAYS_MS[(int)length - 1], on_search_timeout);
		else
			_search_timeout_id = GLib.Timeout.add(8, on_search_timeout);
	}

	public bool on_search_timeout()
	{
		_search_timeout_id = 0;
		search_changed();
		return GLib.Source.REMOVE;
	}

	public bool on_focus_in(Gdk.EventFocus ev)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(_entry);

		return Gdk.EVENT_PROPAGATE;
	}

	public bool on_focus_out(Gdk.EventFocus ef)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_out(_entry);

		return Gdk.EVENT_PROPAGATE;
	}
}

} /* namespace Crown */
