/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class TextureSettingsDialog : Gtk.Window
{
	public Project _project;
	public Database _database;
	public Database _texture_database;
	public Guid _texture_id;
	public PropertyGridSet _texture_set;
	public Gtk.ListStore _platforms_store;
	public Gtk.TreeView _platforms;
	public Gtk.Stack _stack;
	public bool _never_opened_before;
	public string _texture_path;
	// Input page.
	public InputResource _texture_name;
	public InputString _source;
	// Output page.
	public InputEnum _format;
	public InputBool _generate_mips;
	public InputDouble _mip_skip_smallest;
	public InputBool _normal_map;
	public InputBool _linear;
	public InputBool _premultiply_alpha;
	public Gtk.Box _box;
	public Gtk.EventControllerKey _controller_key;
	public Gtk.Button _cancel;
	public Gtk.Button _save;
	public Gtk.HeaderBar _header_bar;

	public signal void texture_saved();

	public void text_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value? platform;
		model.get_value(iter, 0, out platform);

		cell.set_property("text", ((TargetPlatform)platform).to_label());
	}

	public TextureSettingsDialog(Project project, Database database)
	{
		_project = project;
		_database = database;
		_texture_database = new Database(project);
		_texture_id = GUID_ZERO;

		create_object_types(_texture_database);

		_platforms_store = new Gtk.ListStore(1
			, typeof(TargetPlatform) // platform name
			);
		for (int p = 0; p < TargetPlatform.COUNT; ++p) {
			Gtk.TreeIter iter;
			_platforms_store.insert_with_values(out iter, -1, 0, (TargetPlatform)p, -1);
		}

		Gtk.CellRendererText text_renderer = new Gtk.CellRendererText();
		Gtk.TreeViewColumn column = new Gtk.TreeViewColumn.with_attributes("Target Platform", text_renderer, null);
		column.set_cell_data_func(text_renderer, text_func);

		_platforms = new Gtk.TreeView.with_model(_platforms_store);
		_platforms.append_column(column);
		_platforms.get_selection().set_mode(Gtk.SelectionMode.MULTIPLE);
		_platforms.get_selection().changed.connect(on_platforms_selection_changed);

		this.set_icon_name(CROWN_EDITOR_ICON_NAME);

		_texture_set = new PropertyGridSet();

		_texture_path = "";

		// Input grid.
		_texture_name = new InputResource(OBJECT_TYPE_TEXTURE, database);
		_texture_name.value_changed.connect(on_texture_resource_value_changed);

		_source = new InputString();
		_source.sensitive = false;

		PropertyGrid cv;
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Name", _texture_name);
		_texture_set.add_property_grid(cv, "Texture");

		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Source", _source);
		_texture_set.add_property_grid(cv, "Input");

		// Output grid.
		string[] texture_formats = new string[TextureFormat.COUNT];
		for (int tf = 0; tf < TextureFormat.COUNT; ++tf)
			texture_formats[tf] = ((TextureFormat)tf).to_key();

		_format = new InputEnum(texture_formats[TextureFormat.BC1]
			, texture_formats
			, texture_formats
			);
		_format.value_changed.connect(on_format_value_changed);

		_generate_mips = new InputBool();
		_generate_mips.value = true;
		_generate_mips.value_changed.connect(on_generate_mips_value_changed);

		_mip_skip_smallest = new InputDouble(0, 0, 32);
		_mip_skip_smallest.value_changed.connect(on_mip_skip_smallest_value_changed);

		_normal_map = new InputBool();
		_normal_map.value = false;
		_normal_map.value_changed.connect(on_normal_map_value_changed);

		_linear = new InputBool();
		_linear.value = false;
		_linear.value_changed.connect(on_linear_value_changed);

		_premultiply_alpha = new InputBool();
		_premultiply_alpha.value = false;
		_premultiply_alpha.value_changed.connect(on_premultiply_alpha_value_changed);

		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Format", _format);
		cv.add_row("Generate Mips", _generate_mips);
		cv.add_row("Skip Smallest Mips", _mip_skip_smallest);
		cv.add_row("Normal Map", _normal_map);
		cv.add_row("Linear", _linear);
		cv.add_row("Premultiply Alpha", _premultiply_alpha);
		_texture_set.add_property_grid(cv, "Output");

		_stack = new Gtk.Stack();
		_stack.add_named(new Gtk.Label("Select one or more platforms to change its settings"), "none-selected");
		_stack.add_named(_texture_set, "some-selected");

		_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_box.pack_start(_platforms, false, true, 0);
		_box.pack_start(_stack, false, true, 0);
		_box.vexpand = true;

		this.add(_box);

		_controller_key = new Gtk.EventControllerKey(this);
		_controller_key.key_pressed.connect((keyval, keycode, state) => {
				if (keyval == Gdk.Key.Escape) {
					close();
					return Gdk.EVENT_STOP;
				}

				return Gdk.EVENT_PROPAGATE;
			});

		_cancel = new Gtk.Button.with_label("Cancel");
		_cancel.clicked.connect(() => {
				close();
			});
		_save = new Gtk.Button.with_label("Save & Reload");
		_save.get_style_context().add_class("suggested-action");
		_save.clicked.connect(() => {
				save();
			});
		_header_bar = new Gtk.HeaderBar();
		_header_bar.title = "Texture Settings";
		_header_bar.show_close_button = true;
		_header_bar.pack_start(_cancel);
		_header_bar.pack_end(_save);
		this.set_titlebar(_header_bar);

		_never_opened_before = true;
		_stack.map.connect(on_stack_map);

		this.delete_event.connect(on_delete_event);
	}

	public void on_stack_map()
	{
		if (_never_opened_before) {
			_never_opened_before = false;

			TargetPlatform host_platform = TargetPlatform.COUNT;
#if CROWN_PLATFORM_WINDOWS
			host_platform = TargetPlatform.WINDOWS;
#elif CROWN_PLATFORM_LINUX
			host_platform = TargetPlatform.LINUX;
#endif

			if (host_platform == TargetPlatform.COUNT) {
				_platforms.get_selection().select_path(new Gtk.TreePath.first());
			} else {
				_platforms_store.foreach((model, path, iter) => {
						Value platform;
						model.get_value(iter, 0, out platform);

						if (((TargetPlatform)platform) == host_platform) {
							_platforms.get_selection().select_iter(iter);
							return true;
						}

						return false;
					});
			}
		}

		_platforms.grab_focus();
	}

	public int load_texture(string texture_name)
	{
		string new_texture_path = texture_name + ".texture";

		if (_texture_path != new_texture_path)
			save();

		_texture_database.reset();
		if (_texture_database.add_from_resource_path(out _texture_id, new_texture_path) != 0) {
			_texture_id = GUID_ZERO;
			return -1;
		}

		_texture_path = new_texture_path;

		on_platforms_selection_changed();
		return 0;
	}

	public void set_texture(string texture_name)
	{
		if (load_texture(texture_name) == 0)
			_texture_name.value = texture_name;
	}

	public void on_texture_resource_value_changed()
	{
		load_texture(_texture_name.value);
	}

	public bool are_values_equal(Value? a, Value? b)
	{
		if (a.type() != b.type())
			return false;

		if (a.holds(typeof(bool))) {
			return (bool)a == (bool)b;
		} else if (a.holds(typeof(double))) {
			return (double)a == (double)b;
		} else if (a.holds(typeof(string))) {
			return (string)a == (string)b;
		} else if (a == null && b == null) {
			return true;
		}

		return false;
	}

	public void on_platforms_selection_changed()
	{
		if (_texture_id == GUID_ZERO)
			return;

		if (_platforms.get_selection().count_selected_rows() > 0) {
			_stack.set_visible_child_full("some-selected", Gtk.StackTransitionType.NONE);
		} else {
			_stack.set_visible_child_full("none-selected", Gtk.StackTransitionType.NONE);
			return;
		}

		string property_names[] = { "source", "format", "generate_mips", "mip_skip_smallest", "normal_map", "linear", "premultiply_alpha" };
		InputField properties[] = { _source, _format, _generate_mips, _mip_skip_smallest, _normal_map, _linear, _premultiply_alpha };
		_format.value_changed.disconnect(on_format_value_changed);
		_generate_mips.value_changed.disconnect(on_generate_mips_value_changed);
		_mip_skip_smallest.value_changed.disconnect(on_mip_skip_smallest_value_changed);
		_normal_map.value_changed.disconnect(on_normal_map_value_changed);
		_linear.value_changed.disconnect(on_linear_value_changed);
		_premultiply_alpha.value_changed.disconnect(on_premultiply_alpha_value_changed);

		for (int i = 0; i < properties.length; ++i)
			properties[i].set_data("init", false);

		for (int i = 0; i < properties.length; ++i) {
			_platforms.get_selection().selected_foreach((model, path, iter) => {
					Value? platform;
					model.get_value(iter, 0, out platform);
					string key = platform_property(((TargetPlatform)platform).to_key(), property_names[i]);
					bool init = properties[i].get_data<bool>("init");

					// Try <platform>.<property> first. Fallback to <property>.
					if (!_texture_database.has_property(_texture_id, key))
						key = property_names[i];

					if (_texture_database.has_property(_texture_id, key)) {
						Value? val = _texture_database.get_property(_texture_id, key);

						if (!init) {
							properties[i].set_data("init", true);
							properties[i].set_union_value(val);
							properties[i].set_inconsistent(false);
						} else if (!are_values_equal(val, properties[i].union_value())) {
							properties[i].set_inconsistent(true);
						}
					} else {
						properties[i].set_inconsistent(true);

						if (!init) {
							properties[i].set_data("init", true);
						}
					}
				});
		}

		_format.value_changed.connect(on_format_value_changed);
		_generate_mips.value_changed.connect(on_generate_mips_value_changed);
		_mip_skip_smallest.value_changed.connect(on_mip_skip_smallest_value_changed);
		_normal_map.value_changed.connect(on_normal_map_value_changed);
		_linear.value_changed.connect(on_linear_value_changed);
		_premultiply_alpha.value_changed.connect(on_premultiply_alpha_value_changed);
	}

	public void on_format_value_changed()
	{
		on_property_value_changed("format", _format);
	}

	public void on_generate_mips_value_changed()
	{
		on_property_value_changed("generate_mips", _generate_mips);
	}

	public void on_mip_skip_smallest_value_changed()
	{
		on_property_value_changed("mip_skip_smallest", _mip_skip_smallest);
	}

	public void on_normal_map_value_changed()
	{
		on_property_value_changed("normal_map", _normal_map);
	}

	public void on_linear_value_changed()
	{
		on_property_value_changed("linear", _linear);
	}

	public void on_premultiply_alpha_value_changed()
	{
		on_property_value_changed("premultiply_alpha", _premultiply_alpha);
	}

	public void on_property_value_changed(string property_name, InputField property_value)
	{
		if (_texture_id == GUID_ZERO)
			return;

		Value val = property_value.union_value();

		// For backward compatibility.
		if (property_name == "generate_mips" || property_name == "normal_map") {
			if (_texture_database.has_property(_texture_id, property_name))
				_texture_database.set_property(_texture_id, property_name, val);
		}

		_platforms.get_selection().selected_foreach((model, path, iter) => {
				Value? platform;
				model.get_value(iter, 0, out platform);

				string key = platform_property(((TargetPlatform)platform).to_key(), property_name);

				_texture_database.set_property(_texture_id, key, val);
			});
	}

	public void save()
	{
		if (_texture_id == GUID_ZERO)
			return;

		if (_texture_database.dump(_project.absolute_path(_texture_path), _texture_id) == 0)
			texture_saved();
	}

	public string platform_property(string platform_name, string property)
	{
		return "output."
			+ platform_name
			+ "."
			+ property
			;
	}

	public bool on_delete_event(Gdk.EventAny event)
	{
		_texture_id = GUID_ZERO;
		return Gdk.EVENT_PROPAGATE;
	}
}

} /* namespace Crown */
