/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class EntryAngle : Property, Gtk.Box
{
	public double _radians;
	public EntryDouble _degrees;

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
		this.value = (double)v;
	}

	public double value
	{
		get
		{
			return _radians;
		}
		set
		{
			_radians = value;

			// Convert to radians for displaying.
			_degrees.value_changed.disconnect(on_value_changed);
			_degrees.value = MathUtils.deg(_radians);
			_degrees.value_changed.connect(on_value_changed);
			value_changed(this);
		}
	}

	public EntryAngle(double val = 0.0, double min = 0.0, double max = 360.0)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 4);

		_degrees = new EntryDouble(val, min, max, "%.4g");
		_degrees.value_changed.connect(on_value_changed);

		this.pack_start(_degrees, true);
	}

	private void on_value_changed(Property p)
	{
		double new_radians = MathUtils.rad((double)_degrees.value);

		if (_radians != new_radians) {
			_radians = new_radians;
			value_changed(this);
		}
	}
}

} /* namespace Crown */
