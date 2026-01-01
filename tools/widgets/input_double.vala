/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputDouble : InputField, Gtk.Entry
{
	public bool _inconsistent;
	public double _min;
	public double _max;
	public double _value;
	public int _preview_decimals;
	public int _edit_decimals;
	public Gtk.GestureMultiPress _gesture_click;
	public Gtk.EventControllerScroll _controller_scroll;

	public void set_inconsistent(bool inconsistent)
	{
		if (_inconsistent != inconsistent) {
			_inconsistent = inconsistent;

			if (_inconsistent) {
				this.text = INCONSISTENT_LABEL;
			} else {
				set_value_safe(string_to_double(this.text, _value));
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
		this.input_purpose = Gtk.InputPurpose.NUMBER;
		this.set_width_chars(1);

		this.activate.connect(on_activate);
		this.focus_in_event.connect(on_focus_in);
		this.focus_out_event.connect(on_focus_out);

		_inconsistent = false;
		_min = min;
		_max = max;
		_preview_decimals = preview_decimals;
		_edit_decimals = edit_decimals;

		set_value_safe(val);

		_gesture_click = new Gtk.GestureMultiPress(this);
		_gesture_click.pressed.connect(on_button_pressed);
		_gesture_click.released.connect(on_button_released);

#if CROWN_GTK3
		this.scroll_event.connect(() => {
				GLib.Signal.stop_emission_by_name(this, "scroll-event");
				return Gdk.EVENT_PROPAGATE;
			});
#else
		_controller_scroll = new Gtk.EventControllerScroll(this, Gtk.EventControllerScrollFlags.BOTH_AXES);
		_controller_scroll.set_propagation_phase(Gtk.PropagationPhase.CAPTURE);
		_controller_scroll.scroll.connect(() => {
				// Do nothing, just consume the event to stop
				// the annoying scroll default behavior.
			});
#endif
	}

	public void on_button_pressed(int n_press, double x, double y)
	{
		this.grab_focus();
	}

	public void on_button_released(int n_press, double x, double y)
	{
		uint button = _gesture_click.get_current_button();

		if (button == Gdk.BUTTON_PRIMARY && this.has_focus) {
			if (_inconsistent)
				this.text = "";
			else
				this.text = print_max_decimals(_value, _edit_decimals);

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

		if (this.text != print_max_decimals(_value, _edit_decimals))
			set_value_safe(string_to_double(this.text, _value));
		else
			this.text = print_max_decimals(_value, _preview_decimals);
	}

	public bool on_focus_in(Gdk.EventFocus ev)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(this);

		if (_inconsistent)
			this.text = "";
		else
			this.text = print_max_decimals(_value, _edit_decimals);

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
				set_value_safe(string_to_double(this.text, _value));
			} else {
				this.text = INCONSISTENT_LABEL;
			}
		} else {
			if (this.text != print_max_decimals(_value, _edit_decimals))
				set_value_safe(string_to_double(this.text, _value));
			else
				this.text = print_max_decimals(_value, _preview_decimals);
		}

		this.select_region(0, 0);

		return Gdk.EVENT_PROPAGATE;
	}

	public void set_value_safe(double val)
	{
		double clamped = val.clamp(_min, _max);

		// Convert to text for displaying.
		this.text = print_max_decimals(clamped, _preview_decimals);

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
