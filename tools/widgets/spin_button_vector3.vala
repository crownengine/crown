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
	public class SpinButtonVector3 : Gtk.HBox
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private SpinButtonDouble _x;
		private SpinButtonDouble _y;
		private SpinButtonDouble _z;

		public Vector3 value
		{
			get
			{
				return Vector3(_x.value, _y.value, _z.value);
			}
			set
			{
				_stop_emit = true;
				Vector3 val = (Vector3)value;
				_x.value = val.x;
				_y.value = val.y;
				_z.value = val.z;
				_stop_emit = false;
			}
		}

		// Events
		public signal void value_changed();

		public SpinButtonVector3(Vector3 xyz, Vector3 min, Vector3 max)
		{
			this.hexpand = true;

			// Data
			_stop_emit = false;

			// Widgets
			_x = new SpinButtonDouble(xyz.x, min.x, max.x);
			_y = new SpinButtonDouble(xyz.y, min.y, max.y);
			_z = new SpinButtonDouble(xyz.z, min.z, max.z);

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
