/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gdk;

namespace Crown
{
	public class EntryDouble : Gtk.Entry
	{
		public double _min;
		public double _max;
		public double _value;
		public bool _stop_emit;
		public string _format;

		public double value
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

		// Signals
		public signal void value_changed();

		public EntryDouble(double val, double min, double max, string fmt = "%.6g")
		{
			this.input_purpose = Gtk.InputPurpose.DIGITS;
			this.set_width_chars(1);

			this.scroll_event.connect(on_scroll);
			this.button_release_event.connect(on_button_release);
			this.activate.connect(on_activate);
			this.focus_out_event.connect(on_focus_out);

			this._min = min;
			this._max = max;
			this._format = fmt;

			_stop_emit = true;
			set_value_safe(val);
			_stop_emit = false;
		}

		private bool on_scroll(Gdk.EventScroll ev)
		{
			GLib.Signal.stop_emission_by_name(this, "scroll-event");
			return false; // Propagate the event
		}

		private bool on_button_release(Gdk.EventButton ev)
		{
			if (ev.button == Gdk.BUTTON_PRIMARY && this.has_focus)
			{
				this.text = "%.6g".printf(_value);
				this.set_position(-1);
				this.select_region(0, -1);
				return true; // Do not propagate
			}

			return false;
		}

		private void on_activate()
		{
			this.select_region(0, 0);
			this.set_position(-1);
			set_value_safe(string_to_double(this.text, _value));
		}

		private bool on_focus_out(Gdk.EventFocus ef)
		{
			set_value_safe(string_to_double(this.text, _value));
			return Gdk.EVENT_PROPAGATE;
		}

		private void set_value_safe(double val)
		{
			double clamped = val.clamp(_min, _max);

			// Convert to text for displaying
			this.text = _format.printf(clamped);

			// Notify value changed
			if (_value != clamped)
			{
				_value = clamped;
				if (!_stop_emit)
					value_changed();
			}
		}

		/// Returns @a str as double or @a deffault if conversion fails.
		private double string_to_double(string str, double deffault)
		{
			double result;
			return double.try_parse(str, out result) ? result : deffault;
		}
	}
}
