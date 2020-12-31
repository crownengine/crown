/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class ComboBoxMap : Gtk.ComboBoxText
{
	// Data
	public bool _stop_emit;

	// Signals
	public signal void value_changed();

	public string value
	{
		get
		{
			return this.get_active_id();
		}
		set
		{
			_stop_emit = true;
			this.set_active_id((string)value);
			_stop_emit = false;
		}
	}

	public ComboBoxMap()
	{
		// Data
		_stop_emit = true;

		// Widgets
		this.changed.connect(on_changed);
		this.scroll_event.connect(on_scroll);
	}

	private void on_changed()
	{
		if (!_stop_emit)
			value_changed();
	}

	private bool on_scroll(Gdk.EventScroll ev)
	{
		GLib.Signal.stop_emission_by_name(this, "scroll-event");
		return Gdk.EVENT_PROPAGATE;
	}
}

}
