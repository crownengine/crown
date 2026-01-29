/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputFile : InputField
{
	public string? _path;
	public Gtk.FileChooserAction _action;
	public Gtk.Label _label;
	public Gtk.Button _button;

	public override void set_inconsistent(bool inconsistent)
	{
	}

	public override bool is_inconsistent()
	{
		return false;
	}

	public override GLib.Value union_value()
	{
		return this.value;
	}

	public override void set_union_value(GLib.Value v)
	{
		this.value = (string)v;
	}

	public string? value
	{
		get
		{
			return _path;
		}
		set
		{
			if (value == null) {
				_path = null;
				_label.set_text("(None)");
			} else {
				GLib.File f = GLib.File.new_for_path(value);
				_path = f.get_path();
				_label.set_text(f.get_basename());
			}
		}
	}

	public InputFile(Gtk.FileChooserAction action = Gtk.FileChooserAction.OPEN)
	{
		_path = null;
		_action = action;

		_label = new Gtk.Label("(None)");
		_label.xalign = 0.0f;

		_button = new Gtk.Button();
		_button.add(_label);
		_button.clicked.connect(on_selector_clicked);

		this.add(_button);
	}

	public void on_selector_clicked()
	{
		string label = _action == Gtk.FileChooserAction.SELECT_FOLDER ? "Folder" : "File";
		Gtk.FileChooserDialog dlg = new Gtk.FileChooserDialog("Select %s".printf(label)
			, (Gtk.Window)this.get_toplevel()
			, _action
			, "Cancel"
			, Gtk.ResponseType.CANCEL
			, "Open"
			, Gtk.ResponseType.ACCEPT
			);

		dlg.response.connect((response_id) => {
				if (response_id == Gtk.ResponseType.ACCEPT)
					this.value = dlg.get_file().get_path();
				dlg.destroy();
			});
		dlg.show_all();
	}
}

} /* namespace Crown */
