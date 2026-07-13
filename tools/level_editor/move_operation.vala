/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class MoveOperation
{
	public Project _project;
	public Database _database;
	public Level _level;

	public MoveOperation(Project project, Database database, Level level)
	{
		_project = project;
		_database = database;
		_level = level;
	}

	public bool rewrite_database_references(Database db
		, string old_path
		, string new_path
		, string? old_type
		, string? old_name
		, string? new_name
		)
	{
		bool changed = false;
		bool old_type_is_database_type = old_type != null && db.has_type(StringId64(old_type));

		foreach (Guid? id in db.all_objects_of_type(OBJECT_TYPE_ANY)) {
			string object_type = db.object_type(id);
			unowned PropertyDefinition[]? properties = db.object_definition(StringId64(object_type));

			if (properties != null) {
				foreach (PropertyDefinition def in properties) {
					if (def.type == PropertyType.RESOURCE
						&& old_type != null
						&& old_name != null
						&& new_name != null
						&& def.resource_type == old_type
						&& db.get_resource(id, def.name) == old_name
						) {
						db.set_resource(id, def.name, new_name);
						changed = true;
					}
				}
			}

			foreach (string key in db.get_keys(id)) {
				Value? val = db.get_property(id, key);
				if (val == null)
					continue;

				bool is_resource = val.holds(typeof(Resource));
				if (!is_resource && !val.holds(typeof(string)))
					continue;

				string? value = is_resource ? ((Resource)val).name : (string)val;
				string? replacement = null;
				if (value == old_path)
					replacement = new_path;
				else if (old_type_is_database_type && old_name != null && new_name != null && value == old_name)
					replacement = new_name;
				if (replacement == null)
					continue;

				if (is_resource)
					db.set_resource(id, key, replacement);
				else
					db.set_string(id, key, replacement);
				changed = true;
			}
		}

		return changed;
	}

	public bool rewrite_sjson_value(ref Value? value
		, string old_path
		, string new_path
		, string? old_name
		, string? new_name
		)
	{
		if (value == null)
			return false;

		if (value.holds(typeof(string))) {
			string str = (string)value;
			if (str == old_path) {
				value = new_path;
				return true;
			}
			if (old_name != null && new_name != null && str == old_name) {
				value = new_name;
				return true;
			}
			return false;
		}

		bool changed = false;
		if (value.holds(typeof(Hashtable))) {
			Hashtable object = (Hashtable)value;
			string[] keys = object.keys.to_array();
			foreach (string key in keys) {
				Value? item = object[key];
				if (rewrite_sjson_value(ref item, old_path, new_path, old_name, new_name)) {
					object[key] = item;
					changed = true;
				}
			}
		} else if (value.holds(typeof(Gee.ArrayList))) {
			Gee.ArrayList<Value?> array = (Gee.ArrayList<Value?>)value;
			for (int i = 0; i < array.size; ++i) {
				Value? item = array[i];
				if (rewrite_sjson_value(ref item, old_path, new_path, old_name, new_name)) {
					array[i] = item;
					changed = true;
				}
			}
		}

		return changed;
	}

	public int rewrite_sjson_references_in_path(out string error
		, string path
		, string old_path
		, string new_path
		, string? old_name
		, string? new_name
		)
	{
		error = "";

		try {
			Hashtable json = SJSON.load_from_path(_project.absolute_path(path));
			Value? root = json;
			if (!rewrite_sjson_value(ref root, old_path, new_path, old_name, new_name))
				return 0;

			SJSON.save(json, _project.absolute_path(path));
		} catch (GLib.Error e) {
			error = e.message;
			return -1;
		}

		return 0;
	}

	public int rewrite_references_in_path(out string error
		, string path
		, string old_path
		, string new_path
		, string? old_type
		, string? old_name
		, string? new_name
		)
	{
		error = "";
		string active_level_path = _level._name != null
			? ResourceId.path(OBJECT_TYPE_LEVEL, _level._name)
			: ""
			;

		if (path == active_level_path) {
			if (rewrite_database_references(_database, old_path, new_path, old_type, old_name, new_name)) {
				if (_level.save(_level._name) != 0) {
					error = "Failed to save rewritten references in %s".printf(path);
					return -1;
				}
			}
			return 0;
		}

		string? path_type = ResourceId.type(path);
		if (path_type == null || !_database.has_type(StringId64(path_type)))
			return rewrite_sjson_references_in_path(out error, path, old_path, new_path, old_name, new_name);

		Database db = new Database(_project, null);
		create_object_types(db);

		Guid id;
		LoadError err = db.load_from_path(out id, _project.absolute_path(path), path);
		if (err != LoadError.SUCCESS) {
			error = "Failed to load resource for reference rewrite: %s".printf(path);
			return -1;
		}

		if (!rewrite_database_references(db, old_path, new_path, old_type, old_name, new_name))
			return 0;

		if (db.save(_project.absolute_path(path), id) != 0) {
			error = "Failed to save rewritten references in %s".printf(path);
			return -1;
		}

		return 0;
	}

	public int rewrite_references(out string error
		, Hashtable preview
		, string from
		, string to
		)
	{
		error = "";
		string? old_type = ResourceId.type(from);
		string? old_name = ResourceId.name(from);
		string? new_type = ResourceId.type(to);
		string? new_name = ResourceId.name(to);

		if (old_type != new_type) {
			error = "Move requires matching file extensions";
			return -1;
		}

		Gee.ArrayList<Value?> rewrite = (Gee.ArrayList<Value?>)preview["rewrite"];
		foreach (Value? item_value in rewrite) {
			Hashtable item = (Hashtable)item_value;
			if ((string)item["from"] != from || (string)item["to"] != to)
				continue;

			string path = (string)item["path"];
			if (rewrite_references_in_path(out error, path, from, to, old_type, old_name, new_name) != 0)
				return -1;
		}

		return 0;
	}

	public int make_target_directories(out string error, string[] paths)
	{
		error = "";
		var dirs = new Gee.HashSet<string>();
		foreach (string path in paths) {
			string dir = ResourceId.parent_folder(path);
			if (dir == "" || !dirs.add(dir))
				continue;

			string absolute_dir = _project.absolute_path(dir);
			try {
				GLib.File.new_for_path(absolute_dir).make_directory_with_parents();
			} catch (GLib.IOError.EXISTS e) {
				if (!GLib.FileUtils.test(absolute_dir, FileTest.IS_DIR)) {
					error = "%s exists and is not a directory".printf(dir);
					return -1;
				}
			} catch (GLib.Error e) {
				error = "Failed to create directory %s: %s".printf(dir, e.message);
				return -1;
			}
		}

		return 0;
	}

	public int prepare(out string error
		, out string[] prune_dirs
		, Hashtable preview
		, string[] checked_from
		, string[] checked_to
		)
	{
		error = "";
		prune_dirs = {};
		if (make_target_directories(out error, checked_to) != 0)
			return -1;

		for (int i = 0; i < checked_from.length; ++i) {
			if (rewrite_references(out error, preview, checked_from[i], checked_to[i]) != 0)
				return -1;
		}

		var dirs = new Gee.ArrayList<string>();
		Gee.ArrayList<Value?> preview_prune_dirs = (Gee.ArrayList<Value?>)preview["prune_dirs"];
		foreach (Value? dir in preview_prune_dirs)
			dirs.add((string)dir);

		prune_dirs = dirs.to_array();
		return 0;
	}
}

} /* namespace Crown */
