/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputObject : InputField
{
	public const Gtk.TargetEntry[] DND_TARGETS =
	{
		{ "GUID", Gtk.TargetFlags.SAME_APP, TargetInfo.GUID },
	};

	public const string UNSET_OBJECT = "(None)";
	public const string MISSING_OBJECT = "(Missing)";

	public StringId64 _type;
	public Guid _object;
	public Database _database;
	public InputString _name;
	public Gtk.Button _selector;
	public SelectObjectDialog _dialog;
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
		this.value = (Guid)v;
	}

	public new Guid value
	{
		get
		{
			return _object;
		}
		set
		{
			_object = value;

			if (_object == GUID_ZERO) {
				_name.value = UNSET_OBJECT;
			} else if (!_database.is_alive(_object)) {
				_name.value = MISSING_OBJECT;
			} else {
				Aspect? name_aspect = _database.get_aspect(StringId64(_database.object_type(value)), StringId64("name"));
				if (name_aspect == null)
					name_aspect = default_name_aspect;

				string object_name;
				name_aspect(out object_name, _database, _object);
				_name.value = object_name;
			}
		}
	}

	public InputObject(StringId64 type, Database database)
	{
		_type = type;
		_database = database;

		_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);

		_name = new InputString();
		_name._entry.set_editable(false);
		_name.hexpand = true;
		_name.value_changed.connect(on_name_value_changed);
		_box.pack_start(_name, true, true);

		_selector = new Gtk.Button.from_icon_name("document-open-symbolic");
		_selector.set_tooltip_text("Select a resource.");
		_selector.clicked.connect(on_selector_clicked);
		_selector.set_can_focus(false);
		_box.pack_end(_selector, false);

		_controller_key = new Gtk.EventControllerKey(_name._entry);
		_controller_key.key_pressed.connect(on_key_pressed);

		Gtk.drag_dest_set(_name._entry
			, Gtk.DestDefaults.MOTION
			| Gtk.DestDefaults.HIGHLIGHT
			, DND_TARGETS
			, Gdk.DragAction.COPY
			);
		_name._entry.drag_data_received.connect(on_drag_data_received);
		_name._entry.drag_motion.connect(on_drag_motion);
		_name._entry.drag_drop.connect(on_drag_drop);

		this.value = GUID_ZERO;
		this.add(_box);
	}

	public void on_selector_clicked()
	{
		if (_dialog == null) {
			_dialog = ((LevelEditorApplication)GLib.Application.get_default()).new_select_object_dialog(_type, _database);
			_dialog.object_selected.connect(on_select_object_dialog_object_selected);
		}

		_dialog.show_all();
		_dialog.present();
	}

	public void on_select_object_dialog_object_selected(Guid id)
	{
		this.value = id;
		_dialog.hide();
	}

	public void on_name_value_changed()
	{
		value_changed(this);
	}

	public bool on_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval == Gdk.Key.Delete)
			this.value = GUID_ZERO;

		return Gdk.EVENT_PROPAGATE;
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
		bool success = false;
		unowned uint8[] raw_data = data.get_data_with_length();

		if (raw_data.length != -1) {
			Guid object_id = GUID_ZERO;
			Memory.copy(&object_id, raw_data, sizeof(Guid));

			StringId64 object_type = StringId64(_database.object_type(object_id));
			if (object_type == _type) {
				this.value = object_id;
				success = true;
			}
		}

		Gtk.drag_finish(context, success, false, time_);
	}
}

} /* namespace Crown */
