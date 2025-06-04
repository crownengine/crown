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
#if CROWN_GTK3
	public Gtk.Entry _name;
#else
	public InputString _name;
#endif
	public Gtk.Button _selector;
	public Gtk.EventControllerKey _controller_key;
#if CROWN_GTK3
	public Gtk.GestureMultiPress _gesture_click;
#endif
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
#if CROWN_GTK3
				_name.text = _("(None)");
#else
				_name.value = _("(None)");
#endif
			} else {
				GLib.File f = GLib.File.new_for_path(value);
				_path = f.get_path();
#if CROWN_GTK3
				_name.text = f.get_basename();
#else
				_name.value = f.get_basename();
#endif
			}

			if (_path != old_path)
				value_changed(this);
		}
	}

	public InputFile(Gtk.FileChooserAction action = Gtk.FileChooserAction.OPEN)
	{
		_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
#if CROWN_GTK3
		_box.get_style_context().add_class(Gtk.STYLE_CLASS_LINKED);
#else
		_box.add_css_class("linked");
#endif

		_path = null;
		_action = action;

#if CROWN_GTK3
		_name = new Gtk.Entry();
		_name.set_width_chars(0);
		_name.set_editable(false);
#else
		_name = new InputString();
		_name._entry.set_editable(false);
#endif
		_name.hexpand = true;
#if CROWN_GTK3
		_name.activate.connect(on_name_activate);
		_name.focus_in_event.connect(on_name_focus_in);
		_name.focus_out_event.connect(on_name_focus_out);
		_box.pack_start(_name, true, true);

		_gesture_click = new Gtk.GestureMultiPress(_name);
		_gesture_click.pressed.connect(on_name_button_pressed);
		_gesture_click.released.connect(on_name_button_released);
#else
		_box.append(_name);
#endif

		_selector = new Gtk.Button.from_icon_name("document-open-symbolic");
		_selector.clicked.connect(on_selector_clicked);
		_selector.set_can_focus(false);
#if CROWN_GTK3
		_box.pack_end(_selector, false);

		_controller_key = new Gtk.EventControllerKey(_name);
#else
		_box.append(_selector);

		_controller_key = new Gtk.EventControllerKey();
#endif
		_controller_key.key_pressed.connect(on_key_pressed);
#if !CROWN_GTK3
		_name._entry.add_controller(_controller_key);
#endif

		this.value = null;
#if CROWN_GTK3
		this.add(_box);
#else
		this.set_child(_box);
#endif
	}

	public void on_selector_clicked()
	{
		string label = _action == Gtk.FileChooserAction.SELECT_FOLDER ? _("Folder") : _("File");
		Gtk.FileChooserDialog dlg = new Gtk.FileChooserDialog(_("Select %s").printf(label)
#if CROWN_GTK3
			, (Gtk.Window)this.get_toplevel()
#else
			, (Gtk.Window)this.get_root()
#endif
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
#if CROWN_GTK3
		dlg.show_all();
#else
		dlg.present();
#endif
	}

#if CROWN_GTK3
	public void on_name_button_pressed(int n_press, double x, double y)
	{
		_name.grab_focus();
	}
#endif

#if CROWN_GTK3
	public void on_name_button_released(int n_press, double x, double y)
	{
		if (_gesture_click.get_current_button() == Gdk.BUTTON_PRIMARY)
			GLib.Idle.add(on_name_select);
	}
#endif

#if CROWN_GTK3
	public bool on_name_select()
	{
		_name.set_position(-1);
		_name.select_region(0, -1);
		return GLib.Source.REMOVE;
	}
#endif

#if CROWN_GTK3
	public void on_name_activate()
	{
		_name.select_region(0, 0);
		_name.set_position(-1);
	}
#endif

#if CROWN_GTK3
	public bool on_name_focus_in(Gdk.EventFocus ev)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(_name);

		_name.set_position(-1);
		_name.select_region(0, -1);

		return Gdk.EVENT_PROPAGATE;
	}
#endif

#if CROWN_GTK3
	public bool on_name_focus_out(Gdk.EventFocus ev)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_out(_name);

		_name.select_region(0, 0);

		return Gdk.EVENT_PROPAGATE;
	}
#endif

	public bool on_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval == Gdk.Key.Delete)
			this.value = null;

		return Gdk.EVENT_PROPAGATE;
	}
}

} /* namespace Crown */
