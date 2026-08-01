/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputDouble : InputField
{
	public const double INFINITY_VALUE = (double)float.MAX;
	public const string INFINITY_LABEL = "Infinity";
	public const int DEFAULT_PREVIEW_DECIMALS = 4;
	public const int DEFAULT_EDIT_DECIMALS = 5;

	public InputDoubleFlags _flags;
	public bool _inconsistent;
	public double _min;
	public double _max;
	public double _value;
	public int _preview_decimals;
	public int _edit_decimals;
	public Gtk.Entry _entry;
	public Gtk.Label _label;
	public Gtk.EventBox _event_box;
	public Gtk.Overlay _overlay;
	public Gtk.GestureMultiPress _gesture_click;
	public Gtk.EventControllerMotion _controller_motion;
	public Gtk.EventControllerScroll _controller_scroll;

	public bool _pressed;
	public bool _dragging;
	public double _drag_start_x;
	public double _drag_start_y;
	public double _drag_start_value;

	public override void set_inconsistent(bool inconsistent)
	{
		if (_inconsistent != inconsistent) {
			_inconsistent = inconsistent;

			if (_inconsistent) {
				_entry.text = INCONSISTENT_LABEL;
				_label.set_text(_entry.text);
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

	public InputDouble(double val, double min, double max, int preview_decimals = DEFAULT_PREVIEW_DECIMALS, int edit_decimals = DEFAULT_EDIT_DECIMALS, InputDoubleFlags flags = InputDoubleFlags.NONE)
	{
		_flags = flags;

		_entry = new Gtk.Entry();
		_entry.input_purpose = Gtk.InputPurpose.FREE_FORM;
		_entry.set_width_chars(1);
		_entry.editable = false;

		_entry.activate.connect(on_activate);
		_entry.focus_in_event.connect(on_focus_in);
		_entry.focus_out_event.connect(on_focus_out);

		_label = new Gtk.Label(_entry.text);
		_label.halign = Gtk.Align.FILL;
		_label.get_style_context().add_class("label-button");

		_event_box = new Gtk.EventBox();
		_event_box.add(_label);
		_event_box.can_focus = false;
		_event_box.set_visible_window(false);

		_overlay = new Gtk.Overlay();
		_overlay.add(_entry);
		_overlay.add_overlay(_event_box);

		_inconsistent = false;
		_min = min;
		_max = max;
		_preview_decimals = preview_decimals;
		_edit_decimals = edit_decimals;

		set_value_safe(val);

		_controller_motion = new Gtk.EventControllerMotion(_event_box);
		_controller_motion.enter.connect(on_enter);
		_controller_motion.leave.connect(on_leave);
		_controller_motion.motion.connect(on_motion);

		_gesture_click = new Gtk.GestureMultiPress(_event_box);
		_gesture_click.pressed.connect(on_button_pressed);
		_gesture_click.released.connect(on_button_released);

		_pressed = false;
		_dragging = false;

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

		this.add(_overlay);
	}

	public void on_button_pressed(int n_press, double x, double y)
	{
		// Unfocus active widgets
		var window = get_toplevel() as Gtk.Window;
		if (window != null && window.get_focus() != null)
			window.set_focus(null);

		_pressed = true;
		_dragging = false;
		_drag_start_x = x;
		_drag_start_y = y;
		_drag_start_value = _value;
	}

	public void on_button_released(int n_press, double x, double y)
	{
		logi("button released");
		_pressed = false;

		if (_dragging) {
			logi("was dragging");
			_dragging = false;

			double drag_end_value = _value;
			set_value_safe(_drag_start_value, -1); // Avoids unnecessary update
			set_value_safe(drag_end_value); // Fires the last commit

			return;
		}

		// Begin editing
		_event_box.visible = false;
		_entry.editable = true;
		_entry.grab_focus();

		if (_inconsistent)
			_entry.text = "";
		else
			_entry.text = format_value(_value, _edit_decimals);

		GLib.Idle.add(() => {
			_entry.set_position(-1);
			_entry.select_region(0, -1);
			return GLib.Source.REMOVE;
			});
	}

	public void on_enter()
	{
		Gdk.Display display = Gdk.Display.get_default();
		Gdk.Cursor col_resize = new Gdk.Cursor.from_name(display, "col-resize");
		_event_box.get_window().set_cursor(col_resize);
	}

	public void on_leave()
	{
		Gdk.Display display = Gdk.Display.get_default();
		Gdk.Cursor deffault = new Gdk.Cursor.from_name(display, "default");
		_event_box.get_window().set_cursor(deffault);
	}

	public void on_motion(double x, double y)
	{
		if (!_pressed) {
			logi("motion ignored");
			return;
		}

		const double ACTIVATION_MARGIN = 5.0;
		double dx = x - _drag_start_x;

		if (!_dragging) {
			if (dx.abs() > ACTIVATION_MARGIN)
				_dragging = true;
		} else {
			double scale = 0.01;
			double dx_adjusted;

			if (dx > ACTIVATION_MARGIN)
				dx_adjusted = dx - ACTIVATION_MARGIN;
			else if (dx < -ACTIVATION_MARGIN)
				dx_adjusted = dx + ACTIVATION_MARGIN;
			else
				dx_adjusted = 0.0;

			set_value_safe(_drag_start_value + dx_adjusted*scale);
		}
	}

	public void on_activate()
	{
		_entry.select_region(0, 0);
		_entry.set_position(-1);

		if (_entry.text != format_value(_value, _edit_decimals))
			set_value_safe(string_to_double(_entry.text, _value));
		else
			_entry.text = format_value(_value, _preview_decimals);

		_label.set_text(_entry.text);
	}

	public bool on_focus_in(Gdk.EventFocus ev)
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(_entry);

		if (_event_box.visible)
			_event_box.visible = false;

		_entry.editable = true;

		if (_inconsistent)
			_entry.text = "";
		else
			_entry.text = format_value(_value, _edit_decimals);

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
			if (_entry.text != format_value(_value, _edit_decimals))
				set_value_safe(string_to_double(_entry.text, _value));
			else
				_entry.text = format_value(_value, _preview_decimals);
		}

		_entry.select_region(0, 0);
		_entry.editable = false;
		_label.set_text(_entry.text);
		_event_box.visible = true;

		return Gdk.EVENT_PROPAGATE;
	}

	public void set_value_safe(double val, int undo_redo = (int)!_dragging)
	{
		double clamped = val.clamp(_min, _max);

		// Convert to text for displaying.
		_entry.text = format_value(clamped, _preview_decimals);
		_label.set_text(_entry.text);

		_inconsistent = false;

		// Notify value changed.
		if (_value != clamped) {
			_value = clamped;
			value_changed(this, undo_redo);
		}
	}

	/// Returns @a str as double or @a deffault if conversion fails.
	public double string_to_double(string str, double deffault)
	{
		double special_value = 0.0;
		if ((_flags & InputDoubleFlags.INFINITY) != 0
			&& try_parse_special_literal(str, out special_value))
			return special_value;

		TinyExpr.Variable vars[] =
		{
			{ "x", &_value }
		};

		int err;
		TinyExpr.Expr expr = TinyExpr.compile(str, vars, out err);

		return err == 0 ? TinyExpr.eval(expr) : deffault;
	}

	public string format_value(double value, int max_decimals)
	{
		if ((_flags & InputDoubleFlags.INFINITY) != 0
			&& value == INFINITY_VALUE)
			return INFINITY_LABEL;

		return print_max_decimals(value, max_decimals);
	}

	public bool try_parse_special_literal(string str, out double value)
	{
		string normalized = str.strip().down();

		switch (normalized) {
		case "inf":
		case "+inf":
		case "infinity":
		case "+infinity":
			value = INFINITY_VALUE;
			return true;

		default:
			value = 0.0;
			return false;
		}
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
