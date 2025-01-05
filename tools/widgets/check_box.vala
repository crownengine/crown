/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class CheckBox : Gtk.CheckButton, Property
{
	// Data
	public bool _stop_emit;

	public new void set_inconsistent(bool inconsistent)
	{
		base.set_inconsistent(inconsistent);
	}

	public bool is_inconsistent()
	{
		return this.get_inconsistent();
	}

	public Value? generic_value()
	{
		return this.value;
	}

	public void set_generic_value(Value? val)
	{
		this.value = (bool)val;
	}

	public bool value
	{
		get
		{
			return this.active;
		}
		set
		{
			_stop_emit = true;
			this.active = value;
			_stop_emit = false;
		}
	}

	// Signals
	public signal void value_changed();

	public CheckBox()
	{
		// Data
		_stop_emit = false;

		this.toggled.connect(on_value_changed);
	}

	private void on_value_changed()
	{
		if (base.get_inconsistent()) {
			base.set_inconsistent(false);
			this.value = true;
		}

		if (!_stop_emit)
			value_changed();
	}
}

} /* namespace Crown */
