/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class EntrySearch : Gtk.Box
{
	private Gtk.SearchEntry _search_entry;

	public signal void search_changed();

	public EntrySearch()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL);

		_search_entry = new Gtk.SearchEntry();

		_search_entry.focus_in_event.connect(on_focus_in);
		_search_entry.focus_out_event.connect(on_focus_out);
		_search_entry.search_changed.connect(() => search_changed());

		this.pack_start(_search_entry);
	}

	public string text {
		get { return _search_entry.text; }
		set { _search_entry.text = value; }
	}

	public void set_placeholder_text(string text)
	{
		_search_entry.set_placeholder_text(text);
	}

	private bool on_focus_in(Gdk.EventFocus ev)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(_search_entry);

		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_focus_out(Gdk.EventFocus ef)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_out(_search_entry);

		return Gdk.EVENT_PROPAGATE;
	}
}

} /* namespace Crown */
