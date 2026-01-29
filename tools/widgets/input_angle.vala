/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputAngle : InputField
{
	public double _radians;
	public InputDouble _degrees;

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

	public InputAngle(double val = 0.0, double min = 0.0, double max = 360.0)
	{
		_degrees = new InputDouble(val, min, max);
		_degrees.value_changed.connect(on_value_changed);

		this.add(_degrees);
	}

	public void on_value_changed(InputField p)
	{
		double new_radians = MathUtils.rad((double)_degrees.value);

		if (_radians != new_radians) {
			_radians = new_radians;
			value_changed(this);
		}
	}
}

} /* namespace Crown */
