/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputString : InputField
{
	public bool _inconsistent;
	public string _value;
	public Gtk.Entry _entry;
	public Gtk.GestureMultiPress _gesture_click;

	public override void set_inconsistent(bool inconsistent)
	{
		if (_inconsistent != inconsistent) {
			_inconsistent = inconsistent;

			if (_inconsistent) {
				_entry.text = INCONSISTENT_LABEL;
			} else {
				_entry.text = _value;
			}
		}
	}

	public override bool is_inconsistent()
	{
		return _inconsistent;
	}

	public override GLib.Value union_value()
	{
		return this.value;
	}

	public override void set_union_value(GLib.Value v)
	{
		this.value = (string)v;
	}

	public string value
	{
		get
		{
			return _value;
		}
		set
		{
			set_value_safe(value);
		}
	}

	public InputString()
	{
		_inconsistent = false;
		_value = "";

		_entry = new Gtk.Entry();

		_gesture_click = new Gtk.GestureMultiPress(_entry);
		_gesture_click.pressed.connect(on_button_pressed);
		_gesture_click.released.connect(on_button_released);

		_entry.activate.connect(on_activate);
		_entry.focus_in_event.connect(on_focus_in);
		_entry.focus_out_event.connect(on_focus_out);

		this.add(_entry);
	}

	public void on_button_pressed(int n_press, double x, double y)
	{
		_entry.grab_focus();
	}

	public void on_button_released(int n_press, double x, double y)
	{
		uint button = _gesture_click.get_current_button();

		if (button == Gdk.BUTTON_PRIMARY) {
			if (_inconsistent)
				_entry.text = "";
			else
				_entry.text = _value;

			GLib.Idle.add(() => {
					_entry.set_position(-1);
					_entry.select_region(0, -1);
					return GLib.Source.REMOVE;
				});
		}
	}

	public void on_activate()
	{
		_entry.select_region(0, 0);
		_entry.set_position(-1);
		set_value_safe(_entry.text);
	}

	public bool on_focus_in(Gdk.EventFocus ev)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(_entry);

		if (_inconsistent)
			_entry.text = "";
		else
			_entry.text = _value;

		_entry.set_position(-1);
		_entry.select_region(0, -1);

		return Gdk.EVENT_PROPAGATE;
	}

	public bool on_focus_out(Gdk.EventFocus ef)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_out(_entry);

		if (_inconsistent) {
			if (_entry.text != "") {
				set_value_safe(_entry.text);
			} else {
				_entry.text = INCONSISTENT_LABEL;
			}
		} else {
			set_value_safe(_entry.text);
		}

		_entry.select_region(0, 0);

		return Gdk.EVENT_PROPAGATE;
	}

	public virtual void set_value_safe(string text)
	{
		_entry.text = text;

		_inconsistent = false;

		// Notify value changed.
		if (_value != text) {
			_value = text;
			value_changed(this);
		}
	}
}

} /* namespace Crown */
