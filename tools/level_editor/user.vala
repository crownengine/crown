/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
public class User
{
	// Data
	public Hashtable _data;

	// Signals
	public signal void recent_project_added(string source_dir, string name, string time);
	public signal void recent_project_removed(string source_dir);

	public User()
	{
	}

	public void decode(Hashtable sjson)
	{
		_data = sjson;

		_data.foreach((ee) => {
			if (ee.key == "recent_projects")
			{
				var recent_projects = ee.value as ArrayList<Value?>;
				for (int ii = 0; ii < recent_projects.size; ++ii)
				{
					Hashtable rp = recent_projects[ii] as Hashtable;

					recent_project_added((string)rp["source_dir"]
						, (string)rp["name"]
						, (string)rp["mtime"]
						);
				}
			}
			else
			{
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
		Hashtable sjson = SJSON.load_from_path(path);
		decode(sjson);
	}

	public void save(string path)
	{
		SJSON.save(encode(), path);
	}

	public void add_recent_project(string source_dir, string name)
	{
		Gee.ArrayList<Value?> recent_projects = null;
		bool duplicate = false;

		_data.foreach ((ee) => {
			if (ee.key == "recent_projects")
			{
				recent_projects = ee.value as ArrayList<Value?>;
				for (int ii = 0; ii < recent_projects.size; ++ii)
				{
					Hashtable rp = recent_projects[ii] as Hashtable;

					if ((string)rp["source_dir"] == source_dir)
					{
						duplicate = true;
						return false; // break
					}
				}
			}

			return true;
		});

		if (recent_projects == null)
		{
			recent_projects = new ArrayList<Value?>();
			_data["recent_projects"] = recent_projects;
		}

		if (!duplicate)
		{
			string mtime = new GLib.DateTime.now_utc().to_unix().to_string();
			var new_rp = new Hashtable();
			new_rp["name"]       = source_dir; // FIXME: store project name somewhere inside project directory
			new_rp["source_dir"] = source_dir;
			new_rp["mtime"]      = mtime;
			recent_projects.add(new_rp);

			recent_project_added(source_dir, source_dir, mtime);
		}
	}

	public void remove_recent_project(string source_dir)
	{
		_data.foreach((ee) => {
			if (ee.key == "recent_projects")
			{
				var recent_projects = ee.value as ArrayList<Value?>;
				var it = recent_projects.iterator();
				for (var has_next = it.next(); has_next; has_next = it.next())
				{
					Hashtable rp = it.get() as Hashtable;
					if ((string)rp["source_dir"] == source_dir)
					{
						it.remove();
						recent_project_removed(source_dir);
						return false; // break
					}
				}
			}

			return true;
		});
	}

	public void touch_recent_project(string source_dir, string time)
	{
		_data.foreach((ee) => {
			if (ee.key == "recent_projects")
			{
				var recent_projects = ee.value as ArrayList<Value?>;
				var it = recent_projects.iterator();
				for (var has_next = it.next(); has_next; has_next = it.next())
				{
					Hashtable rp = it.get() as Hashtable;
					if ((string)rp["source_dir"] == source_dir)
					{
						rp["mtime"] = time;
						return false; // break
					}
				}
			}

			return true;
		});
	}
}

}
