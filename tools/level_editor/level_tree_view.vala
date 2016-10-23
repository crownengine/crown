/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
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

			_db = db;
			_db.key_changed.connect(on_database_key_changed);

			// Widgets
			_filter_entry = new Gtk.SearchEntry();
			_filter_entry.set_placeholder_text("Search...");
			_filter_entry.changed.connect(on_filter_entry_text_changed);

			_tree_store = new Gtk.TreeStore(2, typeof(string), typeof(int));

			_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
			_tree_filter.set_visible_func(filter_tree);

			_tree_sort = new Gtk.TreeModelSort.with_model(_tree_filter);
			_tree_sort.set_default_sort_func((model, iter_a, iter_b) => {
				Value type_a;
				Value type_b;
				model.get_value(iter_a, 1, out type_a);
				model.get_value(iter_b, 1, out type_b);
				if ((int)type_a == ItemType.FOLDER || (int)type_b == ItemType.FOLDER)
					return -1;

				Value id_a;
				Value id_b;
				model.get_value(iter_a, 0, out id_a);
				model.get_value(iter_b, 0, out id_b);
				return strcmp((string)id_a, (string)id_b);
			});

			_tree_view = new Gtk.TreeView();
			_tree_view.insert_column_with_attributes(-1, "Objects", new Gtk.CellRendererText(), "text", 0, null);
			// _tree_view.insert_column_with_attributes(-1, "Types", new Gtk.CellRendererText(), "text", 1, null); // DEBUG
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
				model.get_value(iter, 1, out type);
				if ((int)type == ItemType.FOLDER)
					return;

				Value id;
				model.get_value(iter, 0, out id);
				ids += Guid.parse((string)id);
			});

			_level.destroy_objects(ids);
		}

		private bool filter_tree(Gtk.TreeModel model, Gtk.TreeIter iter)
		{
			Value name;
			Value type;
			model.get_value(iter, 0, out name);
			model.get_value(iter, 1, out type);

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
				model.get_value(iter, 1, out type);
				if ((int)type == ItemType.FOLDER)
					return;

				Value id;
				model.get_value(iter, 0, out id);
				ids += Guid.parse((string)id);
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
				model.get_value(iter, 1, out type);
				if ((int)type == ItemType.FOLDER)
					return false;

				Value id;
				model.get_value(iter, 0, out id);
				Guid guid_model = Guid.parse((string)id);

				foreach (Guid? guid in selection)
				{
					if (guid_model == guid)
					{
						_tree_selection.select_iter(iter);
						return false;
					}
				}

				return false;
			});

			_tree_selection.changed.connect(on_tree_selection_changed);
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
			_tree_store.set(unit_iter, 0, "Units", 1, ItemType.FOLDER, -1);
			_tree_store.set(light_iter, 0, "Lights", 1, ItemType.FOLDER, -1);
			_tree_store.set(sound_iter, 0, "Sounds", 1, ItemType.FOLDER, -1);

			Value? units  = _db.get_property(GUID_ZERO, "units");
			Value? sounds = _db.get_property(GUID_ZERO, "sounds");

			if (units != null)
			{
				foreach (Guid unit in (HashSet<Guid?>)units)
				{
					Gtk.TreeIter iter;
					_tree_store.append(out iter, _level.is_light(unit) ? light_iter : unit_iter);
					_tree_store.set(iter, 0, unit.to_string(), 1, ItemType.UNIT, -1);
				}
			}
			if (sounds != null)
			{
				foreach (Guid sound in (HashSet<Guid?>)sounds)
				{
					Gtk.TreeIter iter;
					_tree_store.append(out iter, sound_iter);
					_tree_store.set(iter, 0, sound.to_string(), 1, ItemType.SOUND, -1);
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
	}
}
