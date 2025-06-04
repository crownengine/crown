/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class Statusbar : Gtk.Box
{
	// Data
	public uint _timer_id;

	// Widgets
	public Gtk.Label _status;
	public Gtk.Label _temporary_message;
	public Gtk.Button _donate;
	public Gtk.Label _version;
	public const string IDLE_STATUS = "Idle";

	public Statusbar()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);
		this.margin_start = 8;
		this.margin_end   = 8;

		// Data
		_timer_id = 0;

		// Widgets
		clear_status();
		_temporary_message = new Gtk.Label("");
#if CROWN_GTK3
		_donate = new Gtk.Button.from_icon_name(IconTheme.HEARTH, Gtk.IconSize.SMALL_TOOLBAR);
		_donate.can_focus = false;
		_donate.get_style_context().add_class("flat");
#else
		_temporary_message.hexpand = true;

		_donate = new Gtk.Button.from_icon_name(IconTheme.HEARTH);
		_donate.focusable = false;
		_donate.add_css_class("flat");
#endif
		_donate.clicked.connect(() => {
				GLib.Application.get_default().activate_action("donate", null);
			});
		_donate.halign = Gtk.Align.END;
		_version = new Gtk.Label(null);
#if CROWN_GTK3
		_version.get_style_context().add_class("colorfast-link");
#else
		_version.add_css_class("colorfast-link");
#endif
		_version.set_markup("<a href=\"\">" + CROWN_VERSION + "</a>");
#if CROWN_GTK3
		_version.get_style_context().add_class("version-label");
		_version.can_focus = false;
#else
		_version.add_css_class("version-label");
		_version.focusable = false;
#endif
		_version.activate_link.connect(() => {
				GLib.Application.get_default().activate_action("changelog", null);
				return true;
			});
		_version.halign = Gtk.Align.END;

#if CROWN_GTK3
		this.pack_start(_status, false, false, 0);
		this.pack_start(_temporary_message, false, false, 0);
		this.pack_end(_version, false, false, 0);
		this.pack_end(_donate, false, false, 6);
		this.get_style_context().add_class("statusbar");
#else
		this.append(_status);
		this.append(_temporary_message);
		this.append(_donate);
		this.append(_version);
		this.add_css_class("statusbar");
#endif
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

		if (_timer_id > 0) {
			GLib.Source.remove(_timer_id);
			_timer_id = 0;
		}

		_timer_id = GLib.Timeout.add_seconds(4, () => {
				_temporary_message.set_label("");
				_timer_id = 0;
				return GLib.Source.REMOVE;
			});
	}

	public void set_status(string status)
	{
		_status.set_text(status);
	}

	public void clear_status()
	{
		if (_status == null) {
			_status = new Gtk.Label(IDLE_STATUS);
#if !CROWN_GTK3
			_status.halign = Gtk.Align.START;
#endif
		} else {
			_status.set_text(IDLE_STATUS);
		}
	}
}

} /* namespace Crown */
