/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
#if CROWN_GTK3
public class SaveResourceDialog : Gtk.FileChooserDialog
#else
public class SaveResourceDialog : Gtk.Dialog
#endif
{
#if !CROWN_GTK3
	public Gtk.FileChooserWidget _file_chooser;
#endif
	public Project _project;
	public string _resource_type;

	public signal void safer_response(int response_id, string? path);

	public SaveResourceDialog(string? title, Gtk.Window? parent, string resource_type, string resource_name, Project p)
	{
		if (title != null)
			this.title = title;

		if (parent != null)
			this.set_transient_for(parent);

#if CROWN_GTK3
		this.set_action(Gtk.FileChooserAction.SAVE);
#else
		this.set_modal(true);
#endif
		this.add_button(_("Cancel"), Gtk.ResponseType.CANCEL);
		this.add_button(_("Save"), Gtk.ResponseType.ACCEPT);
#if !CROWN_GTK3
		this.response.connect(on_response);

		_file_chooser = new Gtk.FileChooserWidget(Gtk.FileChooserAction.SAVE);
#endif
		try {
#if CROWN_GTK3
			this.set_current_folder_file(GLib.File.new_for_path(p.source_dir()));
#else
			_file_chooser.set_current_folder(GLib.File.new_for_path(p.source_dir()));
#endif
		} catch (GLib.Error e) {
			loge(e.message);
		}
#if CROWN_GTK3
		this.set_current_name(resource_name);
		this.set_modal(true);
		this.response.connect(on_response);
#else
		_file_chooser.set_current_name(resource_name);
#endif

		Gtk.FileFilter ff = new Gtk.FileFilter();
		ff.set_filter_name("%s (*.%s)".printf(resource_type, resource_type));
		ff.add_pattern("*.%s".printf(resource_type));
#if CROWN_GTK3
		this.add_filter(ff);
#else
		_file_chooser.add_filter(ff);

		this.get_content_area().append(_file_chooser);
#endif

		_project = p;
		_resource_type = resource_type;
	}

	public void on_response(int response_id)
	{
#if CROWN_GTK3
		GLib.File? file = this.get_file();
		if (file == null)
			return;

		string? path = file.get_path();
#else
		GLib.File? file = _file_chooser.get_file();
		string? path = file == null ? null : file.get_path();
#endif

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
					, _("The file must be within the source directory.")
					);
				md.set_default_response(Gtk.ResponseType.OK);
				md.response.connect(() => {
						try {
#if CROWN_GTK3
							this.set_current_folder_file(GLib.File.new_for_path(_project.source_dir()));
#else
							_file_chooser.set_current_folder(GLib.File.new_for_path(_project.source_dir()));
#endif
						} catch (GLib.Error e) {
							loge(e.message);
						}
						md.destroy();
					});
#if CROWN_GTK3
				md.show_all();
#else
				md.show();
#endif
				return;
			}

			// If the path already exits, ask if it should be overwritten.
			if (GLib.FileUtils.test(path, FileTest.EXISTS)) {
				Gtk.MessageDialog md = new Gtk.MessageDialog(this
					, Gtk.DialogFlags.MODAL
					, Gtk.MessageType.QUESTION
					, Gtk.ButtonsType.NONE
					, _("A file named `%s` already exists.\nOverwrite?").printf(GLib.Path.get_basename(path))
					);

				Gtk.Widget btn;
				md.add_button(_("_No"), Gtk.ResponseType.NO);
				btn = md.add_button(_("_Yes"), Gtk.ResponseType.YES);
#if CROWN_GTK3
				btn.get_style_context().add_class("destructive-action");
#else
				btn.add_css_class("destructive-action");
#endif

				md.set_default_response(Gtk.ResponseType.NO);
				md.response.connect((response_id) => {
						if (response_id == Gtk.ResponseType.YES)
							this.safer_response(Gtk.ResponseType.ACCEPT, path);
						md.destroy();
					});
#if CROWN_GTK3
				md.show_all();
#else
				md.show();
#endif
				return;
			}
		}

		this.safer_response(response_id, path);
	}
}

} /* namespace Crown */
