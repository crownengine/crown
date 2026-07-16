/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public struct Mesh
{
	public Project _project;
	public Gee.ArrayList<string> _nodes;

	public Mesh(Project project)
	{
		_project = project;
		_nodes = new Gee.ArrayList<string>();
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

	public void decode_node(Hashtable node)
	{
		foreach (var e in node.entries) {
			if (e.key == "matrix_local") {
				// Do nothing.
			} else if (e.key == "geometry") {
				// Do nothing.
			} else if (e.key == "children") {
				decode_nodes((Hashtable)e.value);
			}
		}
	}

	public void decode_nodes(Hashtable nodes)
	{
		foreach (var e in nodes.entries) {
			_nodes.add(e.key);
			decode_node((Hashtable)e.value);
		}
	}

	public void decode(Hashtable mesh)
	{
		if (mesh.has_key("source")) {
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
			foreach (var e in mesh.entries) {
				if (e.key == "nodes") {
					decode_nodes((Hashtable)e.value);
				} else if (e.key == "geometries") {
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
