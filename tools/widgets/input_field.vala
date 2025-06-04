/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
#if CROWN_GTK3
public abstract class InputField : Gtk.Bin
#else
public abstract class InputField : Gtk.Box
#endif
{
	public const string INCONSISTENT_ID = "-";
	public const string INCONSISTENT_LABEL = "—";
#if !CROWN_GTK3
	protected InputField()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);
		this.hexpand = true;
	}
#endif

	public abstract void set_inconsistent(bool inconsistent);
	public abstract bool is_inconsistent();
	public abstract GLib.Value union_value();
	public abstract void set_union_value(GLib.Value v);

	/// undo_redo == 0 means the undo system is disabled.
	public signal void value_changed(InputField p, int undo_redo = 1);
#if !CROWN_GTK3
	public void set_child(Gtk.Widget child)
	{
		child.hexpand = true;
		child.halign = Gtk.Align.FILL;
		this.append(child);
	}
#endif
}

} /* namespace Crown */
