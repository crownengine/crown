/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class Mesh
{
	public Gee.ArrayList<string> _nodes;

	public Mesh()
	{
		_nodes = new Gee.ArrayList<string>();
	}

	private void decode_node(Hashtable node)
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

	private void decode_nodes(Hashtable nodes)
	{
		foreach (var e in nodes.entries) {
			_nodes.add(e.key);
			decode_node((Hashtable)e.value);
		}
	}

	public void decode(Hashtable mesh)
	{
		foreach (var e in mesh.entries) {
			if (e.key == "nodes") {
				decode_nodes((Hashtable)e.value);
			} else if (e.key == "geometries") {
				// Do nothing.
			}
		}
	}
}

} /* namespace Crown */
