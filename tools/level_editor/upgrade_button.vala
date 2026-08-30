/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class UpgradeButton : Gtk.Button
{
	public uint _hide_timeout;
	public Gtk.Popover _popover;
#if !CROWN_GTK3
	public Gtk.EventControllerMotion _controller_motion;
#endif

	public UpgradeButton()
	{
		this.visible = false;
		this.action_name = "app.upgrade";
#if CROWN_GTK3
		Gtk.Image image = new Gtk.Image.from_icon_name("folder-download-symbolic", Gtk.IconSize.BUTTON);
		this.image = image;
		this.get_style_context().add_class("suggested-action");
		this.can_focus = false;
		this.no_show_all = true;
		this.enter_notify_event.connect(on_motion_enter);
		this.leave_notify_event.connect(on_motion_leave);
#else
		Gtk.Image image = new Gtk.Image.from_icon_name("folder-download-symbolic");
		this.set_child(image);
		this.add_css_class("suggested-action");
		this.focusable = false;
		_controller_motion = new Gtk.EventControllerMotion();
		_controller_motion.enter.connect(on_enter);
		_controller_motion.leave.connect(on_leave);
		this.add_controller(_controller_motion);
#endif /* if CROWN_GTK3 */

		_hide_timeout = 0;

		var label = new Gtk.Label(_("Upgrade Now!"));
		label.margin_top
			= label.margin_end
			= label.margin_bottom
			= label.margin_start
			= 12;
#if CROWN_GTK3
		_popover = new Gtk.Popover(this);
		_popover.add(label);
		_popover.modal = false;
		_popover.show_all();
		_popover.hide();
#else
		_popover = new Gtk.Popover();
		_popover.set_child(label);
		_popover.autohide = false;
		_popover.set_parent(this);
#endif
	}

	public void on_enter()
	{
		if (_hide_timeout != 0) {
			Source.remove(_hide_timeout);
			_hide_timeout = 0;
		}

		_popover.popup();
	}

	public void on_leave()
	{
		if (_hide_timeout != 0)
			Source.remove(_hide_timeout);

		_hide_timeout = GLib.Timeout.add(50, () => {
				_hide_timeout = 0;
				_popover.popdown();
				return Source.REMOVE;
			});
	}

#if CROWN_GTK3
	public bool on_motion_enter(Gtk.Widget widget, Gdk.EventCrossing ev)
	{
		on_enter();
		return Gdk.EVENT_PROPAGATE;
	}

	public bool on_motion_leave(Gtk.Widget widget, Gdk.EventCrossing ev)
	{
		on_leave();
		return Gdk.EVENT_PROPAGATE;
	}
#endif
}

} /* namespace Crown */
