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
	public const int DRAG_ACTIVATION_INTERVAL_MS = 1;
	public const int DRAG_UPDATE_INTERVAL_MS = 8;
	public const double DRAG_ACTIVATION_MARGIN = 5.0;

	public InputDoubleFlags _flags;
	public bool _inconsistent;
	public double _min;
	public double _max;
	public double _value;
	public int _preview_decimals;
	public int _edit_decimals;
	public double _increment_regular;   // Per-pixel.
	public double _increment_precision; // Per-pixel.
	public double _increment;           // Per-pixel.
	public double _snap_multiplier;
	public Gtk.Entry _entry;
	public Gtk.Widget _drag_widget;
	public Gtk.Overlay _overlay;
	public Gtk.EventControllerKey _controller_key;
	public Gtk.EventControllerKey _drag_controller_key;
	public Gtk.EventControllerMotion _controller_motion;
#if !CROWN_GTK3
	public Gtk.EventControllerFocus _controller_focus;
#endif
	public Gtk.GestureSingle _gesture_click;

	public InfiniteDragController _drag;
	public double _drag_start_value;
	public double _drag_offset;
	public bool _resetting_click_gesture;
	public bool _drag_snap;

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

	public InputDouble(double val, double min, double max, int preview_decimals = DEFAULT_PREVIEW_DECIMALS, int edit_decimals = DEFAULT_EDIT_DECIMALS, InputDoubleFlags flags = InputDoubleFlags.NONE)
	{
		_flags = flags;

		_entry = new Gtk.Entry();
		_entry.input_purpose = Gtk.InputPurpose.FREE_FORM;
		_entry.set_width_chars(0);
		_entry.editable = false;
		_entry.activate.connect(on_activate);
#if CROWN_GTK3
		_entry.notify["has-focus"].connect(on_focus_changed);
#else
		_controller_focus = new Gtk.EventControllerFocus();
		_controller_focus.enter.connect(on_focus_in);
		_controller_focus.leave.connect(on_focus_out);
		_entry.add_controller(_controller_focus);
#endif

#if CROWN_GTK3
		Gtk.EventBox event_box = new Gtk.EventBox();
		event_box.can_focus = false;
		event_box.set_visible_window(false);
		_drag_widget = event_box;
		_entry.get_style_context().add_class("label-button");
#else
		_drag_widget = _entry;
		_entry.add_css_class("label-button");
#endif
		_drag = new InfiniteDragController(_drag_widget);
		_drag.activation_margin = DRAG_ACTIVATION_MARGIN;
		_drag.activation_poll_ms = DRAG_ACTIVATION_INTERVAL_MS;
		_drag.update_interval_ms = DRAG_UPDATE_INTERVAL_MS;
		_drag.axis_mode = InfiniteDragController.Axis.X;
		_drag.cancel_button = Gdk.BUTTON_SECONDARY;
		_drag.preserve_legacy_events = true; // let GTK observe the matching primary-button release
		_drag.drag_started.connect(on_drag_started);
		_drag.drag_delta.connect(on_drag_delta);
		_drag.drag_committed.connect(on_drag_committed);
		_drag.drag_cancelled.connect(on_drag_cancelled);
		_drag.drag_finished.connect(on_drag_finished);
		_drag.release_detected_externally.connect(reset_click_gesture_after_sampler_release);

		_overlay = new Gtk.Overlay();
#if CROWN_GTK3
		_overlay.add(_entry);
#else
		_overlay.set_child(_entry);
#endif
		_overlay.add_overlay(_drag_widget);

		_inconsistent = false;
		_min = min;
		_max = max;
		_preview_decimals = preview_decimals;
		_edit_decimals = edit_decimals;

		set_increments(0.01, 0.001);
		_snap_multiplier = 1.0;

		drag_reset();

		_value = val.clamp(_min, _max);
		char buffer[PRINT_MAX_DECIMALS_BUFFER_SIZE];
		_entry.text = format_value(buffer, _value, _preview_decimals);

#if CROWN_GTK3
		_gesture_click = new Gtk.GestureMultiPress(_drag_widget);
		((Gtk.GestureMultiPress)_gesture_click).pressed.connect(on_button_pressed);
		((Gtk.GestureMultiPress)_gesture_click).released.connect(on_button_released);
		((Gtk.GestureMultiPress)_gesture_click).cancel.connect(on_gesture_cancelled);
#else
		_gesture_click = new Gtk.GestureClick();
		((Gtk.GestureClick)_gesture_click).pressed.connect(on_button_pressed);
		((Gtk.GestureClick)_gesture_click).released.connect(on_button_released);
		((Gtk.GestureClick)_gesture_click).cancel.connect(on_gesture_cancelled);
		_drag_widget.add_controller(_gesture_click);
#endif

#if CROWN_GTK3
		_controller_key = new Gtk.EventControllerKey(_entry);
#else
		_controller_key = new Gtk.EventControllerKey();
		_entry.add_controller(_controller_key);
#endif
		_controller_key.key_pressed.connect(on_key_pressed);

#if CROWN_GTK3
		_controller_motion = new Gtk.EventControllerMotion(_drag_widget);
#else
		_controller_motion = new Gtk.EventControllerMotion();
		_drag_widget.add_controller(_controller_motion);
#endif
		_controller_motion.enter.connect(on_enter);
		_controller_motion.leave.connect(on_leave);

#if CROWN_GTK3
		_drag_controller_key = new Gtk.EventControllerKey(_drag_widget);
#else
		_drag_controller_key = new Gtk.EventControllerKey();
		_drag_widget.add_controller(_drag_controller_key);
#endif
		_drag_controller_key.key_pressed.connect(on_event_box_key_pressed);
		_drag_controller_key.key_released.connect(on_event_box_key_released);

#if CROWN_GTK3
		_entry.scroll_event.connect(on_scroll_event);
#endif

#if CROWN_GTK3
		this.add(_overlay);
#else
		this.set_child(_overlay);
#endif
	}

	public void set_increments(double regular, double precision)
	{
		_increment_regular = regular;
		_increment_precision = precision;
		_increment = regular;
	}

	public void clear_focus()
	{
#if CROWN_GTK3
		var window = get_toplevel() as Gtk.Window;
		if (window != null && window.get_focus() != null)
			window.set_focus(null);
#else
		Gtk.Root? root = this.get_root();
		if (root != null && root.get_focus() != null)
			root.set_focus(null);
#endif
	}

	public bool on_scroll_event()
	{
		// Consume the event to avoid GTK changing values when scrolling over the widget.
		GLib.Signal.stop_emission_by_name(_entry, "scroll-event");
		return Gdk.EVENT_PROPAGATE;
	}

	public bool on_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval != Gdk.Key.Escape || !_entry.editable)
			return Gdk.EVENT_PROPAGATE;

		char buffer[PRINT_MAX_DECIMALS_BUFFER_SIZE];
		if (_inconsistent)
			_entry.text = "";
		else
			_entry.text = format_value(buffer, _value, _edit_decimals);

		clear_focus();

		return Gdk.EVENT_STOP;
	}

	public void on_button_pressed(int n_press, double x, double y)
	{
		clear_focus();

		_drag_start_value = _value;
		_drag_offset = 0.0;
		_drag.start();
	}

	public void on_button_released(int n_press, double x, double y)
	{
		_drag.release();
	}

	public bool on_event_box_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval == Gdk.Key.Control_L)
			_drag_snap = true;
		else if (keyval == Gdk.Key.Shift_L)
			_increment = _increment_precision;
		else
			return Gdk.EVENT_PROPAGATE;

		if (_drag.dragging)
			set_value_safe(drag_target_with_modifiers());

		return Gdk.EVENT_STOP;
	}

	public void on_event_box_key_released(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval == Gdk.Key.Control_L)
			_drag_snap = false;
		else if (keyval == Gdk.Key.Shift_L)
			_increment = _increment_regular;
		else
			return;

		if (_drag.dragging)
			set_value_safe(drag_target_with_modifiers());
	}

	public void on_gesture_cancelled(Gdk.EventSequence? sequence)
	{
		if (_resetting_click_gesture)
			return;

		_drag.cancel();
	}

	public void reset_click_gesture_after_sampler_release()
	{
		_resetting_click_gesture = true;
		_gesture_click.reset();
		_resetting_click_gesture = false;
	}

	public void on_enter()
	{
		if (_drag.dragging)
			return;

		set_drag_cursor("ew-resize");
	}

	public void on_leave()
	{
		if (_drag.dragging)
			return;

		set_drag_cursor("default");
	}

	public void set_drag_cursor(string name)
	{
#if CROWN_GTK3
		_drag_widget.get_window().set_cursor(new Gdk.Cursor.from_name(Gdk.Display.get_default(), name));
#else
		_drag_widget.set_cursor_from_name(name);
#endif
	}

	public void drag_reset()
	{
		_increment = _increment_regular;
		_drag_snap = false;
	}

	public double drag_target_with_modifiers()
	{
		double actual_dx = _drag._total_dx + _drag_offset;
		double dx_pixels = (double)(int)actual_dx;
		double target_val = _drag_start_value + dx_pixels * _increment;

		if (_drag_snap) {
			double rounded = Math.round(target_val / _snap_multiplier);
			if (rounded == 0.0)
				rounded = 0.0; // Avoid negative zero.
			target_val = rounded * _snap_multiplier;
		}

		if (target_val > _max) {
			_drag_offset += (_max - target_val) / _increment;
			target_val = _max;
		} else if (target_val < _min) {
			_drag_offset += (_min - target_val) / _increment;
			target_val = _min;
		}

		return target_val;
	}

	public void on_drag_started()
	{
		_drag_offset = -_drag._total_dx;
		set_drag_cursor("none");
	}

	public void on_drag_delta(double dx, double dy, double total_dx, double total_dy)
	{
		set_value_safe(drag_target_with_modifiers());
	}

	public void on_drag_committed()
	{
		drag_reset();
		double drag_end_value = _value;
		set_value_safe(_drag_start_value, -1);             // avoid a redundant undo entry
		set_value_safe(drag_end_value);                    // fires the real commit
		set_drag_cursor("ew-resize");
	}

	public void on_drag_cancelled()
	{
		drag_reset();
		set_value_safe(_drag_start_value, 0);              // revert, undo disabled
		set_drag_cursor("ew-resize");
	}

	public void on_drag_finished(bool was_dragging)
	{
		if (was_dragging)
			return; // already handled by drag_committed/drag_cancelled

		drag_reset();

		_drag_widget.visible = false;
#if CROWN_GTK3
		_entry.get_style_context().remove_class("label-button");
#else
		_entry.remove_css_class("label-button");
#endif
		_entry.editable = true;
		_entry.grab_focus();

		char buffer[PRINT_MAX_DECIMALS_BUFFER_SIZE];
		if (_inconsistent)
			_entry.text = "";
		else
			_entry.text = format_value(buffer, _value, _edit_decimals);

		GLib.Idle.add(() => {
				_entry.set_position(-1);
				_entry.select_region(0, -1);
				return GLib.Source.REMOVE;
			});
	}

	public void on_activate()
	{
		_entry.select_region(0, 0);
		_entry.set_position(-1);

		char buffer[PRINT_MAX_DECIMALS_BUFFER_SIZE];
		if (_entry.text != format_value(buffer, _value, _edit_decimals))
			set_value_safe(string_to_double(_entry.text, _value));

		_entry.text = format_value(buffer, _value, _edit_decimals);

		clear_focus();
	}

	public void on_focus_changed(GLib.ParamSpec pspec)
	{
		if (!_entry.has_focus) {
			on_focus_out();
			return;
		}

		on_focus_in();
	}

	public void on_focus_in()
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_in(_entry);

		if (_drag_widget.visible) {
			_drag_widget.visible = false;
#if CROWN_GTK3
			_entry.get_style_context().remove_class("label-button");
#else
			_entry.remove_css_class("label-button");
#endif
		}

		_entry.editable = true;

		char buffer[PRINT_MAX_DECIMALS_BUFFER_SIZE];
		if (_inconsistent)
			_entry.text = "";
		else
			_entry.text = format_value(buffer, _value, _edit_decimals);

		_entry.set_position(-1);
		_entry.select_region(0, -1);
	}

	public void on_focus_out()
	{
		var app = (LevelEditorApplication)GLib.Application.get_default();
		app.entry_any_focus_out(_entry);
		char buffer[PRINT_MAX_DECIMALS_BUFFER_SIZE];

		if (_inconsistent) {
			if (_entry.text != "") {
				set_value_safe(string_to_double(_entry.text, _value));
			} else {
				_entry.text = INCONSISTENT_LABEL;
			}
		} else {
			if (_entry.text != format_value(buffer, _value, _edit_decimals))
				set_value_safe(string_to_double(_entry.text, _value));
			else
				_entry.text = format_value(buffer, _value, _edit_decimals);
		}

		_entry.select_region(0, 0);
		_entry.editable = false;
		_drag_widget.visible = true;
#if CROWN_GTK3
		_entry.get_style_context().add_class("label-button");
#else
		_entry.add_css_class("label-button");
#endif
	}

	public void set_value_safe(double val, int undo_redo = (int)!_drag.dragging)
	{
		double clamped = val.clamp(_min, _max);

		_inconsistent = false;

		if (_value != clamped) {
			char buffer[PRINT_MAX_DECIMALS_BUFFER_SIZE];
			format_value(buffer, clamped, _preview_decimals);
			_entry.text = (string)buffer;
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

	public unowned string format_value(char[] buffer, double value, int max_decimals)
	{
		if ((_flags & InputDoubleFlags.INFINITY) != 0
			&& value == INFINITY_VALUE)
			return INFINITY_LABEL;

		return print_max_decimals(buffer, value, max_decimals);
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
