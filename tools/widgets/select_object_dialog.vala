/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class SelectObjectDialog : Gtk.Window
{
	public StringId64 _object_type;
	public Database _database;
	public Gtk.EventControllerKey _controller_key;
	public ObjectChooser _chooser;
	public Gtk.HeaderBar _header_bar;

	public signal void object_selected(Guid id);

	public SelectObjectDialog(StringId64 obj_type, Database database, Gtk.Window? parent)
	{
		_object_type = obj_type;
		_database = database;

		this.set_icon_name(CROWN_EDITOR_ICON_NAME);

		if (parent != null) {
			this.set_transient_for(parent);
			this.set_modal(true);
		}
		this.delete_event.connect(on_close);

		_controller_key = new Gtk.EventControllerKey(this);
		_controller_key.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
		_controller_key.key_pressed.connect((keyval) => {
				if (keyval == Gdk.Key.Escape) {
					this.close();
					return Gdk.EVENT_STOP;
				}

				return Gdk.EVENT_PROPAGATE;
			});

		_header_bar = new Gtk.HeaderBar();
		_header_bar.title = "Select a %s".printf(_database.type_name(obj_type));
		_header_bar.show_close_button = true;
		this.set_titlebar(_header_bar);

		_chooser = new ObjectChooser(database);
		_chooser.set_type_filter(on_object_chooser_filter);
		_chooser.object_selected.connect(on_object_chooser_object_selected);
		this.add(_chooser);
	}

	public bool on_close()
	{
		this.hide();
		return Gdk.EVENT_STOP;
	}

	public bool on_object_chooser_filter(Guid id)
	{
		if (!_database.is_alive(id))
			return false;

		StringId64 type = StringId64(_database.object_type(id));
		return type == _object_type;
	}

	public void on_object_chooser_object_selected(Guid id)
	{
		object_selected(id);
	}
}

} /* namespace Crown */
