/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputVector4 : InputField, Gtk.Box
{
	public bool _stop_emit;
	public InputDouble _x;
	public InputDouble _y;
	public InputDouble _z;
	public InputDouble _w;

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
		this.value = (Vector4)v;
	}

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

	public InputVector4(Vector4 xyz, Vector4 min, Vector4 max, int preview_decimals = 4)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 4);

		// Data
		_stop_emit = false;

		// Widgets
		_x = new InputDouble(xyz.x, min.x, max.x, preview_decimals);
		_x.get_style_context().add_class("axis");
		_x.get_style_context().add_class("x");
		_y = new InputDouble(xyz.y, min.y, max.y, preview_decimals);
		_y.get_style_context().add_class("axis");
		_y.get_style_context().add_class("y");
		_z = new InputDouble(xyz.z, min.z, max.z, preview_decimals);
		_z.get_style_context().add_class("axis");
		_z.get_style_context().add_class("z");
		_w = new InputDouble(xyz.w, min.w, max.w, preview_decimals);
		_w.get_style_context().add_class("axis");
		_w.get_style_context().add_class("w");

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);
		_z.value_changed.connect(on_value_changed);
		_w.value_changed.connect(on_value_changed);

		this.pack_start(_x, true);
		this.pack_start(_y, true);
		this.pack_start(_z, true);
		this.pack_start(_w, true);
	}

	public void on_value_changed()
	{
		if (!_stop_emit)
			value_changed(this);
	}
}

} /* namespace Crown */
