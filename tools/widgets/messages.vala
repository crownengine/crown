/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public Gtk.Box message_widget(Gtk.Label h1, Gtk.Widget? p = null)
{
	var box = new Gtk.Box(Gtk.Orientation.VERTICAL, 12);
	box.valign = Gtk.Align.CENTER;
	box.pack_start(h1);
	if (p != null)
		box.pack_start(p);

	return box;
}

public Gtk.Box long_running_task(string markup)
{
	var h1 = new Gtk.Label(null);
	h1.set_markup(markup);
	h1.valign = Gtk.Align.CENTER;

	var spinner = new Gtk.Spinner();
	spinner.active = true;

	return message_widget(h1, spinner);
}

public Gtk.Box compiling_data()
{
	return long_running_task("<span font_weight=\"bold\">Compiling data...</span>");
}

public Gtk.Box connecting_to_data_compiler()
{
	return long_running_task("<span font_weight=\"bold\">Connecting to Data Compiler...</span>");
}

public Gtk.Box restart_compiler(string markup)
{
	Gtk.Label h1 = new Gtk.Label(null);
	h1.set_markup(markup);

	Gtk.Label p = new Gtk.Label(null);
	p.get_style_context().add_class("colorfast-link");
	p.set_markup("Fix errors and <a href=\"restart\">restart the compiler</a> to continue.");
	p.activate_link.connect(() => {
			GLib.Application.get_default().activate_action("restart-backend", null);
			return true;
		});

	return message_widget(h1, p);
}

public Gtk.Box compiler_failed_compilation()
{
	return restart_compiler("<span font_weight=\"bold\">Data compilation failed.</span>");
}

public Gtk.Box compiler_crashed()
{
	return restart_compiler("<span font_weight=\"bold\">Data Compiler disconnected unexpectedly.</span>");
}

public delegate void RestartVieport();

public Gtk.Box editor_oops(RestartVieport restart_viewport)
{
	var h1 = new Gtk.Label(null);
	h1.set_markup("<span font_weight=\"bold\">Something went wrong.</span>");

	var p = new Gtk.Label(null);
	p.get_style_context().add_class("colorfast-link");
	p.set_markup("Try to <a href=\"restart\">restart this view</a>.");
	p.activate_link.connect(() => {
			restart_viewport();
			return true;
		});

	return message_widget(h1, p);
}

public Gtk.Box editor_disconnected()
{
	Gtk.Label h1 = new Gtk.Label(null);
	h1.set_markup("<span font_weight=\"bold\">Disconnected.</span>");

	return message_widget(h1);
}

public Gtk.Box stopping_backend()
{
	return long_running_task("<span font_weight=\"bold\">Stopping Backend...</span>");
}

public Gtk.Dialog new_resource_changed_dialog(Gtk.Window? parent, string resource_name)
{
	Gtk.MessageDialog md = new Gtk.MessageDialog(parent
		, Gtk.DialogFlags.MODAL
		, Gtk.MessageType.WARNING
		, Gtk.ButtonsType.NONE
		, "Save changes to '%s' before closing?".printf(resource_name)
		);
	Gtk.Widget btn;
	btn = md.add_button("Close _without Saving", Gtk.ResponseType.NO);
	btn.get_style_context().add_class("destructive-action");
	md.add_button("_Cancel", Gtk.ResponseType.CANCEL);
	md.add_button("_Save", Gtk.ResponseType.YES);
	md.set_default_response(Gtk.ResponseType.YES);
	return md;
}

} /* namespace Crown */
