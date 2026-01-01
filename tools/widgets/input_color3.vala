/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputColor3 : Gtk.Box, InputField
{
	public Gtk.ColorButton _color_button;

	public void set_inconsistent(bool inconsistent)
	{
	}

	public bool is_inconsistent()
	{
		return false;
	}

	public GLib.Value union_value()
	{
		return this.value;
	}

	public void set_union_value(GLib.Value v)
	{
		this.value = (Vector3)v;
	}

	public Vector3 value
	{
		get
		{
			Gdk.RGBA rgba = _color_button.get_rgba();
			return Vector3(rgba.red, rgba.green, rgba.blue);
		}
		set
		{
			Vector3 rgb = (Vector3)value;
			_color_button.set_rgba({ rgb.x, rgb.y, rgb.z, 1.0 });
		}
	}

	public InputColor3()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL);

		_color_button = new Gtk.ColorButton();
		_color_button.color_set.connect(on_color_set);

		this.pack_start(_color_button);
	}

	public void on_color_set()
	{
		value_changed(this);
	}
}

} /* namespace Crown */
