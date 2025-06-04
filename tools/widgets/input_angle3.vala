/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputAngle3 : InputField
{
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
		this.value = (Vector3)v;
	}

	public Vector3 value
	{
		get
		{
			return Vector3(MathUtils.rad((double)_x.value)
				, MathUtils.rad((double)_y.value)
				, MathUtils.rad((double)_z.value)
				);
		}
		set
		{
			_x.value_changed.disconnect(on_value_changed);
			_y.value_changed.disconnect(on_value_changed);
			_z.value_changed.disconnect(on_value_changed);
			_x.value = MathUtils.deg(value.x);
			_y.value = MathUtils.deg(value.y);
			_z.value = MathUtils.deg(value.z);
			_x.value_changed.connect(on_value_changed);
			_y.value_changed.connect(on_value_changed);
			_z.value_changed.connect(on_value_changed);
			value_changed(this);
		}
	}

	public InputAngle3(Vector3 xyz, Vector3 min, Vector3 max, int preview_decimals = 4, int edit_decimals = 5)
	{
		_x = new InputDouble(MathUtils.deg(xyz.x), MathUtils.deg(min.x), MathUtils.deg(max.x), preview_decimals, edit_decimals);
		_x.set_increments(1.0, 0.1);
		_x._snap_multiplier = 10.0;
#if CROWN_GTK3
		_x.get_style_context().add_class("axis");
		_x.get_style_context().add_class("x");
#else
		_x.hexpand = true;
		_x.add_css_class("axis");
		_x.add_css_class("x");
#endif
		_y = new InputDouble(MathUtils.deg(xyz.y), MathUtils.deg(min.y), MathUtils.deg(max.y), preview_decimals, edit_decimals);
		_y.set_increments(1.0, 0.1);
		_y._snap_multiplier = 10.0;
#if CROWN_GTK3
		_y.get_style_context().add_class("axis");
		_y.get_style_context().add_class("y");
#else
		_y.hexpand = true;
		_y.add_css_class("axis");
		_y.add_css_class("y");
#endif
		_z = new InputDouble(MathUtils.deg(xyz.z), MathUtils.deg(min.z), MathUtils.deg(max.z), preview_decimals, edit_decimals);
		_z.set_increments(1.0, 0.1);
		_z._snap_multiplier = 10.0;
#if CROWN_GTK3
		_z.get_style_context().add_class("axis");
		_z.get_style_context().add_class("z");
#else
		_z.hexpand = true;
		_z.add_css_class("axis");
		_z.add_css_class("z");
#endif

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);
		_z.value_changed.connect(on_value_changed);

		_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 4);
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
		value_changed(this, undo_redo);
	}
}

} /* namespace Crown */
