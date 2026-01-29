/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public interface InputField : Gtk.Widget
{
	public const string INCONSISTENT_ID = "-";
	public const string INCONSISTENT_LABEL = "—";

	public abstract void set_inconsistent(bool inconsistent);
	public abstract bool is_inconsistent();
	public abstract GLib.Value union_value();
	public abstract void set_union_value(GLib.Value v);

	/// undo_redo == 0 means the undo system is disabled.
	public signal void value_changed(InputField p, int undo_redo = 1);
}

} /* namespace Crown */
