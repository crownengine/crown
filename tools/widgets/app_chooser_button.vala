/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class AppChooserButton : Gtk.Box
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
	public Gtk.AppChooserButton _app_chooser_button;

	public AppChooserButton(string mime_type)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL);

		_app_chooser_button = new Gtk.AppChooserButton(mime_type);
		_app_chooser_button.append_custom_item(APP_DEFAULT, "Open by extension", null);
		_app_chooser_button.set_active_custom_item(APP_DEFAULT);
#if CROWN_PLATFORM_LINUX
		_app_chooser_button.show_dialog_item = true;
#endif

#if CROWN_GTK3
		_app_chooser_button.scroll_event.connect(() => {
				GLib.Signal.stop_emission_by_name(_app_chooser_button, "scroll-event");
				return Gdk.EVENT_PROPAGATE;
			});
#else
		_controller_scroll = new Gtk.EventControllerScroll(Gtk.EventControllerScrollFlags.BOTH_AXES);
		_controller_scroll.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
		_controller_scroll.scroll.connect(() => {
				// Do nothing, just consume the event to stop
				// the annoying scroll default behavior.
			});
		_app_chooser_button.add_controller(_controller_scroll);
#endif

		this.pack_start(_app_chooser_button);
	}

	public GLib.AppInfo? get_app_info()
	{
		return _app_chooser_button.get_app_info();
	}

	/// Sets the app to @a app_name. If @a app_name is APP_PREDEFINED, it tries
	/// to set the predefined app based @a app_id.
	public void set_app(string app_name, string? app_id)
	{
		if (app_name != APP_PREDEFINED) {
			_app_chooser_button.set_active_custom_item(app_name);
			return;
		}

		if (app_id == null) {
			_app_chooser_button.set_active_custom_item(APP_DEFAULT);
			return;
		}

		_app_chooser_button.model.foreach((model, path, iter) => {
				Value val;
				model.get_value(iter, ModelColumn.APP_INFO, out val);

				GLib.AppInfo app_info = (GLib.AppInfo)val;
				if (app_info != null && app_info.get_id() == app_id) {
					_app_chooser_button.set_active_iter(iter);
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
		if (_app_chooser_button.get_active_iter(out iter)) {
			Value val;
			_app_chooser_button.model.get_value(iter, ModelColumn.NAME, out val);
			string name = (string)val;
			if (name != null)
				return name;

			GLib.AppInfo app_info = _app_chooser_button.get_app_info();
			if (app_info != null) {
				app_id = app_info.get_id();
				return AppChooserButton.APP_PREDEFINED;
			}
		}

		return APP_DEFAULT;
	}
}

} /* namespace Crown */
