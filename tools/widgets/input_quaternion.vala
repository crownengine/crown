/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputQuaternion : InputField, Gtk.Box
{
	public Quaternion _rotation;
	public InputDouble _x;
	public InputDouble _y;
	public InputDouble _z;
	public Gtk.Label _x_label;
	public Gtk.Label _y_label;
	public Gtk.Label _z_label;
	public Gtk.Box _x_box;
	public Gtk.Box _y_box;
	public Gtk.Box _z_box;

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

	public InputQuaternion()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 4);

		_rotation = QUATERNION_IDENTITY;
		_x = new InputDouble(0.0, -180.0, 180.0, "%.4g");
		_y = new InputDouble(0.0, -180.0, 180.0, "%.4g");
		_z = new InputDouble(0.0, -180.0, 180.0, "%.4g");

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);
		_z.value_changed.connect(on_value_changed);

		_x_label = new Gtk.Label("X");
		_x_label.get_style_context().add_class("axis");
		_x_label.get_style_context().add_class("x");
		_y_label = new Gtk.Label("Y");
		_y_label.get_style_context().add_class("axis");
		_y_label.get_style_context().add_class("y");
		_z_label = new Gtk.Label("Z");
		_z_label.get_style_context().add_class("axis");
		_z_label.get_style_context().add_class("z");

		_x_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_x_box.pack_start(_x_label, false);
		_x_box.pack_start(_x, true);

		_y_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_y_box.pack_start(_y_label, false);
		_y_box.pack_start(_y, true);

		_z_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_z_box.pack_start(_z_label, false);
		_z_box.pack_start(_z, true);

		this.pack_start(_x_box, true);
		this.pack_start(_y_box, true);
		this.pack_start(_z_box, true);
	}

	private void on_value_changed(InputField p)
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
