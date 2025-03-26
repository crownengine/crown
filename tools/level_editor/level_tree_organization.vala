/*
* Copyright (c) 2012-2025 Daniele Bartolini et al.
* SPDX-License-Identifier: GPL-3.0-or-later
*/
using Gee;
using Gtk;

namespace Crown
{
    // Manage drag and drop and folder organization for the LevelTreeView
    public class LevelTreeOrganization
    {
        public static bool on_drag_drop_internal(LevelTreeView view_i, string d_target, Gtk.Widget widget, Gdk.DragContext context, int x, int y, uint time) {
            Gtk.drag_get_data(widget, context, Gdk.Atom.intern(d_target, false), time);
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
        
                // Retrieve the GUID
                Value guid_val;
                model.get_value(iter, LevelTreeView.Column.GUID, out guid_val);
                Guid guid = (Guid)guid_val;
        
                Value? item_type = view_i.db.get_property(guid, "_type");
        
                string type_str = (string)item_type;
        
                if (type_str.has_suffix("_folder")) {
                    print("Folder drag ignored: %s\n", type_str);
                    Gtk.drag_cancel(context);
                    return;
                }
        
                // Find the corresponding source_set
                bool type_found = false;
                foreach (var root_info in get_root_folder_info()) {
                    if (type_str == root_info.contains_item_type_str) {
                        source_set = root_info.contains_source_set_str;
                        type_found = true;
                        break;
                    }
                }
        
                if (!type_found) {
                    print("Unhandled type: %s\n", type_str);
                    continue;
                }
        
                data_builder.append(guid.to_string() + ",");
            }
        
            if (data_builder.len > 0) {
                data_builder.truncate(data_builder.len - 1); // Remove trailing comma
                string data_str = @"$source_set;$(data_builder.str)";
                var target = Gdk.Atom.intern(d_target, false);
                selection_data.set(target, 8, data_str.data);
            } else {
                print("No draggable items - cancelling drag operation.");
                Gtk.drag_finish(context, false, false, time);
            }
        }        
        
        // TODO SUPPORT FOLDER DRAG AND DROPPING LATER
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
            if (raw_data == null || raw_data.strip() == null) {
                print("ERROR: Received empty or null drag data (if you tried to drag and dropped a folder its normal that you had this error ,for now its not supported).\n");
                Gtk.drag_finish(context, false, false, time);
                view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
                return;
            }
        
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
                    parent_guid = GUID_ZERO;
                    foreach (var root_info in get_root_folder_info()) {
                        if (item_type == root_info.contains_item_type_str) {
                            parent_guid = root_info.guid;
                            break;
                        }
                    }
        
                    if (parent_guid == GUID_ZERO) {
                        print("Error: Unable to determine the root parent of element %s", guid.to_string());
                        continue;
                    }
                } else {
                    parent_guid = (Guid)parent_guid_val;
                }
        
                Value? parent_type_val = view_i.db.get_property(parent_guid, "_type");
                string parent_type = (string)parent_type_val;
                moving_type = parent_type;
                
                Guid target_guid;
                if (!Guid.try_parse(target_parent_guid, out target_guid)) {
                    print("Error: Invalid target GUID for parent %s", target_parent_guid);
                    view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
                    return;
                }
                string target_type = null;
                foreach (var root_info in get_root_folder_info()) {
                    if (Guid.equal_func(target_guid, root_info.guid)) {
                        target_type = root_info.object_type;
                        break;
                    }
                }
                if (target_type == null || target_type.strip().length == 0) {
                    Value? target_type_val_db = view_i.db.get_property(target_guid, "_type");
                    if (target_type_val_db != null) {
                        target_type = (string)target_type_val_db;
                    } else {
                        print("Error: Unable to determine target type for GUID %s", target_guid.to_string());
                        view_i.tree_selection.changed.connect(view_i.on_tree_selection_changed);
                        return;
                    }
                }  
                string current_item_type = (string)view_i.db.get_property(guid, "_type");
         
                bool valid_move = false;
                foreach (var root_info in get_root_folder_info()) {
                    if (target_type == root_info.object_type && current_item_type == root_info.contains_item_type_str) {
                        valid_move = true;
                        break;
                    }
                }

                if (!valid_move) {
                    print("Error: Cannot move %s into %s", current_item_type, target_type);
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
                            LevelTreeSynching.append_to_tree_store(view_i, parent_iter, root_info.contains_item_type, guid, view_i.level.object_editor_name(guid));
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
        
        private static void add_folders_recursively(LevelTreeView view_i, string parent_guid, HashTable<string, Gtk.TreeIter?> folder_map) {
            foreach (var folder in view_i.level._folders) {
                if (folder.parent_id.to_string() == parent_guid) {
                    Gtk.TreeIter? parent_iter = folder_map[parent_guid];
                    Gtk.TreeIter iter = LevelTreeSynching.append_to_tree_store(view_i, parent_iter, LevelTreeView.ItemType.FOLDER, folder.id, folder.name);
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