/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
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

	public string? value
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

	public ComboBoxMap(int default_id = 0, string[]? labels = null, string[]? ids = null)
	{
		// Data
		_stop_emit = false;

		if (labels != null) {
			for (int ii = 0; ii < ids.length; ++ii) {
				unowned string? id = ids != null ? ids[ii] : null;
				this.append(id, labels[ii]);
			}
			if (ids != null && default_id < ids.length)
				this.value = ids[default_id];
		}

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

} /* namespace Crown */
