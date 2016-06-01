/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;

namespace Crown
{
	/// <summary>
	/// Vector3 ColorButton
	/// </summary>
	public class ColorButtonVector3 : Gtk.Bin
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private Gtk.ColorButton _color_button;

		// Signals
		public signal void value_changed();

		public Vector3 value
		{
			get
			{
				Gdk.RGBA rgba = _color_button.get_rgba();
				double r = rgba.red;
				double g = rgba.green;
				double b = rgba.blue;
				return Vector3(r, g, b);
			}
			set
			{
				_stop_emit = true;
				Vector3 val = (Vector3)value;
				double r = val.x;
				double g = val.y;
				double b = val.z;
				double a = 1.0;
				_color_button.set_rgba({ r, g, b, a });
				_stop_emit = false;
			}
		}

		public ColorButtonVector3()
		{
			_stop_emit = false;

			_color_button = new Gtk.ColorButton();
			_color_button.color_set.connect(on_color_set);

			add(_color_button);
		}

		private void on_color_set()
		{
			if (!_stop_emit)
				value_changed();
		}
	}
}
