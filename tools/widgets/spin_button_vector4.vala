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
	public class SpinButtonVector4 : Gtk.Box
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private SpinButtonDouble _x;
		private SpinButtonDouble _y;
		private SpinButtonDouble _z;
		private SpinButtonDouble _w;

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

		// Signals
		public signal void value_changed();

		public SpinButtonVector4(Vector4 xyz, Vector4 min, Vector4 max)
		{
			Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);

			// Data
			_stop_emit = false;

			// Widgets
			_x = new SpinButtonDouble(xyz.x, min.x, max.x);
			_y = new SpinButtonDouble(xyz.y, min.y, max.y);
			_z = new SpinButtonDouble(xyz.z, min.z, max.z);
			_w = new SpinButtonDouble(xyz.w, min.w, max.w);

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
