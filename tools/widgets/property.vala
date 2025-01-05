/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public interface Property : GLib.Object
{
	public const string INCONSISTENT_ID = "-";
	public const string INCONSISTENT_LABEL = "—";

	public abstract void set_inconsistent(bool inconsistent);
	public abstract bool is_inconsistent();
	public abstract Value? generic_value();
	public abstract void set_generic_value(Value? val);
}

} /* namespace Crown */
