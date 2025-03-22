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
        
                // Detect original name
                string original_name = "";
                if (resources_to_export.size > 0) {
                    string first_resource = resources_to_export[0];
                    original_name = Path.get_basename(first_resource).split(".")[0];
                }
                foreach (string resource_path in resources_to_export) {
                    string abs_path = Path.is_absolute(resource_path) 
                        ? resource_path 
                        : unit._db.get_project().absolute_path(resource_path);
        
                    // Original Relative Path
                    string original_relative_path = abs_path.replace(project_dir + GLib.Path.DIR_SEPARATOR_S, "");
                    original_relative_path = original_relative_path.replace("\\", "/");
                    string original_without_ext = original_relative_path.substring(0, original_relative_path.last_index_of_char('.'));
        
                    // Exported Relative Path
                    string exported_path = Path.build_filename(export_dir, unit_basename);
                    string new_relative_path = exported_path.replace(project_dir + GLib.Path.DIR_SEPARATOR_S, "");
                    path_map[original_without_ext] = new_relative_path;
        
                    // Copying and processing files
                    string ext = abs_path.substring(abs_path.last_index_of_char('.') + 1);
                    string new_filename = unit_basename + "." + ext;
                    string dest_path = Path.build_filename(export_dir, new_filename);
                    File src = File.new_for_path(abs_path);
                    File dest = File.new_for_path(dest_path);
                    src.copy(dest, FileCopyFlags.OVERWRITE);        
                    if (ext == "mesh") {
                        try {
                            string mesh_content;
                            if (FileUtils.get_contents(abs_path, out mesh_content)) {
                                print("Mesh file content successfully read from: %s".printf(abs_path));
                                foreach (string line in mesh_content.split("\n")) {
                                    if (line.contains("source =")) {
                                        print("Found 'source =' line: %s".printf(line));

                                        // Extract source path
                                        string[] parts = line.split("=", 2);
                                        if (parts.length == 2) {
                                            string source_path = parts[1]
                                                .strip()
                                                .replace("\"", "")
                                                .replace("'", "");
                                            print("Extracted source path: %s".printf(source_path));

                                            // Check if it ends with .fbx
                                            if (source_path.has_suffix(".fbx")) {
                                                print("Source path has .fbx suffix.");

                                                // Resolve absolute path for the FBX
                                                string abs_fbx = Path.is_absolute(source_path)
                                                    ? source_path
                                                    : unit._db.get_project().absolute_path(source_path);
                                                print("Resolved absolute FBX path: %s".printf(abs_fbx));
    
                                                // Validate the existence of the FBX file
                                                if (FileUtils.test(abs_fbx, FileTest.EXISTS)) {
                                                    print("FBX file exists: %s".printf(abs_fbx));

                                                    // Rename FBX file to match unit's name
                                                    string new_fbx_filename = unit_basename + ".fbx";  // Rename FBX to match the unit name
                                                    string fbx_dest_path = Path.build_filename(export_dir, new_fbx_filename);
                                                    File fbx_src = File.new_for_path(abs_fbx);
                                                    File fbx_dest = File.new_for_path(fbx_dest_path);
                                                    print("Copying FBX file to: %s".printf(fbx_dest.get_path()));
                                                    fbx_src.copy(fbx_dest, FileCopyFlags.OVERWRITE);

                                                    // Modify the mesh file's source path
                                                    new_relative_path = fbx_dest_path.replace(project_dir + GLib.Path.DIR_SEPARATOR_S, "");
                                                    print("Relative path: %s".printf(new_relative_path));

                                                    // Replace old source path with the new relative path
                                                    string new_source = Path.build_filename(Path.get_dirname(new_relative_path), unit_basename + ".fbx");
                                                    // Ensure all backslashes are replaced with forward slashes
                                                    new_source = new_source.replace("\\", "/");                                  
                                                    // Update the mesh content
                                                    mesh_content = mesh_content.replace(source_path, new_source);
                                                    FileUtils.set_contents(dest_path, mesh_content);  // Write the updated content back to the .mesh file
                                                    
                                                    // Handle importer settings
                                                    string fbx_basename_without_extension = Path.get_basename(abs_fbx).replace(".fbx", "");
                                                    string importer_src_path = Path.build_filename(Path.get_dirname(abs_fbx), fbx_basename_without_extension + ".importer_settings");
                                                    print("Checking for importer settings file: %s".printf(importer_src_path));
                                                    if (FileUtils.test(importer_src_path, FileTest.EXISTS)) {
                                                        print("Importer settings file exists: %s".printf(importer_src_path));
                                                        
                                                        // Rename the importer settings to match the unit's base name
                                                        string importer_dest_filename = unit_basename + ".importer_settings";
                                                        string importer_dest_path = Path.build_filename(export_dir, importer_dest_filename);
                                                        
                                                        File importer_file_src = File.new_for_path(importer_src_path);
                                                        File importer_file_dest = File.new_for_path(importer_dest_path);
                                                        
                                                        print("Copying importer settings file to: %s".printf(importer_file_dest.get_path()));
                                                        importer_file_src.copy(importer_file_dest, FileCopyFlags.OVERWRITE);
                                                    } else {
                                                        print("Importer settings file does not exist.");
                                                    }
                                                } else {
                                                    print("FBX file does not exist at path: %s".printf(abs_fbx));
                                                }
                                            } else {
                                                print("Source path does not end with .fbx: %s".printf(source_path));
                                            }
                                        } else {
                                            print("Malformed 'source =' line: %s".printf(line));
                                        }
                                    }
                                }
                            } else {
                                print("Failed to read mesh file content: %s".printf(abs_path));
                            }
                        } catch (Error e) {
                            warning("Error processing .mesh file: %s".printf(e.message));
                        }
                    }
                }
                unit_data = unit_data.replace(original_name, unit_basename);
        
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
                sb.append("\t\t_guid = \"%s\"\n".printf(component_id.to_string()));
                sb.append("\t\t_type = \"%s\"\n".printf(component_type));
                sb.append("\t\tdata = {\n");
                if (component_type == "transform") {
                    sb.append("\t\t\tposition = [0.000, 0.000, 0.000]\n");
                    sb.append("\t\t\trotation = [0.000, 0.000, 0.000, 1.000]\n");
                    sb.append("\t\t\tscale = [1.000, 1.000, 1.000]\n");
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
        
                    Unit child_unit = new Unit(unit._db, child_id);
                    string child_data = generate_export_data(child_unit,resources);
                    
                    string[] lines = child_data.split("\n");
                    sb.append("\t{\n");
        
                    foreach (string line in lines) {
                        if (line.strip().length == 0) continue;
                        
                        if (line.contains("_type = \"transform\"")) {
                            sb.append("\t\t\tposition = [0.000, 0.000, 0.000]\n");
                            sb.append("\t\t\trotation = [0.000, 0.000, 0.000, 1.000]\n");
                            sb.append("\t\t\tscale = [1.000, 1.000, 1.000]\n");
                        }
    
                        sb.append("\t\t%s\n".printf(line));
                    }
                    sb.append("\t},\n");
                }
        
                sb.append("]\n");
            }
        }
        
        private static Gee.Collection<Guid?> get_all_children(Unit unit) {
            Gee.ArrayList<Guid?> children = new Gee.ArrayList<Guid?>();
            // Get direct children from the unit
            Value? direct_children = unit._db.get_property(unit._id, "children");
            if (direct_children != null) {
                children.add_all((Gee.Collection<Guid?>)direct_children);
            }
            // Check for a prefab associated with the unit
            Value? prefab = unit._db.get_property(unit._id, "prefab");
            if (prefab != null) {
                string prefab_path = (string)prefab;  // Get the prefab path
                Guid prefab_id = resolve_prefab_id(unit, prefab_path);

                if (prefab_id != GUID_ZERO) {
                    // Create a Unit from the prefab ID
                    Unit prefab_unit = new Unit(unit._db, prefab_id);
                    // Recursively get children from the prefab unit
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
                warning("Prefab non trouvé: %s", prefab_path);
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

        private static string value_to_lua(Value val)
        {
            Type type = val.type();
            if (type == typeof(string)) {
                return "\"%s\"".printf((string)val);
            }
            if (type == typeof(Guid)) {
                return "\"%s\"".printf(((Guid)val).to_string());
            }
            if (type == typeof(bool)) {
                return ((bool)val) ? "true" : "false";
            }
            if (type == typeof(double)) {
                return ((double)val).to_string();
            }
            if (type == typeof(Vector3)) {
                Vector3 v = (Vector3)val;
                return "[%.3f, %.3f, %.3f]".printf(v.x, v.y, v.z);
            }
            if (type == typeof(Quaternion)) {
                Quaternion q = (Quaternion)val;
                return "[%.3f, %.3f, %.3f, %.3f]".printf(q.x, q.y, q.z, q.w); 
            }
            return "nil";
        }
    }
}