/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;
using System;

namespace Crown
{
	public delegate void Vector2ValueChanged(object o, EventArgs args);

	/// <summary>
	/// Vector2 spin button.
	/// </summary>
	public class Vector2SpinButton : Gtk.HBox
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private Gtk.SpinButton _x;
		private Gtk.SpinButton _y;

		public Vector2 Value
		{
			get
			{
				return new Vector2(_x.Value, _y.Value);
			}
			set
			{
				_stop_emit = true;
				Vector2 val = (Vector2)value;
				_x.Value = val.x;
				_y.Value = val.y;
				_stop_emit = false;
			}
		}

		// Events
		public event Vector2ValueChanged ValueChanged;

		public Vector2SpinButton(Vector2 xyz, Vector2 min, Vector2 max) : base()
		{
			// Data
			_stop_emit = false;

			// Widgets
			_x = new SpinButton(new Adjustment(xyz.x, min.x, max.x, 1.0, 10.0, 0.0), 1.0, 4);
			_y = new SpinButton(new Adjustment(xyz.y, min.y, max.y, 1.0, 10.0, 0.0), 1.0, 4);

			_x.ValueChanged += new EventHandler(OnValueChanged);
			_y.ValueChanged += new EventHandler(OnValueChanged);

			Add(_x);
			Add(_y);
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
