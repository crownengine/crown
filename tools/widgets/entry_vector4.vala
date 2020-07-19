/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
	public class EntryVector4 : Gtk.Box
	{
		// Data
		public bool _stop_emit;

		// Widgets
		public EntryDouble _x;
		public EntryDouble _y;
		public EntryDouble _z;
		public EntryDouble _w;

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

		public EntryVector4(Vector4 xyz, Vector4 min, Vector4 max)
		{
			Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);

			// Data
			_stop_emit = false;

			// Widgets
			_x = new EntryDouble(xyz.x, min.x, max.x);
			_y = new EntryDouble(xyz.y, min.y, max.y);
			_z = new EntryDouble(xyz.z, min.z, max.z);
			_w = new EntryDouble(xyz.w, min.w, max.w);

			_x.value_changed.connect(on_value_changed);
			_y.value_changed.connect(on_value_changed);
			_z.value_changed.connect(on_value_changed);
			_w.value_changed.connect(on_value_changed);

			this.pack_start(_x, true, true);
			this.pack_start(_y, true, true);
			this.pack_start(_z, true, true);
			this.pack_start(_w, true, true);
		}

		private void on_value_changed()
		{
			if (!_stop_emit)
				value_changed();
		}
	}
}
