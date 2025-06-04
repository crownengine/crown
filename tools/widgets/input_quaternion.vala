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
			if (Quaternion.equal_func(_rotation, value))
				return;

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
		_x = new InputDouble(0.0, -double.MAX, double.MAX, preview_decimals, edit_decimals);
		_x.set_increments(1.0, 0.1);
		_x._snap_multiplier = 10.0;
#if CROWN_GTK3
		_x.get_style_context().add_class("axis");
		_x.get_style_context().add_class("x");
#else
		_x.add_css_class("axis");
		_x.add_css_class("x");
#endif
		_y = new InputDouble(0.0, -double.MAX, double.MAX, preview_decimals, edit_decimals);
		_y.set_increments(1.0, 0.1);
		_y._snap_multiplier = 10.0;
#if CROWN_GTK3
		_y.get_style_context().add_class("axis");
		_y.get_style_context().add_class("y");
#else
		_y.add_css_class("axis");
		_y.add_css_class("y");
#endif
		_z = new InputDouble(0.0, -double.MAX, double.MAX, preview_decimals, edit_decimals);
		_z.set_increments(1.0, 0.1);
		_z._snap_multiplier = 10.0;
#if CROWN_GTK3
		_z.get_style_context().add_class("axis");
		_z.get_style_context().add_class("z");
#else
		_z.add_css_class("axis");
		_z.add_css_class("z");
#endif

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);
		_z.value_changed.connect(on_value_changed);

		_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 4);
		_box.homogeneous = true;
#if CROWN_GTK3
		_box.pack_start(_x, true);
		_box.pack_start(_y, true);
		_box.pack_start(_z, true);

		this.add(_box);
#else
		_box.append(_x);
		_box.append(_y);
		_box.append(_z);

		this.set_child(_box);
#endif
	}

	public void on_value_changed(InputField p, int undo_redo)
	{
		double x = MathUtils.rad((double)_x.value);
		double y = MathUtils.rad((double)_y.value);
		double z = MathUtils.rad((double)_z.value);
		Quaternion new_rotation = Quaternion.from_euler(x, y, z);

		if (_rotation != new_rotation) {
			_rotation = new_rotation;
			value_changed(this, undo_redo);
		}
	}
}

} /* namespace Crown */
