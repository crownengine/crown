/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputResource : InputField, Gtk.Box
{
	// Data
	public string _type;

	// Widgets
	public InputString _name;
	public Gtk.Button _selector;
	public Gtk.Button _revealer;
	public SelectResourceDialog _dialog;

	public void set_inconsistent(bool inconsistent)
	{
	}

	public bool is_inconsistent()
	{
		return false;
	}

	public GLib.Value union_value()
	{
		return this.value;
	}

	public void set_union_value(GLib.Value v)
	{
		this.value = (string)v;
	}

	public string value
	{
		get
		{
			return _name.value;
		}
		set
		{
			_name.value = value;
		}
	}

	public InputResource(string type, Database db)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);

		// Data
		_type = type;

		// Widgets
		_name = new InputString();
		_name.set_editable(false);
		_name.hexpand = true;
		_name.value_changed.connect(on_name_value_changed);
		this.pack_start(_name, true, true);

		_revealer = new Gtk.Button.from_icon_name("go-jump-symbolic");
		_revealer.clicked.connect(on_revealer_clicked);
		this.pack_end(_revealer, false);

		_selector = new Gtk.Button.from_icon_name("document-open-symbolic");
		_selector.clicked.connect(on_selector_clicked);
		this.pack_end(_selector, false);

		db._project.file_added.connect(on_file_added_or_changed);
		db._project.file_changed.connect(on_file_added_or_changed);
		db._project.file_removed.connect(on_file_removed);
	}

	private void on_selector_clicked()
	{
		if (_dialog == null) {
			_dialog = ((LevelEditorApplication)GLib.Application.get_default()).new_select_resource_dialog(_type);
			_dialog.resource_selected.connect(on_select_resource_dialog_resource_selected);
		}

		_dialog.show_all();
		_dialog.present();
	}

	private void on_select_resource_dialog_resource_selected(string type, string name)
	{
		_name.value = name;
		_dialog.hide();
	}

	private void on_revealer_clicked()
	{
		var tuple = new GLib.Variant.tuple({_type, _name.value});
		GLib.Application.get_default().activate_action("reveal-resource", tuple);
	}

	private void on_name_value_changed()
	{
		value_changed(this);
	}

	private void on_file_added_or_changed(string type, string name, uint64 size, uint64 mtime)
	{
		if (type == _type && name == _name.value)
			value_changed(this);
	}

	private void on_file_removed(string type, string name)
	{
		if (type == _type && name == _name.value)
			value_changed(this);
	}
}

} /* namespace Crown */
