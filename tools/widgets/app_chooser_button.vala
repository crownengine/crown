/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;
using Gdk;

namespace Crown
{
public class AppChooserButton : Gtk.AppChooserButton
{
	public const string APP_PREDEFINED = "predefined"; ///< Open with a predefined system application.
	public const string APP_DEFAULT    = "default";    ///< Open with the default application for the file type.
	public const string APP_CUSTOM     = "custom";     ///< Open with a custom application defined by the user.

	// See: https://gitlab.gnome.org/GNOME/gtk/-/blob/gtk-3-24/gtk/gtkappchooserbutton.c
	public enum ModelColumn
	{
		APP_INFO,
		NAME
	}

	public Gtk.EventControllerScroll _controller_scroll;

	public AppChooserButton(string mime_type)
	{
		Object(content_type: mime_type);

		this.append_custom_item(APP_DEFAULT, "Open by extension", null);
		this.set_active_custom_item(APP_DEFAULT);

		_controller_scroll = new Gtk.EventControllerScroll(this, Gtk.EventControllerScrollFlags.BOTH_AXES);
		_controller_scroll.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
		_controller_scroll.scroll.connect(() => {
				// Do nothing, just consume the event to stop
				// the annoying scroll default behavior.
			});
	}

	/// Sets the app to @a app_name. If @a app_name is APP_PREDEFINED, it tries
	/// to set the predefined app based @a app_id.
	public void set_app(string app_name, string? app_id)
	{
		if (app_name != APP_PREDEFINED) {
			this.set_active_custom_item(app_name);
			return;
		}

		if (app_id == null) {
			this.set_active_custom_item(APP_DEFAULT);
			return;
		}

		this.model.foreach((model, path, iter) => {
				Value val;
				model.get_value(iter, ModelColumn.APP_INFO, out val);

				GLib.AppInfo app_info = (GLib.AppInfo)val;
				if (app_info != null && app_info.get_id() == app_id) {
					this.set_active_iter(iter);
					return true;
				}

				return false;
			});
	}

	/// Returns the item name of the selected application. If the application is predefined,
	/// it returns its @a app_id.
	public string selected_app(out string? app_id)
	{
		app_id = null;

		Gtk.TreeIter iter;
		if (this.get_active_iter(out iter)) {
			Value val;
			this.model.get_value(iter, ModelColumn.NAME, out val);
			string name = (string)val;
			if (name != null)
				return name;

			GLib.AppInfo app_info = this.get_app_info();
			if (app_info != null) {
				app_id = app_info.get_id();
				return AppChooserButton.APP_PREDEFINED;
			}
		}

		return APP_DEFAULT;
	}
}

} /* namespace Crown */
