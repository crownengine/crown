/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
using Gee;

namespace Crown
{
    public class UnitExporter
    {
        public static bool export_to_file(Unit unit, string path) {
            try {
                Gee.ArrayList<string> resources_to_export = new Gee.ArrayList<string>();
                string unit_data = generate_export_data(unit, resources_to_export);
        
                string export_dir = Path.get_dirname(path);
                string unit_basename = Path.get_basename(path).replace(".unit", "");
                string project_dir = unit._db.get_project().source_dir();
                Gee.HashMap<string, string> path_map = new Gee.HashMap<string, string>();
        
                string original_name = "";
                if (resources_to_export.size > 0) {
                    string first_resource = resources_to_export[0];
                    original_name = Path.get_basename(first_resource).split(".")[0];
                }
                foreach (string resource_path in resources_to_export) {
                    string abs_path = Path.is_absolute(resource_path) 
                        ? resource_path 
                        : unit._db.get_project().absolute_path(resource_path);
        
                    string original_relative_path = abs_path.replace(project_dir + GLib.Path.DIR_SEPARATOR_S, "");
                    original_relative_path = original_relative_path.replace("\\", "/");
                    string original_without_ext = original_relative_path.substring(0, original_relative_path.last_index_of_char('.'));
        
                    string exported_path = Path.build_filename(export_dir, unit_basename);
                    string new_relative_path = exported_path.replace(project_dir + GLib.Path.DIR_SEPARATOR_S, "");
                    new_relative_path = new_relative_path.replace("\\", "/"); 
        
                    path_map[original_without_ext] = new_relative_path;
                    string ext = abs_path.substring(abs_path.last_index_of_char('.') + 1);
                    string new_filename = unit_basename + "." + ext;
                    string dest_path = Path.build_filename(export_dir, new_filename);
                    File src = File.new_for_path(abs_path);
                    File dest = File.new_for_path(dest_path);
                    src.copy(dest, FileCopyFlags.OVERWRITE);        
                    if (ext == "mesh") {
                        process_mesh_file(unit,abs_path, export_dir, dest_path, unit_basename, project_dir);
                    }                    
                }
                var entries = new Gee.ArrayList<Gee.Map.Entry<string, string>>();
                entries.add_all(path_map.entries);
                entries.sort((a, b) => b.key.length - a.key.length);
        
                foreach (var entry in entries) {
                    string key_normalized = entry.key.replace("\\", "/");
                    unit_data = unit_data.replace(key_normalized, entry.value);
                    unit_data = unit_data.replace(entry.key, entry.value);
                }
        
                FileUtils.set_contents(path, unit_data);
        
                return true;
            } catch (Error e) {
                error("Export failed: %s", e.message);
            }
        }

        private static void process_mesh_file(Unit unit, string abs_path, string export_dir,string dest_path, string unit_basename, string project_dir) {
            try {
                string mesh_content;
        
                // Handle potential file read exceptions
                if (FileUtils.get_contents(abs_path, out mesh_content)) {
                    foreach (string line in mesh_content.split("\n")) {
                        try {
                            if (line.contains("_guid =")) {
                                string[] parts = line.split("=", 2);
                                if (parts.length == 2) {
                                    string new_guid = Guid.new_guid().to_string();
                                    string new_guid_line = "_guid = \"" + new_guid + "\"";
                                    mesh_content = mesh_content.replace(line, new_guid_line);
                                }
                            }
        
                            if (line.contains("source =")) {
                                string[] parts = line.split("=", 2);
                                if (parts.length == 2) {
                                    string source_path = parts[1]
                                        .strip()
                                        .replace("\"", "")
                                        .replace("'", "");
        
                                    if (source_path.has_suffix(".fbx")) {
                                        string abs_fbx = Path.is_absolute(source_path)
                                            ? source_path
                                            : unit._db.get_project().absolute_path(source_path);
        
                                        if (FileUtils.test(abs_fbx, FileTest.EXISTS)) {
                                            string new_fbx_filename = unit_basename + ".fbx"; 
                                            string fbx_dest_path = Path.build_filename(export_dir, new_fbx_filename);
                                            File fbx_src = File.new_for_path(abs_fbx);
                                            File fbx_dest = File.new_for_path(fbx_dest_path);
                                            fbx_src.copy(fbx_dest, FileCopyFlags.OVERWRITE);
                            
                                            string fbx_new_relative_path = fbx_dest_path.replace(project_dir + GLib.Path.DIR_SEPARATOR_S, "");
                                            string new_source = Path.build_filename(Path.get_dirname(fbx_new_relative_path), unit_basename + ".fbx").replace("\\", "/");
                                            mesh_content = mesh_content.replace(source_path, new_source);
                                            FileUtils.set_contents(dest_path, mesh_content);
                            
                                            string fbx_basename_without_extension = Path.get_basename(abs_fbx).replace(".fbx", "");
                                            string importer_src_path = Path.build_filename(Path.get_dirname(abs_fbx), fbx_basename_without_extension + ".importer_settings");
                                            if (FileUtils.test(importer_src_path, FileTest.EXISTS)) {
                                                string importer_dest_filename = unit_basename + ".importer_settings";
                                                string importer_dest_path = Path.build_filename(export_dir, importer_dest_filename);
                            
                                                File importer_file_src = File.new_for_path(importer_src_path);
                                                File importer_file_dest = File.new_for_path(importer_dest_path);
                                                importer_file_src.copy(importer_file_dest, FileCopyFlags.OVERWRITE);
                                            }
                                        }
                                    }
                                }
                            }
                        } catch (Error e) {
                            warning("Error processing line in mesh content: %s".printf(e.message));
                        }
                    }
                } else {
                    error("Failed to read mesh file content: %s".printf(abs_path));
                }
            } catch (FileError e) {
                error("File error encountered while processing mesh file: %s".printf(e.message));
            }
        }
        
        private static string generate_export_data(Unit unit,Gee.List<string> resources) {
            StringBuilder sb = new StringBuilder();
            sb.append("_guid = \"%s\"\n".printf(Guid.new_guid().to_string()));
            sb.append("_type = \"unit\"\n");

            process_components_section(unit, sb, resources);

            process_children_section(unit, sb, resources);

            return sb.str;
        }

        private static void process_components_section(Unit unit, StringBuilder sb, Gee.List<string> resources) {
            sb.append("components = [\n");
            
            HashSet<Guid?> components = new HashSet<Guid?>(Guid.hash_func, Guid.equal_func);
            collect_direct_components(unit,unit._id, components);
            
            foreach (Guid? component_id in components) {
                if (component_id == null) continue;
                string component_type = unit._db.object_type(component_id);
                sb.append("\t{\n");
                sb.append("\t\t_guid = \"%s\"\n".printf(Guid.new_guid().to_string()));
                sb.append("\t\t_type = \"%s\"\n".printf(component_type));
                sb.append("\t\tdata = {\n");
                if (component_type == "transform") {
                    process_transform_data(sb, unit, component_id);
                } else {
                    string[] keys = unit._db.get_keys(component_id);
                    foreach (string key in keys) {
                        if (key == "_type" || key == "_guid" || key == "type") continue;
                        string cleaned_key = key.replace("data.", "");
                        Value? val = unit.get_component_property(component_id, key);
                        if (val != null) {
                            sb.append("\t\t\t%s = %s\n".printf(cleaned_key, value_to_lua(val)));
                            if (component_type == "mesh_renderer" && (cleaned_key == "mesh_resource" || cleaned_key == "material")) {
                                string resource_path = (string)val;
                                if (cleaned_key == "mesh_resource" && !resource_path.has_suffix(".mesh")) {
                                    resource_path += ".mesh";
                                } else if (cleaned_key == "material" && !resource_path.has_suffix(".material")) {
                                    resource_path += ".material";
                                }
                                resources.add(resource_path);
                            }
                        }
                    }
                }
                sb.append("\t\t}\n");
                sb.append("\t\ttype = \"%s\"\n".printf(component_type));
                sb.append("\t},\n");
            }
            
            sb.append("]\n");
        }

        private static void process_children_section(Unit unit, StringBuilder sb, Gee.List<string> resources) {
            Gee.Collection<Guid?> children = get_all_children(unit);
        
            if (children.size > 0) {
                sb.append("children = [\n");
        
                foreach (Guid? child_id in children) {
                    if (child_id == null) continue;
        
                    Unit child_unit = Unit(unit._db, child_id);
                    string child_data = generate_export_data(child_unit,resources);
                    
                    string[] lines = child_data.split("\n");
                    sb.append("\t{\n");
        
                    foreach (string line in lines) {
                        if (line.strip().length == 0) continue;
                        
                        if (line.contains("_type = \"transform\"")) {
                            process_transform_data(sb, unit, child_id);
                        }
    
                        sb.append("\t\t%s\n".printf(line));
                    }
                    sb.append("\t},\n");
                }
        
                sb.append("]\n");
            }
        }

        private static void process_transform_data(StringBuilder sb, Unit unit, Guid? component_id) {
            sb.append("\t\t\tposition = [0.000, 0.000, 0.000]\n");
        
            Value? rotation = unit.get_component_property(component_id, "data.rotation");
            if (rotation != null) {
                sb.append("\t\t\trotation = %s\n".printf(value_to_lua(rotation)));
            } else {
                sb.append("\t\t\trotation = [0.000, 0.000, 0.000, 1.000]\n");
            }
        
            Value? scale = unit.get_component_property(component_id, "data.scale");
            if (scale != null) {
                sb.append("\t\t\tscale = %s\n".printf(value_to_lua(scale)));
            } else {
                sb.append("\t\t\tscale = [1.000, 1.000, 1.000]\n");
            }
        }
        
        private static Gee.Collection<Guid?> get_all_children(Unit unit) {
            Gee.ArrayList<Guid?> children = new Gee.ArrayList<Guid?>();
            Value? direct_children = unit._db.get_property(unit._id, "children");
            if (direct_children != null) {
                children.add_all((Gee.Collection<Guid?>)direct_children);
            }
            Value? prefab = unit._db.get_property(unit._id, "prefab");
            if (prefab != null) {
                string prefab_path = (string)prefab;
                Guid prefab_id = resolve_prefab_id(unit, prefab_path);

                if (prefab_id != GUID_ZERO) {
                    Unit prefab_unit = Unit(unit._db, prefab_id);
                    children.add_all(get_all_children(prefab_unit));
                }
            }
            return children;
        }
        
        private static Guid resolve_prefab_id(Unit unit,string prefab_path) {
            string unit_path = prefab_path.has_suffix(".unit") 
                ? prefab_path 
                : prefab_path + ".unit";
            
            Guid guid = unit._db.get_property_guid(GUID_ZERO, unit_path);
            
            if (guid == GUID_ZERO) {
                warning("Prefab not found: %s", prefab_path);
            }
            
            return guid;
        }

        private static void collect_direct_components(Unit unit,Guid unit_id, Gee.Set<Guid?> components) {
            Value? direct = unit._db.get_property(unit_id, "components");
            if (direct != null) {
                components.add_all((Gee.Collection<Guid?>)direct);
            }
            
            Value? prefab = unit._db.get_property(unit_id, "prefab");
            if (prefab != null) {
                string prefab_path = (string)prefab;
                Guid prefab_id = resolve_prefab_id(unit,prefab_path);
                if (prefab_id != GUID_ZERO) {
                    collect_direct_components(unit,prefab_id, components);
                }
            }
            
            remove_deleted_components(unit,unit_id, components); 
        }

        private static void remove_deleted_components(Unit unit,Guid unit_id, Gee.Set<Guid?> components) {
            string[] deleted_keys = unit._db.get_keys(unit_id);
            foreach (string key in deleted_keys) {
                if (key.has_prefix("deleted_components.#")) {
                    Guid deleted_id = Guid.parse(key.split("#")[1]);
                    components.remove(deleted_id);
                }
            }
        }

        private static string value_to_lua(Value val) {
            Type type = val.type();
            if (type == typeof(string)) {
                return "\"%s\"".printf((string)val);
            } else if (type == typeof(Guid)) {
                return "\"%s\"".printf(((Guid)val).to_string());
            } else if (type == typeof(bool)) {
                return ((bool)val) ? "true" : "false";
            } else if (type == typeof(double)) {
                return ((double)val).to_string();
            } else if (type == typeof(Vector3)) {
                Vector3 v = (Vector3)val;
                return "[%.3f, %.3f, %.3f]".printf(v.x, v.y, v.z);
            } else if (type == typeof(Quaternion)) {
                Quaternion q = (Quaternion)val;
                return "[%.3f, %.3f, %.3f, %.3f]".printf(q.x, q.y, q.z, q.w);
            } else {
                return "nil";
            }
        }        
    }
}