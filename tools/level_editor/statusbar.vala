/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class Statusbar : Gtk.Box
{
	// Data
	public uint _timer_id;

	// Widgets
	public Gtk.Label _status;
	public Gtk.Label _temporary_message;

	public Statusbar()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);

		// Data
		_timer_id = 0;

		// Widgets
		_status = new Gtk.Label("Done");
		_temporary_message = new Gtk.Label("");

		this.pack_start(_status, false, false, 0);
		this.pack_start(_temporary_message, false, false, 0);
		this.get_style_context().add_class("statusbar");
	}

	~Statusbar()
	{
		if (_timer_id > 0)
			GLib.Source.remove(_timer_id);
	}

	/// Shows a message on the statusbar and removes it after 4 seconds.
	public void set_temporary_message(string message)
	{
		_temporary_message.set_label("; " + message);

		if (_timer_id > 0)
		{
			GLib.Source.remove(_timer_id);
			_timer_id = 0;
		}

		_timer_id = GLib.Timeout.add_seconds(4, () => {
			_temporary_message.set_label("");
			_timer_id = 0;
			return false; // Remove the source
		});
	}
}

}
