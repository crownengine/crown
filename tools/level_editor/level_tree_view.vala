/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;
using GLib;
using Gtk;

namespace Crown
{
	public class LevelTreeView : Gtk.Bin
	{
		private enum ItemType
		{
			FOLDER,
			UNIT,
			SOUND
		}

		private enum Column
		{
			TYPE,
			GUID,
			NAME,

			COUNT
		}

		// Data
		private Level _level;
		private Database _db;

		// Widgets
		private Gtk.Entry _filter_entry;
		private Gtk.TreeStore _tree_store;
		private Gtk.TreeModelFilter _tree_filter;
		private Gtk.TreeModelSort _tree_sort;
		private Gtk.TreeView _tree_view;
		private Gtk.TreeSelection _tree_selection;
		private Gtk.ScrolledWindow _scrolled_window;
		private Gtk.Box _vbox;

		public LevelTreeView(Database db, Level level)
		{
			// Data
			_level = level;
			_level.selection_changed.connect(on_level_selection_changed);
			_level.object_editor_name_changed.connect(on_object_editor_name_changed);

			_db = db;
			_db.key_changed.connect(on_database_key_changed);

			// Widgets
			_filter_entry = new Gtk.SearchEntry();
			_filter_entry.set_placeholder_text("Search...");
			_filter_entry.changed.connect(on_filter_entry_text_changed);

			_tree_store = new Gtk.TreeStore(Column.COUNT
				, typeof(int)    // Column.TYPE
				, typeof(Guid)   // Column.GUID
				, typeof(string) // Column.NAME
				);

			_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
			_tree_filter.set_visible_func(filter_tree);

			_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
			_tree_sort.set_default_sort_func((model, iter_a, iter_b) => {
				Value type_a;
				Value type_b;
				model.get_value(iter_a, Column.TYPE, out type_a);
				model.get_value(iter_b, Column.TYPE, out type_b);
				if ((int)type_a == ItemType.FOLDER || (int)type_b == ItemType.FOLDER)
					return -1;

				Value id_a;
				Value id_b;
				model.get_value(iter_a, Column.NAME, out id_a);
				model.get_value(iter_b, Column.NAME, out id_b);
				return strcmp((string)id_a, (string)id_b);
			});

			_tree_view = new Gtk.TreeView();
			/*
			_tree_view.insert_column_with_attributes(-1
				, "Guids"
				, new gtk.CellRendererText()
				, "text"
				, Column.GUID
				, null
				);
			*/
			Gtk.CellRendererText cell = new Gtk.CellRendererText();
			cell.editable = true;
			cell.edited.connect(on_object_name_edited);
			_tree_view.insert_column_with_attributes(-1
				, "Names"
				, cell
				, "text"
				, Column.NAME
				, null
				);
			_tree_view.headers_clickable = false;
			_tree_view.headers_visible = false;
			_tree_view.model = _tree_sort;
			_tree_view.button_press_event.connect(on_button_pressed);

			_tree_selection = _tree_view.get_selection();
			_tree_selection.set_mode(Gtk.SelectionMode.MULTIPLE);
			_tree_selection.changed.connect(on_tree_selection_changed);

			_scrolled_window = new Gtk.ScrolledWindow(null, null);
			_scrolled_window.add(_tree_view);

			_vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
			_vbox.pack_start(_filter_entry, false, true, 0);
			_vbox.pack_start(_scrolled_window, true, true, 0);

			add(_vbox);

			set_size_request(300, 250);
			show_all();
		}

		private void on_object_name_edited(string path, string new_text)
		{
			Gtk.TreeIter iter;
			_tree_sort.get_iter_from_string(out iter, path);

			Value type;
			_tree_sort.get_value(iter, Column.TYPE, out type);
			if ((int)type == ItemType.FOLDER)
				return;

			Value name;
			_tree_sort.get_value(iter, Column.NAME, out name);
			if ((string)name == new_text)
				return;

			Value guid;
			_tree_sort.get_value(iter, Column.GUID, out guid);

			_level.object_set_editor_name((Guid)guid, new_text);
		}

		private bool on_button_pressed(Gdk.EventButton ev)
		{
			uint button = ev.button;

			if (button == 3) // Right
			{
				Gtk.Menu menu = new Gtk.Menu();
				Gtk.MenuItem mi = new Gtk.MenuItem.with_label("Delete");
				mi.activate.connect(on_popup_delete);
				menu.add(mi);
				menu.show_all();
				menu.popup(null, null, null, button, ev.time);
				return true;
			}

			return false;
		}

		private void on_popup_delete()
		{
			Guid[] ids = {};
			_tree_selection.selected_foreach((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				if ((int)type == ItemType.FOLDER)
					return;

				Value id;
				model.get_value(iter, Column.GUID, out id);
				ids += (Guid)id;
			});

			_level.destroy_objects(ids);
		}

		private bool filter_tree(Gtk.TreeModel model, Gtk.TreeIter iter)
		{
			Value type, name;
			model.get_value(iter, Column.TYPE, out type);
			model.get_value(iter, Column.NAME, out name);

			_tree_view.expand_all();

			if ((int)type == ItemType.FOLDER)
				return true;

			string name_str = (string)name;
			string filter_text = _filter_entry.text.down();

			return name_str != null
				&& (filter_text == "" || name_str.down().index_of(filter_text) > -1)
				;
		}

		private void on_tree_selection_changed()
		{
			_level.selection_changed.disconnect(on_level_selection_changed);

			Guid[] ids = {};
			_tree_selection.selected_foreach((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				if ((int)type == ItemType.FOLDER)
					return;

				Value id;
				model.get_value(iter, Column.GUID, out id);
				ids += (Guid)id;
			});

			_level.selection_set(ids);
			_level.selection_changed.connect(on_level_selection_changed);
		}

		private void on_level_selection_changed(Gee.ArrayList<Guid?> selection)
		{
			_tree_selection.changed.disconnect(on_tree_selection_changed);
			_tree_selection.unselect_all();

			_tree_sort.foreach ((model, path, iter) => {
				Value type;
				model.get_value(iter, Column.TYPE, out type);
				if ((int)type == ItemType.FOLDER)
					return false;

				Value id;
				model.get_value(iter, Column.GUID, out id);

				foreach (Guid? guid in selection)
				{
					if ((Guid)id == guid)
					{
						_tree_selection.select_iter(iter);
						return false;
					}
				}

				return false;
			});

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

				if (guid_model == object_id)
				{
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

		private void on_database_key_changed(Guid id, string key)
		{
			if (id != GUID_ZERO)
				return;

			if (key != "units" && key != "sounds")
				return;

			_tree_selection.changed.disconnect(on_tree_selection_changed);
			_tree_view.model = null;
			_tree_store.clear();

			Gtk.TreeIter unit_iter;
			Gtk.TreeIter light_iter;
			Gtk.TreeIter sound_iter;
			_tree_store.append(out unit_iter, null);
			_tree_store.append(out light_iter, null);
			_tree_store.append(out sound_iter, null);
			_tree_store.set(unit_iter
				, Column.TYPE
				, ItemType.FOLDER
				, Column.GUID
				, GUID_ZERO
				, Column.NAME
				, "Units"
				, -1
				);
			_tree_store.set(light_iter
				, Column.TYPE
				, ItemType.FOLDER
				, Column.GUID
				, GUID_ZERO
				, Column.NAME
				, "Lights"
				, -1
				);
			_tree_store.set(sound_iter
				, Column.TYPE
				, ItemType.FOLDER
				, Column.GUID
				, GUID_ZERO
				, Column.NAME
				, "Sounds"
				, -1
				);

			HashSet<Guid?> units  = _db.get_property_set(GUID_ZERO, "units", new HashSet<Guid?>());
			HashSet<Guid?> sounds = _db.get_property_set(GUID_ZERO, "sounds", new HashSet<Guid?>());

			foreach (Guid unit in units)
			{
				Unit u = new Unit(_level._db, unit, _level._prefabs);
				Gtk.TreeIter iter;
				_tree_store.append(out iter, u.is_light() ? light_iter : unit_iter);
				_tree_store.set(iter
					, Column.TYPE
					, ItemType.UNIT
					, Column.GUID
					, unit
					, Column.NAME
					, _level.object_editor_name(unit)
					, -1
					);
			}
			foreach (Guid sound in sounds)
			{
				Gtk.TreeIter iter;
				_tree_store.append(out iter, sound_iter);
				_tree_store.set(iter
					, Column.TYPE
					, ItemType.SOUND
					, Column.GUID
					, sound
					, Column.NAME
					, _level.object_editor_name(sound)
					, -1
					);
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
	}
}
