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
	public ProjectStore _project_store;
	public ResourceChooser _chooser;
	public Gtk.Dialog _dialog;
	public Gtk.EventControllerKey _dialog_controller_key;

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

	public InputResource(ProjectStore store, string type)
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

		_project_store = store;

		_chooser = new ResourceChooser(null, _project_store);
		_chooser.set_type_filter(type_filter);

		_project_store._project.file_added.connect(on_file_added_or_changed);
		_project_store._project.file_changed.connect(on_file_added_or_changed);
		_project_store._project.file_removed.connect(on_file_removed);
	}

	~InputResource()
	{
		// Prevents a crash when the parent window gets destroyed.
		_chooser.set_type_filter((type, name) => { return false; });
	}

	private void on_selector_clicked()
	{
		if (_dialog == null) {
			_dialog = new Gtk.Dialog.with_buttons("Select a %s".printf(_type)
				, (Gtk.Window)this.get_toplevel()
				, Gtk.DialogFlags.MODAL
				, null
				);
			_dialog.delete_event.connect(_dialog.hide_on_delete);

			_chooser.resource_selected.connect(() => {
					_name.value = _chooser._name;
					_dialog.hide();
				});

			_dialog_controller_key = new Gtk.EventControllerKey(_dialog);
			_dialog_controller_key.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
			_dialog_controller_key.key_pressed.connect((keyval) => {
					if (keyval == Gdk.Key.Escape) {
						_dialog.hide();
						return Gdk.EVENT_STOP;
					}

					return Gdk.EVENT_PROPAGATE;
				});
			_dialog.skip_taskbar_hint = true;
			_dialog.get_content_area().pack_start(_chooser, true, true, 0);
		}

		_dialog.show_all();
		_dialog.present();
	}

	private void on_revealer_clicked()
	{
		var tuple = new GLib.Variant.tuple({_type, _name.value});
		GLib.Application.get_default().activate_action("reveal-resource", tuple);
	}

	private bool type_filter(string type, string name)
	{
		return _type == type;
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
