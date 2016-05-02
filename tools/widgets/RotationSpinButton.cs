/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;
using System;

namespace Crown
{
	public delegate void RotationChanged(object o, EventArgs args);

	/// <summary>
	/// Vector3 spin button.
	/// </summary>
	public class RotationSpinButton : Gtk.HBox
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private Gtk.SpinButton _x;
		private Gtk.SpinButton _y;
		private Gtk.SpinButton _z;

		public Quaternion Value
		{
			get
			{
				double x = MathUtils.Rad(_x.Value);
				double y = MathUtils.Rad(_y.Value);
				double z = MathUtils.Rad(_z.Value);
				return Quaternion.FromEuler(x, y, z);
			}
			set
			{
				_stop_emit = true;
				Vector3 euler = ((Quaternion)value).ToEuler();
				_x.Value = MathUtils.Deg(euler.x);
				_y.Value = MathUtils.Deg(euler.y);
				_z.Value = MathUtils.Deg(euler.z);
				_stop_emit = false;
			}
		}

		// Events
		public event RotationChanged ValueChanged;

		public RotationSpinButton(Vector3 xyz) : base()
		{
			// Data
			_stop_emit = false;

			// Widgets
			_x = new SpinButton(new Adjustment(xyz.x, -180.0, 180.0, 1.0, 10.0, 0.0), 1.0, 4);
			_y = new SpinButton(new Adjustment(xyz.y, -180.0, 180.0, 1.0, 10.0, 0.0), 1.0, 4);
			_z = new SpinButton(new Adjustment(xyz.z, -180.0, 180.0, 1.0, 10.0, 0.0), 1.0, 4);

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
