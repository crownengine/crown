/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputResource : InputField
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
		_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);

		// Data
		_type = type;
		_name_unset = true;

		// Widgets
		_name = new InputString();
		_name._entry.set_editable(false);
		_name.hexpand = true;
		_name.value_changed.connect(on_name_value_changed);
		_box.pack_start(_name, true, true);

		_revealer = new Gtk.Button.from_icon_name("go-jump-symbolic");
		_revealer.clicked.connect(on_revealer_clicked);
		_revealer.set_tooltip_text("Reveal in the project browser.");
		_revealer.set_can_focus(false);
		_box.pack_end(_revealer, false);

		_selector = new Gtk.Button.from_icon_name("document-open-symbolic");
		_selector.set_tooltip_text("Select a resource.");
		_selector.clicked.connect(on_selector_clicked);
		_selector.set_can_focus(false);
		_box.pack_end(_selector, false);

		Gtk.drag_dest_set(_name._entry
			, Gtk.DestDefaults.MOTION
			| Gtk.DestDefaults.HIGHLIGHT
			, dnd_targets
			, Gdk.DragAction.COPY
			);
		_name._entry.drag_data_received.connect(on_drag_data_received);
		_name._entry.drag_motion.connect(on_drag_motion);
		_name._entry.drag_drop.connect(on_drag_drop);

		this.value = null;

		db._project.file_added.connect(on_file_added_or_changed);
		db._project.file_changed.connect(on_file_added_or_changed);
		db._project.file_removed.connect(on_file_removed);

		this.add(_box);
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

	public bool on_drag_motion(Gdk.DragContext context, int x, int y, uint time_)
	{
		Gdk.Atom target = Gtk.drag_dest_find_target(_name._entry, context, null);
		Gdk.drag_status(context, target != Gdk.Atom.NONE ? Gdk.DragAction.COPY : 0, time_);
		return true;
	}

	public bool on_drag_drop(Gdk.DragContext context, int x, int y, uint time_)
	{
		Gdk.Atom target = Gtk.drag_dest_find_target(_name._entry, context, null);
		if (target == Gdk.Atom.NONE)
			return false;

		Gtk.drag_get_data(_name._entry, context, target, time_);
		return true;
	}

	public void on_drag_data_received(Gdk.DragContext context, int x, int y, Gtk.SelectionData data, uint info, uint time_)
	{
		unowned uint8[] raw_data = data.get_data_with_length();
		bool success = false;

		if (raw_data.length != -1) {
			string resource_path = (string)raw_data;
			if (ResourceId.type(resource_path) == _type) {
				string? resource_name = ResourceId.name(resource_path);
				if (resource_name != null) {
					this.value = resource_name;
					success = true;
				}
			}
		}

		Gtk.drag_finish(context, success, false, time_);
	}
}

} /* namespace Crown */
