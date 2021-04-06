/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
public class PreferencesDialog : Gtk.Dialog
{
	// Data
	public LevelEditorApplication _application;

	// Widgets
	public ColorButtonVector3 _grid_color_button;
	public ColorButtonVector3 _grid_disabled_color_button;
	public ColorButtonVector3 _axis_x_color_button;
	public ColorButtonVector3 _axis_y_color_button;
	public ColorButtonVector3 _axis_z_color_button;
	public ColorButtonVector3 _axis_selected_color_button;
	public EntryDouble _gizmo_size_spin_button;
	public Grid _document_grid;

	public EntryDouble _level_autosave_spin_button;
	public Grid _viewport_grid;

	public EntryDouble _log_delete_after_days;
	public EntryDouble _console_max_lines;
	public Grid _system_grid;

	public PropertyGridSet _document_set;
	public PropertyGridSet _viewport_set;
	public PropertyGridSet _system_set;
	public Gtk.Notebook _notebook;

	public PreferencesDialog(LevelEditorApplication app)
	{
		this.title = "Preferences";
		this.border_width = 0;

		// Data
		_application = app;

		// Widgets
		_document_set = new PropertyGridSet();
		_document_set.border_width = 12;
		_viewport_set = new PropertyGridSet();
		_viewport_set.border_width = 12;
		_system_set = new PropertyGridSet();
		_system_set.border_width = 12;

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
		cv.add_row(           "Grid", _grid_color_button);
		cv.add_row("Grid (Disabled)", _grid_disabled_color_button);
		_document_set.add_property_grid(cv, "Grid");

		cv = new PropertyGrid();
		cv.add_row(  "X Axis", _axis_x_color_button);
		cv.add_row(  "Y Axis", _axis_y_color_button);
		cv.add_row(  "Z Axis", _axis_z_color_button);
		cv.add_row("Selected", _axis_selected_color_button);
		_document_set.add_property_grid(cv, "Axes");

		_gizmo_size_spin_button = new EntryDouble(85, 10, 200);
		_gizmo_size_spin_button.value_changed.connect(on_gizmo_size_value_changed);

		cv = new PropertyGrid();
		cv.add_row("Size", _gizmo_size_spin_button);
		_document_set.add_property_grid(cv, "Gizmo");

		_level_autosave_spin_button = new EntryDouble(5, 1, 60);
		_level_autosave_spin_button.value_changed.connect(on_level_autosave_value_changed);

		cv = new PropertyGrid();
		cv.add_row("Autosave (mins)", _level_autosave_spin_button);
		_viewport_set.add_property_grid(cv, "Level");

		_log_delete_after_days = new EntryDouble(10, 0, 90);
		_console_max_lines = new EntryDouble(256, 10, 1024);
		cv = new PropertyGrid();
		cv.add_row("Delete logs older than (days)", _log_delete_after_days);
		cv.add_row("Console max lines", _console_max_lines);
		_system_set.add_property_grid(cv, "Memory and Limits");

		_notebook = new Gtk.Notebook();
		_notebook.append_page(_document_set, new Gtk.Label("Document"));
		_notebook.append_page(_viewport_set, new Gtk.Label("Viewport"));
		_notebook.append_page(_system_set, new Gtk.Label("System"));
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

	public void load(Hashtable preferences)
	{
		_grid_color_button.value          = Vector3.from_array(preferences.has_key("grid") ? (Gee.ArrayList<GLib.Value?>)preferences["grid"] : _grid_color_button.value.to_array());
		_grid_disabled_color_button.value = Vector3.from_array(preferences.has_key("grid_disabled") ? (Gee.ArrayList<GLib.Value?>)preferences["grid_disabled"] : _grid_disabled_color_button.value.to_array());
		_axis_x_color_button.value        = Vector3.from_array(preferences.has_key("axis_x") ? (Gee.ArrayList<GLib.Value?>)preferences["axis_x"] : _axis_x_color_button.value.to_array());
		_axis_y_color_button.value        = Vector3.from_array(preferences.has_key("axis_y") ? (Gee.ArrayList<GLib.Value?>)preferences["axis_y"] : _axis_y_color_button.value.to_array());
		_axis_z_color_button.value        = Vector3.from_array(preferences.has_key("axis_z") ? (Gee.ArrayList<GLib.Value?>)preferences["axis_z"] : _axis_z_color_button.value.to_array());
		_axis_selected_color_button.value = Vector3.from_array(preferences.has_key("axis_selected") ? (Gee.ArrayList<GLib.Value?>)preferences["axis_selected"] : _axis_selected_color_button.value.to_array());
		_gizmo_size_spin_button.value     = preferences.has_key("gizmo_size") ? (double)preferences["gizmo_size"] : _gizmo_size_spin_button.value;
		_level_autosave_spin_button.value = preferences.has_key("autosave_timer") ? (double)preferences["autosave_timer"] : _level_autosave_spin_button.value;
		_log_delete_after_days.value      = preferences.has_key("log_expiration") ? (double)preferences["log_expiration"] : _log_delete_after_days.value;
		_console_max_lines.value          = preferences.has_key("console_max_lines") ? (double)preferences["console_max_lines"] : _console_max_lines.value;
	}

	public void save(Hashtable preferences)
	{
		preferences["grid"]           = _grid_color_button.value.to_array();
		preferences["grid_disabled"]  = _grid_disabled_color_button.value.to_array();
		preferences["axis_x"]         = _axis_x_color_button.value.to_array();
		preferences["axis_y"]         = _axis_y_color_button.value.to_array();
		preferences["axis_z"]         = _axis_z_color_button.value.to_array();
		preferences["axis_selected"]  = _axis_selected_color_button.value.to_array();
		preferences["gizmo_size"]     = _gizmo_size_spin_button.value;
		preferences["autosave_timer"] = _level_autosave_spin_button.value;
		preferences["log_expiration"] = _log_delete_after_days.value;
		preferences["console_max_lines"] = _console_max_lines.value;
	}

	public void apply()
	{
		GLib.Signal.emit_by_name(_grid_color_button, "color-set");
		GLib.Signal.emit_by_name(_gizmo_size_spin_button, "value-changed");
		GLib.Signal.emit_by_name(_level_autosave_spin_button, "value-changed");
		GLib.Signal.emit_by_name(_log_delete_after_days, "value-changed");
	}
}

}
