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
			ufbx.Scene? scene = ufbx.Scene.load_file(_project.absolute_path((string)mesh["source"]), load_opts, ref error);

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
