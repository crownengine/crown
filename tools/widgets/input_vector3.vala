/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputVector3 : InputField, Gtk.Box
{
	public InputDouble _x;
	public InputDouble _y;
	public InputDouble _z;

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
			return Vector3(_x.value, _y.value, _z.value);
		}
		set
		{
			Vector3 val = value;
			_x.value_changed.disconnect(on_value_changed);
			_y.value_changed.disconnect(on_value_changed);
			_z.value_changed.disconnect(on_value_changed);
			_x.value = val.x;
			_y.value = val.y;
			_z.value = val.z;
			_x.value_changed.connect(on_value_changed);
			_y.value_changed.connect(on_value_changed);
			_z.value_changed.connect(on_value_changed);
			value_changed(this);
		}
	}

	public InputVector3(Vector3 xyz, Vector3 min, Vector3 max, int preview_decimals = 4, int edit_decimals = 5)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 4);

		_x = new InputDouble(xyz.x, min.x, max.x, preview_decimals, edit_decimals);
		_x.get_style_context().add_class("axis");
		_x.get_style_context().add_class("x");
		_y = new InputDouble(xyz.y, min.y, max.y, preview_decimals, edit_decimals);
		_y.get_style_context().add_class("axis");
		_y.get_style_context().add_class("y");
		_z = new InputDouble(xyz.z, min.z, max.z, preview_decimals, edit_decimals);
		_z.get_style_context().add_class("axis");
		_z.get_style_context().add_class("z");

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);
		_z.value_changed.connect(on_value_changed);

		this.pack_start(_x, true);
		this.pack_start(_y, true);
		this.pack_start(_z, true);
	}

	public void on_value_changed()
	{
		value_changed(this);
	}
}

} /* namespace Crown */
