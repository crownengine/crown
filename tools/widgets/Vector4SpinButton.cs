/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;
using GLib;
using System;

namespace Crown
{
	public delegate void Vector4ValueChanged(object o, EventArgs args);

	/// <summary>
	/// Vector4 spin button.
	/// </summary>
	public class Vector4SpinButton : Gtk.HBox
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private Gtk.SpinButton _x;
		private Gtk.SpinButton _y;
		private Gtk.SpinButton _z;
		private Gtk.SpinButton _w;

		public Vector4 Value
		{
			get
			{
				return new Vector4(_x.Value, _y.Value, _z.Value, _w.Value);
			}
			set
			{
				_stop_emit = true;
				Vector4 val = (Vector4)value;
				_x.Value = val.x;
				_y.Value = val.y;
				_z.Value = val.z;
				_w.Value = val.w;
				_stop_emit = false;
			}
		}

		// Events
		public event Vector4ValueChanged ValueChanged;

		public Vector4SpinButton(Vector4 xyz, Vector4 min, Vector4 max) : base()
		{
			// Data
			_stop_emit = false;

			// Widgets
			_x = new SpinButton(new Adjustment(xyz.x, min.x, max.x, 1.0, 10.0, 0.0), 1.0, 4);
			_y = new SpinButton(new Adjustment(xyz.y, min.y, max.y, 1.0, 10.0, 0.0), 1.0, 4);
			_z = new SpinButton(new Adjustment(xyz.z, min.z, max.z, 1.0, 10.0, 0.0), 1.0, 4);
			_w = new SpinButton(new Adjustment(xyz.w, min.w, max.w, 1.0, 10.0, 0.0), 1.0, 4);

			_x.ValueChanged += new EventHandler(OnValueChanged);
			_y.ValueChanged += new EventHandler(OnValueChanged);
			_z.ValueChanged += new EventHandler(OnValueChanged);
			_w.ValueChanged += new EventHandler(OnValueChanged);

			Add(_x);
			Add(_y);
			Add(_z);
			Add(_w);
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
