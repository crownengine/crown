/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class PropertyGrid : Gtk.Grid
{
	// Data
	public Database? _db;
	public Guid _id;
	public Guid _component_id;
	public int _rows;

	public PropertyGrid(Database? db = null)
	{
		this.row_spacing = 4;
		this.row_homogeneous = true;
		this.column_spacing = 12;

		// Data
		_db = db;
		_id = GUID_ZERO;
		_component_id = GUID_ZERO;
		_rows = 0;
	}

	public Gtk.Widget add_row(string label, Gtk.Widget w)
	{
		Gtk.Label l = new Gtk.Label(label);
		l.width_chars = 13;
		l.set_alignment(1.0f, 0.5f);

		w.hexpand = true;

		this.attach(l, 0, (int)_rows);
		this.attach(w, 1, (int)_rows);
		++_rows;

		return l;
	}

	public virtual void update()
	{
	}
}

public class PropertyGridSet : Gtk.Box
{
	public PropertyGridSet()
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);
	}

	public Expander add_property_grid(PropertyGrid cv, string label)
	{
		Gtk.Label l = new Gtk.Label(null);
		l.set_markup("<b>%s</b>".printf(label));
		l.set_alignment(0.0f, 0.5f);

		Expander e = new Expander();
		e.custom_header = l;
		e.expanded = true;
		e.add(cv);
		this.pack_start(e, false, true, 0);

		return e;
	}

	public Expander add_property_grid_optional(PropertyGrid cv, string label, CheckBox checkbox)
	{
		Gtk.Label l = new Gtk.Label(null);
		l.set_markup("<b>%s</b>".printf(label));
		l.set_alignment(0.0f, 0.5f);

		Gtk.Box b = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		b.pack_start(checkbox, false, false);
		b.pack_start(l, false, false);

		Expander e = new Expander();
		e.custom_header = b;
		e.expanded = true;
		e.add(cv);
		this.pack_start(e, false, true, 0);

		return e;
	}
}

} /* namespace Crown */
