/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class EntryVector3 : Property, Gtk.Box
{
	public EntryDouble _x;
	public EntryDouble _y;
	public EntryDouble _z;
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

	public EntryVector3(Vector3 xyz, Vector3 min, Vector3 max, string preview_fmt = "%.4g")
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 4);

		_x = new EntryDouble(xyz.x, min.x, max.x, preview_fmt);
		_y = new EntryDouble(xyz.y, min.y, max.y, preview_fmt);
		_z = new EntryDouble(xyz.z, min.z, max.z, preview_fmt);

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

	private void on_value_changed()
	{
		value_changed(this);
	}
}

} /* namespace Crown */
