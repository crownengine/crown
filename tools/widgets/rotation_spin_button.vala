/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;

namespace Crown
{
	/// <summary>
	/// Vector3 spin button.
	/// </summary>
	public class RotationSpinButton : Gtk.HBox
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private DoubleSpinButton _x;
		private DoubleSpinButton _y;
		private DoubleSpinButton _z;

		public Quaternion value
		{
			get
			{
				double x = MathUtils.rad(_x.value);
				double y = MathUtils.rad(_y.value);
				double z = MathUtils.rad(_z.value);
				return Quaternion.from_euler(x, y, z);
			}
			set
			{
				_stop_emit = true;
				Vector3 euler = ((Quaternion)value).to_euler();
				_x.value = MathUtils.deg(euler.x);
				_y.value = MathUtils.deg(euler.y);
				_z.value = MathUtils.deg(euler.z);
				_stop_emit = false;
			}
		}

		// Events
		public signal void value_changed();

		public RotationSpinButton(Vector3 xyz)
		{
			// Data
			_stop_emit = false;

			// Widgets
			_x = new DoubleSpinButton(xyz.x, -180.0, 180.0);
			_y = new DoubleSpinButton(xyz.y, -180.0, 180.0);
			_z = new DoubleSpinButton(xyz.z, -180.0, 180.0);

			_x.value_changed.connect(on_value_changed);
			_y.value_changed.connect(on_value_changed);
			_z.value_changed.connect(on_value_changed);

			add(_x);
			add(_y);
			add(_z);
		}

		private void on_value_changed()
		{
			if (!_stop_emit)
				value_changed();
		}
	}
}
