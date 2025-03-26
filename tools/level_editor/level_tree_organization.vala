/*
* Copyright (c) 2012-2025 Daniele Bartolini et al.
* SPDX-License-Identifier: GPL-3.0-or-later
*/
using Gee;
using Gtk;

namespace Crown
{
    // Manage drag and drop , folder organization and items synching for the LevelTreeView
    public class LevelTreeOrganization
    {
        public static bool on_drag_drop_internal(LevelTreeView view_i, string d_target, Gtk.Widget widget, Gdk.DragContext context, int x, int y, uint time) {
            Gtk.drag_get_data(
                widget,         // will receive 'drag-data-received' signal
                context,
                Gdk.Atom.intern(d_target, false),
                time
            );
            Signal.stop_emission_by_name(view_i.tree_view, "drag-drop");
            return true;
        }
        
        public static bool on_drag_motion_internal(LevelTreeView view_i, Gtk.Widget widget, Gdk.DragContext context, int x, int y, uint time) {
            Gtk.TreePath path;
            Gtk.TreeViewDropPosition pos;
            if (view_i.tree_view.get_dest_row_at_pos(x, y, out path, out pos)) {
                view_i.tree_view.set_drag_dest_row(path, pos);
                Gdk.drag_status(context, Gdk.DragAction.MOVE, time);
                return true;
            }
            return false;
        }

        public static void on_drag_data_get_internal(LevelTreeView view_i, string d_target, Gtk.Widget widget, Gdk.DragContext context, Gtk.SelectionData selection_data, uint info, uint time) {
            Gtk.TreeModel model;
            GLib.List<Gtk.TreePath> paths = view_i.tree_selection.get_selected_rows(out model);
            StringBuilder data_builder = new StringBuilder();
            string source_set = null;

            foreach (Gtk.TreePath path in paths) {
                Gtk.TreeIter iter;
                model.get_iter(out iter, path);
                // Get GUID of the selected item
                Value guid_val;
                model.get_value(iter, LevelTreeView.Column.GUID, out guid_val);
                Guid guid = (Guid) guid_val;
                
                if (guid == GUID_ZERO) {
                    print("Warning: GUID is GUID_ZERO, skipping...\n");
                    continue;
                }
                Value? item_type = view_i.db.get_property(guid, "_type");
            
                if (item_type != null) {
                    string type_str = (string)item_type; 
                    source_set = null;
                    foreach (var root_info in get_root_folder_info()) {
                        if (type_str == root_info.contains_item_type_str) {
                            source_set = root_info.contains_source_set_str;
                            break;
                        }
                    }

                    if (source_set == null) {
                        continue; 
                    }
                } else {
                    print("Error: Item type is not a string or is null for GUID: %s\n", guid.to_string());
                    continue;
                }
                data_builder.append(guid.to_string() + ",");
            }
        
            // REMOVE TRAILING COMMA
            if (data_builder.len > 0) {
                data_builder.truncate(data_builder.len - 1);
            }
        
            if (data_builder.len > 0) {
                string data_str = @"$source_set;$(data_builder.str)";
                var target = Gdk.Atom.intern(d_target, false);
                selection_data.set(target, 8, data_str.data);
            } else {
                print("Invalid data: no GUIDs found.");
                Gtk.drag_finish(context, false, false, time);
                return;
            }
        }
        
        public static void on_drag_data_received_internal(LevelTreeView view_i, string d_target, Gtk.Widget widget, Gdk.DragContext context, int x, int y, Gtk.SelectionData selection_data, uint info, uint time) {		
            view_i.tree_selection.changed.disconnect(view_i.on_tree_selection_changed);
            Signal.stop_emission_by_name(view_i.tree_view, "drag-data-received");
            
            var expected_target = Gdk.Atom.intern(d_target, false);
            if (selection_data.get_target() != expected_target) {
                Gtk.drag_finish(context, false, false, time);
                view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
                return;
            }

            string raw_data = (string)selection_data.get_data();
            
            raw_data = raw_data.strip(); 

            string[] data = raw_data.split(";", 2);

            string source_set = data[0].strip();
            string[] guids = data[1].split(",");

            Gtk.TreePath path;
            Gtk.TreeViewDropPosition pos;
            if (!view_i.tree_view.get_dest_row_at_pos(x, y, out path, out pos)) {
                Gtk.drag_finish(context, false, false, time);
                view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
                return;
            }

            Gtk.TreeIter target_iter;
            view_i.tree_sort.get_iter(out target_iter, path);
        
            string target_parent_guid = "";
            Value target_guid_val;
            Value target_type_val;
        
            view_i.tree_sort.get_value(target_iter, LevelTreeView.Column.TYPE, out target_type_val);
            if (target_type_val == LevelTreeView.ItemType.FOLDER) {
                view_i.tree_sort.get_value(target_iter, LevelTreeView.Column.GUID, out target_guid_val);
                target_parent_guid = ((Guid)target_guid_val).to_string();
            } else {
                view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
                return;
            }
        
            string? moving_type = null;
            foreach (string guid_str in guids) {
                if (guid_str.strip().length == 0) continue;	
                Guid guid = Guid.parse(guid_str);
        
                Value? parent_guid_val = view_i.db.get_property(guid, "parent_folder");
                Guid parent_guid;
        
                if (parent_guid_val == null) {
                    string item_type = (string)view_i.db.get_property(guid, "_type");
                    if (item_type == "unit") {
                        parent_guid = GUID_UNIT_FOLDER;
                    } else if (item_type == "sound") {
                        parent_guid = GUID_SOUND_FOLDER;
                    } else {
                        print("Error: Unable to determine the root parent of element %s", guid.to_string());
                        continue;
                    }
                } else {
                    parent_guid = (Guid)parent_guid_val;
                }
        
                Value? parent_type_val = view_i.db.get_property(parent_guid, "_type");
        
                string parent_type = (string)parent_type_val;
                moving_type = parent_type;
        
                if (target_parent_guid == null || target_parent_guid.strip().length == 0) {
                    print("Error: Invalid target GUID (NULL or empty)");
                    view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
                    return;
                }
        
                Guid target_guid;
                if (!Guid.try_parse(target_parent_guid, out target_guid)) {
                    print("Error: Invalid target GUID for parent %s", target_parent_guid);
                    view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
                    return;
                }
                
                Value? target_type_val_db = null;
                string target_type;
                
                if (target_guid == GUID_UNIT_FOLDER) {
                    target_type = "unit_folder";
                } else if (target_guid == GUID_SOUND_FOLDER) {
                    target_type = "sound_folder";
                } else {
                    target_type_val_db = view_i.db.get_property(target_guid, "_type");
                
                    if (target_type_val_db != null) {
                        target_type = (string)target_type_val_db;
                    } else {
                        print("Error: Unable to determine target type for GUID %s\n", target_guid.to_string());
                        view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
                        return;
                    }
                }
                
                string current_item_type = (string)view_i.db.get_property(guid, "_type");
                if (current_item_type == "unit_folder" || current_item_type == "sound_folder") {
                    continue;
                }
            
                if (!((current_item_type == "unit" && target_type == "unit_folder") || 
                    (current_item_type == "sound" && target_type == "sound_folder"))) {
                    print("Error: Cannot move a %s into %s", current_item_type, target_type);
                    view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
                    return;
                }
                if (parent_guid != target_guid) {
                    view_i.db.set_property(guid, "parent_folder", target_guid); 
                }
    
                TreeIter? parent_iter = find_parent_iter(view_i, target_guid);
                if (parent_iter != null) {
                    Gtk.TreeIter iter;
                    bool type_managed = false;
                
                    foreach (var root_info in get_root_folder_info()) {
                        if (root_info.object_type == target_type) {
                            view_i.tree_store.append(out iter, parent_iter);
                            view_i.tree_store.set(
                                iter,
                                LevelTreeView.Column.TYPE, root_info.contains_item_type,
                                LevelTreeView.Column.GUID, guid,
                                LevelTreeView.Column.NAME, view_i.level.object_editor_name(guid),
                                -1
                            );
                            type_managed = true;
                            break;
                        }
                    }
                
                    if (!type_managed) {
                        print("target_type folder : " + target_type + " not managed");
                    }
                }
                
                view_i.tree_view.expand_all();  
            }
            Gtk.drag_finish(context, true, false, time);
            view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
        }  


        public static void on_database_key_changed_internal(LevelTreeView view_i, Guid id, string key) {
            if (id != view_i.level._id || (key != "units" && key != "sounds")) return;
        
            view_i.tree_selection.changed.disconnect(view_i.on_tree_selection_changed);
        
            // Define target folder and item type based on the key
            Guid target_folder = (key == "units") ? GUID_UNIT_FOLDER : GUID_SOUND_FOLDER;
            LevelTreeView.ItemType item_type = (key == "units") ? LevelTreeView.ItemType.UNIT : LevelTreeView.ItemType.SOUND;
        
            // Track existing GUIDs within the folder
            var existing_guids = new HashTable<Guid?, Gtk.TreeIter?>(Guid.hash_func, Guid.equal_func);
            Gtk.TreeIter target_iter;
            bool folder_exists = false;
        
            // Check if the target folder already exists
            if (view_i.tree_store.get_iter_first(out target_iter)) {
                do {
                    Value guid_val;
                    view_i.tree_store.get_value(target_iter, LevelTreeView.Column.GUID, out guid_val);
        
                    // Ensure the value holds a Guid
                    if (guid_val.holds(typeof(Guid))) {
                        Guid guid = (Guid)guid_val;
        
                        // Check if the extracted Guid matches the target folder
                        if (guid == target_folder) {
                            folder_exists = true;
                            break;
                        }
                    }
                } while (view_i.tree_store.iter_next(ref target_iter));
            }
        
        
            // If the folder doesn't exist, create it
            if (!folder_exists) {
                view_i.tree_store.append(out target_iter, null);
                view_i.tree_store.set(
                    target_iter,
                    LevelTreeView.Column.TYPE, LevelTreeView.ItemType.FOLDER,
                    LevelTreeView.Column.GUID, target_folder,
                    LevelTreeView.Column.NAME, (key == "units") ? "Units" : "Sounds",
                    -1
                );
            }
        
            // Populate existing GUIDs in the folder
            Gtk.TreeIter child;
            if (view_i.tree_store.iter_children(out child, target_iter)) {
                do {
                    Value guid_val;
                    view_i.tree_store.get_value(child, LevelTreeView.Column.GUID, out guid_val);
                    if (guid_val.holds(typeof(Guid))) {
                        existing_guids[(Guid)guid_val] = child;
                    }
                } while (view_i.tree_store.iter_next(ref child));
            }
        
            // Retrieve updated GUIDs from the database
            var current_guids = view_i.db.get_property_set(id, key, new HashSet<Guid?>());
        
            // Remove outdated items but avoid removing subfolders
            existing_guids.foreach((guid, iter) => {
                // Only remove items that are not folders
                Value item_type_val;
                view_i.tree_store.get_value(iter, LevelTreeView.Column.TYPE, out item_type_val);
                if (item_type_val.holds(typeof(LevelTreeView.ItemType)) && (LevelTreeView.ItemType)item_type_val != LevelTreeView.ItemType.FOLDER) {
                    if (!current_guids.contains(guid)) {
                        view_i.tree_store.remove(ref iter);
                    }
                }
            });
        
            // Synchronize parent folders and add missing items
            foreach (Guid guid in current_guids) {
                // Check if the GUID is already present in the folder
                if (!existing_guids.contains(guid)) {
                    string item_name = view_i.level.object_editor_name(guid);
    
                    Value? parent_value = view_i.db.get_property(guid, "parent_folder");
                    Guid parent_id = (parent_value != null) ? (Guid)parent_value : target_folder;
    
                    // Check if we have the correct parent iter for the item
                    Gtk.TreeIter? parent_iter = find_parent_iter(view_i, parent_id);
    
                    if (parent_iter != null) {
                        // Ensure we don't duplicate this item under the same parent
                        bool is_duplicate = false;
    
                        // Check if the GUID is already under the parent folder
                        Gtk.TreeIter sibling_iter;
                        if (view_i.tree_store.iter_children(out sibling_iter, parent_iter)) {
                            do {
                                Value sibling_guid_val;
                                view_i.tree_store.get_value(sibling_iter, LevelTreeView.Column.GUID, out sibling_guid_val);
    
                                Guid sibling_guid;
                                if (sibling_guid_val.holds(typeof(Guid))) {
                                    sibling_guid = (Guid)sibling_guid_val;  // Cast the Value directly to Guid
                                    if (sibling_guid == guid) {
                                        is_duplicate = true;
                                        break;  // If found, don't add it again
                                    }
                                }
                                
                            } while (view_i.tree_store.iter_next(ref sibling_iter));
                        }
                        // Add the item if it is not a duplicate
                        if (!is_duplicate) {
                            Gtk.TreeIter iter;
                            view_i.tree_store.append(out iter, parent_iter);
                            view_i.tree_store.set(
                                iter,
                                LevelTreeView.Column.TYPE, item_type,
                                LevelTreeView.Column.GUID, guid,
                                LevelTreeView.Column.NAME, item_name,
                                -1
                            );
                        }
                    } else {
                        print("ERROR: Parent iter for GUID %s not found in the tree!", guid.to_string());
                    }
                }
            }
            
            view_i.tree_view.expand_all();
            view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
        }

        public static void rebuild_tree(LevelTreeView view_i) {
            view_i.tree_store.clear();
            var folder_map = new HashTable<string, Gtk.TreeIter?>(str_hash, str_equal);
        
            foreach (var root_info in get_root_folder_info()) {
                Gtk.TreeIter iter;
                view_i.tree_store.append(out iter, null);
                view_i.tree_store.set(
                    iter,
                    LevelTreeView.Column.TYPE, root_info.item_type,
                    LevelTreeView.Column.GUID, root_info.guid,
                    LevelTreeView.Column.NAME, root_info.name,
                    -1
                );
                folder_map[root_info.guid.to_string()] = iter;
            }

            foreach (var folder in view_i.level._folders) {
                if (folder.id in get_root_folder_guids()) {
                    continue;
                }
        
                Gtk.TreeIter iter;
                Gtk.TreeIter? parent_iter = folder_map[folder.parent_id.to_string()];
        
                view_i.tree_store.append(out iter, parent_iter);
                view_i.tree_store.set(
                    iter,
                    LevelTreeView.Column.TYPE, LevelTreeView.ItemType.FOLDER,
                    LevelTreeView.Column.GUID, folder.id,
                    LevelTreeView.Column.NAME, folder.name,
                    -1
                );
        
                folder_map[folder.id.to_string()] = iter;
            }
        
            sync_existing_units_and_sounds(view_i);
        }
        public static void sync_existing_units_and_sounds(LevelTreeView view_i)
        {
            foreach (var folder in get_root_folder_info())
            {
                sync_items(view_i, folder.contains_source_set_str, folder.guid, folder.contains_item_type);
            }
        }
        
        public static void sync_items(LevelTreeView view_i, string property_name, Guid default_folder, LevelTreeView.ItemType item_type)
        {
            var items = view_i.db.get_property_set(view_i.level._id, property_name, new HashSet<Guid?>());
            if (items.size == 0) return;
        
            // Cache parent GUID -> TreeIter mappings
            var parent_iter_cache = new Gee.HashMap<Guid?, Gtk.TreeIter?>(Guid.hash_func, Guid.equal_func);
        
            foreach (Guid guid in items) {
                // Get parent GUID (with default fallback)
                var parent_value = view_i.db.get_property(guid, "parent_folder");
                Guid parent_id = (parent_value != null) ? (Guid)parent_value : default_folder;
                if (parent_id == GUID_ZERO) parent_id = default_folder;
        
                // Cache lookup/creation
                Gtk.TreeIter? parent_iter = null;
                if (!parent_iter_cache.has_key(parent_id)) {
                    parent_iter = LevelTreeOrganization.find_parent_iter(view_i, parent_id);
                    parent_iter_cache[parent_id] = parent_iter;
                } else {
                    parent_iter = parent_iter_cache[parent_id];
                }
        
                if (parent_iter != null) {
                    Gtk.TreeIter iter;
                    view_i.tree_store.append(out iter, parent_iter);
                    view_i.tree_store.set(
                        iter,
                        LevelTreeView.Column.TYPE, item_type,
                        LevelTreeView.Column.GUID, guid,
                        LevelTreeView.Column.NAME, view_i.level.object_editor_name(guid),
                        -1
                    );
                } else {
                    print("ERROR: Parent iter for " + item_type.to_string() + " GUID " + guid.to_string() + " not found!");
                }
            }
        }
        private static void add_folders_recursively(LevelTreeView view_i, string parent_guid, HashTable<string, Gtk.TreeIter?> folder_map) {
            foreach (var folder in view_i.level._folders) {
                if (folder.parent_id.to_string() == parent_guid) {
                    Gtk.TreeIter iter;
                    Gtk.TreeIter? parent_iter = folder_map[parent_guid];
                    view_i.tree_store.append(out iter, parent_iter);
                    
                    view_i.tree_store.set(
                        iter,
                        LevelTreeView.Column.TYPE, LevelTreeView.ItemType.FOLDER,  
                        LevelTreeView.Column.GUID, folder.id,
                        LevelTreeView.Column.NAME, folder.name,
                        -1
                    );
                    
                    folder_map[folder.id.to_string()] = iter;
                    add_folders_recursively(view_i, folder.id.to_string(), folder_map);
                }
            }
        }     
        public static void add_folder_to_tree(LevelTreeView view_i, bool save_to_file, Gtk.TreeIter? parent_iter, string name,string type, Guid guid)
        {
            if (save_to_file) {
                Guid parent_guid = get_parent_guid(view_i, parent_iter);
                view_i.level.add_folder(guid, name,type, parent_guid);
            }
        
            Gtk.TreeIter iter;
            view_i.tree_store.insert_with_values(out iter, parent_iter, -1,
                LevelTreeView.Column.TYPE, LevelTreeView.ItemType.FOLDER,
                LevelTreeView.Column.GUID, guid,
                LevelTreeView.Column.NAME, name
            );
        
            if (parent_iter != null) {
                Gtk.TreePath path = view_i.tree_store.get_path(parent_iter);
                view_i.tree_view.expand_row(path, false);
            }
        }
        public static Guid get_parent_guid(LevelTreeView view_i, Gtk.TreeIter? parent_iter)
        {
            if (parent_iter == null)
                return GUID_NONE_FOLDER;
    
            Value val;
            view_i.tree_store.get_value(parent_iter, LevelTreeView.Column.GUID, out val);
            return (Guid)val;
        }
        public static Gtk.TreeIter? find_parent_iter(LevelTreeView view_i, Guid parent_id)
        {
            Gtk.TreeIter? found = null;
            view_i.tree_store.foreach((model, path, iter) => {
                Value val;
                model.get_value(iter, LevelTreeView.Column.GUID, out val);
                if (val.holds(typeof(Guid))) {
                    Guid guid = (Guid)val;
                    if (guid == parent_id) {
                        found = iter;
                        return true;
                    }
                }
                return false;
            });		
            return found;
        }
    }
}