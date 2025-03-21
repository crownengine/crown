/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
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

private string project_path(string type, string name)
{
	if (type == "<folder>")
		return name;

	return ResourceId.path(type, name);
}

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

					var tuple = new GLib.Variant.tuple({(string)name, sb.text});
					GLib.Application.get_default().activate_action("create-unit", tuple);
				}

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
				string path = project_path(type, name);
				GLib.Application.get_default().activate_action("delete-file", new GLib.Variant.string(path));
			});
		menu.add(mi);

		if (type == "animation_skeleton") {
			mi = new Gtk.MenuItem.with_label("New State Machine...");
			mi.activate.connect(() => {
					Gtk.Dialog dg = new Gtk.Dialog.with_buttons("State Machine Name"
						, ((Gtk.Application)GLib.Application.get_default()).active_window
						, DialogFlags.MODAL
						, "Cancel"
						, ResponseType.CANCEL
						, "Ok"
						, ResponseType.OK
						, null
						);

					EntryText sb = new EntryText();
					sb.value = GLib.Path.get_basename((string)name);
					sb.activate.connect(() => { dg.response(ResponseType.OK); });
					dg.get_content_area().add(sb);
					dg.skip_taskbar_hint = true;
					dg.show_all();

					if (dg.run() == (int)ResponseType.OK) {
						if (sb.text.strip() == "") {
							dg.destroy();
							return;
						}

						var tuple = new GLib.Variant.tuple({ResourceId.parent_folder(name), sb.text});
						GLib.Application.get_default().activate_action("create-state-machine", tuple);
					}

					dg.destroy();
				});
			menu.add(mi);
		}
	}

	// Add shared menu items.
	Gtk.MenuItem mi;

	mi = new Gtk.SeparatorMenuItem();
	menu.add(mi);

	mi = new Gtk.MenuItem.with_label("Copy Path");
	mi.activate.connect(() => {
			string path = project_path(type, name);
			GLib.Application.get_default().activate_action("copy-path", new GLib.Variant.string(path));
		});
	menu.add(mi);

	mi = new Gtk.MenuItem.with_label("Copy Name");
	mi.activate.connect(() => {
			GLib.Application.get_default().activate_action("copy-name", new GLib.Variant.string(name));
		});
	menu.add(mi);

	mi = new Gtk.MenuItem.with_label("Open Containing Folder...");
	mi.activate.connect(() => {
			GLib.Application.get_default().activate_action("open-containing", new GLib.Variant.string(name));
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
			string path = project_path(type, name);
			GLib.Application.get_default().activate_action("copy-path", new GLib.Variant.string(path));
		});
	menu.add(mi);

	mi = new Gtk.MenuItem.with_label("Copy Name");
	mi.activate.connect(() => {
			GLib.Application.get_default().activate_action("copy-name", new GLib.Variant.string(name));
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
	private static string prettify_type(string type)
	{
		if (type == "<folder>")
			return "Folder";
		else
			return type;
	}

	private static string prettify_size(uint64 size)
	{
		uint64 si_size;
		string si_unit;

		if (size >= 1024*1024*1024) {
			si_size = size / (1024*1024*1024);
			si_unit = "GiB";
		} else if (size >= 1024*1024) {
			si_size = size / (1024*1024);
			si_unit = "MiB";
		} else if (size >= 1024) {
			si_size = size / 1024;
			si_unit = "KiB";
		} else {
			si_size = size;
			si_unit = size > 1 ? "bytes" : "byte";
		}

		return "%d %s".printf((int)si_size, si_unit);
	}

	private static string prettify_time(uint64 time)
	{
		int64 mtime_secs = (int64)(time / (1000*1000*1000));
		GLib.DateTime date_time = new GLib.DateTime.from_unix_local(mtime_secs);
		return date_time.format("%d %b %Y; %H:%M:%S");
	}

	public enum Column
	{
		TYPE,
		NAME,
		PIXBUF,
		SIZE,
		MTIME,

		COUNT
	}

	public string _selected_type;
	public string _selected_name;
	public ProjectStore _project_store;
	public ThumbnailCache _thumbnail_cache;
	public Gtk.ListStore _list_store;
	public Gtk.IconView _icon_view;
	public Gtk.TreeView _list_view;
	public Gtk.CellRendererPixbuf _cell_renderer_pixbuf;
	public Gtk.CellRendererText _cell_renderer_text;
	public Gdk.Pixbuf _empty_pixbuf;
	public bool _showing_project_folder;
	public Gtk.ScrolledWindow _icon_view_window;
	public Gtk.ScrolledWindow _list_view_window;
	public Gtk.Stack _stack;

	public ProjectFolderView(ProjectStore project_store, ThumbnailCache thumbnail_cache)
	{
		_project_store = project_store;
		_thumbnail_cache = thumbnail_cache;

		_list_store = new Gtk.ListStore(Column.COUNT
			, typeof(string)     // Column.TYPE
			, typeof(string)     // Column.NAME
			, typeof(Gdk.Pixbuf) // Column.PIXBUF
			, typeof(uint64)     // Column.SIZE
			, typeof(uint64)     // Column.MTIME
			);

		_icon_view = new Gtk.IconView();
		_icon_view.set_model(_list_store);
		_icon_view.set_item_width(80);
		_icon_view.button_press_event.connect(on_button_pressed);
		_icon_view.enable_model_drag_source(Gdk.ModifierType.BUTTON1_MASK, dnd_targets, Gdk.DragAction.COPY);
		_icon_view.drag_data_get.connect(on_drag_data_get);
		_icon_view.drag_begin.connect_after(on_drag_begin);
		_icon_view.drag_end.connect(on_drag_end);
		_icon_view.has_tooltip = true;
		_icon_view.query_tooltip.connect(on_icon_view_query_tooltip);

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
		_icon_view.set_cell_data_func(_cell_renderer_text, icon_view_text_func);

		_cell_renderer_pixbuf = new Gtk.CellRendererPixbuf();
		_cell_renderer_pixbuf.stock_size = Gtk.IconSize.DIALOG;
		_icon_view.pack_start(_cell_renderer_pixbuf, false);
		_icon_view.set_cell_data_func(_cell_renderer_pixbuf, icon_view_pixbuf_func);

		_list_view = new Gtk.TreeView();
		_list_view.set_model(_list_store);
		_list_view.button_press_event.connect(on_button_pressed);
		_list_view.enable_model_drag_source(Gdk.ModifierType.BUTTON1_MASK, dnd_targets, Gdk.DragAction.COPY);
		_list_view.drag_data_get.connect(on_drag_data_get);
		_list_view.drag_begin.connect_after(on_drag_begin);
		_list_view.drag_end.connect(on_drag_end);

		var cell_pixbuf = new Gtk.CellRendererPixbuf();
		cell_pixbuf.stock_size = Gtk.IconSize.DND;
		var cell_text = new Gtk.CellRendererText();

		Gtk.TreeViewColumn column = null;
		column = new Gtk.TreeViewColumn();
		// column.title = "Thumbnail";
		column.pack_start(cell_pixbuf, false);
		column.set_cell_data_func(cell_pixbuf, list_view_pixbuf_func);
		_list_view.append_column(column);

		column = new Gtk.TreeViewColumn();
		column.title = "Basename";
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_text, list_view_basename_text_func);
		_list_view.append_column(column);

		column = new Gtk.TreeViewColumn();
		column.title = "Type";
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_text, list_view_type_text_func);
		_list_view.append_column(column);

		column = new Gtk.TreeViewColumn();
		column.title = "Size";
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_text, list_view_size_text_func);
		_list_view.append_column(column);

		column = new Gtk.TreeViewColumn();
		column.title = "Modified";
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_text, list_view_mtime_text_func);
		_list_view.append_column(column);

		column = new Gtk.TreeViewColumn();
		column.title = "Name";
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_text, list_view_name_text_func);
		_list_view.append_column(column);

		_empty_pixbuf = new Gdk.Pixbuf.from_data({ 0x00, 0x00, 0x00, 0x00 }, Gdk.Colorspace.RGB, true, 8, 1, 1, 4);

		_showing_project_folder = true;

		_icon_view_window = new Gtk.ScrolledWindow(null, null);
		_icon_view_window.add(_icon_view);
		_list_view_window = new Gtk.ScrolledWindow(null, null);
		_list_view_window.add(_list_view);

		_stack = new Gtk.Stack();
		_stack.add_named(_icon_view_window, "icon-view");
		_stack.add_named(_list_view_window, "list-view");
		_stack.set_visible_child_full("icon-view", Gtk.StackTransitionType.NONE);

		this.add(_stack);
	}

	private void on_drag_data_get(Gdk.DragContext context, Gtk.SelectionData data, uint info, uint time_)
	{
		// https://valadoc.org/gtk+-3.0/Gtk.Widget.drag_data_get.html
		Gtk.TreePath path;
		if (!selected_path(out path))
			return;

		Gtk.TreeIter iter;
		_list_store.get_iter(out iter, path);

		Value val;
		string type;
		string name;
		_list_store.get_value(iter, Column.TYPE, out val);
		type = (string)val;
		_list_store.get_value(iter, Column.NAME, out val);
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
		Gtk.TreePath? path;
		if (_stack.get_visible_child() == _icon_view_window) {
			path = _icon_view.get_path_at_pos((int)ev.x, (int)ev.y);
		} else if (_stack.get_visible_child() == _list_view_window) {
			if (!_list_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, null, null))
				path = null;
		} else {
			assert(false);
			return Gdk.EVENT_PROPAGATE;
		}

		if (ev.button == Gdk.BUTTON_SECONDARY) {
			string type;
			string name;

			if (path != null) {
				_icon_view.select_path(path);
				_icon_view.scroll_to_path(path, false, 0.0f, 0.0f);

				Gtk.TreeIter iter;
				_list_store.get_iter(out iter, path);

				Value val;
				_list_store.get_value(iter, Column.TYPE, out val);
				type = (string)val;
				_list_store.get_value(iter, Column.NAME, out val);
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
			if (path != null) {
				Gtk.TreeIter iter;
				_list_store.get_iter(out iter, path);

				Value type;
				Value name;
				_list_store.get_value(iter, Column.TYPE, out type);
				_list_store.get_value(iter, Column.NAME, out name);

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

	private void icon_view_pixbuf_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
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

	private void icon_view_text_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
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

	private void list_view_pixbuf_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value val;
		string type;
		string name;
		model.get_value(iter, Column.TYPE, out val);
		type = (string)val;
		model.get_value(iter, Column.NAME, out val);
		name = (string)val;

		set_thumbnail(cell, type, name, 32, _thumbnail_cache);
	}

	private void list_view_basename_text_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value name;
		model.get_value(iter, Column.NAME, out name);

		if (name == "..")
			cell.set_property("text", name);
		else
			cell.set_property("text", GLib.Path.get_basename((string)name));
	}

	private void list_view_type_text_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value type;
		model.get_value(iter, Column.TYPE, out type);

		cell.set_property("text", prettify_type((string)type));
	}

	private void list_view_size_text_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value val;
		model.get_value(iter, Column.SIZE, out val);
		uint64 size = (uint64)val;

		if (size != 0)
			cell.set_property("text", prettify_size(size));
		else
			cell.set_property("text", "n/a");
	}

	private void list_view_mtime_text_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value type;
		model.get_value(iter, Column.MTIME, out type);
		uint64 mtime = (uint64)type;

		if (mtime != 0)
			cell.set_property("text", prettify_time(mtime));
		else
			cell.set_property("text", "n/a");
	}

	private void list_view_name_text_func(Gtk.CellLayout cell_layout, Gtk.CellRenderer cell, Gtk.TreeModel model, Gtk.TreeIter iter)
	{
		Value name;
		model.get_value(iter, Column.NAME, out name);

		if (name == "..")
			cell.set_property("text", "n/a");
		else
			cell.set_property("text", (string)name);
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
					_list_view.get_selection().select_path(path);
					_list_view.scroll_to_cell(path, null, false, 0.0f, 0.0f);
					return true;
				}

				return false;
			});
	}

	public bool selected_path(out Gtk.TreePath? path)
	{
		if (_stack.get_visible_child() == _icon_view_window) {
			GLib.List<Gtk.TreePath> selected_paths = _icon_view.get_selected_items();
			if (selected_paths.length() == 0u) {
				path = null;
				return false;
			}

			path = selected_paths.nth(0).data;
			return true;
		} else if (_stack.get_visible_child() == _list_view_window) {
			Gtk.TreeModel selected_model;
			Gtk.TreeIter iter;
			if (!_list_view.get_selection().get_selected(out selected_model, out iter)) {
				path = null;
				return false;
			}

			path = selected_model.get_path(iter);
			return true;
		} else {
			path = null;
			return false;
		}
	}

	private bool on_icon_view_query_tooltip(int x, int y, bool keyboard_tooltip, Gtk.Tooltip tooltip)
	{
		Gtk.TreePath? path = _icon_view.get_path_at_pos(x, y);
		if (path == null)
			return false;

		Gtk.TreeIter iter;
		_list_store.get_iter(out iter, path);

		Value val;
		_list_store.get_value(iter, Column.TYPE, out val);
		string type = (string)val;
		_list_store.get_value(iter, Column.NAME, out val);
		string name = (string)val;
		_list_store.get_value(iter, Column.SIZE, out val);
		uint64 size = (uint64)val;
		_list_store.get_value(iter, Column.MTIME, out val);
		uint64 mtime = (uint64)val;

		string text = "<b>%s</b>\nType: %s\nSize: %s\nModified: %s".printf(GLib.Markup.escape_text(name)
			, GLib.Markup.escape_text(prettify_type(type))
			, size == 0 ? "n/a" : prettify_size(size)
			, mtime == 0 ? "n/a" : prettify_time(mtime)
			);
		tooltip.set_markup(text);

		return true;
	}

}

public class ProjectBrowser : Gtk.Bin
{
	public enum SortMode
	{
		NAME_AZ,
		NAME_ZA,
		TYPE_AZ,
		TYPE_ZA,
		SIZE_MIN_MAX,
		SIZE_MAX_MIN,
		LAST_MTIME,
		FIRST_MTIME,

		COUNT;

		public string to_label()
		{
			switch (this) {
			case NAME_AZ:
				return "Name A-Z";
			case NAME_ZA:
				return "Name Z-A";
			case TYPE_AZ:
				return "Type A-Z";
			case TYPE_ZA:
				return "Type Z-A";
			case SIZE_MIN_MAX:
				return "Size min-Max";
			case SIZE_MAX_MIN:
				return "Size Max-min";
			case LAST_MTIME:
				return "Last Modified";
			case FIRST_MTIME:
				return "First Modified";
			default:
				return "Unknown";
			}
		}
	}

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
	public Gtk.Button _toggle_icon_view;
	public Gtk.ListStore _folder_list_store;
	public Gtk.TreeModelSort _folder_list_sort;
	public SortMode _sort_mode;
	public Gtk.Box _sort_items_box;
	public Gtk.Popover _sort_items_popover;
	public Gtk.MenuButton _sort_items;
	public Gtk.Box _empty_favorites_box;
	public Gtk.Stack _folder_stack;
	public Gtk.Box _folder_view_content;
	public Gtk.ScrolledWindow _scrolled_window;
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
		_project_store._tree_store.row_changed.connect((path, iter) => { update_folder_view(); });
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

					Gtk.TreePath selected_path;
					if (_folder_view.selected_path(out selected_path)) {
						Gtk.TreeIter iter;
						_folder_view._list_store.get_iter(out iter, selected_path);

						GLib.Value val;
						_folder_view._list_store.get_value(iter, ProjectFolderView.Column.TYPE, out val);
						selected_type = (string)val;
						_folder_view._list_store.get_value(iter, ProjectFolderView.Column.NAME, out val);
						selected_name = (string)val;
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
		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_tree_view);

		var _tree_view_control = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_tree_view_control.pack_end(_toggle_folder_view, false, false);

		_tree_view_content = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_tree_view_content.pack_start(_tree_view_control, false);
		_tree_view_content.pack_start(_scrolled_window, true, true);

		// Setup sort menu button popover.
		_sort_items_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

		Gtk.RadioButton? button = null;
		for (int i = 0; i < SortMode.COUNT; ++i)
			button = add_sort_item(button, (SortMode)i);

		_sort_items_box.show_all();
		_sort_items_popover = new Gtk.Popover(null);
		_sort_items_popover.add(_sort_items_box);
		_sort_items = new Gtk.MenuButton();
		_sort_items.add(new Gtk.Image.from_icon_name("list-sort", Gtk.IconSize.SMALL_TOOLBAR));
		_sort_items.get_style_context().add_class("flat");
		_sort_items.can_focus = false;
		_sort_items.set_popover(_sort_items_popover);

		bool _show_icon_view = true;
		_toggle_icon_view = new Gtk.Button.from_icon_name("browser-list-view", Gtk.IconSize.SMALL_TOOLBAR);
		_toggle_icon_view.get_style_context().add_class("flat");
		_toggle_icon_view.can_focus = false;
		_toggle_icon_view.clicked.connect(() => {
				Gtk.TreePath path;
				bool any_selected = _folder_view.selected_path(out path);

				if (_show_icon_view) {
					if (any_selected) {
						Gtk.TreeIter iter;
						_folder_view._list_store.get_iter(out iter, path);
						_folder_view._list_view.get_selection().select_iter(iter);
					}

					_folder_view._stack.set_visible_child_full("list-view", Gtk.StackTransitionType.NONE);
					_toggle_icon_view.set_image(new Gtk.Image.from_icon_name("browser-icon-view", Gtk.IconSize.SMALL_TOOLBAR));
				} else {
					if (any_selected)
						_folder_view._icon_view.select_path(path);

					_folder_view._stack.set_visible_child_full("icon-view", Gtk.StackTransitionType.NONE);
					_toggle_icon_view.set_image(new Gtk.Image.from_icon_name("browser-list-view", Gtk.IconSize.SMALL_TOOLBAR));
				}

				_show_icon_view = !_show_icon_view;
			});

		var _folder_view_control = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_folder_view_control.pack_end(_toggle_icon_view, false, false);
		_folder_view_control.pack_end(_sort_items, false, false);

		_empty_favorites_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_empty_favorites_box.valign = Gtk.Align.CENTER;
		_empty_favorites_box.pack_start(new Gtk.Image.from_icon_name("browser-favorites", Gtk.IconSize.DIALOG), false, false);
		_empty_favorites_box.pack_start(new Gtk.Label("Favorites is empty"), false, false);

		_folder_stack = new Gtk.Stack();
		_folder_stack.add_named(_folder_view, "folder-view");
		_folder_stack.add_named(_empty_favorites_box, "empty-favorites");
		_folder_stack.set_visible_child_full("folder-view", Gtk.StackTransitionType.NONE);

		_folder_view_content = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_folder_view_content.pack_start(_folder_view_control, false);
		_folder_view_content.pack_start(_folder_stack, true, true);

		_paned = new Gtk.Paned(Gtk.Orientation.VERTICAL);
		_paned.pack1(_tree_view_content, true, false);
		_paned.pack2(_folder_view_content, true, false);
		_paned.set_position(400);

		this.add(_paned);

		_hide_core_resources = true;

		_folder_list_store = new Gtk.ListStore(ProjectStore.Column.COUNT
			, typeof(string) // ProjectStore.Column.NAME
			, typeof(string) // ProjectStore.Column.TYPE
			, typeof(uint64) // ProjectStore.Column.SIZE
			, typeof(uint64) // ProjectStore.Column.MTIME
			);

		_folder_list_sort = new Gtk.TreeModelSort.with_model(_folder_list_store);
		_folder_list_sort.set_default_sort_func((model, iter_a, iter_b) => {
				Value type_a;
				Value type_b;
				model.get_value(iter_a, ProjectStore.Column.TYPE, out type_a);
				model.get_value(iter_b, ProjectStore.Column.TYPE, out type_b);
				Value name_a;
				Value name_b;
				model.get_value(iter_a, ProjectStore.Column.NAME, out name_a);
				model.get_value(iter_b, ProjectStore.Column.NAME, out name_b);

				// Folders are always on top.
				if ((string)type_a == "<folder>" && (string)type_b != "<folder>") {
					return -1;
				} else if ((string)type_a != "<folder>" && (string)type_b == "<folder>") {
					return 1;
				} else if ((string)type_a == "<folder>" && (string)type_b == "<folder>") {
					// Special folders always first.
					if ((string)name_a == "..")
						return -1;
					else if ((string)name_b == "..")
						return 1;
				}

				switch (_sort_mode) {
				case SortMode.NAME_AZ:
				case SortMode.NAME_ZA: {
					int cmp = strcmp((string)name_a, (string)name_b);
					return _sort_mode == SortMode.NAME_AZ ? cmp : -cmp;
				}

				case SortMode.TYPE_AZ:
				case SortMode.TYPE_ZA: {
					int cmp = strcmp((string)type_a, (string)type_b);
					return _sort_mode == SortMode.TYPE_AZ ? cmp : -cmp;

				}

				case SortMode.SIZE_MIN_MAX:
				case SortMode.SIZE_MAX_MIN: {
					Value size_a;
					Value size_b;
					model.get_value(iter_a, ProjectStore.Column.SIZE, out size_a);
					model.get_value(iter_b, ProjectStore.Column.SIZE, out size_b);

					int cmp = (uint64)size_a <= (uint64)size_b ? -1 : 1;
					return _sort_mode == SortMode.SIZE_MIN_MAX ? cmp : -cmp;
				}

				case SortMode.LAST_MTIME:
				case SortMode.FIRST_MTIME: {
					Value mtime_a;
					Value mtime_b;
					model.get_value(iter_a, ProjectStore.Column.MTIME, out mtime_a);
					model.get_value(iter_b, ProjectStore.Column.MTIME, out mtime_b);

					int cmp = (uint64)mtime_a >= (uint64)mtime_b ? -1 : 1;
					return _sort_mode == SortMode.LAST_MTIME ? cmp : -cmp;
				}

				default:
					return 0;
				}
			});

		// Actions.
		GLib.ActionEntry[] action_entries =
		{
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
			|| _project_store._project.is_type_importable(type)
			;
	}

	public void reveal(string type, string name)
	{
		if (name.has_prefix("core/")) {
			_hide_core_resources = false;
			_tree_filter.refilter();
		}

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
		_folder_list_store.clear();
		_folder_view._list_store.clear();

		// Get the selected node's type and name.
		Gtk.TreeModel selected_model;
		Gtk.TreeIter selected_iter;
		if (!_tree_selection.get_selected(out selected_model, out selected_iter))
			return;

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
				_folder_list_store.insert_with_values(out dummy
					, -1
					, ProjectStore.Column.TYPE
					, "<folder>"
					, ProjectStore.Column.NAME
					, ".."
					, ProjectStore.Column.SIZE
					, 0u
					, ProjectStore.Column.MTIME
					, 0u
					, -1
					);
			}

			// Fill the intermediate icon view list with paths matching the selected node's name.
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
					if (ResourceId.parent_folder(name) != selected_name)
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

					uint64 size;
					uint64 mtime;
					model.get_value(iter, ProjectStore.Column.SIZE, out val);
					size = (uint64)val;
					model.get_value(iter, ProjectStore.Column.MTIME, out val);
					mtime = (uint64)val;

					// Add the path to the list.
					Gtk.TreeIter dummy;
					_folder_list_store.insert_with_values(out dummy
						, -1
						, ProjectStore.Column.TYPE
						, type
						, ProjectStore.Column.NAME
						, name
						, ProjectStore.Column.SIZE
						, size
						, ProjectStore.Column.MTIME
						, mtime
						, -1
						);
					return false;
				});

			_folder_view._selected_type = selected_type;
			_folder_view._selected_name = selected_name;

			_folder_stack.set_visible_child_full("folder-view", Gtk.StackTransitionType.NONE);
		} else if (selected_type == "<favorites>") {
			_folder_view._showing_project_folder = false;
			int num_items = 0;

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

					uint64 size;
					uint64 mtime;
					model.get_value(iter, ProjectStore.Column.SIZE, out val);
					size = (uint64)val;
					model.get_value(iter, ProjectStore.Column.MTIME, out val);
					mtime = (uint64)val;

					// Add the path to the list.
					Gtk.TreeIter dummy;
					_folder_list_store.insert_with_values(out dummy
						, -1
						, ProjectStore.Column.TYPE
						, type
						, ProjectStore.Column.NAME
						, name
						, ProjectStore.Column.SIZE
						, size
						, ProjectStore.Column.MTIME
						, mtime
						, -1
						);
					++num_items;
					return false;
				});

				if (num_items == 0)
					_folder_stack.set_visible_child_full("empty-favorites", Gtk.StackTransitionType.NONE);
				else
					_folder_stack.set_visible_child_full("folder-view", Gtk.StackTransitionType.NONE);
		}

		// Now, fill the actual icon view list with correctly sorted paths.
		_folder_list_sort.foreach((model, path, iter) => {
				string type;
				string name;
				uint64 size;
				uint64 mtime;
				model.get_value(iter, ProjectStore.Column.TYPE, out val);
				type = (string)val;
				model.get_value(iter, ProjectStore.Column.NAME, out val);
				name = (string)val;
				model.get_value(iter, ProjectStore.Column.SIZE, out val);
				size = (uint64)val;
				model.get_value(iter, ProjectStore.Column.MTIME, out val);
				mtime = (uint64)val;

				// Add the path to the list.
				Gtk.TreeIter dummy;
				_folder_view._list_store.insert_with_values(out dummy
					, -1
					, ProjectFolderView.Column.TYPE
					, type
					, ProjectFolderView.Column.NAME
					, name
					, ProjectFolderView.Column.SIZE
					, size
					, ProjectFolderView.Column.MTIME
					, mtime
					, -1
					);
				return false;
			});
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

	private Gtk.RadioButton add_sort_item(Gtk.RadioButton? group, SortMode mode)
	{
		var button = new Gtk.RadioButton.with_label_from_widget(group, mode.to_label());
		button.toggled.connect(() => {
				_sort_mode = mode;
				update_folder_view();
				_sort_items_popover.popdown();
			});
		_sort_items_box.pack_start(button, false, false);
		return button;
	}
}

} /* namespace Crown */
