/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class User
{
	// Data
	public Hashtable _data;

	// Signals
	public signal void recent_project_added(string source_dir, string name, string time);
	public signal void recent_project_touched(string source_dir, string time);
	public signal void recent_project_removed(string source_dir);

	public User()
	{
	}

	public void decode(Hashtable sjson)
	{
		_data = sjson;

		_data.foreach((ee) => {
				if (ee.key == "recent_projects") {
					var recent_projects = ee.value as Gee.ArrayList<Value?>;
					for (int ii = 0; ii < recent_projects.size; ++ii) {
						Hashtable rp = recent_projects[ii] as Hashtable;

						recent_project_added((string)rp["source_dir"]
							, (string)rp["name"]
							, (string)rp["mtime"]
							);
					}
				} else {
					logw("Unknown key: `%s`".printf(ee.key));
				}

				return true;
			});
	}

	public Hashtable encode()
	{
		return _data;
	}

	public void load(string path)
	{
		try {
			Hashtable sjson = SJSON.load_from_path(path);
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
		Gee.ArrayList<Value?> recent_projects = null;
		Hashtable? project = null;

		_data.foreach ((ee) => {
				if (ee.key == "recent_projects") {
					recent_projects = ee.value as Gee.ArrayList<Value?>;
					for (int ii = 0; ii < recent_projects.size; ++ii) {
						Hashtable rp = recent_projects[ii] as Hashtable;

						if ((string)rp["source_dir"] == source_dir) {
							project = rp;
							return false; // break
						}
					}
				}

				return true;
			});

		if (recent_projects == null) {
			recent_projects = new Gee.ArrayList<Value?>();
			_data["recent_projects"] = recent_projects;
		}

		string mtime = new GLib.DateTime.now_utc().to_unix().to_string();

		if (project == null) {
			project = new Hashtable();
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
		_data.foreach((ee) => {
				if (ee.key == "recent_projects") {
					var recent_projects = ee.value as Gee.ArrayList<Value?>;
					var it = recent_projects.iterator();
					for (var has_next = it.next(); has_next; has_next = it.next()) {
						Hashtable rp = it.get() as Hashtable;
						if ((string)rp["source_dir"] == source_dir) {
							it.remove();
							recent_project_removed(source_dir);
							return false; // break
						}
					}
				}

				return true;
			});
	}
}

} /* namespace Crown */
