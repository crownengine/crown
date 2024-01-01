/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class CheckBox : Gtk.CheckButton
{
	// Data
	public bool _stop_emit;

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
		if (!_stop_emit)
			value_changed();
	}
}

} /* namespace Crown */
