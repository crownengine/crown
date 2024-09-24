/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class EntryText : Gtk.Entry, Property
{
	// Data
	public bool _inconsistent;
	public bool _stop_emit;
	public string _value;

	// Signals
	public signal void value_changed();

	public void set_inconsistent(bool inconsistent)
	{
		if (_inconsistent != inconsistent) {
			_inconsistent = inconsistent;

			_stop_emit = true;
			if (_inconsistent) {
				this.text = INCONSISTENT_LABEL;
			} else {
				this.text = _value;
			}
			_stop_emit = false;
		}
	}

	public bool is_inconsistent()
	{
		return _inconsistent;
	}

	public Value? generic_value()
	{
		return this.value;
	}

	public void set_generic_value(Value? val)
	{
		this.value = (string)val;
	}

	public string value
	{
		get
		{
			return _value;
		}
		set
		{
			_stop_emit = true;
			set_value_safe(value);
			_stop_emit = false;
		}
	}

	public EntryText()
	{
		_stop_emit = false;
		_inconsistent = false;
		_value = "";

		this.activate.connect(on_activate);
		this.button_press_event.connect(on_button_press);
		this.button_release_event.connect(on_button_release);
		this.focus_in_event.connect(on_focus_in);
		this.focus_out_event.connect(on_focus_out);
	}

	private bool on_button_press(Gdk.EventButton ev)
	{
		this.grab_focus();

		return Gdk.EVENT_PROPAGATE;
	}

	private bool on_button_release(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_PRIMARY) {
			if (_inconsistent)
				this.text = "";
			else
				this.text = _value;

			this.set_position(-1);
			this.select_region(0, -1);

			return Gdk.EVENT_STOP;
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private void on_activate()
	{
		this.select_region(0, 0);
		this.set_position(-1);
		set_value_safe(this.text);
	}

	private bool on_focus_in(Gdk.EventFocus ev)
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

	private bool on_focus_out(Gdk.EventFocus ef)
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

	protected virtual void set_value_safe(string text)
	{
		this.text = text;

		_inconsistent = false;

		// Notify value changed.
		if (_value != text) {
			_value = text;
			if (!_stop_emit)
				value_changed();
		}
	}
}

} /* namespace Crown */
