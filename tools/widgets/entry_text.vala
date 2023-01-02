/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class EntryText : Gtk.Entry
{
	public EntryText()
	{
		this.focus_in_event.connect(on_focus_in);
		this.focus_out_event.connect(on_focus_out);
	}

	private bool on_focus_in(Gdk.EventFocus ev)
	{
		Gtk.Window win = this.get_toplevel() as Gtk.Window;
		LevelEditorApplication app = (LevelEditorApplication)win.application;
		if (app != null)
			app.entry_any_focus_in(this);

		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_focus_out(Gdk.EventFocus ef)
	{
		Gtk.Window win = this.get_toplevel() as Gtk.Window;
		LevelEditorApplication app = (LevelEditorApplication)win.application;
		if (app != null)
			app.entry_any_focus_out(this);

		return Gdk.EVENT_PROPAGATE;
	}
}

} /* namespace Crown */
