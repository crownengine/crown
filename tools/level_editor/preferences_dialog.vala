/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
 * License: https://github.com/crownengine/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
public class PreferencesDialog : Gtk.Dialog
{
	// Data
	public LevelEditorApplication _application;

	// Document page.
	public ColorButtonVector3 _grid_color_button;
	public ColorButtonVector3 _grid_disabled_color_button;
	public ColorButtonVector3 _axis_x_color_button;
	public ColorButtonVector3 _axis_y_color_button;
	public ColorButtonVector3 _axis_z_color_button;
	public ColorButtonVector3 _axis_selected_color_button;
	public EntryDouble _gizmo_size_spin_button;
	public ComboBoxMap _theme_combo;
	public PropertyGridSet _document_set;

	// Viewport page.
	public EntryDouble _level_autosave_spin_button;
	public PropertyGridSet _viewport_set;

	// System page.
	public EntryDouble _undo_redo_max_size;
	public EntryDouble _log_delete_after_days;
	public EntryDouble _console_max_lines;
	public PropertyGridSet _system_set;

	// External Tools page.
	public AppChooserButton _lua_external_tool_button;
	public AppChooserButton _image_external_tool_button;
	public PropertyGridSet _external_tools_set;

	public Gtk.Notebook _notebook;

	public PreferencesDialog(LevelEditorApplication app)
	{
		this.title = "Preferences";
		this.border_width = 0;
		this.set_icon_name(CROWN_ICON_NAME);

		// Data
		_application = app;

		// Widgets
		_document_set = new PropertyGridSet();
		_document_set.border_width = 12;
		_viewport_set = new PropertyGridSet();
		_viewport_set.border_width = 12;
		_system_set = new PropertyGridSet();
		_system_set.border_width = 12;
		_external_tools_set = new PropertyGridSet();
		_external_tools_set.border_width = 12;

		// Document page.
		_grid_color_button = new ColorButtonVector3();
		_grid_color_button.value = Vector3(102.0/255.0, 102.0/255.0, 102.0/255.0);
		_grid_color_button.value_changed.connect(on_color_set);
		_grid_disabled_color_button = new ColorButtonVector3();
		_grid_disabled_color_button.value = Vector3(102.0/255.0, 102.0/255.0, 102.0/255.0);
		_grid_disabled_color_button.value_changed.connect(on_color_set);
		_axis_x_color_button = new ColorButtonVector3();
		_axis_x_color_button.value = Vector3(217.0/255.0, 0.0/255.0, 0.0/255.0);
		_axis_x_color_button.value_changed.connect(on_color_set);
		_axis_y_color_button = new ColorButtonVector3();
		_axis_y_color_button.value = Vector3(0.0/255.0, 217.0/255.0, 0.0/255.0);
		_axis_y_color_button.value_changed.connect(on_color_set);
		_axis_z_color_button = new ColorButtonVector3();
		_axis_z_color_button.value = Vector3(0.0/255.0, 0.0/255.0, 217.0/255.0);
		_axis_z_color_button.value_changed.connect(on_color_set);
		_axis_selected_color_button = new ColorButtonVector3();
		_axis_selected_color_button.value = Vector3(217.0/255.0, 217.0/255.0, 0.0/255.0);
		_axis_selected_color_button.value_changed.connect(on_color_set);

		PropertyGrid cv;
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Grid", _grid_color_button);
		cv.add_row("Grid (Disabled)", _grid_disabled_color_button);
		_document_set.add_property_grid(cv, "Grid");

		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("X Axis", _axis_x_color_button);
		cv.add_row("Y Axis", _axis_y_color_button);
		cv.add_row("Z Axis", _axis_z_color_button);
		cv.add_row("Selected", _axis_selected_color_button);
		_document_set.add_property_grid(cv, "Axes");

		_gizmo_size_spin_button = new EntryDouble(85, 10, 200);
		_gizmo_size_spin_button.value_changed.connect(on_gizmo_size_value_changed);

		_theme_combo = new ComboBoxMap(Theme.DARK
			, new string[] { "Dark", "Light" }
			, new string[] { "dark", "light" }
			);
		_theme_combo.value_changed.connect(() => {
				_application.set_theme_from_name(_theme_combo.value);
			});

		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Size", _gizmo_size_spin_button);
		_document_set.add_property_grid(cv, "Gizmo");

		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Theme", _theme_combo);
		_document_set.add_property_grid(cv, "UI");

		// Level page.
		_level_autosave_spin_button = new EntryDouble(5, 1, 60);
		_level_autosave_spin_button.value_changed.connect(on_level_autosave_value_changed);

		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Autosave (mins)", _level_autosave_spin_button);
		_viewport_set.add_property_grid(cv, "Level");

		// Memory and limits page.
		_undo_redo_max_size = new EntryDouble(8, 1, 2048);
		_log_delete_after_days = new EntryDouble(10, 0, 90);
		_console_max_lines = new EntryDouble(256, 10, 1024);

		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Undo/Redo max size (MiB)", _undo_redo_max_size);
		cv.add_row("Delete logs older than (days)", _log_delete_after_days);
		cv.add_row("Console max lines", _console_max_lines);
		_system_set.add_property_grid(cv, "Memory and Limits");

		// External tools page.
		_lua_external_tool_button = new AppChooserButton("text/plain");
		_image_external_tool_button = new AppChooserButton("image/*");
#if CROWN_PLATFORM_LINUX
		_lua_external_tool_button.show_dialog_item = true;
		_image_external_tool_button.show_dialog_item = true;
#endif
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("External Lua editor", _lua_external_tool_button);
		cv.add_row("External image editor", _image_external_tool_button);
		_external_tools_set.add_property_grid(cv, "External Editors");

		// Add pages.
		_notebook = new Gtk.Notebook();
		_notebook.append_page(_document_set, new Gtk.Label("Document"));
		_notebook.append_page(_viewport_set, new Gtk.Label("Viewport"));
		_notebook.append_page(_system_set, new Gtk.Label("System"));
		_notebook.append_page(_external_tools_set, new Gtk.Label("External Tools"));
		_notebook.vexpand = true;
		_notebook.show_border = false;

		this.get_content_area().border_width = 0;
		this.get_content_area().add(_notebook);
	}

	private void on_color_set()
	{
		_application._editor.send_script(LevelEditorApi.set_color("grid", _grid_color_button.value));
		_application._editor.send_script(LevelEditorApi.set_color("grid_disabled", _grid_disabled_color_button.value));
		_application._editor.send_script(LevelEditorApi.set_color("axis_x", _axis_x_color_button.value));
		_application._editor.send_script(LevelEditorApi.set_color("axis_y", _axis_y_color_button.value));
		_application._editor.send_script(LevelEditorApi.set_color("axis_z", _axis_z_color_button.value));
		_application._editor.send_script(LevelEditorApi.set_color("axis_selected", _axis_selected_color_button.value));
		_application._editor.send(DeviceApi.frame());
	}

	private void on_gizmo_size_value_changed()
	{
		_application._editor.send_script("Gizmo.size = %f".printf(_gizmo_size_spin_button.value));
		_application._editor.send(DeviceApi.frame());
	}

	private void on_level_autosave_value_changed()
	{
		_application.set_autosave_timer((uint)_level_autosave_spin_button.value);
	}

	public void decode(Hashtable settings)
	{
		Hashtable preferences = settings.has_key("preferences")
			? (Hashtable)settings["preferences"]
			: new Hashtable()
			;

		_grid_color_button.value          = Vector3.from_array(preferences.has_key("grid") ? (Gee.ArrayList<GLib.Value?>)preferences["grid"] : _grid_color_button.value.to_array());
		_grid_disabled_color_button.value = Vector3.from_array(preferences.has_key("grid_disabled") ? (Gee.ArrayList<GLib.Value?>)preferences["grid_disabled"] : _grid_disabled_color_button.value.to_array());
		_axis_x_color_button.value        = Vector3.from_array(preferences.has_key("axis_x") ? (Gee.ArrayList<GLib.Value?>)preferences["axis_x"] : _axis_x_color_button.value.to_array());
		_axis_y_color_button.value        = Vector3.from_array(preferences.has_key("axis_y") ? (Gee.ArrayList<GLib.Value?>)preferences["axis_y"] : _axis_y_color_button.value.to_array());
		_axis_z_color_button.value        = Vector3.from_array(preferences.has_key("axis_z") ? (Gee.ArrayList<GLib.Value?>)preferences["axis_z"] : _axis_z_color_button.value.to_array());
		_axis_selected_color_button.value = Vector3.from_array(preferences.has_key("axis_selected") ? (Gee.ArrayList<GLib.Value?>)preferences["axis_selected"] : _axis_selected_color_button.value.to_array());
		_gizmo_size_spin_button.value     = preferences.has_key("gizmo_size") ? (double)preferences["gizmo_size"] : _gizmo_size_spin_button.value;
		_level_autosave_spin_button.value = preferences.has_key("autosave_timer") ? (double)preferences["autosave_timer"] : _level_autosave_spin_button.value;
		_undo_redo_max_size.value         = (preferences.has_key("undo_redo_max_size") ? (double)preferences["undo_redo_max_size"] : _undo_redo_max_size.value);
		_log_delete_after_days.value      = preferences.has_key("log_expiration") ? (double)preferences["log_expiration"] : _log_delete_after_days.value;
		_console_max_lines.value          = preferences.has_key("console_max_lines") ? (double)preferences["console_max_lines"] : _console_max_lines.value;

		if (preferences.has_key("theme"))
			_theme_combo.value = (string)preferences["theme"];

#if CROWN_PLATFORM_WINDOWS
		_lua_external_tool_button.set_app(AppChooserButton.APP_DEFAULT, null);
		_image_external_tool_button.set_app(AppChooserButton.APP_DEFAULT, null);
#else
		// External tools.
		Hashtable external_tools = preferences.has_key("external_tools")
			? (Hashtable)preferences["external_tools"]
			: new Hashtable()
			;

		Hashtable lua_editor = external_tools.has_key("lua_editor")
			? (Hashtable)external_tools["lua_editor"]
			: new Hashtable()
			;

		string app = "";
		string app_id = "";

		if (lua_editor.has_key("app"))
			app = (string)lua_editor["app"];
		else
			app = AppChooserButton.APP_DEFAULT;

		if (lua_editor.has_key("app_id"))
			app_id = (string)lua_editor["app_id"];
		else
			app_id = null;
		_lua_external_tool_button.set_app(app, app_id);

		Hashtable image_editor = external_tools.has_key("image_editor")
			? (Hashtable)external_tools["image_editor"]
			: new Hashtable()
			;
		if (image_editor.has_key("app"))
			app = (string)image_editor["app"];
		else
			app = AppChooserButton.APP_DEFAULT;

		if (image_editor.has_key("app_id"))
			app_id = (string)image_editor["app_id"];
		else
			app_id = null;
		_image_external_tool_button.set_app(app, app_id);
#endif /* if CROWN_PLATFORM_WINDOWS */
	}

	public void encode(Hashtable settings)
	{
		Hashtable preferences = settings.has_key("preferences")
			? (Hashtable)settings["preferences"]
			: new Hashtable()
			;
		settings["preferences"] = preferences;

		preferences["grid"]           = _grid_color_button.value.to_array();
		preferences["grid_disabled"]  = _grid_disabled_color_button.value.to_array();
		preferences["axis_x"]         = _axis_x_color_button.value.to_array();
		preferences["axis_y"]         = _axis_y_color_button.value.to_array();
		preferences["axis_z"]         = _axis_z_color_button.value.to_array();
		preferences["axis_selected"]  = _axis_selected_color_button.value.to_array();
		preferences["gizmo_size"]     = _gizmo_size_spin_button.value;
		preferences["autosave_timer"] = _level_autosave_spin_button.value;
		preferences["undo_redo_max_size"] = _undo_redo_max_size.value;
		preferences["log_expiration"] = _log_delete_after_days.value;
		preferences["console_max_lines"] = _console_max_lines.value;
		preferences["theme"]          = _theme_combo.value;

		// External tools.
		string app;
		string? app_id;

		// FIXME: make proper interface so that we can have
		// for example: set(settings, "preferences.foo.bar", 42);
		Hashtable external_tools = preferences.has_key("external_tools")
			? (Hashtable)preferences["external_tools"]
			: new Hashtable()
			;
		preferences["external_tools"] = external_tools;

		Hashtable lua_editor = external_tools.has_key("lua_editor")
			? (Hashtable)external_tools["lua_editor"]
			: new Hashtable()
			;
		external_tools["lua_editor"] = lua_editor;

		app = _lua_external_tool_button.selected_app(out app_id);
		lua_editor["app"] = app;
		lua_editor["app_id"] = app_id != null ? app_id : "";

		Hashtable image_editor = external_tools.has_key("image_editor")
			? (Hashtable)external_tools["image_editor"]
			: new Hashtable()
			;
		external_tools["image_editor"] = image_editor;

		app = _image_external_tool_button.selected_app(out app_id);
		image_editor["app"] = app;
		image_editor["app_id"] = app_id != null ? app_id : "";
	}

	public void apply()
	{
		GLib.Signal.emit_by_name(_grid_color_button, "color-set");
		GLib.Signal.emit_by_name(_gizmo_size_spin_button, "value-changed");
		GLib.Signal.emit_by_name(_level_autosave_spin_button, "value-changed");
		GLib.Signal.emit_by_name(_log_delete_after_days, "value-changed");
	}
}

} /* namespace Crown */
