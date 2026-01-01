/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class EntrySearch : Gtk.Box
{
	public Gtk.SearchEntry _entry;

	public signal void search_changed();

	public EntrySearch()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL);

		_entry = new Gtk.SearchEntry();

		_entry.focus_in_event.connect(on_focus_in);
		_entry.focus_out_event.connect(on_focus_out);
		_entry.search_changed.connect(() => search_changed());

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
