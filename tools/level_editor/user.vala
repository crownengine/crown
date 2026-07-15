/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class User
{
	// Data
	public GLib.HashTable<string, Value?> _data;

	// Signals
	public signal void recent_project_added(string source_dir, string name, string time);
	public signal void recent_project_touched(string source_dir, string time);
	public signal void recent_project_removed(string source_dir);

	public User()
	{
	}

	public void decode(GLib.HashTable<string, Value?> sjson)
	{
		_data = sjson;

		_data.foreach((key, value) => {
				if (key == "recent_projects") {
					var recent_projects = value as GLib.GenericArray<Value?>;
					for (int ii = 0; ii < recent_projects.length; ++ii) {
						GLib.HashTable<string, Value?> rp = recent_projects[ii] as GLib.HashTable<string, Value?>;

						recent_project_added((string)rp["source_dir"]
							, (string)rp["name"]
							, (string)rp["mtime"]
							);
					}
				} else {
					logw("Unknown key: `%s`".printf(key));
				}
			});
	}

	public GLib.HashTable<string, Value?> encode()
	{
		return _data;
	}

	public void load(string path)
	{
		try {
			GLib.HashTable<string, Value?> sjson = SJSON.load_from_path(path);
			decode(sjson);
		} catch (JsonSyntaxError e) {
			loge(e.message);
		}
	}

	public void save(string path)
	{
		try {
			SJSON.save(encode(), path);
		} catch (JsonWriteError e) {
			loge(e.message);
		}
	}

	public void add_or_touch_recent_project(string source_dir, string name)
	{
		GLib.GenericArray<Value?> recent_projects = null;
		GLib.HashTable<string, Value?>? project = null;

		_data.foreach((key, value) => {
				if (key == "recent_projects") {
					recent_projects = value as GLib.GenericArray<Value?>;
					for (int ii = 0; ii < recent_projects.length; ++ii) {
						GLib.HashTable<string, Value?> rp = recent_projects[ii] as GLib.HashTable<string, Value?>;

						if ((string)rp["source_dir"] == source_dir) {
							project = rp;
							break;
						}
					}
				}
			});

		if (recent_projects == null) {
			recent_projects = new GLib.GenericArray<Value?>();
			_data["recent_projects"] = recent_projects;
		}

		string mtime = new GLib.DateTime.now_utc().to_unix().to_string();

		if (project == null) {
			project = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
			project["name"]       = source_dir; // FIXME: store project name somewhere inside project directory
			project["source_dir"] = source_dir;
			project["mtime"]      = mtime;
			recent_projects.add(project);
			recent_project_added(source_dir, source_dir, mtime);
		} else {
			project["mtime"] = mtime;
			recent_project_touched(source_dir, mtime);
		}
	}

	public void remove_recent_project(string source_dir)
	{
		_data.foreach((key, value) => {
				if (key == "recent_projects") {
					var recent_projects = value as GLib.GenericArray<Value?>;
					for (int i = 0; i < recent_projects.length; ++i) {
						GLib.HashTable<string, Value?> rp = recent_projects[i] as GLib.HashTable<string, Value?>;
						if ((string)rp["source_dir"] == source_dir) {
							recent_projects.remove_index(i);
							recent_project_removed(source_dir);
							break;
						}
					}
				}
			});
	}

	public int num_projects()
	{
		int num = 0;

		_data.foreach((key, value) => {
				if (key == "recent_projects") {
					var recent_projects = value as GLib.GenericArray<Value?>;
					if (recent_projects != null)
						num = recent_projects.length;
				}
			});

		return num;
	}
}

} /* namespace Crown */
