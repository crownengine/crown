/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputDouble : InputField
{
	public bool _inconsistent;
	public double _min;
	public double _max;
	public double _value;
	public int _preview_decimals;
	public int _edit_decimals;
	public Gtk.Entry _entry;
	public Gtk.GestureMultiPress _gesture_click;
	public Gtk.EventControllerScroll _controller_scroll;

	public override void set_inconsistent(bool inconsistent)
	{
		if (_inconsistent != inconsistent) {
			_inconsistent = inconsistent;

			if (_inconsistent) {
				_entry.text = INCONSISTENT_LABEL;
			} else {
				set_value_safe(string_to_double(_entry.text, _value));
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
		this.value = (double)v;
	}

	public double value
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

	public InputDouble(double val, double min, double max, int preview_decimals = 4, int edit_decimals = 5)
	{
		_entry = new Gtk.Entry();
		_entry.input_purpose = Gtk.InputPurpose.NUMBER;
		_entry.set_width_chars(1);

		_entry.activate.connect(on_activate);
		_entry.focus_in_event.connect(on_focus_in);
		_entry.focus_out_event.connect(on_focus_out);

		_inconsistent = false;
		_min = min;
		_max = max;
		_preview_decimals = preview_decimals;
		_edit_decimals = edit_decimals;

		set_value_safe(val);

		_gesture_click = new Gtk.GestureMultiPress(_entry);
		_gesture_click.pressed.connect(on_button_pressed);
		_gesture_click.released.connect(on_button_released);

#if CROWN_GTK3
		_entry.scroll_event.connect(() => {
				GLib.Signal.stop_emission_by_name(_entry, "scroll-event");
				return Gdk.EVENT_PROPAGATE;
			});
#else
		_controller_scroll = new Gtk.EventControllerScroll(_entry, Gtk.EventControllerScrollFlags.BOTH_AXES);
		_controller_scroll.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
		_controller_scroll.scroll.connect(() => {
				// Do nothing, just consume the event to stop
				// the annoying scroll default behavior.
			});
#endif

		this.add(_entry);
	}

	public void on_button_pressed(int n_press, double x, double y)
	{
		_entry.grab_focus();
	}

	public void on_button_released(int n_press, double x, double y)
	{
		uint button = _gesture_click.get_current_button();

		if (button == Gdk.BUTTON_PRIMARY && _entry.has_focus) {
			if (_inconsistent)
				_entry.text = "";
			else
				_entry.text = print_max_decimals(_value, _edit_decimals);

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

		if (_entry.text != print_max_decimals(_value, _edit_decimals))
			set_value_safe(string_to_double(_entry.text, _value));
		else
			_entry.text = print_max_decimals(_value, _preview_decimals);
	}

	public bool on_focus_in(Gdk.EventFocus ev)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(_entry);

		if (_inconsistent)
			_entry.text = "";
		else
			_entry.text = print_max_decimals(_value, _edit_decimals);

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
				set_value_safe(string_to_double(_entry.text, _value));
			} else {
				_entry.text = INCONSISTENT_LABEL;
			}
		} else {
			if (_entry.text != print_max_decimals(_value, _edit_decimals))
				set_value_safe(string_to_double(_entry.text, _value));
			else
				_entry.text = print_max_decimals(_value, _preview_decimals);
		}

		_entry.select_region(0, 0);

		return Gdk.EVENT_PROPAGATE;
	}

	public void set_value_safe(double val)
	{
		double clamped = val.clamp(_min, _max);

		// Convert to text for displaying.
		_entry.text = print_max_decimals(clamped, _preview_decimals);

		_inconsistent = false;

		// Notify value changed.
		if (_value != clamped) {
			_value = clamped;
			value_changed(this);
		}
	}

	/// Returns @a str as double or @a deffault if conversion fails.
	public double string_to_double(string str, double deffault)
	{
		TinyExpr.Variable vars[] =
		{
			{ "x", &_value }
		};

		int err;
		TinyExpr.Expr expr = TinyExpr.compile(str, vars, out err);

		return err == 0 ? TinyExpr.eval(expr) : deffault;
	}

	public void set_min(double min)
	{
		_min = min;
		set_value_safe(_value);
	}

	public void set_max(double max)
	{
		_max = max;
		set_value_safe(_value);
	}
}

} /* namespace Crown */
