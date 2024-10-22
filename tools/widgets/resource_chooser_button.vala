/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class ResourceChooserButton : Gtk.Box
{
	// Data
	public string _type;

	// Widgets
	public EntryText _name;
	public Gtk.Button _selector;
	public Gtk.Button _revealer;
	public ProjectStore _project_store;

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

	// Signals
	public signal void value_changed();

	public ResourceChooserButton(ProjectStore store, string type)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);

		// Data
		_type = type;

		// Widgets
		_name = new EntryText();
		_name.set_editable(false);
		_name.hexpand = true;
		this.pack_start(_name, true, true);

		_revealer = new Gtk.Button.from_icon_name("go-jump-symbolic");
		_revealer.clicked.connect(on_revealer_clicked);
		this.pack_end(_revealer, false);

		_selector = new Gtk.Button.from_icon_name("document-open-symbolic");
		_selector.clicked.connect(on_selector_clicked);
		this.pack_end(_selector, false);

		_project_store = store;
	}

	private void on_selector_clicked()
	{
		Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Select Resource"
			, (Gtk.Window)this.get_toplevel()
			, DialogFlags.MODAL
			, null
			);

		var rb = new ResourceChooser(null, _project_store);
		rb.set_type_filter(type_filter);
		rb.resource_selected.connect(() => {
				_name.value = rb._name;
				value_changed();
				dg.response(ResponseType.OK);
			});

		dg.key_press_event.connect((ev) => {
				if (ev.keyval == Gdk.Key.Escape) {
					dg.destroy();
					return Gdk.EVENT_STOP;
				}

				return Gdk.EVENT_PROPAGATE;
			});
		dg.skip_taskbar_hint = true;
		dg.get_content_area().pack_start(rb, true, true, 0);
		dg.show_all();
		dg.run();
		dg.destroy();
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
}

} /* namespace Crown */
