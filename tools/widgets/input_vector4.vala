/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputVector4 : InputField
{
	public bool _stop_emit;
	public InputDouble _x;
	public InputDouble _y;
	public InputDouble _z;
	public InputDouble _w;
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
		// Data
		_stop_emit = false;

		// Widgets
		_x = new InputDouble(xyz.x, min.x, max.x, preview_decimals);
#if CROWN_GTK3
		_x.get_style_context().add_class("axis");
		_x.get_style_context().add_class("x");
#else
		_x.add_css_class("axis");
		_x.add_css_class("x");
#endif
		_y = new InputDouble(xyz.y, min.y, max.y, preview_decimals);
#if CROWN_GTK3
		_y.get_style_context().add_class("axis");
		_y.get_style_context().add_class("y");
#else
		_y.add_css_class("axis");
		_y.add_css_class("y");
#endif
		_z = new InputDouble(xyz.z, min.z, max.z, preview_decimals);
#if CROWN_GTK3
		_z.get_style_context().add_class("axis");
		_z.get_style_context().add_class("z");
#else
		_z.add_css_class("axis");
		_z.add_css_class("z");
#endif
		_w = new InputDouble(xyz.w, min.w, max.w, preview_decimals);
#if CROWN_GTK3
		_w.get_style_context().add_class("axis");
		_w.get_style_context().add_class("w");
#else
		_w.add_css_class("axis");
		_w.add_css_class("w");
#endif

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);
		_z.value_changed.connect(on_value_changed);
		_w.value_changed.connect(on_value_changed);

		_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 4);
#if CROWN_GTK3
		_box.pack_start(_x, true);
		_box.pack_start(_y, true);
		_box.pack_start(_z, true);
		_box.pack_start(_w, true);

		this.add(_box);
#else
		_box.append(_x);
		_box.append(_y);
		_box.append(_z);
		_box.append(_w);

		this.set_child(_box);
#endif
	}

	public void on_value_changed(InputField p, int undo_redo)
	{
		if (!_stop_emit)
			value_changed(this, undo_redo);
	}
}

} /* namespace Crown */
