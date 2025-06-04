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
#if !CROWN_GTK3
		this.add_css_class("toolbar");
#endif

		add_tool_buttons();
		add_snap_buttons();
		add_reference_system_buttons();
		add_snap_to_grid_buttons();
	}

	public void add_tool_buttons()
	{
#if CROWN_GTK3
		this.pack_start(make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.PLACE), IconTheme.TOOL_PLACE));
		this.pack_start(make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.MOVE), IconTheme.TOOL_MOVE));
		this.pack_start(make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.ROTATE), IconTheme.TOOL_ROTATE));
#else
		this.append(make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.PLACE), IconTheme.TOOL_PLACE));
		this.append(make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.MOVE), IconTheme.TOOL_MOVE));
		this.append(make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.ROTATE), IconTheme.TOOL_ROTATE));
#endif
		var last = make_toggle_button("app.tool", new GLib.Variant.int32(ToolType.SCALE), IconTheme.TOOL_SCALE);
		last.margin_bottom = last.margin_bottom + 8;
#if CROWN_GTK3
		this.pack_start(last);
#else
		this.append(last);
#endif
	}

	public void add_snap_buttons()
	{
#if CROWN_GTK3
		this.pack_start(make_toggle_button("app.snap", new GLib.Variant.int32(SnapMode.RELATIVE), IconTheme.REFERENCE_LOCAL));
#else
		this.append(make_toggle_button("app.snap", new GLib.Variant.int32(SnapMode.RELATIVE), IconTheme.REFERENCE_LOCAL));
#endif
		var last = make_toggle_button("app.snap", new GLib.Variant.int32(SnapMode.ABSOLUTE), IconTheme.REFERENCE_WORLD);
		last.margin_bottom = last.margin_bottom + 8;
#if CROWN_GTK3
		this.pack_start(last);
#else
		this.append(last);
#endif
	}

	public void add_reference_system_buttons()
	{
#if CROWN_GTK3
		this.pack_start(make_toggle_button("app.reference-system", new GLib.Variant.int32(ReferenceSystem.LOCAL), IconTheme.AXIS_LOCAL));
#else
		this.append(make_toggle_button("app.reference-system", new GLib.Variant.int32(ReferenceSystem.LOCAL), IconTheme.AXIS_LOCAL));
#endif
		var last = make_toggle_button("app.reference-system", new GLib.Variant.int32(ReferenceSystem.WORLD), IconTheme.AXIS_WORLD);
		last.margin_bottom = last.margin_bottom + 8;
#if CROWN_GTK3
		this.pack_start(last);
#else
		this.append(last);
#endif
	}

	public void add_snap_to_grid_buttons()
	{
		var last = make_toggle_button("app.snap-to-grid", null, IconTheme.SNAP_TO_GRID);
		last.margin_bottom = last.margin_bottom + 8;
#if CROWN_GTK3
		this.pack_start(last);
#else
		this.append(last);
#endif
	}

	public Gtk.ToggleButton make_toggle_button(string action_name
		, GLib.Variant? action_target
		, string icon_name
#if CROWN_GTK3
		, Gtk.IconSize icon_size = Gtk.IconSize.LARGE_TOOLBAR
#endif
		)
	{
		var btn = new Gtk.ToggleButton();
		btn.action_name = action_name;
		if (action_target != null)
			btn.action_target = action_target;
#if CROWN_GTK3
		btn.can_focus = false;
		btn.get_style_context().add_class("flat");
		btn.get_style_context().add_class("image-button");

		var img = new Gtk.Image.from_icon_name(icon_name, icon_size);
#else
		btn.focusable = false;
		btn.add_css_class("flat");
		btn.add_css_class("image-button");

		var img = new Gtk.Image.from_icon_name(icon_name);
#endif
		img.margin_bottom
			= img.margin_end
			= img.margin_start
			= img.margin_top
			= 8
			;
#if CROWN_GTK3
		btn.add(img);
#else
		btn.set_child(img);
#endif

		return btn;
	}
}

} /* namespace Crown */
