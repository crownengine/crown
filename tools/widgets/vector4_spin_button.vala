/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;

namespace Crown
{
	/// <summary>
	/// Vector4 spin button.
	/// </summary>
	public class Vector4SpinButton : Gtk.HBox
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private DoubleSpinButton _x;
		private DoubleSpinButton _y;
		private DoubleSpinButton _z;
		private DoubleSpinButton _w;

		public Vector4 value
		{
			get
			{
				return Vector4(_x.value, _y.value, _z.value, _w.value);
			}
			set
			{
				_stop_emit = true;
				Vector4 val = (Vector4)value;
				_x.value = val.x;
				_y.value = val.y;
				_z.value = val.z;
				_w.value = val.w;
				_stop_emit = false;
			}
		}

		// Events
		public signal void value_changed();

		public Vector4SpinButton(Vector4 xyz, Vector4 min, Vector4 max)
		{
			// Data
			_stop_emit = false;

			// Widgets
			_x = new DoubleSpinButton(xyz.x, min.x, max.x);
			_y = new DoubleSpinButton(xyz.y, min.y, max.y);
			_z = new DoubleSpinButton(xyz.z, min.z, max.z);
			_w = new DoubleSpinButton(xyz.w, min.w, max.w);

			_x.value_changed.connect(on_value_changed);
			_y.value_changed.connect(on_value_changed);
			_z.value_changed.connect(on_value_changed);
			_w.value_changed.connect(on_value_changed);

			add(_x);
			add(_y);
			add(_z);
			add(_w);
		}

		private void on_value_changed()
		{
			if (!_stop_emit)
				value_changed();
		}
	}
}
