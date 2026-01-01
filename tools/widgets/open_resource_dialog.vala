/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class OpenResourceDialog : Gtk.FileChooserDialog
{
	public Project _project;
	public string _resource_type;

	public signal void safer_response(int response_id, string? path);

	public OpenResourceDialog(string? title, Gtk.Window? parent, string resource_type, Project p)
	{
		if (title != null)
			this.title = title;

		if (parent != null)
			this.set_transient_for(parent);

		this.set_action(Gtk.FileChooserAction.OPEN);
		this.add_button("Cancel", Gtk.ResponseType.CANCEL);
		this.add_button("Open", Gtk.ResponseType.ACCEPT);
		try {
			this.set_current_folder_file(GLib.File.new_for_path(p.source_dir()));
		} catch (GLib.Error e) {
			loge(e.message);
		}
		this.set_modal(true);
		this.response.connect(on_response);

		Gtk.FileFilter ff = new Gtk.FileFilter();
		ff.set_filter_name("%s (*.%s)".printf(resource_type, resource_type));
		ff.add_pattern("*.%s".printf(resource_type));
		this.add_filter(ff);

		_project = p;
		_resource_type = resource_type;
	}

	public void on_response(int response_id)
	{
		string? path = this.get_file().get_path();

		if (response_id == Gtk.ResponseType.ACCEPT && path != null) {
			if (!path.has_suffix("." + _resource_type))
				path += "." + _resource_type;

			// If the path is outside the source dir, show a warning
			// and point the file chooser back to the source dir.
			if (!_project.path_is_within_source_dir(path)) {
				Gtk.MessageDialog md = new Gtk.MessageDialog(this
					, Gtk.DialogFlags.MODAL
					, Gtk.MessageType.WARNING
					, Gtk.ButtonsType.OK
					, "The file must be within the source directory."
					);
				md.set_default_response(Gtk.ResponseType.OK);
				md.response.connect(() => {
						try {
							this.set_current_folder_file(GLib.File.new_for_path(_project.source_dir()));
						} catch (GLib.Error e) {
							loge(e.message);
						}
						md.destroy();
					});
				md.show_all();
				return;
			}
		}

		this.safer_response(response_id, path);
	}
}

} /* namespace Crown */
