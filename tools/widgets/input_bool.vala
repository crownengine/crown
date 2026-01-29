/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputBool : InputField
{
	public Gtk.CheckButton _check;

	public override void set_inconsistent(bool inconsistent)
	{
		_check.set_inconsistent(inconsistent);
	}

	public override bool is_inconsistent()
	{
		return _check.get_inconsistent();
	}

	public override GLib.Value union_value()
	{
		return this.value;
	}

	public override void set_union_value(GLib.Value v)
	{
		this.value = (bool)v;
	}

	public bool value
	{
		get
		{
			return _check.active;
		}
		set
		{
			_check.active = value;
		}
	}

	public InputBool()
	{
		_check = new Gtk.CheckButton();
		_check.toggled.connect(on_value_changed);

		this.add(_check);
	}

	public void on_value_changed()
	{
		if (_check.get_inconsistent()) {
			_check.set_inconsistent(false);
			this.value = true;
		}

		value_changed(this);
	}
}

} /* namespace Crown */
