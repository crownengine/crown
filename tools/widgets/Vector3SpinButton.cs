/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;
using System;

namespace Crown
{
	public delegate void Vector3ValueChanged(object o, EventArgs args);

	/// <summary>
	/// Vector3 spin button.
	/// </summary>
	public class Vector3SpinButton : Gtk.HBox
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private Gtk.SpinButton _x;
		private Gtk.SpinButton _y;
		private Gtk.SpinButton _z;

		public Vector3 Value
		{
			get
			{
				return new Vector3(_x.Value, _y.Value, _z.Value);
			}
			set
			{
				_stop_emit = true;
				Vector3 val = (Vector3)value;
				_x.Value = val.x;
				_y.Value = val.y;
				_z.Value = val.z;
				_stop_emit = false;
			}
		}

		// Events
		public event Vector3ValueChanged ValueChanged;

		public Vector3SpinButton(Vector3 xyz, Vector3 min, Vector3 max) : base()
		{
			// Data
			_stop_emit = false;

			// Widgets
			_x = new SpinButton(new Adjustment(xyz.x, min.x, max.x, 1.0, 10.0, 0.0), 1.0, 4);
			_y = new SpinButton(new Adjustment(xyz.y, min.y, max.y, 1.0, 10.0, 0.0), 1.0, 4);
			_z = new SpinButton(new Adjustment(xyz.z, min.z, max.z, 1.0, 10.0, 0.0), 1.0, 4);

			_x.ValueChanged += new EventHandler(OnValueChanged);
			_y.ValueChanged += new EventHandler(OnValueChanged);
			_z.ValueChanged += new EventHandler(OnValueChanged);

			Add(_x);
			Add(_y);
			Add(_z);
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
