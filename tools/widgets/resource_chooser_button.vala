/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class ResourceChooserButton : Gtk.Box
{
	// Data
	public bool _stop_emit;
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
			return _name.text;
		}
		set
		{
			_stop_emit = true;
			_name.text = value;
			_stop_emit = false;
		}
	}

	// Signals
	public signal void value_changed();

	public ResourceChooserButton(ProjectStore store, string type)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);

		// Data
		_stop_emit = false;
		_type = type;

		// Widgets
		_name = new EntryText();
		_name.sensitive = false;
		_name.hexpand = true;
		_name.changed.connect(on_value_changed);
		this.pack_start(_name, true, true);

		_revealer = new Gtk.Button.from_icon_name("go-jump-symbolic");
		_revealer.clicked.connect(on_revealer_clicked);
		this.pack_end(_revealer, false);

		_selector = new Gtk.Button.from_icon_name("document-open-symbolic");
		_selector.clicked.connect(on_selector_clicked);
		this.pack_end(_selector, false);

		_project_store = store;
	}

	private void on_value_changed()
	{
		if (!_stop_emit)
			value_changed();
	}

	private void on_selector_clicked()
	{
		Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Select Resource"
			, (Gtk.Window)this.get_toplevel()
			, DialogFlags.MODAL
			, null
			);

		var rb = new ResourceChooser(null, _project_store, false);
		rb.set_type_filter(type_filter);
		rb.resource_selected.connect(() => { _name.text = rb._name; dg.response(ResponseType.OK); });

		dg.skip_taskbar_hint = true;
		dg.get_content_area().pack_start(rb, true, true, 0);
		dg.show_all();
		dg.run();
		dg.destroy();
	}

	private void on_revealer_clicked()
	{
		Gtk.Application app = ((Gtk.Window)this.get_toplevel()).application;
		app.activate_action("project-browser-reveal", new GLib.Variant.string(_name.text + "." + _type));
	}

	private bool type_filter(string type, string name)
	{
		return _type == type;
	}
}

}
