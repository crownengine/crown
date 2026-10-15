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
	public CommandBar _command_bar;
	public Gtk.Widget _viewport;
	public Gtk.Box _messages;
	public Gtk.Box _links;
	public Gtk.Label _status;
	public Gtk.Label _temporary_message;
	public Gtk.Button _donate;
	public Gtk.Label _version;
	public const string IDLE_STATUS = "Idle";

	public Statusbar(CommandBar command_bar, Gtk.Widget viewport)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);
#if !CROWN_GTK3
		this.set_layout_manager(null);
#endif
		_command_bar = command_bar;
		_command_bar.margin_top = 4;
		_command_bar.margin_bottom = 4;
		_viewport = viewport;

		// Data
		_timer_id = 0;

		// Widgets
		clear_status();
		_temporary_message = new Gtk.Label("");
		_status.ellipsize = Pango.EllipsizeMode.END;
		_temporary_message.ellipsize = Pango.EllipsizeMode.END;
		_temporary_message.hexpand = true;
		_messages = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_messages.margin_top = 4;
		_messages.margin_bottom = 4;
		_messages.margin_start = 8;
		_messages.margin_end = 8;
		_links = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		_links.margin_top = 4;
		_links.margin_bottom = 4;
		_links.margin_start = 8;
		_links.margin_end = 8;
#if CROWN_GTK3
		_donate = new Gtk.Button.from_icon_name(IconTheme.HEARTH, Gtk.IconSize.SMALL_TOOLBAR);
		_donate.can_focus = false;
		_donate.get_style_context().add_class("flat");
#else
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
		_messages.pack_start(_status, false, false, 0);
		_messages.pack_start(_temporary_message, true, true, 0);
		_links.pack_start(_donate, false, false, 0);
		_links.pack_start(_version, false, false, 0);
		this.pack_start(_messages, false, false, 0);
		this.pack_start(_command_bar, true, true, 0);
		this.pack_start(_links, false, false, 0);
		this.get_style_context().add_class("statusbar");
#else
		_messages.append(_status);
		_messages.append(_temporary_message);
		_links.append(_donate);
		_links.append(_version);
		this.append(_messages);
		this.append(_command_bar);
		this.append(_links);
		this.add_css_class("statusbar");
#endif /* if CROWN_GTK3 */
	}

	~Statusbar()
	{
		if (_timer_id > 0)
			GLib.Source.remove(_timer_id);
	}

	// The viewport determines the command width; status text must not constrain the panes.
#if CROWN_GTK3
	public override void get_preferred_width(out int minimum, out int natural)
	{
		minimum = 0;
		natural = 0;
	}

	public override void size_allocate(Gtk.Allocation allocation)
	{
		base.size_allocate(allocation);
		allocate_contents(allocation.width, allocation.height);
	}
#else
	public override void measure(Gtk.Orientation orientation, int for_size
		, out int minimum, out int natural
		, out int minimum_baseline, out int natural_baseline)
	{
		minimum = 0;
		natural = 0;
		minimum_baseline = -1;
		natural_baseline = -1;
		if (orientation == Gtk.Orientation.HORIZONTAL)
			return;

		Gtk.Widget[] children = { _messages, _command_bar, _links };
		foreach (Gtk.Widget child in children) {
			int child_minimum;
			int child_natural;
			int baseline;
			child.measure(orientation, -1, out child_minimum, out child_natural, out baseline, out baseline);
			minimum = int.max(minimum, child_minimum);
			natural = int.max(natural, child_natural);
		}
	}

	public override void size_allocate(int width, int height, int baseline)
	{
		allocate_contents(width, height);
	}
#endif /* if CROWN_GTK3 */

	public void allocate_contents(int width, int height)
	{
#if CROWN_GTK3
		int viewport_x;
		int viewport_y;
#else
		double viewport_x;
		double viewport_y;
#endif
		if (!_viewport.translate_coordinates(this, 0, 0, out viewport_x, out viewport_y))
			return;

		int command_x = int.max(0, int.min((int)viewport_x, width));
		int command_width = int.max(0, int.min(_viewport.get_allocated_width(), width - command_x));
		int links_minimum;
		int links_natural;
#if CROWN_GTK3
		_links.get_preferred_width(out links_minimum, out links_natural);
#else
		int baseline;
		_links.measure(Gtk.Orientation.HORIZONTAL, -1, out links_minimum, out links_natural, out baseline, out baseline);
#endif
		allocate_child(_messages, 0, command_x, height);
		allocate_child(_command_bar, command_x, command_width, height);
		allocate_child(_links, width - links_natural, width - command_x - command_width >= links_natural ? links_natural : 0, height);
	}

	public void allocate_child(Gtk.Widget child, int x, int width, int height)
	{
		int minimum;
		int natural;
#if CROWN_GTK3
		child.get_preferred_width(out minimum, out natural);
#else
		int baseline;
		child.measure(Gtk.Orientation.HORIZONTAL, -1, out minimum, out natural, out baseline, out baseline);
#endif
		child.set_child_visible(width > 0 && width >= minimum);
		if (!child.get_child_visible())
			return;

#if CROWN_GTK3
		Gtk.Allocation allocation;
		this.get_allocation(out allocation);
		allocation.x += x;
		allocation.width = width;
		allocation.height = height;
		child.size_allocate(allocation);
#else
		Gsk.Transform transform = new Gsk.Transform().translate({ (float)x, 0.0f });
		child.allocate(width, height, -1, transform);
#endif
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
