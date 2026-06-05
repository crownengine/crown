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
	public InputString _name;
	public Gtk.Button _selector;
	public Gtk.EventControllerKey _controller_key;
	public Gtk.Box _box;

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
			string? old_path = _path;

			if (value == null) {
				_path = null;
				_name.value = _("(None)");
			} else {
				GLib.File f = GLib.File.new_for_path(value);
				_path = f.get_path();
				_name.value = f.get_basename();
			}

			if (_path != old_path)
				value_changed(this);
		}
	}

	public InputFile(Gtk.FileChooserAction action = Gtk.FileChooserAction.OPEN)
	{
		_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);

		_path = null;
		_action = action;

		_name = new InputString();
		_name._entry.set_editable(false);
		_name.hexpand = true;
		_box.pack_start(_name, true, true);

		_selector = new Gtk.Button.from_icon_name("document-open-symbolic");
		_selector.clicked.connect(on_selector_clicked);
		_selector.set_can_focus(false);
		_box.pack_end(_selector, false);

		_controller_key = new Gtk.EventControllerKey(_name._entry);
		_controller_key.key_pressed.connect(on_key_pressed);

		this.value = null;
		this.add(_box);
	}

	public void on_selector_clicked()
	{
		string label = _action == Gtk.FileChooserAction.SELECT_FOLDER ? _("Folder") : _("File");
		Gtk.FileChooserDialog dlg = new Gtk.FileChooserDialog(_("Select %s").printf(label)
			, (Gtk.Window)this.get_toplevel()
			, _action
			, _("Cancel")
			, Gtk.ResponseType.CANCEL
			, _("Open")
			, Gtk.ResponseType.ACCEPT
			);

		dlg.response.connect((response_id) => {
				if (response_id == Gtk.ResponseType.ACCEPT)
					this.value = dlg.get_file().get_path();
				dlg.destroy();
			});
		dlg.show_all();
	}

	public bool on_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval == Gdk.Key.Delete)
			this.value = null;

		return Gdk.EVENT_PROPAGATE;
	}
}

} /* namespace Crown */
