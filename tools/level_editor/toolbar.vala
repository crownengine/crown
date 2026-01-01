/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class Toolbar : Gtk.Box
{
	public Toolbar()
	{
		this.orientation = Gtk.Orientation.VERTICAL;
		this.halign = Gtk.Align.START;
		this.valign = Gtk.Align.START;
		this.margin_top = 8;
		this.margin_start = 8;

		add_tool_buttons();
		add_snap_buttons();
		add_reference_system_buttons();
		add_snap_to_grid_buttons();
	}

	public void add_tool_buttons()
	{
		this.pack_start(make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.PLACE), "tool-place"));
		this.pack_start(make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.MOVE), "tool-move"));
		this.pack_start(make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.ROTATE), "tool-rotate"));
		var last = make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.SCALE), "tool-scale");
		last.margin_bottom = last.margin_bottom + 8;
		this.pack_start(last);
	}

	public void add_snap_buttons()
	{
		this.pack_start(make_toggle_button("app.snap", new GLib.Variant.int32(SnapMode.RELATIVE), "reference-local"));
		var last = make_toggle_button("app.snap", new GLib.Variant.int32(SnapMode.ABSOLUTE), "reference-world");
		last.margin_bottom = last.margin_bottom + 8;
		this.pack_start(last);
	}

	public void add_reference_system_buttons()
	{
		this.pack_start(make_toggle_button("app.reference-system", new GLib.Variant.int32(ReferenceSystem.LOCAL), "axis-local"));
		var last = make_toggle_button("app.reference-system", new GLib.Variant.int32(ReferenceSystem.WORLD), "axis-world");
		last.margin_bottom = last.margin_bottom + 8;
		this.pack_start(last);
	}

	public void add_snap_to_grid_buttons()
	{
		var last = make_toggle_button("app.snap-to-grid", null, "snap-to-grid");
		last.margin_bottom = last.margin_bottom + 8;
		this.pack_start(last);
	}

	public Gtk.ToggleButton make_toggle_button(string action_name
		, GLib.Variant? action_target
		, string icon_name
		, Gtk.IconSize icon_size = Gtk.IconSize.LARGE_TOOLBAR
		)
	{
		var btn = new Gtk.ToggleButton();
		btn.action_name = action_name;
		if (action_target != null)
			btn.action_target = action_target;
		btn.can_focus = false;
		btn.get_style_context().add_class("flat");
		btn.get_style_context().add_class("image-button");

		var img = new Gtk.Image.from_icon_name(icon_name, icon_size);
		img.margin_bottom
			= img.margin_end
			= img.margin_start
			= img.margin_top
			= 8
			;
		btn.add(img);

		return btn;
	}
}

} /* namespace Crown */
