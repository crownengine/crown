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
		{ CROWN_DND_TARGET, Gtk.TargetFlags.SAME_APP, 0 }
	};	
	private const string CROWN_DND_TARGET = "application/x-crown-set";

	public enum ItemType
	{
		FOLDER,
		CAMERA,
		LIGHT,
		SOUND,
		UNIT
	}

	public enum Column
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

	// Getters
	public Level level { get { return _level; } }
	public Database db { get { return _db; } }
	public EntrySearch filter_entry { get { return _filter_entry; } }
	public Gtk.TreeStore tree_store { get { return _tree_store; } }
	public Gtk.TreeModelFilter tree_filter { get { return _tree_filter; } }
	public Gtk.TreeModelSort tree_sort { get { return _tree_sort; } }
	public Gtk.TreeView tree_view { get { return _tree_view; } }
	public Gtk.TreeSelection tree_selection { get { return _tree_selection; } }
	public Gtk.ScrolledWindow scrolled_window { get { return _scrolled_window; } }
	public SortMode sort_mode { get { return _sort_mode; } }
	public Gtk.Box sort_items_box { get { return _sort_items_box; } }
	public Gtk.Popover sort_items_popover { get { return _sort_items_popover; } }
	public Gtk.MenuButton sort_items { get { return _sort_items; } }

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
			, typeof(ItemType)// Column.TYPE
			, typeof(Guid)    // Column.GUID
			, typeof(string)  // Column.NAME
			);

		_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
		_tree_filter.set_visible_func((model, iter) => {
				_tree_view.expand_all();

				Value type;
				Value name;
				model.get_value(iter, Column.TYPE, out type);
				model.get_value(iter, Column.NAME, out name);

				if (type == ItemType.FOLDER)
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
				
				// Ensure folders come first (prioritize folders over units/sounds)
				if ((ItemType)type_a == ItemType.FOLDER && (ItemType)type_b != ItemType.FOLDER)
					return -1; // folder should be before non-folder
				if ((ItemType)type_b == ItemType.FOLDER && (ItemType)type_a != ItemType.FOLDER)
					return 1; // folder should be after non-folder
				
				// If both are folders, or both are non-folders, sort by the usual criteria
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
				model.get_value(iter, Column.TYPE, out type);

				if (type == ItemType.FOLDER)
					cell.set_property("icon-name", "browser-folder-symbolic");
				else if (type == ItemType.UNIT)
					cell.set_property("icon-name", "level-object-unit");
				else if (type == ItemType.SOUND)
					cell.set_property("icon-name", "level-object-sound");
				else if (type == ItemType.LIGHT)
					cell.set_property("icon-name", "level-object-light");
				else if (type == ItemType.CAMERA)
					cell.set_property("icon-name", "level-object-camera");
				else
					cell.set_property("icon-name", "level-object-unknown");
			});
		column.set_cell_data_func(cell_text, (cell_layout, cell, model, iter) => {
				Value name;
				model.get_value(iter, Column.NAME, out name);

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
		_tree_view.drag_drop.connect(on_drag_drop);
		_tree_view.drag_motion.connect(on_drag_motion);

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

		_level.level_loaded.connect(() => {
			LevelTreeOrganization.rebuild_tree(this);
		});
		LevelTreeOrganization.rebuild_tree(this);
	}

	private bool on_button_pressed(Gdk.EventButton ev)
	{
		if (ev.button == Gdk.BUTTON_SECONDARY) {
			Gtk.TreePath path;
			Gtk.TreeViewColumn column;
			if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, out column, null, null)) {
				if (!_tree_selection.path_is_selected(path)) {
					_tree_selection.unselect_all();
					_tree_selection.select_path(path);
				}
			} else { // Clicked on empty space.
				return Gdk.EVENT_STOP;
			}
			Gtk.Menu menu = new Gtk.Menu();
			Gtk.MenuItem mi = null;
			_tree_selection.selected_foreach((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				
				if (type == ItemType.FOLDER) {
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
								print("The internal model of the filter is not a TreeStore.");
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
							Guid parent_guid = LevelTreeOrganization.get_parent_guid(this, parent_iter); 
							print("Retrieved parent GUID: %s", parent_guid.to_string());
							Value? parent_type_val = null;
							if (parent_guid == GUID_UNIT_FOLDER) {
								parent_type_val = "unit_folder"; 
							} else if (parent_guid == GUID_SOUND_FOLDER) {
								parent_type_val = "sound_folder"; 
							} else {
								parent_type_val = _db.get_property(parent_guid, "_type"); 
							}
				
							if (parent_type_val.holds(typeof(string))) {
								string parent_type_str = (string)parent_type_val;
					
								switch (parent_type_str) {
									case "unit_folder":
										create_new_folder(parent_iter, OBJECT_TYPE_FOLDER_UNIT);
										break;
									case "sound_folder":
										create_new_folder(parent_iter, OBJECT_TYPE_FOLDER_SOUND);
										break;
									default:
										print("Unmanaged or invalid type: %s", parent_type_str);
										break;
								}
							} else {
								print("Error: Parent type is not a string. Type found: %s", parent_type_val.type().name());
							}
						});
						menu.add(menu_item);
					} else {
						print("Error: Invalid conversion or iterator");
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
								if (type == ItemType.FOLDER)
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
	private void create_new_folder(Gtk.TreeIter? parent_iter = null,string type)
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
			Guid folder_guid = Guid.new_guid();
			LevelTreeOrganization.add_folder_to_tree(this, true, parent_iter, folder_name,type, folder_guid);
		} else {
			dialog.destroy();
		}
	}
	
	private bool on_drag_drop(Gtk.Widget widget, Gdk.DragContext context, int x, int y, uint time) {
		return LevelTreeOrganization.on_drag_drop_internal(this, CROWN_DND_TARGET, widget, context, x, y, time);
	}
	
	private bool on_drag_motion(Gtk.Widget widget, Gdk.DragContext context, int x, int y, uint time) {
		return LevelTreeOrganization.on_drag_motion_internal(this, widget, context, x, y, time);
	}

	private void on_drag_data_get(Gtk.Widget widget, Gdk.DragContext context, Gtk.SelectionData selection_data, uint info, uint time) {
		LevelTreeOrganization.on_drag_data_get_internal(this, CROWN_DND_TARGET,widget, context, selection_data, info, time);
	}
	
	private void on_drag_data_received(Gtk.Widget widget, Gdk.DragContext context, int x, int y, Gtk.SelectionData selection_data, uint info, uint time) {		
		LevelTreeOrganization.on_drag_data_received_internal(this, CROWN_DND_TARGET, widget, context, x, y, selection_data, info, time);
	}	
	
	public void on_tree_selection_changed()
	{
		_level.selection_changed.disconnect(on_level_selection_changed);

		Gee.ArrayList<Guid?> ids = new Gee.ArrayList<Guid?>();
		_tree_selection.selected_foreach((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				if (type == ItemType.FOLDER)
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
				if (type == ItemType.FOLDER)
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
				if (type == ItemType.FOLDER)
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
		LevelTreeOrganization.on_database_key_changed_internal(this, id, key);
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
