/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;
using System;

namespace Crown
{
	public delegate void DoubleValueChanged(object o, EventArgs args);

	/// <summary>
	/// Double spin button.
	/// </summary>
	public class DoubleSpinButton : Gtk.HBox
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private Gtk.SpinButton _x;

		public double Value
		{
			get
			{
				return _x.Value;
			}
			set
			{
				_stop_emit = true;
				_x.Value = value;
				_stop_emit = false;
			}
		}

		// Events
		public event Vector2ValueChanged ValueChanged;

		public DoubleSpinButton(double x, double min, double max) : base()
		{
			// Data
			_stop_emit = false;

			// Widgets
			_x = new SpinButton(new Adjustment(x, min, max, 1.0, 10.0, 0.0), 1.0, 4);

			_x.ValueChanged += new EventHandler(OnValueChanged);

			Add(_x);
		}

		private void OnValueChanged(object o, EventArgs args)
		{
			if (!_stop_emit)
				EmitValueChanged(this, args);
		}

		// Events
		private void EmitValueChanged(object o, EventArgs args)
		{
			if (ValueChanged != null)
				ValueChanged(o, args);
		}
	}
}
