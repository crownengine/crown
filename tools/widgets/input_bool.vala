/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputBool : InputField, Gtk.CheckButton
{
	public new void set_inconsistent(bool inconsistent)
	{
		base.set_inconsistent(inconsistent);
	}

	public bool is_inconsistent()
	{
		return this.get_inconsistent();
	}

	public GLib.Value union_value()
	{
		return this.value;
	}

	public void set_union_value(GLib.Value v)
	{
		this.value = (bool)v;
	}

	public bool value
	{
		get
		{
			return this.active;
		}
		set
		{
			this.active = value;
		}
	}

	public InputBool()
	{
		// Data
		this.toggled.connect(on_value_changed);
	}

	public void on_value_changed()
	{
		if (base.get_inconsistent()) {
			base.set_inconsistent(false);
			this.value = true;
		}

		value_changed(this);
	}
}

} /* namespace Crown */
