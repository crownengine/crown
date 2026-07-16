/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public struct Mesh
{
	public Project _project;
	public GLib.GenericArray<string> _nodes;

	public Mesh(Project project)
	{
		_project = project;
		_nodes = new GLib.GenericArray<string>();
	}

	public static bool parse_lod_name(out string base_name, out int lod_index, string name)
	{
		base_name = "";
		lod_index = -1;

		string name_lower = name.down();
		int lod_pos = name_lower.last_index_of("_lod");
		if (lod_pos == -1)
			return false;

		string index_string = name_lower.substring(lod_pos + 4);
		if (index_string.length == 0)
			return false;
		for (int i = 0; i < index_string.length; ++i) {
			if (!index_string[i].isdigit())
				return false;
		}
		if (!int.try_parse(index_string, out lod_index))
			return false;

		base_name = name.substring(0, lod_pos);
		return true;
	}

	public void decode_node(GLib.HashTable<string, Value?> node)
	{
		GLib.HashTableIter<string, Value?> iter = GLib.HashTableIter<string, Value?>(node);
		unowned string key;
		unowned Value? value;
		while (iter.next(out key, out value)) {
			if (key == "matrix_local") {
				// Do nothing.
			} else if (key == "geometry") {
				// Do nothing.
			} else if (key == "children") {
				decode_nodes((GLib.HashTable<string, Value?>)value);
			}
		}
	}

	public void decode_nodes(GLib.HashTable<string, Value?> nodes)
	{
		GLib.HashTableIter<string, Value?> iter = GLib.HashTableIter<string, Value?>(nodes);
		unowned string key;
		unowned Value? value;
		while (iter.next(out key, out value)) {
			_nodes.add(key);
			decode_node((GLib.HashTable<string, Value?>)value);
		}
	}

	public void decode(GLib.HashTable<string, Value?> mesh)
	{
		if (mesh.contains("source")) {
			ufbx.Error error = {};
			ufbx.LoadOpts load_opts = {};
			load_opts.ignore_all_content = true;
			string source_path = _project.absolute_path((string)mesh["source"]);
			if (source_path.down().has_suffix(".obj"))
				load_opts.file_format = ufbx.FileFormat.OBJ;
			ufbx.Scene? scene = ufbx.Scene.load_file(source_path, load_opts, ref error);
			if (scene == null) {
				loge("ufbx: %s".printf((string)error.description.data));
				return;
			}

			for (size_t i = 0; i < scene.nodes.data.length; ++i) {
				unowned ufbx.Node node = scene.nodes.data[i];
				if (node.mesh != null)
					_nodes.add((string)node.name.data);
			}
		} else {
			GLib.HashTableIter<string, Value?> iter = GLib.HashTableIter<string, Value?>(mesh);
			unowned string key;
			unowned Value? value;
			while (iter.next(out key, out value)) {
				if (key == "nodes") {
					decode_nodes((GLib.HashTable<string, Value?>)value);
				} else if (key == "geometries") {
					// Do nothing.
				}
			}
		}
	}

	public static Mesh load_from_path(Project project, string path) throws JsonSyntaxError
	{
		Mesh mesh = Mesh(project);
		mesh.decode(SJSON.load_from_path(project.absolute_path(path)));
		return mesh;
	}
}

} /* namespace Crown */
