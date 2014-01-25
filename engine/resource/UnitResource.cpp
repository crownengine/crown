/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Allocator.h"
#include "File.h"
#include "Filesystem.h"
#include "Hash.h"
#include "JSONParser.h"
#include "List.h"
#include "Log.h"
#include "Matrix4x4.h"
#include "PhysicsTypes.h"
#include "Quaternion.h"
#include "Resource.h"
#include "TempAllocator.h"
#include "Types.h"
#include "UnitResource.h"
#include "Vector3.h"

namespace crown
{
namespace unit_resource
{

const StringId32 NO_PARENT = 0xFFFFFFFF;

struct GraphNode
{
	StringId32 name;
	StringId32 parent;
	Vector3 position;
	Quaternion rotation;
};

struct GraphNodeDepth
{
	StringId32 name;
	uint32_t index;
	uint32_t depth;

	bool operator()(const GraphNodeDepth& a, const GraphNodeDepth& b)
	{
		return a.depth < b.depth;
	}
};

//-----------------------------------------------------------------------------
uint32_t compute_link_depth(const GraphNode& node, const List<GraphNode>& nodes)
{
	if (node.parent == NO_PARENT) return 0;
	else
	{
		for (uint32_t i = 0; i < nodes.size(); i++)
		{
			if (nodes[i].name == node.parent)
			{
				return 1 + compute_link_depth(nodes[i], nodes);
			}
		}
	}

	CE_FATAL("Node not found");
}

//-----------------------------------------------------------------------------
uint32_t find_node_index(StringId32 name, const List<GraphNodeDepth>& node_depths)
{
	for (uint32_t i = 0; i < node_depths.size(); i++)
	{
		if (node_depths[i].name == name)
		{
			return i;
		}
	}

	CE_FATAL("Node not found");
	return 0;
}

//-----------------------------------------------------------------------------
int32_t find_node_parent_index(uint32_t node, const List<GraphNode>& nodes, const List<GraphNodeDepth>& node_depths)
{
	StringId32 parent_name = nodes[node_depths[node].index].parent;

	if (parent_name == NO_PARENT) return -1;
	for (uint32_t i = 0; i < node_depths.size(); i++)
	{
		if (parent_name == node_depths[i].name)
		{
			return i;
		}
	}

	CE_FATAL("Node not found");
	return 0;
}

//-----------------------------------------------------------------------------
void parse_nodes(JSONElement e, List<GraphNode>& nodes, List<GraphNodeDepth>& node_depths)
{
	Vector<DynamicString> keys(default_allocator());
	e.key_value(keys);

	for (uint32_t k = 0; k < keys.size(); k++)
	{
		const char* node_name = keys[k].c_str();
		JSONElement node = e.key(node_name);

		GraphNode gn;
		gn.name = hash::murmur2_32(node_name, string::strlen(node_name));
		gn.parent = NO_PARENT;

		if (!node.key("parent").is_nil())
		{
			DynamicString parent_name;
			node.key("parent").string_value(parent_name);
			gn.parent = hash::murmur2_32(parent_name.c_str(), parent_name.length(), 0);
		}

		JSONElement pos = node.key("position");
		JSONElement rot = node.key("rotation");
		gn.position = Vector3(pos[0].float_value(), pos[1].float_value(), pos[2].float_value());
		gn.rotation = Quaternion(Vector3(rot[0].float_value(), rot[1].float_value(), rot[2].float_value()), rot[3].float_value());

		GraphNodeDepth gnd;
		gnd.name = gn.name;
		gnd.index = nodes.size();
		gnd.depth = 0;

		nodes.push_back(gn);
		node_depths.push_back(gnd);
	}
}

//-----------------------------------------------------------------------------
void parse_cameras(JSONElement e, List<UnitCamera>& cameras, const List<GraphNodeDepth>& node_depths)
{
	Vector<DynamicString> keys(default_allocator());
	e.key_value(keys);

	for (uint32_t k = 0; k < keys.size(); k++)
	{
		const char* camera_name = keys[k].c_str();
		JSONElement camera = e.key(camera_name);

		DynamicString node_name;
		camera.key("node").string_value(node_name);

		StringId32 node_name_hash = hash::murmur2_32(node_name.c_str(), node_name.length());

		UnitCamera cn;
		cn.name = hash::murmur2_32(camera_name, string::strlen(camera_name));
		cn.node = find_node_index(node_name_hash, node_depths);

		cameras.push_back(cn);
	}
}

//-----------------------------------------------------------------------------
void parse_renderables(JSONElement e, List<UnitRenderable>& renderables, const List<GraphNodeDepth>& node_depths)
{
	Vector<DynamicString> keys(default_allocator());
	e.key_value(keys);

	for (uint32_t k = 0; k < keys.size(); k++)
	{
		const char* renderable_name = keys[k].c_str();
		JSONElement renderable = e.key(renderable_name);

		DynamicString node_name; renderable.key("node").string_value(node_name);
		StringId32 node_name_hash = hash::murmur2_32(node_name.c_str(), node_name.length(), 0);

		UnitRenderable rn;
		rn.name = hash::murmur2_32(renderable_name, string::strlen(renderable_name), 0);
		rn.node = find_node_index(node_name_hash, node_depths);
		rn.visible = renderable.key("visible").bool_value();

		DynamicString res_type; renderable.key("type").string_value(res_type);
		DynamicString resource_name; renderable.key("resource").string_value(resource_name);
		DynamicString res_name;

		if (res_type == "mesh")
		{
			rn.type = UnitRenderable::MESH;
			res_name += resource_name;
			res_name += ".mesh";
		}
		else if (res_type == "sprite")
		{
			rn.type = UnitRenderable::SPRITE;
			res_name += resource_name;
			res_name += ".sprite";
		}
		else
		{
			CE_ASSERT(false, "Oops, unknown renderable type: '%s'", res_type.c_str());
		}
		rn.resource.id = hash::murmur2_64(res_name.c_str(), res_name.length(), 0);

		renderables.push_back(rn);
	}
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);

	char file_buf[4096];
	file->read(file_buf, file->size());
	fs.close(file);

	JSONParser json(file_buf);
	JSONElement root = json.root();

	ResourceId				m_physics_resource;
	ResourceId				m_material_resource;
	List<GraphNode>			m_nodes(default_allocator());
	List<GraphNodeDepth>	m_node_depths(default_allocator());
	List<UnitCamera>		m_cameras(default_allocator());
	List<UnitRenderable>	m_renderables(default_allocator());

	// Check for nodes
	if (root.has_key("nodes")) parse_nodes(root.key("nodes"), m_nodes, m_node_depths);

	for (uint32_t i = 0; i < m_nodes.size(); i++)
	{
		m_node_depths[i].depth = compute_link_depth(m_nodes[i], m_nodes);
	}

	std::sort(m_node_depths.begin(), m_node_depths.end(), GraphNodeDepth());

	if (root.has_key("renderables")) parse_renderables(root.key("renderables"), m_renderables, m_node_depths);
	if (root.has_key("cameras")) parse_cameras(root.key("cameras"), m_cameras, m_node_depths);

	// Check if the unit has a .physics resource
	DynamicString unit_name(resource_path);
	unit_name.strip_trailing("unit");
	DynamicString physics_name = unit_name;
	physics_name += "physics";
	if (fs.is_file(physics_name.c_str()))
	{
		m_physics_resource.id = hash::murmur2_64(physics_name.c_str(), string::strlen(physics_name.c_str()), 0);
	}
	else
	{
		m_physics_resource.id = 0;
	}

	// Check if the unit has a .material resource
	DynamicString material_name = unit_name;
	material_name += "material";
	if (fs.is_file(material_name.c_str()))
	{
		m_material_resource.id = hash::murmur2_64(material_name.c_str(), string::strlen(material_name.c_str()), 0);
	}
	else
	{
		m_material_resource.id = 0;
	}


	UnitHeader h;
	h.physics_resource = m_physics_resource;
	h.material_resource = m_material_resource;
	h.num_renderables = m_renderables.size();
	h.num_cameras = m_cameras.size();
	h.num_scene_graph_nodes = m_nodes.size();

	uint32_t offt = sizeof(UnitHeader);
	h.renderables_offset         = offt; offt += sizeof(UnitRenderable) * h.num_renderables;
	h.cameras_offset             = offt; offt += sizeof(UnitCamera) * h.num_cameras;
	h.scene_graph_names_offset   = offt; offt += sizeof(StringId32) * h.num_scene_graph_nodes;
	h.scene_graph_poses_offset   = offt; offt += sizeof(Matrix4x4) * h.num_scene_graph_nodes;
	h.scene_graph_parents_offset = offt; offt += sizeof(int32_t) * h.num_scene_graph_nodes;

	// Write header
	out_file->write((char*) &h, sizeof(UnitHeader));

	// Write renderables
	if (m_renderables.size())
		out_file->write((char*) m_renderables.begin(), sizeof(UnitRenderable) * h.num_renderables);

	// Write cameras
	if (m_cameras.size())
		out_file->write((char*) m_cameras.begin(), sizeof(UnitCamera) * h.num_cameras);

	// Write node names
	for (uint32_t i = 0; i < h.num_scene_graph_nodes; i++)
	{
		StringId32 name = m_node_depths[i].name;
		out_file->write((char*) &name, sizeof(StringId32));
	}

	// Write node poses
	for (uint32_t i = 0; i < h.num_scene_graph_nodes; i++)
	{
		uint32_t node_index = m_node_depths[i].index;
		GraphNode& node = m_nodes[node_index];
		Matrix4x4 pose(node.rotation, node.position);
		out_file->write((char*) pose.to_float_ptr(), sizeof(float) * 16);
	}

	// Write parent hierarchy
	for (uint32_t i = 0; i < h.num_scene_graph_nodes; i++)
	{
		int32_t parent = find_node_parent_index(i, m_nodes, m_node_depths);
		out_file->write((char*) &parent, sizeof(int32_t));
	}
}

} // namespace unit_resource
} // namespace crown
