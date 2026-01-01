/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public GLib.Menu make_file_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	mi = new GLib.MenuItem("New Level", null);
	mi.set_detailed_action("app.new-level");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Open Level...", null);
	mi.set_detailed_action("app.open-level");
	mi.set_attribute_value("target", new GLib.Variant.string(""));
	ms.append_item(mi);

	mi = new GLib.MenuItem("New Project...", null);
	mi.set_detailed_action("app.new-project");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Open Project...", null);
	mi.set_detailed_action("app.open-project-null");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Save", null);
	mi.set_detailed_action("app.save");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Save As...", null);
	mi.set_detailed_action("app.save-as");
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Import...", null);
	mi.set_detailed_action("app.import-null");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Preferences", null);
	mi.set_detailed_action("app.preferences");
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Deploy", null);
	mi.set_detailed_action("app.deploy");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Close Project", null);
	mi.set_detailed_action("app.close-project");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Quit", null);
	mi.set_detailed_action("app.quit");
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_grid_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	mi = new GLib.MenuItem("Show Grid", null);
	mi.set_detailed_action("app.grid-show");
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Custom", null);
	mi.set_detailed_action("app.grid-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 0));
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("0.1m", null);
	mi.set_detailed_action("app.grid-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 1));
	ms.append_item(mi);

	mi = new GLib.MenuItem("0.2m", null);
	mi.set_detailed_action("app.grid-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 2));
	ms.append_item(mi);

	mi = new GLib.MenuItem("0.5m", null);
	mi.set_detailed_action("app.grid-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 5));
	ms.append_item(mi);

	mi = new GLib.MenuItem("1m", null);
	mi.set_detailed_action("app.grid-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 10));
	ms.append_item(mi);

	mi = new GLib.MenuItem("2m", null);
	mi.set_detailed_action("app.grid-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 20));
	ms.append_item(mi);

	mi = new GLib.MenuItem("5m", null);
	mi.set_detailed_action("app.grid-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 50));
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_rotation_snap_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	mi = new GLib.MenuItem("Custom", null);
	mi.set_detailed_action("app.rotation-snap-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 0));
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("1°", null);
	mi.set_detailed_action("app.rotation-snap-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 1));
	ms.append_item(mi);

	mi = new GLib.MenuItem("15°", null);
	mi.set_detailed_action("app.rotation-snap-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 15));
	ms.append_item(mi);

	mi = new GLib.MenuItem("30°", null);
	mi.set_detailed_action("app.rotation-snap-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 30));
	ms.append_item(mi);

	mi = new GLib.MenuItem("45°", null);
	mi.set_detailed_action("app.rotation-snap-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 45));
	ms.append_item(mi);

	mi = new GLib.MenuItem("90°", null);
	mi.set_detailed_action("app.rotation-snap-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 90));
	ms.append_item(mi);

	mi = new GLib.MenuItem("180°", null);
	mi.set_detailed_action("app.rotation-snap-size");
	mi.set_attribute_value("target", new GLib.Variant("i", 180));
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_primitives_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	mi = new GLib.MenuItem("Cube", null);
	mi.set_detailed_action("app.primitive-cube");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Sphere", null);
	mi.set_detailed_action("app.primitive-sphere");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Cone", null);
	mi.set_detailed_action("app.primitive-cone");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Cylinder", null);
	mi.set_detailed_action("app.primitive-cylinder");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Plane", null);
	mi.set_detailed_action("app.primitive-plane");
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_database_editor_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	mi = new GLib.MenuItem("Undo", null);
	mi.set_detailed_action("database.undo");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Redo", null);
	mi.set_detailed_action("database.redo");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Duplicate", null);
	mi.set_detailed_action("database.duplicate");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Delete", null);
	mi.set_detailed_action("database.delete");
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_edit_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	menu.append_section(null, make_database_editor_menu());
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Place", null);
	mi.set_detailed_action("app.tool");
	mi.set_attribute_value("target", new GLib.Variant("i", ToolType.MOVE));
	ms.append_item(mi);

	mi = new GLib.MenuItem("Move", null);
	mi.set_detailed_action("app.tool");
	mi.set_attribute_value("target", new GLib.Variant("i", ToolType.PLACE));
	ms.append_item(mi);

	mi = new GLib.MenuItem("Rotate", null);
	mi.set_detailed_action("app.tool");
	mi.set_attribute_value("target", new GLib.Variant("i", ToolType.ROTATE));
	ms.append_item(mi);

	mi = new GLib.MenuItem("Scale", null);
	mi.set_detailed_action("app.tool");
	mi.set_attribute_value("target", new GLib.Variant("i", ToolType.SCALE));
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Relative Snap", null);
	mi.set_detailed_action("app.snap");
	mi.set_attribute_value("target", new GLib.Variant("i", Crown.SnapMode.RELATIVE));
	ms.append_item(mi);

	mi = new GLib.MenuItem("Absolute Snap", null);
	mi.set_detailed_action("app.snap");
	mi.set_attribute_value("target", new GLib.Variant("i", Crown.SnapMode.ABSOLUTE));
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Local Axis", null);
	mi.set_detailed_action("app.reference-system");
	mi.set_attribute_value("target", new GLib.Variant("i", Crown.ReferenceSystem.LOCAL));
	ms.append_item(mi);

	mi = new GLib.MenuItem("World Axis", null);
	mi.set_detailed_action("app.reference-system");
	mi.set_attribute_value("target", new GLib.Variant("i", Crown.ReferenceSystem.WORLD));
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Snap to Grid", null);
	mi.set_detailed_action("app.snap-to-grid");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Grid", null);
	mi.set_detailed_action("app.menu-grid");
	mi.set_submenu(make_grid_menu());
	ms.append_item(mi);

	mi = new GLib.MenuItem("Rotation Snap", null);
	mi.set_detailed_action("app.menu-rotation-snap");
	mi.set_submenu(make_rotation_snap_menu());
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_spawn_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	mi = new GLib.MenuItem("Primitives", null);
	mi.set_submenu(make_primitives_menu());
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Camera", null);
	mi.set_detailed_action("app.camera");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Light", null);
	mi.set_detailed_action("app.light");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Sound Source", null);
	mi.set_detailed_action("app.sound-source");
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Empty Unit", null);
	mi.set_detailed_action("app.unit-empty");
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Shading Environment", null);
	mi.set_detailed_action("app.shading-environment");
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_camera_view_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	for (int i = 0; i < CameraViewType.COUNT; ++i) {
		CameraViewType view = (CameraViewType)i;
		mi = new GLib.MenuItem(view.to_label(), null);
		mi.set_detailed_action("viewport.camera-view");
		mi.set_attribute_value("target", new GLib.Variant("i", view));
		ms.append_item(mi);
	}

	mi = new GLib.MenuItem("Frame Selected", null);
	mi.set_detailed_action("viewport.camera-frame-selected");
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_view_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	menu.append_section(null, make_camera_view_menu());
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Frame All", null);
	mi.set_detailed_action("app.camera-frame-all");
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_window_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	mi = new GLib.MenuItem("Show/Hide Project", null);
	mi.set_detailed_action("app.project-browser");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Show/Hide Console", null);
	mi.set_detailed_action("app.console");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Show/Hide Statusbar", null);
	mi.set_detailed_action("app.statusbar");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Show/Hide Inspector", null);
	mi.set_detailed_action("app.inspector");
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Fullscreen", null);
	mi.set_detailed_action("win.fullscreen");
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Debug Render World", null);
	mi.set_detailed_action("app.debug-render-world");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Debug Physics World", null);
	mi.set_detailed_action("app.debug-physics-world");
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_debug_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	mi = new GLib.MenuItem("Test Level", null);
	mi.set_detailed_action("app.test-level");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Run Game", null);
	mi.set_detailed_action("app.run-game");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Build Data", null);
	mi.set_detailed_action("app.build-data");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Reload All Resources", null);
	mi.set_detailed_action("app.reload-all");
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("_Restart Editor View", null);
	mi.set_detailed_action("app.restart-editor-view");
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Browse Logs...", null);
	mi.set_detailed_action("app.browse-logs");
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_help_menu()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.Menu ms = new GLib.Menu();
	GLib.MenuItem mi = null;

	mi = new GLib.MenuItem("Manual", null);
	mi.set_detailed_action("app.manual");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Report an Issue", null);
	mi.set_detailed_action("app.report-issue");
	ms.append_item(mi);

	menu.append_section(null, ms);
	ms = new GLib.Menu();

	mi = new GLib.MenuItem("Changelog", null);
	mi.set_detailed_action("app.changelog");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Donate", null);
	mi.set_detailed_action("app.donate");
	ms.append_item(mi);

	mi = new GLib.MenuItem("Credits", null);
	mi.set_detailed_action("app.credits");
	ms.append_item(mi);

	menu.append_section(null, ms);

	return menu;
}

public GLib.Menu make_menubar()
{
	GLib.Menu menu = new GLib.Menu();
	GLib.MenuItem mi = null;

	mi = new GLib.MenuItem("_File", null);
	mi.set_submenu(make_file_menu());
	menu.append_item(mi);

	mi = new GLib.MenuItem("Edit", null);
	mi.set_submenu(make_edit_menu());
	menu.append_item(mi);

	mi = new GLib.MenuItem("Spawn", null);
	mi.set_submenu(make_spawn_menu());
	menu.append_item(mi);

	mi = new GLib.MenuItem("Camera", null);
	mi.set_submenu(make_view_menu());
	menu.append_item(mi);

	mi = new GLib.MenuItem("View", null);
	mi.set_submenu(make_window_menu());
	menu.append_item(mi);

	mi = new GLib.MenuItem("Debug", null);
	mi.set_submenu(make_debug_menu());
	menu.append_item(mi);

	mi = new GLib.MenuItem("Help", null);
	mi.set_submenu(make_help_menu());
	menu.append_item(mi);

	return menu;
}

} /* namespace Crown */
