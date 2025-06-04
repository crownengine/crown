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
#if !CROWN_GTK3
	public Gtk.EventControllerFocus _controller_focus;
#endif

#if CROWN_GTK3
	public signal void search_changed();
#else
	public signal void search_changed(EntrySearch entry);
#endif

	public EntrySearch()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL);

		_entry = new Gtk.SearchEntry();
		_entry.set_width_chars(0);
		_entry.hexpand = true;
		_search_timeout_id = 0;

#if CROWN_GTK3
		_entry.focus_in_event.connect(on_focus_in);
		_entry.focus_out_event.connect(on_focus_out);
		this.pack_start(_entry);
#else
		_controller_focus = new Gtk.EventControllerFocus();
		_controller_focus.enter.connect(on_focus_enter);
		_controller_focus.leave.connect(on_focus_leave);
		_entry.add_controller(_controller_focus);
		this.append(_entry);
#endif
		_entry.changed.connect(on_entry_changed);
	}

	public string text {
		get { return _entry.text; }
		set { _entry.text = value; }
	}

	public void set_placeholder_text(string text)
	{
		_entry.placeholder_text = text;
	}

	public void on_entry_changed()
	{
		if (_search_timeout_id != 0)
			GLib.Source.remove(_search_timeout_id);

		const uint[] SEARCH_DELAYS_MS = { 150, 75, 38, 16 };
#if CROWN_GTK3
		uint length = _entry.get_buffer().length;
#else
		uint length = (uint)_entry.text.char_count();
#endif
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
#if CROWN_GTK3
		search_changed();
#else
		search_changed(this);
#endif
		return GLib.Source.REMOVE;
	}

#if CROWN_GTK3
	public bool on_focus_in(Gdk.EventFocus ev)
#else
	public void on_focus_enter()
#endif
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(_entry);
#if CROWN_GTK3
		return Gdk.EVENT_PROPAGATE;
#endif
	}

#if CROWN_GTK3
	public bool on_focus_out(Gdk.EventFocus ef)
#else
	public void on_focus_leave()
#endif
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_out(_entry);
#if CROWN_GTK3
		return Gdk.EVENT_PROPAGATE;
#endif
	}
}

} /* namespace Crown */
