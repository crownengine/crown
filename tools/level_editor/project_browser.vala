/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;
using Gee;

namespace Crown
{
public const Gtk.TargetEntry[] dnd_targets =
{
	{ "RESOURCE_PATH", Gtk.TargetFlags.SAME_APP, 0 },
};

// Menu to open when clicking on project's files and folders.
private Gtk.Menu? project_entry_menu_create(string type, string name)
{
	Gtk.Menu? menu;

	if (type == "<folder>") {
		if (name == "..")
			return null;

		menu = new Gtk.Menu();

		Gtk.MenuItem mi;

		mi = new Gtk.MenuItem.with_label("Import...");
		mi.activate.connect(() => {
				GLib.Application.get_default().activate_action("import", new GLib.Variant.string((string)name));
			});
		menu.add(mi);

		mi = new Gtk.SeparatorMenuItem();
		menu.add(mi);

		mi = new Gtk.MenuItem.with_label("New Script...");
		mi.activate.connect(() => {
				Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Script Name"
					, ((Gtk.Application)GLib.Application.get_default()).active_window
					, DialogFlags.MODAL
					, "Cancel"
					, ResponseType.CANCEL
					, "Ok"
					, ResponseType.OK
					, null
					);

				EntryText sb = new EntryText();
				sb.activate.connect(() => { dg.response(ResponseType.OK); });
				dg.get_content_area().add(sb);
				dg.skip_taskbar_hint = true;
				dg.show_all();

				if (dg.run() == (int)ResponseType.OK) {
					if (sb.text.strip() == "") {
						dg.destroy();
						return;
					}

					var tuple = new GLib.Variant.tuple({(string)name, sb.text, true});
					GLib.Application.get_default().activate_action("create-script", tuple);
				}

				dg.destroy();
			});
		menu.add(mi);

		mi = new Gtk.MenuItem.with_label("New Script (Unit)...");
		mi.activate.connect(() => {
				Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Script Name"
					, ((Gtk.Application)GLib.Application.get_default()).active_window
					, DialogFlags.MODAL
					, "Cancel"
					, ResponseType.CANCEL
					, "Ok"
					, ResponseType.OK
					, null
					);

				EntryText sb = new EntryText();
				sb.activate.connect(() => { dg.response(ResponseType.OK); });
				dg.get_content_area().add(sb);
				dg.skip_taskbar_hint = true;
				dg.show_all();

				if (dg.run() == (int)ResponseType.OK) {
					if (sb.text.strip() == "") {
						dg.destroy();
						return;
					}

					var tuple = new GLib.Variant.tuple({(string)name, sb.text, false});
					GLib.Application.get_default().activate_action("create-script", tuple);
				}

				dg.destroy();
			});
		menu.add(mi);

		mi = new Gtk.SeparatorMenuItem();
		menu.add(mi);

		mi = new Gtk.MenuItem.with_label("New Unit...");
		mi.activate.connect(() => {
				Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Unit Name"
					, ((Gtk.Application)GLib.Application.get_default()).active_window
					, DialogFlags.MODAL
					, "Cancel"
					, ResponseType.CANCEL
					, "Ok"
					, ResponseType.OK
					, null
					);

				EntryText sb = new EntryText();
				sb.activate.connect(() => { dg.response(ResponseType.OK); });
				dg.get_content_area().add(sb);
				dg.skip_taskbar_hint = true;
				dg.show_all();

				if (dg.run() == (int)ResponseType.OK) {
					if (sb.text.strip() == "") {
						dg.destroy();
						return;
					}
				}

				var tuple = new GLib.Variant.tuple({(string)name, sb.text});
				GLib.Application.get_default().activate_action("create-unit", tuple);

				dg.destroy();
			});
		menu.add(mi);

		mi = new Gtk.SeparatorMenuItem();
		menu.add(mi);

		mi = new Gtk.MenuItem.with_label("New Folder...");
		mi.activate.connect(() => {
				Gtk.Dialog dg = new Gtk.Dialog.with_buttons("Folder Name"
					, ((Gtk.Application)GLib.Application.get_default()).active_window
					, DialogFlags.MODAL
					, "Cancel"
					, ResponseType.CANCEL
					, "Ok"
					, ResponseType.OK
					, null
					);

				EntryText sb = new EntryText();
				sb.activate.connect(() => { dg.response(ResponseType.OK); });
				dg.get_content_area().add(sb);
				dg.skip_taskbar_hint = true;
				dg.show_all();

				if (dg.run() == (int)ResponseType.OK) {
					if (sb.text.strip() == "") {
						dg.destroy();
						return;
					}

					var tuple = new GLib.Variant.tuple({(string)name, sb.text});
					GLib.Application.get_default().activate_action("create-directory", tuple);
				}

				dg.destroy();
			});
		menu.add(mi);

		if ((string)name != ProjectStore.ROOT_FOLDER) {
			mi = new Gtk.MenuItem.with_label("Delete Folder");
			mi.activate.connect(() => {
					Gtk.MessageDialog md = new Gtk.MessageDialog(((Gtk.Application)GLib.Application.get_default()).active_window
						, Gtk.DialogFlags.MODAL
						, Gtk.MessageType.WARNING
						, Gtk.ButtonsType.NONE
						, "Delete Folder " + (string)name + "?"
						);

					Gtk.Widget btn;
					md.add_button("_Cancel", ResponseType.CANCEL);
					btn = md.add_button("_Delete", ResponseType.YES);
					btn.get_style_context().add_class(Gtk.STYLE_CLASS_DESTRUCTIVE_ACTION);

					md.set_default_response(ResponseType.CANCEL);

					int rt = md.run();
					md.destroy();

					if (rt != (int)ResponseType.YES)
						return;

					GLib.Application.get_default().activate_action("delete-directory", new GLib.Variant.string((string)name));
				});
			menu.add(mi);
		}
	} else { // If file
		menu = new Gtk.Menu();

		Gtk.MenuItem mi;

		mi = new Gtk.MenuItem.with_label("Delete File");
		mi.activate.connect(() => {
				GLib.Application.get_default().activate_action("delete-file", new GLib.Variant.string(ResourceId.path((string)type, (string)name)));
			});
		menu.add(mi);

		mi = new Gtk.MenuItem.with_label("Open Containing Folder...");
		mi.activate.connect(() => {
				GLib.Application.get_default().activate_action("open-containing", new GLib.Variant.string(name));
			});
		menu.add(mi);
	}

	// Add shared menu items.
	Gtk.MenuItem mi;

	mi = new Gtk.SeparatorMenuItem();
	menu.add(mi);

	mi = new Gtk.MenuItem.with_label("Copy Path");
	mi.activate.connect(() => {
			string path;
			if (type == "<folder>")
				path = name;
			else
				path = ResourceId.path(type, name);

			GLib.Application.get_default().activate_action("copy-path", new GLib.Variant.string(path));
		});
	menu.add(mi);

	if (type != "<folder>" || name != "") {
		mi = new Gtk.MenuItem.with_label("Add to Favorites");
		mi.activate.connect(() => {
				var tuple = new GLib.Variant.tuple({type, name});
				GLib.Application.get_default().activate_action("favorite-resource", tuple);
			});
		menu.add(mi);
	}

	return menu;
}

// Menu to open when clicking on favorites' entries.
private Gtk.Menu? favorites_entry_menu_create(string type, string name)
{
	Gtk.Menu? menu;

	menu = new Gtk.Menu();

	Gtk.MenuItem mi;

	mi = new Gtk.MenuItem.with_label("Open Containing Folder...");
	mi.activate.connect(() => {
			GLib.Application.get_default().activate_action("open-containing", new GLib.Variant.string(name));
		});
	menu.add(mi);

	mi = new Gtk.SeparatorMenuItem();
	menu.add(mi);

	mi = new Gtk.MenuItem.with_label("Copy Path");
	mi.activate.connect(() => {
			string path;
			if (type == "<folder>")
				path = name;
			else
				path = ResourceId.path(type, name);

			GLib.Application.get_default().activate_action("copy-path", new GLib.Variant.string(path));
		});
	menu.add(mi);

	mi = new Gtk.MenuItem.with_label("Remove from Favorites");
	mi.activate.connect(() => {
			var tuple = new GLib.Variant.tuple({type, name});
			GLib.Application.get_default().activate_action("unfavorite-resource", tuple);
		});
	menu.add(mi);

	mi = new Gtk.MenuItem.with_label("Reveal");
	mi.activate.connect(() => {
			var tuple = new GLib.Variant.tuple({type, name});
			GLib.Application.get_default().activate_action("reveal-resource", tuple);
		});
	menu.add(mi);

	return menu;
}

private void set_thumbnail(Gtk.CellRenderer cell, string type, string name, int icon_size, ThumbnailCache thumbnail_cache)
{
	// https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
	if (type == "<folder>")
		cell.set_property("icon-name", "browser-folder-symbolic");
	else if ((string)type == "<favorites>")
		cell.set_property("icon-name", "browser-favorites");
	else if ((string)type == "state_machine")
		cell.set_property("icon-name", "text-x-generic-symbolic");
	else if ((string)type == "config")
		cell.set_property("icon-name", "text-x-generic-symbolic");
	else if ((string)type == "font")
		cell.set_property("icon-name", "font-x-generic-symbolic");
	else if ((string)type == "level")
		cell.set_property("icon-name", "text-x-generic-symbolic");
	else if ((string)type == "material")
		cell.set_property("pixbuf", thumbnail_cache.get(type, name, icon_size));
	else if ((string)type == "mesh")
		cell.set_property("icon-name", "text-x-generic-symbolic");
	else if ((string)type == "package")
		cell.set_property("icon-name", "package-x-generic-symbolic");
	else if ((string)type == "physics_config")
		cell.set_property("icon-name", "text-x-generic-symbolic");
	else if ((string)type == "lua")
		cell.set_property("icon-name", "x-office-document-symbolic");
	else if ((string)type == "unit")
		cell.set_property("pixbuf", thumbnail_cache.get(type, name, icon_size));
	else if ((string)type == "shader")
		cell.set_property("icon-name", "text-x-generic-symbolic");
	else if ((string)type == "sound")
		cell.set_property("pixbuf", thumbnail_cache.get(type, name, icon_size));
	else if ((string)type == "sprite_animation")
		cell.set_property("icon-name", "text-x-generic-symbolic");
	else if ((string)type == "sprite")
		cell.set_property("icon-name", "text-x-generic-symbolic");
	else if ((string)type == "texture")
		cell.set_property("pixbuf", thumbnail_cache.get(type, name, icon_size));
	else
		cell.set_property("icon-name", "text-x-generic-symbolic");
}
public class ProjectFolderView : Gtk.Bin
{
	public enum Column
	{
		TYPE,
		NAME,
		PIXBUF,

		COUNT
	}

	public string _selected_type;
	public string _selected_name;
	public ProjectStore _project_store;
	public ThumbnailCache _thumbnail_cache;
	public Gtk.ListStore _list_store;
	public Gtk.IconView _icon_view;
	public Gtk.CellRendererPixbuf _cell_renderer_pixbuf;
	public Gtk.CellRendererText _cell_renderer_text;
	public Gdk.Pixbuf _empty_pixbuf;
	public bool _showing_project_folder;

	public ProjectFolderView(ProjectStore project_store, ThumbnailCache thumbnail_cache)
	{
		_project_store = project_store;
		_thumbnail_cache = thumbnail_cache;

		_list_store = new Gtk.ListStore(Column.COUNT
			, typeof(string)     // Column.TYPE
			, typeof(string)     // Column.NAME
			, typeof(Gdk.Pixbuf) // Column.PIXBUF
			);
		_list_store.set_sort_column_id(Column.TYPE, Gtk.SortType.ASCENDING);

		_icon_view = new Gtk.IconView();
		_icon_view.set_model(_list_store);
		_icon_view.set_item_width(80);
		_icon_view.button_press_event.connect(on_button_pressed);
		_icon_view.enable_model_drag_source(Gdk.ModifierType.BUTTON1_MASK, dnd_targets, Gdk.DragAction.COPY);
		_icon_view.drag_data_get.connect(on_drag_data_get);
		_icon_view.drag_begin.connect_after(on_drag_begin);
		_icon_view.drag_end.connect(on_drag_end);

		// https://gitlab.gnome.org/GNOME/gtk/-/blob/3.24.43/gtk/gtkiconview.c#L5147
		_cell_renderer_text = new Gtk.CellRendererText();
		_cell_renderer_text.set("wrap-mode", Pango.WrapMode.WORD_CHAR);
		_cell_renderer_text.set("alignment", Pango.Alignment.CENTER);
		_cell_renderer_text.set("xalign", 0.5);
		_cell_renderer_text.set("yalign", 0.0);
		int wrap_width = _icon_view.item_width;
		wrap_width -= 2 * _icon_view.item_padding * 2;
		_cell_renderer_text.set("wrap-width", wrap_width);
		_cell_renderer_text.set("width", wrap_width);
		_icon_view.pack_end(_cell_renderer_text, false);
		_icon_view.set_cell_data_func(_cell_renderer_text, text_func);

		_cell_renderer_pixbuf = new Gtk.CellRendererPixbuf();
		_cell_renderer_pixbuf.stock_size = Gtk.IconSize.DIALOG;
		_icon_view.pack_start(_cell_renderer_pixbuf, false);
		_icon_view.set_cell_data_func(_cell_renderer_pixbuf, pixbuf_func);

		_empty_pixbuf = new Gdk.Pixbuf.from_data({ 0x00, 0x00, 0x00, 0x00 }, Gdk.Colorspace.RGB, true, 8, 1, 1, 4);

		_showing_project_folder = true;

		this.add(_icon_view);
	}

	private void on_drag_data_get(Gdk.DragContext context, Gtk.SelectionData data, uint info, uint time_)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_data_get.html
		GLib.List<Gtk.TreePath> selected_paths = _icon_view.get_selected_items();
		if (selected_paths.length() == 0u)
			return;

		Gtk.TreeIter selected_iter;
		_icon_view.model.get_iter(out selected_iter, selected_paths.nth(0).data);

		Value val;
		string type;
		string name;
		_icon_view.model.get_value(selected_iter, Column.TYPE, out val);
		type = (string)val;
		_icon_view.model.get_value(selected_iter, Column.NAME, out val);
		name = (string)val;

		string resource_path = ResourceId.path(type, name);
		data.set(Gdk.Atom.intern_static_string("RESOURCE_PATH"), 8, resource_path.data);
	}

	private void on_drag_begin(Gdk.DragContext context)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_begin.html
		Gtk.drag_set_icon_pixbuf(context, _empty_pixbuf, 0, 0);
	}

	private void on_drag_end(Gdk.DragContext context)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_end.html
		GLib.Application.get_default().activate_action("cancel-place", null);
	}

	private bool on_button_pressed(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_SECONDARY) {
			string type;
			string name;

			Gtk.TreePath path;
			if ((path = _icon_view.get_path_at_pos((int)ev.x, (int)ev.y)) != null) {
				_icon_view.select_path(path);
				_icon_view.scroll_to_path(path, false, 0.0f, 0.0f);

				Gtk.TreeIter iter;
				_icon_view.model.get_iter(out iter, path);

				Value val;
				_icon_view.model.get_value(iter, Column.TYPE, out val);
				type = (string)val;
				_icon_view.model.get_value(iter, Column.NAME, out val);
				name = (string)val;

			} else {
				type = _selected_type;
				name = _selected_name;
			}

			Gtk.Menu? menu;
			menu = project_entry_menu_create(type, name);
			if (_showing_project_folder)
				menu = project_entry_menu_create(type, name);
			else
				menu = favorites_entry_menu_create(type, name);

			if (menu != null) {
				menu.show_all();
				menu.popup_at_pointer(ev);
			}
		} else if (ev.button == Gdk.BUTTON_PRIMARY && ev.type == Gdk.EventType.@2BUTTON_PRESS) {
			Gtk.TreePath path;
			if ((path = _icon_view.get_path_at_pos((int)ev.x, (int)ev.y)) != null) {
				Gtk.TreeIter iter;
				_icon_view.model.get_iter(out iter, path);

				Value type;
				Value name;
				_icon_view.model.get_value(iter, Column.TYPE, out type);
				_icon_view.model.get_value(iter, Column.NAME, out name);

				if ((string)type == "<folder>") {
					string dir_name;
					if ((string)name == "..")
						dir_name = ResourceId.parent_folder((string)_selected_name);
					else
						dir_name = (string)name;

					GLib.Application.get_default().activate_action("open-directory", new GLib.Variant.string(dir_name));
				} else {
					GLib.Application.get_default().activate_action("open-resource", ResourceId.path((string)type, (string)name));
				}
			}
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private void pixbuf_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value val;
		string type;
		string name;
		model.get_value(iter, Column.TYPE, out val);
		type = (string)val;
		model.get_value(iter, Column.NAME, out val);
		name = (string)val;

		set_thumbnail(cell, type, name, 64, _thumbnail_cache);
	}

	private void text_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value type;
		Value name;
		model.get_value(iter, Column.TYPE, out type);
		model.get_value(iter, Column.NAME, out name);

		if (name == "..")
			cell.set_property("text", name);
		else
			cell.set_property("text", GLib.Path.get_basename((string)name));
	}

	public void reveal(string type, string name)
	{
		_list_store.foreach((model, path, iter) => {
				GLib.Value val;
				string store_type;
				string store_name;
				model.get_value(iter, Column.TYPE, out val);
				store_type = (string)val;
				model.get_value(iter, Column.NAME, out val);
				store_name = (string)val;

				if (store_name == name && store_type == type) {
					_icon_view.select_path(path);
					_icon_view.scroll_to_path(path, false, 0.0f, 0.0f);
					return true;
				}

				return false;
			});
	}
}

public class ProjectBrowser : Gtk.Bin
{
	// Data
	public ProjectStore _project_store;
	public ThumbnailCache _thumbnail_cache;

	// Widgets
	public Gtk.TreeModelFilter _tree_filter;
	public Gtk.TreeModelSort _tree_sort;
	public Gtk.TreeView _tree_view;
	public Gtk.TreeSelection _tree_selection;
	public Gdk.Pixbuf _empty_pixbuf;
	public ProjectFolderView _folder_view;
	public bool _show_folder_view;
	public Gtk.Button _toggle_folder_view;
	public Gtk.Box _tree_view_content;
	public Gtk.Box _folder_view_content;
	public Gtk.ScrolledWindow _scrolled_window_a;
	public Gtk.ScrolledWindow _scrolled_window_b;
	public Gtk.Paned _paned;

	public bool _hide_core_resources;

	public ProjectBrowser(ProjectStore project_store, ThumbnailCache thumbnail_cache)
	{
		// Data
		_project_store = project_store;
		_thumbnail_cache = thumbnail_cache;
		_thumbnail_cache.changed.connect(() => {
			_tree_view.queue_draw();
			_folder_view.queue_draw();
		});

		// Widgets
		_tree_filter = new Gtk.TreeModelFilter(_project_store._tree_store, null);
		_tree_filter.set_visible_func((model, iter) => {
				if (_project_store.project_root_path() != null)
					_tree_view.expand_row(_project_store.project_root_path(), false);

				Value type;
				Value name;
				model.get_value(iter, ProjectStore.Column.TYPE, out type);
				model.get_value(iter, ProjectStore.Column.NAME, out name);

				bool should_show = (string)type != null
					&& (string)name != null
					&& !row_should_be_hidden((string)type, (string)name)
					;

				if (_show_folder_view) {
					// Hide all descendants of the favorites root.
					Gtk.TreePath? path = model.get_path(iter);
					if (path != null && _project_store.favorites_root_path() != null && path.is_descendant(_project_store.favorites_root_path()))
						return false;

					return should_show && (type == "<folder>" || type == "<favorites>");
				} else {
					return should_show;
				}
			});

		_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
		_tree_sort.set_default_sort_func((model, iter_a, iter_b) => {
				Value type_a;
				Value type_b;
				model.get_value(iter_a, ProjectStore.Column.TYPE, out type_a);
				model.get_value(iter_b, ProjectStore.Column.TYPE, out type_b);

				// Favorites is always on top.
				if ((string)type_a == "<favorites>")
					return -1;
				if ((string)type_b == "<favorites>")
					return 1;

				// Then folders.
				if ((string)type_a == "<folder>") {
					if ((string)type_b != "<folder>")
						return -1;
				} else if ((string)type_b == "<folder>") {
					if ((string)type_a != "<folder>")
						return 1;
				}

				// And finally, regular files.
				Value id_a;
				Value id_b;
				model.get_value(iter_a, ProjectStore.Column.NAME, out id_a);
				model.get_value(iter_b, ProjectStore.Column.NAME, out id_b);
				return strcmp(GLib.Path.get_basename((string)id_a), GLib.Path.get_basename((string)id_b));
			});

		Gtk.CellRendererPixbuf cell_pixbuf = new Gtk.CellRendererPixbuf();
		cell_pixbuf.stock_size = Gtk.IconSize.SMALL_TOOLBAR;
		Gtk.CellRendererText cell_text = new Gtk.CellRendererText();
		Gtk.TreeViewColumn column = new Gtk.TreeViewColumn();
		column.pack_start(cell_pixbuf, false);
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_pixbuf, pixbuf_func);
		column.set_cell_data_func(cell_text, text_func);
		_tree_view = new Gtk.TreeView();
		_tree_view.append_column(column);
#if 0
		// For debugging.
		_tree_view.insert_column_with_attributes(-1
			, "Segment"
			, new Gtk.CellRendererText()
			, "text"
			, ProjectStore.Column.SEGMENT
			, null
			);
		_tree_view.insert_column_with_attributes(-1
			, "Name"
			, new Gtk.CellRendererText()
			, "text"
			, ProjectStore.Column.NAME
			, null
			);
		_tree_view.insert_column_with_attributes(-1
			, "Type"
			, new Gtk.CellRendererText()
			, "text"
			, ProjectStore.Column.TYPE
			, null
			);
#endif /* if 0 */
		_tree_view.model = _tree_sort;
		_tree_view.headers_visible = false;
		_tree_view.button_press_event.connect(on_button_pressed);

		_tree_view.enable_model_drag_source(Gdk.ModifierType.BUTTON1_MASK, dnd_targets, Gdk.DragAction.COPY);
		_tree_view.drag_data_get.connect(on_drag_data_get);
		_tree_view.drag_begin.connect_after(on_drag_begin);
		_tree_view.drag_end.connect(on_drag_end);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.BROWSE);
		_tree_selection.changed.connect(() => { update_folder_view(); });

		_empty_pixbuf = new Gdk.Pixbuf.from_data({ 0x00, 0x00, 0x00, 0x00 }, Gdk.Colorspace.RGB, true, 8, 1, 1, 4);

		_project_store._tree_store.row_inserted.connect((path, iter) => { update_folder_view(); });
		_project_store._tree_store.row_deleted.connect((path) => { update_folder_view(); });

		// Create icon view.
		_folder_view = new ProjectFolderView(_project_store, thumbnail_cache);

		// Create switch button.
		_show_folder_view = true;
		_toggle_folder_view = new Gtk.Button.from_icon_name("level-tree-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
		_toggle_folder_view.get_style_context().add_class("flat");
		_toggle_folder_view.can_focus = false;
		_toggle_folder_view.clicked.connect(() => {
				_show_folder_view = !_show_folder_view;

				if (_show_folder_view) {
					// Save the currently selected resource and a path to its parent. Those will be
					// used later, after the tree has been refiltered, to show the correct folder
					// and reveal the selected resource in the icon view.
					string? selected_type = null;
					string? selected_name = null;
					Gtk.TreePath? parent_path = null;
					Gtk.TreeModel selected_model;
					Gtk.TreeIter selected_iter;
					if (_tree_selection.get_selected(out selected_model, out selected_iter)) {
						Value val;
						selected_model.get_value(selected_iter, ProjectStore.Column.TYPE, out val);
						selected_type = (string)val;
						selected_model.get_value(selected_iter, ProjectStore.Column.NAME, out val);
						selected_name = (string)val;

						if (selected_type != "<folder>") {
							Gtk.TreeIter parent_iter;
							if (selected_model.iter_parent(out parent_iter, selected_iter))
								parent_path = _tree_view.model.get_path(parent_iter);
						}
					}

					_tree_filter.refilter();

					if (parent_path != null) {
						_tree_selection.select_path(parent_path);
						_folder_view.reveal(selected_type, selected_name);
					}

					_folder_view_content.show_all();
					_toggle_folder_view.set_image(new Gtk.Image.from_icon_name("level-tree-symbolic", Gtk.IconSize.SMALL_TOOLBAR));
				} else {
					// Save the currently selected resource. This will be used later, after the tree
					// has been refiltered, to reveal the selected resource in the tree view.
					string? selected_type = null;
					string? selected_name = null;
					GLib.List<Gtk.TreePath> selected_paths = _folder_view._icon_view.get_selected_items();
					if (selected_paths.length() == 1u) {
						Gtk.TreeIter selected_iter;
						if (_folder_view._list_store.get_iter(out selected_iter, selected_paths.nth(0).data)) {
							GLib.Value val;
							_folder_view._list_store.get_value(selected_iter, ProjectFolderView.Column.TYPE, out val);
							selected_type = (string)val;
							_folder_view._list_store.get_value(selected_iter, ProjectFolderView.Column.NAME, out val);
							selected_name = (string)val;
						}
					}

					_tree_filter.refilter();

					if (selected_type != null && selected_type != "<folder>") {
						reveal(selected_type, selected_name);
					}

					_folder_view_content.hide();
					_toggle_folder_view.set_image(new Gtk.Image.from_icon_name("browser-icon-view", Gtk.IconSize.SMALL_TOOLBAR));

					_tree_view.queue_draw(); // It doesn't draw by itself sometimes...
				}
			});

		// Create paned split-view.
		_scrolled_window_a = new Gtk.ScrolledWindow(null, null);
		_scrolled_window_a.add(_tree_view);

		_scrolled_window_b = new Gtk.ScrolledWindow(null, null);
		_scrolled_window_b.add(_folder_view);

		var _tree_view_control = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_tree_view_control.pack_end(_toggle_folder_view, false, false);

		_tree_view_content = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_tree_view_content.pack_start(_tree_view_control, false);
		_tree_view_content.pack_start(_scrolled_window_a, true, true);

		var _icon_view_control = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		// _icon_view_control.pack_end(..., false, false);

		_folder_view_content = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_folder_view_content.pack_start(_icon_view_control, false);
		_folder_view_content.pack_start(_scrolled_window_b, true, true);

		_paned = new Gtk.Paned(Gtk.Orientation.VERTICAL);
		_paned.pack1(_tree_view_content, true, false);
		_paned.pack2(_folder_view_content, true, false);
		_paned.set_position(400);

		this.add(_paned);

		_hide_core_resources = true;

		// Actions.
		GLib.ActionEntry[] action_entries =
		{
			{ "reveal-resource",     on_reveal,              "(ss)", null },
			{ "open-directory",      on_open_directory,      "s",    null },
			{ "favorite-resource",   on_favorite_resource,   "(ss)", null },
			{ "unfavorite-resource", on_unfavorite_resource, "(ss)", null }
		};
		GLib.Application.get_default().add_action_entries(action_entries, this);
	}

	private void on_drag_data_get(Gdk.DragContext context, Gtk.SelectionData data, uint info, uint time_)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_data_get.html
		Gtk.TreeModel selected_model;
		Gtk.TreeIter selected_iter;
		if (!_tree_selection.get_selected(out selected_model, out selected_iter))
			return;

		Value val;
		string type;
		string name;
		selected_model.get_value(selected_iter, ProjectStore.Column.TYPE, out val);
		type = (string)val;
		selected_model.get_value(selected_iter, ProjectStore.Column.NAME, out val);
		name = (string)val;

		string resource_path = ResourceId.path(type, name);
		data.set(Gdk.Atom.intern_static_string("RESOURCE_PATH"), 8, resource_path.data);
	}

	private void on_drag_begin(Gdk.DragContext context)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_begin.html
		Gtk.drag_set_icon_pixbuf(context, _empty_pixbuf, 0, 0);
	}

	private void on_drag_end(Gdk.DragContext context)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_end.html
		GLib.Application.get_default().activate_action("cancel-place", null);
	}

	// Returns true if the row should be hidden.
	private bool row_should_be_hidden(string type, string name)
	{
		return type == "<folder>" && name == "core" && _hide_core_resources
			|| type == "importer_settings"
			|| name == Project.LEVEL_EDITOR_TEST_NAME
			;
	}

	private void reveal(string type, string name)
	{
		string parent_type = type;
		string parent_name = name;
		Gtk.TreePath filter_path = null;
		do {
			Gtk.TreePath store_path;
			if (!_project_store.path_for_resource_type_name(out store_path, parent_type, parent_name)) {
				break;
			}

			filter_path = _tree_filter.convert_child_path_to_path(store_path);
			if (filter_path == null) {
				// Either the path is not valid or points to a non-visible row in the model.
				parent_type = "<folder>";
				parent_name = ResourceId.parent_folder(parent_name);
				continue;
			}

			Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
			if (sort_path == null) {
				// The path is not valid.
				break;
			}

			_tree_view.expand_to_path(sort_path);
			_tree_view.get_selection().select_path(sort_path);
			_tree_view.scroll_to_cell(sort_path, null, false, 0.0f, 0.0f);
			_folder_view.reveal(type, name);
		} while (filter_path == null);
	}

	private void on_reveal(GLib.SimpleAction action, GLib.Variant? param)
	{
		string type = (string)param.get_child_value(0);
		string name = (string)param.get_child_value(1);

		if (name.has_prefix("core/")) {
			_hide_core_resources = false;
			_tree_filter.refilter();
		}

		reveal(type, name);
	}

	private void on_open_directory(GLib.SimpleAction action, GLib.Variant? param)
	{
		string dir_name = param.get_string();

		if (dir_name.has_prefix("core/") || dir_name == "core") {
			_hide_core_resources = false;
			_tree_filter.refilter();
		}

		Gtk.TreePath store_path;
		if (_project_store.path_for_resource_type_name(out store_path, "<folder>", dir_name)) {
			Gtk.TreePath filter_path = _tree_filter.convert_child_path_to_path(store_path);
			if (filter_path == null) // Either the path is not valid or points to a non-visible row in the model.
				return;
			Gtk.TreePath sort_path = _tree_sort.convert_child_path_to_path(filter_path);
			if (sort_path == null) // The path is not valid.
				return;

			_tree_view.expand_to_path(sort_path);
			_tree_view.get_selection().select_path(sort_path);
		}
	}

	private void on_favorite_resource(GLib.SimpleAction action, GLib.Variant? param)
	{
		string type = (string)param.get_child_value(0);
		string name = (string)param.get_child_value(1);

		_project_store.add_to_favorites(type, name);
	}

	private void on_unfavorite_resource(GLib.SimpleAction action, GLib.Variant? param)
	{
		string type = (string)param.get_child_value(0);
		string name = (string)param.get_child_value(1);

		_project_store.remove_from_favorites(type, name);
	}

	private bool on_button_pressed(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_SECONDARY) {
			Gtk.TreePath path;
			if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, null, null)) {
				Gtk.TreeIter iter;
				_tree_view.model.get_iter(out iter, path);

				Value type;
				Value name;
				_tree_view.model.get_value(iter, ProjectStore.Column.TYPE, out type);
				_tree_view.model.get_value(iter, ProjectStore.Column.NAME, out name);

				Gtk.TreePath? filter_path = _tree_sort.convert_path_to_child_path(path);
				Gtk.TreePath? store_path = _tree_filter.convert_path_to_child_path(filter_path);
				Gtk.Menu? menu;
				if (store_path.is_descendant(_project_store.project_root_path()) || store_path.compare(_project_store.project_root_path()) == 0)
					menu = project_entry_menu_create((string)type, (string)name);
				else if (store_path.is_descendant(_project_store.favorites_root_path()))
					menu = favorites_entry_menu_create((string)type, (string)name);
				else
					menu = null;

				if (menu != null) {
					menu.show_all();
					menu.popup_at_pointer(ev);
				}
			}
		} else if (ev.button == Gdk.BUTTON_PRIMARY && ev.type == Gdk.EventType.@2BUTTON_PRESS) {
			Gtk.TreePath path;
			if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, null, null)) {
				Gtk.TreeIter iter;
				_tree_view.model.get_iter(out iter, path);

				Value type;
				_tree_view.model.get_value(iter, ProjectStore.Column.TYPE, out type);
				if ((string)type == "<folder>")
					return Gdk.EVENT_PROPAGATE;

				Value name;
				_tree_view.model.get_value(iter, ProjectStore.Column.NAME, out name);

				GLib.Application.get_default().activate_action("open-resource", ResourceId.path((string)type, (string)name));
			}
		}

		return Gdk.EVENT_PROPAGATE;
	}

	private void update_folder_view()
	{
		Gtk.TreeModel selected_model;
		Gtk.TreeIter selected_iter;
		if (!_tree_selection.get_selected(out selected_model, out selected_iter))
			return;

		// If there is a selected node.
		_folder_view._list_store.clear();

		// Get the selected node's type and name.
		string selected_type;
		string selected_name;
		Value val;
		selected_model.get_value(selected_iter, ProjectStore.Column.TYPE, out val);
		selected_type = (string)val;
		selected_model.get_value(selected_iter, ProjectStore.Column.NAME, out val);
		selected_name = (string)val;

		if (selected_type == "<folder>") {
			_folder_view._showing_project_folder = true;

			// Add parent folder.
			if (selected_name != "") {
				Gtk.TreeIter dummy;
				_folder_view._list_store.insert_with_values(out dummy
					, -1
					, ProjectFolderView.Column.TYPE
					, "<folder>"
					, ProjectFolderView.Column.NAME
					, ".."
					, -1
					);
			}

			// Fill the icon view list with paths matching the selected node's name.
			_project_store._list_store.foreach((model, path, iter) => {
					string type;
					string name;
					model.get_value(iter, ProjectStore.Column.TYPE, out val);
					type = (string)val;
					model.get_value(iter, ProjectStore.Column.NAME, out val);
					name = (string)val;

					if (row_should_be_hidden(type, name))
						return false;

					// Skip paths without common ancestor.
					if (!name.has_prefix(selected_name))
						return false;

					// Skip paths that are too deep in the hierarchy:
					// selected_name: foo
					// hierarchy:
					//   foo/bar OK
					//   foo/baz OK
					//   foo/bar/baz NOPE
					string name_suffix;
					if (selected_name == "") // Project folder.
						name_suffix = name.substring((selected_name).length);
					else if (selected_name != name) // Folder itself.
						name_suffix = name.substring((selected_name).length + 1);
					else
						return false;

					if (name_suffix.index_of_char('/') != -1)
						return false;

					// Add the path to the list.
					Gtk.TreeIter dummy;
					_folder_view._list_store.insert_with_values(out dummy
						, -1
						, ProjectFolderView.Column.TYPE
						, type
						, ProjectFolderView.Column.NAME
						, name
						, -1
						);
					return false;
				});

			_folder_view._selected_type = selected_type;
			_folder_view._selected_name = selected_name;
		} else if (selected_type == "<favorites>") {
			_folder_view._showing_project_folder = false;

			// Fill the icon view list with paths whose ancestor is the favorites root.
			_project_store._tree_store.foreach((model, path, iter) => {
					string type;
					string name;
					model.get_value(iter, ProjectStore.Column.TYPE, out val);
					type = (string)val;
					model.get_value(iter, ProjectStore.Column.NAME, out val);
					name = (string)val;

					if (!path.is_descendant(_project_store.favorites_root_path()))
						return false;

					// Add the path to the list.
					Gtk.TreeIter dummy;
					_folder_view._list_store.insert_with_values(out dummy
						, -1
						, ProjectFolderView.Column.TYPE
						, type
						, ProjectFolderView.Column.NAME
						, name
						, -1
						);
					return false;
				});
		}
	}

	public void select_project_root()
	{
		Gtk.TreePath? filter_path = _tree_filter.convert_child_path_to_path(_project_store.project_root_path());
		if (filter_path == null)
			return;

		Gtk.TreePath? sort_path = _tree_sort.convert_child_path_to_path(filter_path);
		if (sort_path == null)
			return;

		_tree_selection.select_path(sort_path);
	}

	private void pixbuf_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value val;
		string type;
		string name;
		model.get_value(iter, ProjectStore.Column.TYPE, out val);
		type = (string)val;
		model.get_value(iter, ProjectStore.Column.NAME, out val);
		name = (string)val;

		set_thumbnail(cell, type, name, 16, _thumbnail_cache);
	}

	private void text_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value name;
		Value type;
		model.get_value(iter, ProjectStore.Column.NAME, out name);
		model.get_value(iter, ProjectStore.Column.TYPE, out type);

		string basename = GLib.Path.get_basename((string)name);

		if ((string)type == "<folder>") {
			if ((string)name == "")
				cell.set_property("text", _project_store._project.name());
			else
				cell.set_property("text", basename);
		} else if ((string)type == "<favorites>") {
			cell.set_property("text", "Favorites");
		} else {
			cell.set_property("text", ResourceId.path((string)type, basename));
		}
	}
}

} /* namespace Crown */
