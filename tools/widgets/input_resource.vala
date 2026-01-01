/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputResource : InputField, Gtk.Box
{
	public const string UNSET_RESOURCE = "(None)";

	// Data
	public string _type;
	public bool _name_unset;

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
		this.value = (string?)v;
	}

	public string? value
	{
		get
		{
			return _name_unset ? null : _name.value;
		}
		set
		{
			_name_unset = value == null;
			_name.value = value != null ? value : UNSET_RESOURCE;
			_revealer.sensitive = value != null;
		}
	}

	public InputResource(string type, Database db)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);

		// Data
		_type = type;
		_name_unset = true;

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

		this.value = null;

		db._project.file_added.connect(on_file_added_or_changed);
		db._project.file_changed.connect(on_file_added_or_changed);
		db._project.file_removed.connect(on_file_removed);
	}

	public void on_selector_clicked()
	{
		if (_dialog == null) {
			_dialog = ((LevelEditorApplication)GLib.Application.get_default()).new_select_resource_dialog(_type);
			_dialog.resource_selected.connect(on_select_resource_dialog_resource_selected);
		}

		_dialog.show_all();
		_dialog.present();
	}

	public void on_select_resource_dialog_resource_selected(string type, string name)
	{
		this.value = name;
		_dialog.hide();
	}

	public void on_revealer_clicked()
	{
		var tuple = new GLib.Variant.tuple({_type, this.value});
		GLib.Application.get_default().activate_action("reveal-resource", tuple);
	}

	public void on_name_value_changed()
	{
		value_changed(this);
	}

	public void on_file_added_or_changed(string type, string name, uint64 size, uint64 mtime)
	{
		if (type == _type && name == _name.value)
			value_changed(this);
	}

	public void on_file_removed(string type, string name)
	{
		if (type == _type && name == _name.value)
			value_changed(this);
	}
}

} /* namespace Crown */
