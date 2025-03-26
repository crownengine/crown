/*
* Copyright (c) 2012-2025 Daniele Bartolini et al.
* SPDX-License-Identifier: GPL-3.0-or-later
*/
using Gee;
using Gtk;

namespace Crown
{
    // Manage synching for the LevelTreeView
    public class LevelTreeSynching
    {
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
                append_to_tree_store(view_i, null, LevelTreeView.ItemType.FOLDER, target_folder, (key == "units") ? "Units" : "Sounds");
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
            // TODO SUPPORT FOLDER REMOVING LATER
            existing_guids.foreach((guid, iter) => {
                Value item_type_val;
                view_i.tree_store.get_value(iter, LevelTreeView.Column.TYPE, out item_type_val);
                if (item_type_val.holds(typeof(LevelTreeView.ItemType)) && (LevelTreeView.ItemType)item_type_val != LevelTreeView.ItemType.FOLDER) {
                    if (!current_guids.contains(guid)) {
                        view_i.tree_store.remove(ref iter);
                    }
                }
                remove_outdated_items_in_subfolders(view_i, iter, current_guids);
            });
        
            // Synchronize parent folders and add missing items
            foreach (Guid guid in current_guids) {
                // Check if the GUID is already present in the folder
                if (!existing_guids.contains(guid)) {
                    string item_name = view_i.level.object_editor_name(guid);
    
                    Value? parent_value = view_i.db.get_property(guid, "parent_folder");
                    Guid parent_id = (parent_value != null) ? (Guid)parent_value : target_folder;
    
                    // Check if we have the correct parent iter for the item
                    Gtk.TreeIter? parent_iter = LevelTreeOrganization.find_parent_iter(view_i, parent_id);
    
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
                            append_to_tree_store(view_i, parent_iter, item_type, guid, item_name);
                        }
                    } else {
                        print("ERROR: Parent iter for GUID %s not found in the tree!", guid.to_string());
                    }
                }
            }
            
            view_i.tree_view.expand_all();
            view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
        }
        private static void remove_outdated_items_in_subfolders(LevelTreeView view_i, Gtk.TreeIter iter, HashSet<Guid?> current_guids) {
            // Iterate over all children (subfolders and items) under the current iter
            Gtk.TreeIter child_iter;
            if (view_i.tree_store.iter_children(out child_iter, iter)) {
                do {
                    // Get the GUID and item type of the current child
                    Value guid_val;
                    view_i.tree_store.get_value(child_iter, LevelTreeView.Column.GUID, out guid_val);
                    Value item_type_val;
                    view_i.tree_store.get_value(child_iter, LevelTreeView.Column.TYPE, out item_type_val);
        
                    if (guid_val.holds(typeof(Guid)) && item_type_val.holds(typeof(LevelTreeView.ItemType))) {
                        Guid guid = (Guid)guid_val;
                        LevelTreeView.ItemType item_type = (LevelTreeView.ItemType)item_type_val;
        
                        // If this item is not a folder, check if it's outdated and remove it
                        if (item_type != LevelTreeView.ItemType.FOLDER) {
                            if (!current_guids.contains(guid)) {
                                view_i.tree_store.remove(ref child_iter);
                            }
                        } else {
                            // If it's a folder, recursively check for outdated items inside it
                            remove_outdated_items_in_subfolders(view_i, child_iter, current_guids);
                        }
                    }
                } while (view_i.tree_store.iter_next(ref child_iter));
            }
        }
        public static void rebuild_tree(LevelTreeView view_i) {
            view_i.tree_store.clear();
            var folder_map = new HashTable<string, Gtk.TreeIter?>(str_hash, str_equal);
        
            foreach (var root_info in get_root_folder_info()) {
                Gtk.TreeIter iter = append_to_tree_store(view_i, null, root_info.item_type, root_info.guid, root_info.name);
                folder_map[root_info.guid.to_string()] = iter;
            }

            foreach (var folder in view_i.level._folders) {
                if (folder.id in get_root_folder_guids()) {
                    continue;
                }
                Gtk.TreeIter? parent_iter = folder_map[folder.parent_id.to_string()];
                Gtk.TreeIter iter = append_to_tree_store(view_i, parent_iter, LevelTreeView.ItemType.FOLDER, folder.id, folder.name);
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
                    append_to_tree_store(view_i, parent_iter, item_type, guid, view_i.level.object_editor_name(guid));
                } else {
                    print("ERROR: Parent iter for " + item_type.to_string() + " GUID " + guid.to_string() + " not found!");
                }
            }
        }
        
        public static Gtk.TreeIter append_to_tree_store(LevelTreeView view_i, Gtk.TreeIter? parent_iter, LevelTreeView.ItemType type, Guid guid, string name) {
            Gtk.TreeIter iter;
            view_i.tree_store.append(out iter, parent_iter);
            view_i.tree_store.set(iter, LevelTreeView.Column.TYPE, type, LevelTreeView.Column.GUID, guid, LevelTreeView.Column.NAME, name, -1);
            return iter; 
        }
    }
}