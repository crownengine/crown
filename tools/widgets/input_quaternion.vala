/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputQuaternion : InputField
{
	public Quaternion _rotation;
	public InputDouble _x;
	public InputDouble _y;
	public InputDouble _z;
	public Gtk.Box _box;

	public override void set_inconsistent(bool inconsistent)
	{
	}

	public override bool is_inconsistent()
	{
		return false;
	}

	public override GLib.Value union_value()
	{
		return this.value;
	}

	public override void set_union_value(GLib.Value v)
	{
		this.value = (Quaternion)v;
	}

	public Quaternion value
	{
		get
		{
			return _rotation;
		}
		set
		{
			_rotation = value;

			// Convert to Euler for displaying.
			Vector3 euler = value.to_euler();

			_x.value_changed.disconnect(on_value_changed);
			_y.value_changed.disconnect(on_value_changed);
			_z.value_changed.disconnect(on_value_changed);
			_x.value = MathUtils.deg(euler.x);
			_y.value = MathUtils.deg(euler.y);
			_z.value = MathUtils.deg(euler.z);
			_x.value_changed.connect(on_value_changed);
			_y.value_changed.connect(on_value_changed);
			_z.value_changed.connect(on_value_changed);
			value_changed(this);
		}
	}

	public InputQuaternion(int preview_decimals = 2, int edit_decimals = 3)
	{
		_rotation = QUATERNION_IDENTITY;
		_x = new InputDouble(0.0, -180.0, 180.0, preview_decimals, edit_decimals);
		_x.get_style_context().add_class("axis");
		_x.get_style_context().add_class("x");
		_y = new InputDouble(0.0, -180.0, 180.0, preview_decimals, edit_decimals);
		_y.get_style_context().add_class("axis");
		_y.get_style_context().add_class("y");
		_z = new InputDouble(0.0, -180.0, 180.0, preview_decimals, edit_decimals);
		_z.get_style_context().add_class("axis");
		_z.get_style_context().add_class("z");

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);
		_z.value_changed.connect(on_value_changed);

		_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 4);
		_box.pack_start(_x, true);
		_box.pack_start(_y, true);
		_box.pack_start(_z, true);

		this.add(_box);
	}

	public void on_value_changed(InputField p)
	{
		double x = MathUtils.rad((double)_x.value);
		double y = MathUtils.rad((double)_y.value);
		double z = MathUtils.rad((double)_z.value);
		Quaternion new_rotation = Quaternion.from_euler(x, y, z);

		if (_rotation != new_rotation) {
			_rotation = new_rotation;
			value_changed(this);
		}
	}
}

} /* namespace Crown */
