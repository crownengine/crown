/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gdk;

namespace Crown
{
	/// <summary>
	/// Double spin button.
	/// </summary>
	public class SpinButtonDouble : Gtk.Bin
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private Gtk.SpinButton _x;

		public double value
		{
			get
			{
				return _x.value;
			}
			set
			{
				_stop_emit = true;
				_x.value = value;
				_stop_emit = false;
			}
		}

		// Signals
		public signal void value_changed();

		public SpinButtonDouble(double x, double min, double max)
		{
			this.hexpand = true;

			// Data
			_stop_emit = false;

			// Widgets
			_x = new SpinButton(new Adjustment(x, min, max, 1.0, 10.0, 0.0), 1.0, 4);

			_x.value_changed.connect(on_value_changed);
			_x.scroll_event.connect(on_scroll);
			_x.set_width_chars(1);

			add(_x);
		}

		private void on_value_changed()
		{
			if (!_stop_emit)
				value_changed();
		}

		private bool on_scroll(Gdk.EventScroll ev)
		{
			GLib.Signal.stop_emission_by_name(_x, "scroll-event");
			return false;
		}
	}
}
