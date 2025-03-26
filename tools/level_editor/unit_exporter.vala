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
                // Get the project associated with the unit
                Project project = unit._db.get_project();
                
                // Generate a new GUID for the prefab
                Guid prefab_guid = Guid.new_guid();
                
                // If duplication is successful, generate the prefab's export data
                string prefab_data = generate_export_data(unit, new Gee.ArrayList<string>());
                
                // Write the generated prefab data to the file
                FileUtils.set_contents(path, prefab_data);
                
                // Compile the data
                var app = (LevelEditorApplication)GLib.Application.get_default();
                DataCompiler dc = app.get_data_compiler();
                dc.compile.begin(project.data_dir(), project.platform(), (obj, res) => {
                    try {
                        dc.compile.end(res);
                    } catch (Error e) {
                        error("Data compilation failed after export: %s", e.message);
                    }
                });
        
                return true;
            } catch (Error e) {
                error("Export failed: %s", e.message);
                return false;
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