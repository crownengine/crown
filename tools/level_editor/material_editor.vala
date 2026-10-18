/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public delegate void MaterialEditorSaveCallback();

public class MaterialEditor : Gtk.ApplicationWindow
{
	public DatabaseEditor _database_editor;
	public Database _database;
	public string _material_name;
	public string? _material_path;
	public ShaderMetadataCatalog _catalog;
	public Gtk.Box _view;
	public InputResource _shader_resource;
	public InputEnum _shader_chooser;
	public Gtk.FlowBox _defines;
	public GLib.GenericArray<PropertyDefinition?>? _options_definition;
	public Gtk.Label _explanation;
	public PropertyGrid? _properties;

	public EditorViewport _editor_viewport;
	public RuntimeInstance _runtime;
	public GLib.SimpleAction _primitive;
	public InputResource _render_config;
	public Gtk.Label _preview_notice;
	public ShaderVariantMetadata? _loaded_variant;
	public string? _loaded_name;
	public string _values_script = "";
	public bool _preview_stale;
	public bool _restarting;
	public GLib.SourceFunc? _stop_callback;
	public MaterialDocument? _document;
	public string? _effective_library;
	public ShaderVariantMetadata? _variant;
	public DataCompiler _data_compiler;
	public SaveResourceDialog? _save_dialog;
	public Gtk.Dialog? _confirm_dialog;
	public MaterialEditorSaveCallback? _on_save_success;
	public MaterialEditorSaveCallback? _on_confirm;
	public string _pending_material_name;

	public signal void saved();

	public MaterialEditor(LevelEditorApplication application
		, PreferencesDialog preferences
		, DataCompiler data_compiler
		, Project project
		, string boot_dir
		, string console_addr
		, uint16 console_port
		)
	{
		Object(application: application);

		_material_name = "";
		_data_compiler = data_compiler;
		_database = new Database(project);
		_database_editor = new DatabaseEditor((uint32)preferences._undo_redo_max_size.value * 1024 * 1024, _database);
		this.insert_action_group("database", _database_editor._action_group);

		_database.objects_created.connect(on_objects_changed);
		_database.objects_destroyed.connect(on_objects_changed);
		_database.objects_changed.connect(on_objects_changed);

		_database_editor.load_types();

		_catalog = new ShaderMetadataCatalog();
		_catalog.load(project);

		_view = new Gtk.Box(Gtk.Orientation.VERTICAL, 12);
		_view.margin_top = _view.margin_bottom = _view.margin_start = _view.margin_end = 12;

		_shader_resource = new InputResource("shader");
		_shader_resource._nullable = true;
		_shader_resource.value_changed.connect(on_library_changed);

		_shader_chooser = new InputEnum();
		_shader_chooser.hexpand = true;
		_shader_chooser.value_changed.connect(on_shader_selected);

		_defines = new Gtk.FlowBox();
		_defines.selection_mode = Gtk.SelectionMode.NONE;
		_defines.homogeneous = false;
		_defines.max_children_per_line = 4;

		Gtk.Grid header = new Gtk.Grid();
		header.column_spacing = 12;
		header.row_spacing = 4;
		header.attach(new Gtk.Label(_("Shader Resource")), 0, 0);
		header.attach(_shader_resource, 1, 0);
		header.attach(new Gtk.Label(_("Shader")), 0, 1);
		header.attach(_shader_chooser, 1, 1);
		header.attach(new Gtk.Label(_("Variants")), 0, 2);
		header.attach(_defines, 1, 2);

		_explanation = new Gtk.Label("");
		_explanation.wrap = true;
		_explanation.xalign = 0;
#if CROWN_GTK3
		_view.pack_start(header, false);
		_view.pack_start(_explanation, false);
#else
		_view.append(header);
		_view.append(_explanation);
#endif
		// Material instances are shared within a runtime, so use a dedicated one.
		_editor_viewport = new EditorViewport("material_editor"
			, data_compiler
			, _database_editor
			, project
			, boot_dir
			, console_addr
			, console_port
			, preferences
			);
		this.insert_action_group("viewport", _editor_viewport._action_group);
		_runtime = _editor_viewport._runtime;

		_runtime.connected.connect(on_editor_connected);
		_runtime.refreshed.connect(on_refreshed);
		_runtime.message_received.connect(application.on_message_received);

		_render_config = new InputResource("render_config");
		_render_config.value = default_render_config();
		_render_config.value_changed.connect(on_render_config_changed);

		project.project_loaded.connect(on_project_loaded);

		Gtk.Box config_toolbar = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 4);
		Gtk.Label config_label = new Gtk.Label(_("Render config"));
		config_label.set_tooltip_text(_("Render configuration used by the preview."));
		_render_config.hexpand = true;

		_primitive = new GLib.SimpleAction.stateful("primitive", VariantType.STRING, new Variant.string("core/units/primitives/sphere"));
		_primitive.activate.connect(on_primitive_changed);
		_editor_viewport._action_group.add_action(_primitive);

		Gtk.Box shapes = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		shapes.halign = Gtk.Align.CENTER;
		shapes.valign = Gtk.Align.END;
		shapes.margin_bottom = 8;
#if CROWN_GTK3
		shapes.get_style_context().add_class("linked");
#else
		shapes.add_css_class("linked");
#endif
		string[] names =
		{
			"core/units/primitives/sphere",
			"core/units/primitives/cube",
			"core/units/primitives/cylinder",
			"core/units/primitives/plane"
		};
		string[] labels =
		{
			_("Sphere"),
			_("Cube"),
			_("Cylinder"),
			_("Plane")
		};

		for (int i = 0; i < names.length; ++i) {
			Gtk.ToggleButton button = new Gtk.ToggleButton.with_label(labels[i]);
			button.action_name = "viewport.primitive";
			button.action_target = new Variant.string(names[i]);
#if CROWN_GTK3
			button.can_focus = false;
			shapes.pack_start(button, false);
#else
			button.focusable = false;
			shapes.append(button);
#endif
		}
		_editor_viewport._overlay.add_overlay(shapes);

		_preview_notice = new Gtk.Label("");
		_preview_notice.wrap = true;
		_preview_notice.xalign = 0;

		_editor_viewport.hexpand = true;
		_editor_viewport.vexpand = true;

		Gtk.Box preview = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
#if CROWN_GTK3
		config_toolbar.pack_start(config_label, false);
		config_toolbar.pack_start(_render_config, true);
		preview.pack_start(config_toolbar, false);
		preview.pack_start(_preview_notice, false);
		preview.pack_start(_editor_viewport, true);
#else
		config_toolbar.append(config_label);
		config_toolbar.append(_render_config);
		preview.append(config_toolbar);
		preview.append(_preview_notice);
		preview.append(_editor_viewport);
#endif /* if CROWN_GTK3 */

		Gtk.Button save_button = new Gtk.Button.with_label(_("Save & Reload"));
		save_button.clicked.connect(on_save_clicked);

		Gtk.HeaderBar header_bar = new Gtk.HeaderBar();
		header_bar.pack_end(save_button);
		this.set_titlebar(header_bar);
		this.set_default_size(1280, 720);

		Gtk.ScrolledWindow scroll;
		Gtk.Paned paned = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		GLib.Menu menu = new GLib.Menu();
		menu.append_submenu(_("Edit"), make_database_editor_menu());
		menu.append_submenu(_("Camera"), make_camera_view_menu());
		Gtk.Box box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
#if CROWN_GTK3
		save_button.get_style_context().add_class("suggested-action");
		header_bar.title = _("Material Editor");
		header_bar.show_close_button = true;
		Gtk.MenuBar menubar = new Gtk.MenuBar.from_model(menu);
		header_bar.pack_start(menubar);
		scroll = new Gtk.ScrolledWindow(null, null);
		scroll.add(_view);
		paned.pack1(preview, true, false);
		paned.pack2(scroll, false, false);
		box.pack_start(paned);
		delete_event.connect(on_close_request);
		this.add(box);
#else
		save_button.add_css_class("suggested-action");
		header_bar.title_widget = new Gtk.Label(_("Material Editor"));
		header_bar.show_title_buttons = true;
		this.show_menubar = false;
		Gtk.PopoverMenuBar menubar = new Gtk.PopoverMenuBar.from_model(menu);
		scroll = new Gtk.ScrolledWindow();
		scroll.set_child(_view);
		paned.set_start_child(preview);
		paned.set_end_child(scroll);
		paned.resize_start_child = true;
		paned.resize_end_child = false;
		paned.vexpand = true;
		box.append(menubar);
		box.append(paned);
		close_request.connect(on_close_request);
		this.set_child(box);
#endif /* if CROWN_GTK3 */
		paned.set_position(880);

		_data_compiler.start.connect(on_compile_start);
		_data_compiler.finished.connect(on_compiled);
		project.project_reset.connect(on_project_reset);

		reset();

		restart_runtime.begin();
	}

	public string default_render_config()
	{
		Project project = _database._project;
		if (project._source_dir != null) {
			try {
				GLib.HashTable<string, Value?> boot = SJSON.load_from_path(GLib.Path.build_filename(project.source_dir(), "boot.config"));
				Value? config = boot["render_config"];
				if (config != null && config.holds(typeof(string)))
					return (string)config;
			} catch (JsonSyntaxError e) {
				logw(e.message);
			}
		}
		return "core/renderer/default";
	}

	public void on_project_loaded()
	{
		_render_config.value = default_render_config();
	}

	public void on_render_config_changed(InputField input, int undo_redo)
	{
		if (undo_redo == 1 && !matches_render_config())
			restart_runtime.begin();
	}

	public bool matches_render_config()
	{
		return _editor_viewport._render_config == _render_config.value;
	}

	public bool matches_shader()
	{
		if (_loaded_variant == null || _loaded_name != _material_name
			|| _document == null)
			return false;

		MaterialDocument material = _document;
		return _effective_library == _loaded_variant._library
			&& ShaderVariantMetadata.variant_key(material.shader()) == ShaderVariantMetadata.variant_key(_loaded_variant._shader);
	}

	public void update_notice(string message = "")
	{
		if (_loaded_variant == null)
			_preview_notice.label = _("Save & Reload to generate the material preview.");
		else if (_preview_stale || !matches_shader())
			_preview_notice.label = _("Save & Reload to update preview.");
		else
			_preview_notice.label = message;
		_preview_notice.visible = _preview_notice.label != "";
	}

	public void load_preview()
	{
		_loaded_variant = _document == null ? null : _catalog._materials[ResourceId.path(OBJECT_TYPE_MATERIAL, _material_name)];
		_loaded_name = _document == null ? null : _material_name;
		_values_script = "";
		send();
	}

	public void on_compile_start()
	{
		_preview_stale = true;
		update_notice();
	}

	public void on_editor_connected(RuntimeInstance instance, string address, int port)
	{
		if (_restarting || _stop_callback != null)
			return;

		_preview_stale = false;
		if (!matches_render_config())
			restart_runtime.begin();
		else
			load_preview();
	}

	public void on_refreshed(RuntimeInstance instance, bool success)
	{
		if (!success || _stop_callback != null)
			return;

		_preview_stale = false;
		if (!matches_render_config())
			restart_runtime.begin();
		else
			load_preview();
	}

	public void on_primitive_changed(GLib.SimpleAction action, GLib.Variant? parameter)
	{
		action.set_state(parameter);
		send();
	}

	public void send()
	{
		update_notice();
		if (_loaded_variant == null || !_runtime.is_connected()
			|| _restarting || _stop_callback != null)
			return;

		if (!_preview_stale && matches_render_config() && matches_shader()) {
			MaterialDocument material = _document;
			StringBuilder values = new StringBuilder();
			string message = "";
			foreach (PropertyDefinition def in _loaded_variant._properties) {
				string name = MaterialDocument.entry_name(def);
				if (def.type == PropertyType.RESOURCE) {
					string? texture = material.texture_value(name);
					if (texture != null && !_database._project._data_index.contains(ResourceId.id(ResourceId.path(OBJECT_TYPE_TEXTURE, texture)).to_string())) {
						message = _("Save & Reload to preview the selected texture.");
						texture = null;
					}
					values.append(MaterialEditorApi.set_texture(name, texture)).append_c('\n');
				} else {
					if (def.name.has_suffix(".value.y"))
						continue;
					Quaternion value = material.uniform_value(name);
					values.append(MaterialEditorApi.set_uniform(name, value)).append_c('\n');
				}
			}
			_values_script = values.str;
			update_notice(message);
		}
		_runtime.send_script(MaterialEditorApi.set_material(_loaded_name
			, _loaded_variant._library
			, _primitive.get_state().get_string()
			) + "\n" + _values_script
			);
		_editor_viewport.frame();
	}

	// Only render-configuration changes restart the runtime. Shutdown waits for them.
	public async void restart_runtime()
	{
		if (_restarting || _stop_callback != null || _preview_stale || !_database._project._data_compiled)
			return;

		_restarting = true;
		do {
			_editor_viewport._render_config = _render_config.value;
			yield _editor_viewport.restart_runtime();
		} while (_stop_callback == null && !_preview_stale && !matches_render_config());
		_restarting = false;
		if (_stop_callback != null)
			_stop_callback();
		else if (!_preview_stale && _runtime.is_connected())
			load_preview();
	}

	public async void stop_runtime()
	{
		_stop_callback = stop_runtime.callback;
		if (_restarting)
			yield;
		yield _editor_viewport.stop_runtime();
		_stop_callback = null;
	}

	public void update_chooser()
	{
		_shader_resource.value_changed.disconnect(on_library_changed);
		_shader_chooser.value_changed.disconnect(on_shader_selected);
		string shader = _document == null ? "" : _document.shader();
		_shader_resource.value = _effective_library;
		ShaderMetadataLibrary? metadata = _effective_library == null ? null : _catalog._libraries[_effective_library];
		string base_shader = ShaderVariantMetadata.shader_name(shader);
		_shader_chooser.clear();
		if (metadata != null) {
			GLib.List<unowned string> names = metadata._shaders.get_keys();
			names.sort(GLib.strcmp);
			foreach (string name in names)
				_shader_chooser.append(name, name);
		}
		_shader_chooser.value = base_shader;
		_shader_chooser.sensitive = metadata != null && metadata._shaders.size() > 0;
		GLib.GenericArray<PropertyDefinition?>? definition = metadata == null ? null : metadata._shaders[base_shader];
		if (_options_definition != definition) {
			_options_definition = definition;
#if CROWN_GTK3
			foreach (Gtk.Widget child in _defines.get_children())
				child.destroy();
#else
			Gtk.Widget? child;
			while ((child = _defines.get_first_child()) != null)
				_defines.remove(child);
#endif
			if (definition != null) {
				foreach (PropertyDefinition? define in definition) {
					Gtk.CheckButton checkbox = new Gtk.CheckButton.with_label(define.label);
					checkbox.set_tooltip_text(define.tooltip);
					checkbox.set_data<string>("define", define.name);
					checkbox.notify["active"].connect(on_define_toggled);
#if CROWN_GTK3
					_defines.add(checkbox);
#else
					_defines.insert(checkbox, -1);
#endif
				}
			}
		}
		string[] parts = shader.split("+");
#if CROWN_GTK3
		foreach (Gtk.Widget child in _defines.get_children()) {
#else
		for (Gtk.Widget? child = _defines.get_first_child(); child != null; child = child.get_next_sibling()) {
#endif
			Gtk.CheckButton checkbox = (Gtk.CheckButton)((Gtk.FlowBoxChild)child).get_child();
			checkbox.notify["active"].disconnect(on_define_toggled);
			checkbox.active = checkbox.get_data<string>("define") in parts[1 : parts.length];
			checkbox.notify["active"].connect(on_define_toggled);
		}
#if CROWN_GTK3
		_defines.show_all();
#endif
		_shader_resource.value_changed.connect(on_library_changed);

		_shader_chooser.value_changed.connect(on_shader_selected);
	}

	public void on_define_toggled(GLib.Object button, GLib.ParamSpec pspec)
	{
		Gtk.CheckButton checkbox = (Gtk.CheckButton)button;
		string define = checkbox.get_data<string>("define");
		bool enabled = checkbox.active;
		if (_document == null)
			return;

		GLib.GenericSet<string> flags = new GLib.GenericSet<string>(GLib.str_hash, GLib.str_equal);
		string[] parts = _document.shader().split("+");
		for (int i = 1; i < parts.length; ++i) {
			if (parts[i] != "")
				flags.add(parts[i]);
		}
		if (enabled)
			flags.add(define);
		else
			flags.remove(define);
		StringBuilder value = new StringBuilder(parts[0]);
		foreach (string name in flags.get_values()) {
			value.append_c('+');
			value.append(name);
		}
		_document.select_shader(_document.shader_library(), ShaderVariantMetadata.variant_key(value.str));
	}

	public void refresh_view()
	{
		MaterialDocument? material = _document;
		if (_properties != null && (material == null || _properties._id != material._values_id)) {
#if CROWN_GTK3
			_properties.destroy();
#else
			_view.remove(_properties);
#endif
			_properties = null;
		}
		if (material == null) {
			_explanation.visible = false;
			return;
		}
		Database db = _database;
		if (_properties == null) {
			_properties = new PropertyGrid.from_object_type(db.object_type(material._values_id), db);
			_properties._id = material._values_id;
#if CROWN_GTK3
			_view.pack_start(_properties, false);
			_properties.show_all();
#else
			_view.append(_properties);
#endif
		}
		_properties.read_all_properties();
		ShaderMetadataLibrary? metadata = _effective_library == null ? null : _catalog._libraries[_effective_library];
		if (metadata != null && metadata._shaders.size() == 0)
			_explanation.label = _("The selected shader resource has no shaders.");
		else
			_explanation.label = _("Shader metadata is unavailable. Save & Reload to generate it.");
		_explanation.visible = _variant == null;
	}

	public void on_library_changed(InputField input, int undo_redo)
	{
		if (undo_redo != 1 || _document == null || _shader_resource.value == _effective_library)
			return;
		string? selected_library = _shader_resource.value;
		ShaderMetadataLibrary? metadata = selected_library == null ? null : _catalog._libraries[selected_library];
		string selected = ShaderVariantMetadata.shader_name(_document.shader());
		if (metadata != null && !metadata._shaders.contains(selected)) {
			GLib.List<unowned string> names = metadata._shaders.get_keys();
			names.sort(GLib.strcmp);
			selected = names.length() == 0 ? "" : names.data;
		}
		_document.select_shader(selected_library, selected);
	}

	public void on_shader_selected(InputField input, int undo_redo)
	{
		if (undo_redo == 1 && _document != null && _shader_chooser.value != ShaderVariantMetadata.shader_name(_document.shader()))
			_document.select_shader(_effective_library, _shader_chooser.value);
	}

	public void update_window_title()
	{
		string title = "";

		if (_database.changed())
			title += " • ";

		title += _document == null ? "unnamed" : _material_name;
		title += " - ";
		title += CROWN_EDITOR_NAME;

		if (this.title != title)
			this.title = title;
	}

	public void reset()
	{
		_loaded_variant = null;
		_loaded_name = null;
		_values_script = "";
		_document = null;
		_effective_library = null;
		_variant = null;
		_database.reset();
		_material_name = "";
		_material_path = null;
		_database_editor.selection_read({});
		update_chooser();
		refresh_view();
		update_window_title();
	}

	public void unload()
	{
		if (_runtime.is_connected()) {
			_runtime.send_script(LevelEditorApi.reset());
			_editor_viewport.frame();
		}
		reset();
	}

	public void set_material(string name)
	{
		if (_material_name == name)
			return;

		if (_confirm_dialog == null)
			_pending_material_name = name;
		confirm_changes(load_pending_material);
	}

	public void confirm_changes(owned MaterialEditorSaveCallback continuation)
	{
		if (_confirm_dialog != null) {
			_confirm_dialog.present();
			return;
		}
		if (!_database.changed()) {
			continuation();
			return;
		}
		_on_confirm = (owned)continuation;
		_confirm_dialog = new_resource_changed_dialog(this, _material_name);
		_confirm_dialog.response.connect(on_confirm_response);
#if CROWN_GTK3
		_confirm_dialog.show_all();
#else
		_confirm_dialog.present();
#endif
	}

	public void on_confirm_response(int response_id)
	{
		Gtk.Dialog dialog = _confirm_dialog;
		_confirm_dialog = null;
		MaterialEditorSaveCallback continuation = (owned)_on_confirm;
		if (response_id == Gtk.ResponseType.NO)
			continuation();
		else if (response_id == Gtk.ResponseType.YES)
			save((owned)continuation);
		dialog.destroy();
	}

	public void load_pending_material()
	{
		do_set_material(_pending_material_name);
	}

	public void do_set_material(string name)
	{
		unload();
		string resource_path = ResourceId.path(OBJECT_TYPE_MATERIAL, name);
		string path = _database._project.absolute_path(resource_path);
		_material_name = name;
		_material_path = path;
		Guid id;
		LoadError err = _database.load_from_path(out id, path, resource_path);
		if (err == LoadError.NOT_FOUND) {
			UndoRedo? undo = _database.disable_undo();
			id = Guid.new_guid();
			_database.create(id, STRING_ID_64(OBJECT_TYPE_MATERIAL, 0xeac0b497876adedf));
			_database.set_name(id, GLib.Path.get_basename(name));
			_database.restore_undo(undo);
		} else if (err != LoadError.SUCCESS) {
			update_window_title();
			return;
		}
		_document = new MaterialDocument(_database, id);
		_database_editor.selection_set({ id });
		refresh_document();
		load_preview();
		update_window_title();
	}

	public bool do_save(string path)
	{
		if (_document == null)
			return false;

		string display_name = _material_name;
		string? resource_name = ResourceId.name(ResourceId.normalize(_database._project.resource_filename(path)));
		if (resource_name != null)
			display_name = resource_name;
		if (_document.save(path) != 0) {
			_material_path = null;
			Gtk.MessageDialog dialog = new Gtk.MessageDialog(this
				, Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT
				, Gtk.MessageType.WARNING
				, Gtk.ButtonsType.NONE
				, _("Unable to save object '%s'").printf(display_name)
				);
			dialog.add_button(_("_Ok"), Gtk.ResponseType.OK);
			dialog.set_default_response(Gtk.ResponseType.OK);
			dialog.response.connect(dialog.destroy);
#if CROWN_GTK3
			dialog.show_all();
#else
			dialog.show();
#endif
			update_window_title();
			return false;
		}
		_material_name = display_name;
		_material_path = path;
		saved();
		update_window_title();
		return true;
	}

	public void save_as(string? filename, owned MaterialEditorSaveCallback? on_save_success = null)
	{
		if (filename != null) {
			if (do_save(filename) && on_save_success != null)
				on_save_success();
			return;
		}
		if (_save_dialog != null) {
			_save_dialog.present();
			return;
		}
		_on_save_success = (owned)on_save_success;
		_save_dialog = new SaveResourceDialog(_("Save As...")
			, this
			, OBJECT_TYPE_MATERIAL
			, _material_name == "" ? "" : GLib.Path.get_basename(_material_name)
			, _database._project
			);
		_save_dialog.safer_response.connect(on_save_dialog_response);
		_save_dialog.show();
	}

	public void save(owned MaterialEditorSaveCallback? on_save_success = null)
	{
		if (_document == null)
			return;

		save_as(_material_path, (owned)on_save_success);
	}

	public void on_save_dialog_response(int response_id, string? path)
	{
		SaveResourceDialog dialog = _save_dialog;
		_save_dialog = null;
		MaterialEditorSaveCallback? on_save_success = (owned)_on_save_success;
		if (response_id == Gtk.ResponseType.ACCEPT && path != null
			&& do_save(path) && on_save_success != null)
			on_save_success();
		dialog.destroy();
	}

	public void on_save_clicked()
	{
		save();
	}

	public void on_objects_changed(Guid?[] ids, uint32 flags)
	{
		if (_document == null)
			return;

		_database_editor.selection_read({ _document._id });
		bool source_changed = false;
		foreach (Guid? id in ids)
			source_changed = source_changed || id == _document._id;
		if (source_changed)
			refresh_document();
		else
			refresh_view();
		send();
		update_window_title();
	}

	public void close_and_unload()
	{
		this.hide();
		unload();
	}

#if CROWN_GTK3
	public bool on_close_request(Gdk.EventAny event)
#else
	public bool on_close_request()
#endif
	{
		confirm_changes(close_and_unload);
		return Gdk.EVENT_STOP;
	}

	public void refresh_document()
	{
		if (_document != null) {
			ShaderVariantMetadata? compiled = _catalog._materials[ResourceId.path(OBJECT_TYPE_MATERIAL, _material_name)];
			_effective_library = _document.shader_library() ?? _catalog.resolve_library(_document.shader()) ?? (compiled == null ? null : compiled._library);
			_variant = _catalog.variant(_effective_library, _document.shader());
			_document.apply_metadata(_variant);
		}
		update_chooser();
		refresh_view();
	}

	public void on_project_reset()
	{
		_catalog.clear();
		_render_config.value_changed.disconnect(on_render_config_changed);
		_render_config.value = "core/renderer/default";
		_render_config.value_changed.connect(on_render_config_changed);
		unload();
	}

	public void on_compiled(bool success)
	{
		if (!success) {
			_preview_notice.label = _("Compilation failed. Save & Reload to update preview.");
			_preview_notice.visible = true;
			return;
		}
		_catalog.load(_database._project);
		refresh_document();
		if (_editor_viewport._editor_view == null || !matches_render_config()) {
			_preview_stale = false;
			restart_runtime.begin();
		} else if (_runtime.is_connected() && _runtime._revision == _data_compiler._revision) {
			_preview_stale = false;
			load_preview();
		}
	}

	public override void dispose()
	{
		_data_compiler.start.disconnect(on_compile_start);
		_render_config.value_changed.disconnect(on_render_config_changed);
		_runtime.connected.disconnect(on_editor_connected);
		_runtime.refreshed.disconnect(on_refreshed);
		_database._project.project_loaded.disconnect(on_project_loaded);
		_data_compiler.finished.disconnect(on_compiled);
		_database._project.project_reset.disconnect(on_project_reset);
		_database.objects_created.disconnect(on_objects_changed);
		_database.objects_destroyed.disconnect(on_objects_changed);
		_database.objects_changed.disconnect(on_objects_changed);
		_on_save_success = null;
		_on_confirm = null;
		if (_save_dialog != null)
			_save_dialog.destroy();
		if (_confirm_dialog != null)
			_confirm_dialog.destroy();
		_save_dialog = null;
		_confirm_dialog = null;
		base.dispose();
	}
}

} /* namespace Crown */
