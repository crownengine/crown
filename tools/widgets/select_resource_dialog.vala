/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class SelectResourceDialog : Gtk.Window
{
	public string _resource_type;
	public Gtk.EventControllerKey _controller_key;
	public ResourceChooser _chooser;
	public Gtk.HeaderBar _header_bar;

	public signal void resource_selected(string type, string name);

	public SelectResourceDialog(string resource_type, ProjectStore project_store, Gtk.Window? parent)
	{
		_resource_type = resource_type;

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
		_header_bar.title = "Select a %s".printf(resource_type);
		_header_bar.show_close_button = true;
		this.set_titlebar(_header_bar);

		_chooser = new ResourceChooser(null, project_store);
		_chooser.set_type_filter(on_resource_chooser_filter);
		_chooser.resource_selected.connect(on_resource_chooser_resource_selected);
		this.add(_chooser);
	}

	public bool on_close()
	{
		this.hide();
		return Gdk.EVENT_STOP;
	}

	public bool on_resource_chooser_filter(string type, string name)
	{
		return _resource_type == type;
	}

	public void on_resource_chooser_resource_selected(string type, string name)
	{
		resource_selected(type, name);
	}
}

} /* namespace Crown */
