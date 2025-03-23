/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gee;
using Gtk;

namespace Crown
{
public class LevelTreeView : Gtk.Box
{
	private const Gtk.TargetEntry[] TARGET_ENTRIES = {
		{ "GTK_TREE_MODEL_ROW", Gtk.TargetFlags.SAME_APP, 0 }
	};

	private enum ItemType
	{
		FOLDER,
		CAMERA,
		LIGHT,
		SOUND,
		UNIT
	}

	private enum Column
	{
		TYPE,
		GUID,
		NAME,

		COUNT
	}

	public enum SortMode
	{
		NAME_AZ,
		NAME_ZA,
		TYPE_AZ,
		TYPE_ZA,

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
			default:
				return "Unknown";
			}
		}
	}

	// Data
	private Level _level;
	private Database _db;

	// Widgets
	private EntrySearch _filter_entry;
	private Gtk.TreeStore _tree_store;
	private Gtk.TreeModelFilter _tree_filter;
	private Gtk.TreeModelSort _tree_sort;
	private Gtk.TreeView _tree_view;
	private Gtk.TreeSelection _tree_selection;
	private Gtk.ScrolledWindow _scrolled_window;
	private SortMode _sort_mode;
	private Gtk.Box _sort_items_box;
	private Gtk.Popover _sort_items_popover;
	private Gtk.MenuButton _sort_items;

	public LevelTreeView(Database db, Level level)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		// Data
		_level = level;
		_level.selection_changed.connect(on_level_selection_changed);
		_level.object_editor_name_changed.connect(on_object_editor_name_changed);

		_db = db;
		_db.key_changed.connect(on_database_key_changed);

		// Widgets
		_filter_entry = new EntrySearch();
		_filter_entry.set_placeholder_text("Search...");
		_filter_entry.search_changed.connect(on_filter_entry_text_changed);

		_tree_store = new Gtk.TreeStore(Column.COUNT
			, typeof(int)    // Column.TYPE
			, typeof(Guid)   // Column.GUID
			, typeof(string) // Column.NAME
			);

		_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
		_tree_filter.set_visible_func((model, iter) => {
				_tree_view.expand_all();

				Value type;
				Value name;
				model.get_value(iter, Column.TYPE, out type);
				model.get_value(iter, Column.NAME, out name);

				if ((int)type == ItemType.FOLDER)
					return true;

				string name_str = (string)name;
				string filter_text = _filter_entry.text.down();

				return name_str != null
					&& (filter_text == "" || name_str.down().index_of(filter_text) > -1)
					;
			});

		_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
		_tree_sort.set_default_sort_func((model, iter_a, iter_b) => {
				Value type_a;
				Value type_b;
				model.get_value(iter_a, Column.TYPE, out type_a);
				model.get_value(iter_b, Column.TYPE, out type_b);
				if ((int)type_a == ItemType.FOLDER || (int)type_b == ItemType.FOLDER)
					return -1;

				switch (_sort_mode) {
				case SortMode.NAME_AZ:
				case SortMode.NAME_ZA: {
					Value name_a;
					Value name_b;
					model.get_value(iter_a, Column.NAME, out name_a);
					model.get_value(iter_b, Column.NAME, out name_b);

					int cmp = strcmp((string)name_a, (string)name_b);
					return _sort_mode == SortMode.NAME_AZ ? cmp : -cmp;
				}

				case SortMode.TYPE_AZ:
				case SortMode.TYPE_ZA: {
					int cmp = (int)type_a - (int)type_b;
					return _sort_mode == SortMode.TYPE_AZ ? cmp : -cmp;
				}

				default:
					return 0;
				}
			});

		Gtk.TreeViewColumn column = new Gtk.TreeViewColumn();
		Gtk.CellRendererPixbuf cell_pixbuf = new Gtk.CellRendererPixbuf();
		Gtk.CellRendererText cell_text = new Gtk.CellRendererText();
		column.pack_start(cell_pixbuf, false);
		column.pack_start(cell_text, true);
		column.set_cell_data_func(cell_pixbuf, (cell_layout, cell, model, iter) => {
				Value type;
				model.get_value(iter, LevelTreeView.Column.TYPE, out type);

				if ((int)type == LevelTreeView.ItemType.FOLDER)
					cell.set_property("icon-name", "browser-folder-symbolic");
				else if ((int)type == LevelTreeView.ItemType.UNIT)
					cell.set_property("icon-name", "level-object-unit");
				else if ((int)type == LevelTreeView.ItemType.SOUND)
					cell.set_property("icon-name", "level-object-sound");
				else if ((int)type == LevelTreeView.ItemType.LIGHT)
					cell.set_property("icon-name", "level-object-light");
				else if ((int)type == LevelTreeView.ItemType.CAMERA)
					cell.set_property("icon-name", "level-object-camera");
				else
					cell.set_property("icon-name", "level-object-unknown");
			});
		column.set_cell_data_func(cell_text, (cell_layout, cell, model, iter) => {
				Value name;
				model.get_value(iter, LevelTreeView.Column.NAME, out name);

				cell.set_property("text", (string)name);
			});
		_tree_view = new Gtk.TreeView();
		_tree_view.append_column(column);
#if 0
		// For debugging.
		_tree_view.insert_column_with_attributes(-1
			, "Guids"
			, new gtk.CellRendererText()
			, "text"
			, Column.GUID
			, null
			);
#endif
		_tree_view.headers_clickable = false;
		_tree_view.headers_visible = false;
		_tree_view.model = _tree_sort;
		_tree_view.button_press_event.connect(on_button_pressed);

		// Enable drag and drop
		_tree_view.enable_model_drag_source(Gdk.ModifierType.BUTTON1_MASK, TARGET_ENTRIES, Gdk.DragAction.MOVE);
		_tree_view.enable_model_drag_dest(TARGET_ENTRIES, Gdk.DragAction.MOVE);
		_tree_view.drag_data_get.connect(on_drag_data_get);
		_tree_view.drag_data_received.connect(on_drag_data_received);

		_tree_selection = _tree_view.get_selection();
		_tree_selection.set_mode(Gtk.SelectionMode.MULTIPLE);
		_tree_selection.changed.connect(on_tree_selection_changed);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_tree_view);

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

		var tree_control = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		tree_control.pack_start(_filter_entry, true, true);
		tree_control.pack_end(_sort_items, false, false);

		this.pack_start(tree_control, false, true, 0);
		this.pack_start(_scrolled_window, true, true, 0);
	}

	private bool on_button_pressed(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_SECONDARY) {
			Gtk.Menu menu = new Gtk.Menu();
			Gtk.MenuItem mi = null;
			Gtk.TreePath path;
			Gtk.TreeViewColumn column;
			if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, out column, null, null)) {
				if (!_tree_selection.path_is_selected(path)) {
					_tree_selection.unselect_all();
					_tree_selection.select_path(path);
				}
			} else { // Clicked on empty space.
				mi = new Gtk.MenuItem.with_label("Create New Folder");
				mi.activate.connect(() => {
					create_new_folder();
				});
				menu.add(mi);
				menu.show_all();
				menu.popup_at_pointer(ev);
				return Gdk.EVENT_STOP;
			}
			_tree_selection.selected_foreach((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				
				if ((int)type == ItemType.FOLDER) {
					Gtk.TreeIter parent_iter = Gtk.TreeIter();
					Gtk.TreeStore base_store = null;
					bool valid_conversion = true;
			
					if (model is Gtk.TreeModelSort) {
						Gtk.TreeModelSort sort_model = (Gtk.TreeModelSort)model;
						Gtk.TreeModel base_model = sort_model.model;
					
						if (base_model is Gtk.TreeModelFilter) {
							Gtk.TreeModelFilter filter_model = (Gtk.TreeModelFilter)base_model;
							Gtk.TreeModel inner_model = filter_model.get_model();
							if (inner_model is Gtk.TreeStore) {
								base_store = (Gtk.TreeStore)inner_model;
							} else {
								print("The internal model of the filter is not a TreeStore.\n");
								return;
							}
					
							Gtk.TreePath child_path = sort_model.convert_path_to_child_path(path);
							if (child_path != null) {
								valid_conversion = base_store.get_iter(out parent_iter, child_path);
							} else {
								valid_conversion = false;
							}
						} else {
							valid_conversion = false;
						}
					} else {
						valid_conversion = false;
					}
					
					if (valid_conversion && base_store.iter_is_valid(parent_iter)) {
						var menu_item = new Gtk.MenuItem.with_label("Create Subfolder");
						menu_item.activate.connect(() => {
							if (base_store.iter_is_valid(parent_iter)) {
								create_new_folder(parent_iter);
							} else {
								create_new_folder(); 
							}
						});
						menu.add(menu_item);
					} else {
						print("Failed to convert path to base template\n");
					}
				}
			});
			
			mi = new Gtk.MenuItem.with_label("Rename...");
			mi.activate.connect(() => {
					Gtk.Dialog dg = new Gtk.Dialog.with_buttons("New Name"
						, (Gtk.Window)this.get_toplevel()
						, DialogFlags.MODAL
						, "Cancel"
						, ResponseType.CANCEL
						, "Ok"
						, ResponseType.OK
						, null
						);

					EntryText sb = new EntryText();
					_tree_selection.selected_foreach((model, path, iter) => {
							Value name;
							model.get_value(iter, Column.NAME, out name);
							sb.value = (string)name;
							return;
						});
					sb.activate.connect(() => { dg.response(ResponseType.OK); });
					dg.get_content_area().add(sb);
					dg.skip_taskbar_hint = true;
					dg.show_all();

					if (dg.run() == (int)ResponseType.OK) {
						string cur_name = "";
						string new_name = "";
						Guid object_id = GUID_ZERO;

						_tree_selection.selected_foreach((model, path, iter) => {
								Value type;
								model.get_value(iter, Column.TYPE, out type);
								if ((int)type == ItemType.FOLDER)
									return;

								Value name;
								model.get_value(iter, Column.NAME, out name);
								cur_name = (string)name;

								Value guid;
								model.get_value(iter, Column.GUID, out guid);
								object_id = (Guid)guid;

								new_name = sb.text.strip();
							});

						if (new_name != "" && new_name != cur_name)
							_level.object_set_editor_name(object_id, new_name);
					}

					dg.destroy();
				});
			if (_tree_selection.count_selected_rows() == 1)
				menu.add(mi);

			mi = new Gtk.MenuItem.with_label("Duplicate");
			mi.activate.connect(() => {
					GLib.Application.get_default().activate_action("duplicate", null);
				});
			menu.add(mi);

			mi = new Gtk.MenuItem.with_label("Delete");
			mi.activate.connect(() => {
					GLib.Application.get_default().activate_action("delete", null);
				});
			menu.add(mi);

			menu.show_all();
			menu.popup_at_pointer(ev);
			return Gdk.EVENT_STOP;
		}

		return Gdk.EVENT_PROPAGATE;
	}
	private void create_new_folder(Gtk.TreeIter? parent_iter = null)
	{
		Gtk.Dialog dialog = new Gtk.Dialog.with_buttons(
			"New Folder Name",
			((Gtk.Application)GLib.Application.get_default()).active_window,
			Gtk.DialogFlags.MODAL,
			"Cancel", Gtk.ResponseType.CANCEL,
			"OK", Gtk.ResponseType.OK
		);
	
		Gtk.Entry entry = new Gtk.Entry();
		entry.set_placeholder_text("Enter folder name");
		dialog.get_content_area().pack_start(entry, true, true, 0);
		dialog.show_all();
	
		if (dialog.run() == Gtk.ResponseType.OK) {
			string folder_name = entry.get_text().strip();
			if (folder_name == "") {
				folder_name = "Folder Name";
			}
			dialog.destroy();
			add_folder_to_tree(parent_iter, folder_name);
		} else {
			dialog.destroy();
		}
	}
	
	private void add_folder_to_tree(Gtk.TreeIter? parent_iter, string folder_name)
	{
		Guid folder_guid = Guid.new_guid();
		Gtk.TreeIter iter;
	
		bool parent_valid = parent_iter != null && _tree_store.iter_is_valid(parent_iter);
		
		_tree_store.insert_with_values(out iter, parent_valid ? parent_iter : null, -1,
			Column.TYPE, ItemType.FOLDER,
			Column.GUID, folder_guid,
			Column.NAME, folder_name
		);
	
		if (parent_valid) {
			Gtk.TreePath parent_path = _tree_store.get_path(parent_iter);
			_tree_view.expand_row(parent_path, false);
		}
	}
	
	private void on_drag_data_get(Gtk.Widget widget, Gdk.DragContext context, Gtk.SelectionData selection_data, uint info, uint time) {
		Gtk.TreeModel model;
		GLib.List<Gtk.TreePath> paths = _tree_selection.get_selected_rows(out model);
		string source_set = null;
		StringBuilder data_builder = new StringBuilder();
	
		foreach (Gtk.TreePath path in paths) {
			Gtk.TreeIter iter;
			model.get_iter(out iter, path);
	
			Gtk.TreeIter parent_iter;
			if (model.iter_parent(out parent_iter, iter)) {
				Value parent_name_val;
				model.get_value(parent_iter, Column.NAME, out parent_name_val);
				string parent_name = (string)parent_name_val;
	
				if (parent_name == "Units") {
					source_set = "units";
				} else if (parent_name == "Sounds") {
					source_set = "sounds";
				}
	
				Value guid_val;
				model.get_value(iter, Column.GUID, out guid_val);
				Guid guid = (Guid)guid_val;
				data_builder.append(guid.to_string() + ",");
			}
		}
	
		if (source_set != null) {
			string data_str = source_set + ";" + data_builder.str;
			selection_data.set(selection_data.get_target(), 8, data_str.data);
		}
	}
	private void on_drag_data_received(Gtk.Widget widget, Gdk.DragContext context, int x, int y, Gtk.SelectionData selection_data, uint info, uint time) {
		// 1. Stop IMMÉDIATEMENT le handler par défaut
		Signal.stop_emission_by_name(_tree_view, "drag-data-received");
		
		// 2. Traitement des données
		string[] data = ((string)selection_data.get_data()).split(";");
		if (data.length != 2) return;
	
		string source_set = data[0];
		string[] guids = data[1].split(",");
	
		// 3. Détection de la cible
		Gtk.TreePath path;
		Gtk.TreeViewDropPosition pos;
		if (!_tree_view.get_dest_row_at_pos(x, y, out path, out pos)) return;
	
		// 4. Détermination du set cible
		Gtk.TreeIter target_iter;
		_tree_sort.get_iter(out target_iter, path);
		
		string target_set = null;
		Gtk.TreeIter parent_iter;
		if (_tree_sort.iter_parent(out parent_iter, target_iter)) {
			Value parent_name_val;
			_tree_sort.get_value(parent_iter, Column.NAME, out parent_name_val);
			string parent_name = (string)parent_name_val;
	
			target_set = (parent_name == "Units") ? "units" : "sounds";
		} else {
			Value name_val;
			_tree_sort.get_value(target_iter, Column.NAME, out name_val);
			string name = (string)name_val;
			
			target_set = (name == "Units") ? "units" : "sounds";
		}
	
		// 5. Validation finale
		if (target_set == null || target_set == source_set) return;
	
		// 6. Mise à jour de la base
		foreach (string guid_str in guids) {
			if (guid_str == "") continue;
			Guid guid = Guid.parse(guid_str);
			_db.remove_from_set(_level._id, source_set, guid);
			_db.add_to_set(_level._id, target_set, guid);
		}
	
		// 7. Finalisation du drag
		Gtk.drag_finish(context, true, false, time);
	}

	private void on_tree_selection_changed()
	{
		_level.selection_changed.disconnect(on_level_selection_changed);

		Gee.ArrayList<Guid?> ids = new Gee.ArrayList<Guid?>();
		_tree_selection.selected_foreach((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				if ((int)type == ItemType.FOLDER)
					return;

				Value id;
				model.get_value(iter, Column.GUID, out id);
				ids.add((Guid)id);
			});

		_level.selection_set(ids.to_array());
		_level.selection_changed.connect(on_level_selection_changed);
	}

	private void on_level_selection_changed(Gee.ArrayList<Guid?> selection)
	{
		_tree_selection.changed.disconnect(on_tree_selection_changed);
		_tree_selection.unselect_all();

		Gtk.TreePath? last_selected = null;

		_tree_sort.foreach ((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				if ((int)type == ItemType.FOLDER)
					return false;

				Value id;
				model.get_value(iter, Column.GUID, out id);

				foreach (Guid? guid in selection) {
					if ((Guid)id == guid) {
						_tree_selection.select_iter(iter);
						last_selected = path;
						return false;
					}
				}

				return false;
			});

		if (last_selected != null)
			_tree_view.scroll_to_cell(last_selected, null, false, 0.0f, 0.0f);

		_tree_selection.changed.connect(on_tree_selection_changed);
	}

	private void on_object_editor_name_changed(Guid object_id, string name)
	{
		_tree_sort.foreach ((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				if ((int)type == ItemType.FOLDER)
					return false;

				Value guid;
				model.get_value(iter, Column.GUID, out guid);
				Guid guid_model = (Guid)guid;

				if (guid_model == object_id) {
					Gtk.TreeIter iter_filter;
					Gtk.TreeIter iter_model;
					_tree_sort.convert_iter_to_child_iter(out iter_filter, iter);
					_tree_filter.convert_iter_to_child_iter(out iter_model, iter_filter);

					_tree_store.set(iter_model
						, Column.NAME
						, name
						, -1
						);

					return true;
				}

				return false;
			});
	}

	private ItemType item_type(Unit u)
	{
		if (u.is_light())
			return ItemType.LIGHT;
		else if (u.is_camera())
			return ItemType.CAMERA;
		else
			return ItemType.UNIT;
	}
	
	private void on_database_key_changed(Guid id, string key) {
		if (id != _level._id || (key != "units" && key != "sounds")) return;
	
		_tree_selection.changed.disconnect(on_tree_selection_changed);
	
		string[] required_sections = {"Units", "Sounds"};
		Gtk.TreeIter[] section_iters = new Gtk.TreeIter[2];
	
		for (int i = 0; i < required_sections.length; i++) {
			bool exists = false;
			Gtk.TreeIter iter;
			
			// Search folder
			if (_tree_store.get_iter_first(out iter)) {
				do {
					Value v;
					_tree_store.get_value(iter, Column.NAME, out v);
					if ((string)v == required_sections[i]) {
						section_iters[i] = iter;
						exists = true;
						break;
					}
				} while (_tree_store.iter_next(ref iter));
			}
			
			// Create folder if not exist
			if (!exists) {
				_tree_store.append(out section_iters[i], null);
				_tree_store.set(
					section_iters[i],
					Column.TYPE, ItemType.FOLDER,
					Column.GUID, GUID_ZERO,
					Column.NAME, required_sections[i],
					-1
				);
			}
		}
	
		// Synchronize only modified keys
		int target_index = (key == "units") ? 0 : 1;
		Gtk.TreeIter target_iter = section_iters[target_index];
		
		var current_guids = _db.get_property_set(id, key, new HashSet<Guid?>());
		var existing_guids = new HashTable<Guid?, Gtk.TreeIter?>(Guid.hash_func, Guid.equal_func);
	
		// Update elements
		Gtk.TreeIter child;
		if (_tree_store.iter_children(out child, target_iter)) {
			do {
				Value type_val;
				_tree_store.get_value(child, Column.TYPE, out type_val);
								
				if ((ItemType)type_val.get_int() == ItemType.FOLDER) continue;

				Value guid_val;
				_tree_store.get_value(child, Column.GUID, out guid_val);
				if (guid_val.holds(typeof(Guid))) {
					existing_guids[(Guid)guid_val] = child;
				}
			} while (_tree_store.iter_next(ref child));
		}
			
		existing_guids.foreach((guid, iter) => {
			if (!current_guids.contains(guid)) {
				_tree_store.remove(ref iter);
			}
		});
	
		foreach (Guid guid in current_guids) {
			if (!existing_guids.contains(guid)) {
				Gtk.TreeIter new_iter;
				ItemType type = (key == "units") ? item_type(Unit(_level._db, guid)) : ItemType.SOUND;
				_tree_store.append(out new_iter, target_iter);
				_tree_store.set(
					new_iter,
					Column.TYPE, type,
					Column.GUID, guid,
					Column.NAME, _level.object_editor_name(guid),
					-1
				);
			}
		}
		_tree_view.model = _tree_sort;
		_tree_view.expand_all();
		_tree_selection.changed.connect(on_tree_selection_changed);
	}

	private void on_filter_entry_text_changed()
	{
		_tree_selection.changed.disconnect(on_tree_selection_changed);
		_tree_filter.refilter();
		_tree_selection.changed.connect(on_tree_selection_changed);
	}

	private Gtk.RadioButton add_sort_item(Gtk.RadioButton? group, SortMode mode)
	{
		var button = new Gtk.RadioButton.with_label_from_widget(group, mode.to_label());
		button.toggled.connect(() => {
				_sort_mode = mode;
				_tree_filter.refilter();
				_sort_items_popover.popdown();
			});
		_sort_items_box.pack_start(button, false, false);
		return button;
	}
}

} /* namespace Crown */
