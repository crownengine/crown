/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputString : InputField, Gtk.Entry
{
	public bool _inconsistent;
	public string _value;
	public Gtk.GestureMultiPress _gesture_click;

	public void set_inconsistent(bool inconsistent)
	{
		if (_inconsistent != inconsistent) {
			_inconsistent = inconsistent;

			if (_inconsistent) {
				this.text = INCONSISTENT_LABEL;
			} else {
				this.text = _value;
			}
		}
	}

	public bool is_inconsistent()
	{
		return _inconsistent;
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

		_gesture_click = new Gtk.GestureMultiPress(this);
		_gesture_click.pressed.connect(on_button_pressed);
		_gesture_click.released.connect(on_button_released);

		this.activate.connect(on_activate);
		this.focus_in_event.connect(on_focus_in);
		this.focus_out_event.connect(on_focus_out);
	}

	public void on_button_pressed(int n_press, double x, double y)
	{
		this.grab_focus();
	}

	public void on_button_released(int n_press, double x, double y)
	{
		uint button = _gesture_click.get_current_button();

		if (button == Gdk.BUTTON_PRIMARY) {
			if (_inconsistent)
				this.text = "";
			else
				this.text = _value;

			GLib.Idle.add(() => {
					this.set_position(-1);
					this.select_region(0, -1);
					return GLib.Source.REMOVE;
				});
		}
	}

	public void on_activate()
	{
		this.select_region(0, 0);
		this.set_position(-1);
		set_value_safe(this.text);
	}

	public bool on_focus_in(Gdk.EventFocus ev)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(this);

		if (_inconsistent)
			this.text = "";
		else
			this.text = _value;

		this.set_position(-1);
		this.select_region(0, -1);

		return Gdk.EVENT_PROPAGATE;
	}

	public bool on_focus_out(Gdk.EventFocus ef)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_out(this);

		if (_inconsistent) {
			if (this.text != "") {
				set_value_safe(this.text);
			} else {
				this.text = INCONSISTENT_LABEL;
			}
		} else {
			set_value_safe(this.text);
		}

		this.select_region(0, 0);

		return Gdk.EVENT_PROPAGATE;
	}

	public virtual void set_value_safe(string text)
	{
		this.text = text;

		_inconsistent = false;

		// Notify value changed.
		if (_value != text) {
			_value = text;
			value_changed(this);
		}
	}
}

} /* namespace Crown */
