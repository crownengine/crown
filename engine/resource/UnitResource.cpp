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
}

//-----------------------------------------------------------------------------
void parse_node(JSONElement e, List<GraphNode>& nodes, List<GraphNodeDepth>& node_depths)
{
	JSONElement name = e.key("name");
	JSONElement parent = e.key("parent");
	JSONElement pos = e.key("position");
	JSONElement rot = e.key("rotation");

	DynamicString node_name;
	name.string_value(node_name);

	GraphNode gn;
	gn.name = hash::murmur2_32(node_name.c_str(), node_name.length(), 0);

	if (parent.is_nil())
	{
		gn.parent = NO_PARENT;
	}
	else
	{
		DynamicString parent_name;
		parent.string_value(parent_name);
		hash::murmur2_32(parent_name.c_str(), parent_name.length(), 0);
	}

	gn.position = Vector3(pos[0].float_value(), pos[1].float_value(), pos[2].float_value());
	gn.rotation = Quaternion(Vector3(rot[0].float_value(), rot[1].float_value(), rot[2].float_value()), rot[3].float_value());

	GraphNodeDepth gnd;
	gnd.name = gn.name;
	gnd.index = nodes.size();
	gnd.depth = 0;

	nodes.push_back(gn);
	node_depths.push_back(gnd);
}

//-----------------------------------------------------------------------------
void parse_camera(JSONElement e, List<UnitCamera>& cameras, const List<GraphNodeDepth>& node_depths)
{
	JSONElement name = e.key("name");
	JSONElement node = e.key("node");

	DynamicString node_name;
	DynamicString camera_name;
	node.string_value(node_name);
	name.string_value(camera_name);

	StringId32 node_name_hash = hash::murmur2_32(node_name.c_str(), node_name.length(), 0);

	UnitCamera cn;
	cn.name = hash::murmur2_32(camera_name.c_str(), camera_name.length(), 0);
	cn.node = find_node_index(node_name_hash, node_depths);

	cameras.push_back(cn);
}

//-----------------------------------------------------------------------------
void parse_renderable(JSONElement e, List<UnitRenderable>& renderables, const List<GraphNodeDepth>& node_depths)
{
	JSONElement name = e.key("name");
	JSONElement node = e.key("node");
	JSONElement type = e.key("type");
	JSONElement res = e.key("resource");
	JSONElement vis = e.key("visible");

	DynamicString renderable_name;
	DynamicString node_name;
	name.string_value(renderable_name);
	node.string_value(node_name);

	StringId32 node_name_hash = hash::murmur2_32(node_name.c_str(), node_name.length(), 0);

	UnitRenderable rn;
	rn.name = hash::murmur2_32(renderable_name.c_str(), renderable_name.length(), 0);
	rn.node = find_node_index(node_name_hash, node_depths);
	rn.visible = vis.bool_value();

	DynamicString res_type;
	DynamicString resource_name;
	type.string_value(res_type);
	res.string_value(resource_name);
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

//-----------------------------------------------------------------------------
void parse_actor(JSONElement e, List<UnitActor>& actors, const List<GraphNodeDepth>& node_depths)
{
	JSONElement name = e.key("name");
	JSONElement node = e.key("node");
	JSONElement type = e.key("type");
	JSONElement shape = e.key("shape");
	JSONElement active = e.key("active");

	DynamicString actor_name;
	DynamicString node_name;
	DynamicString type_name;
	DynamicString shape_name;
	name.string_value(actor_name);
	node.string_value(node_name);
	type.string_value(type_name);
	shape.string_value(shape_name);

	StringId32 node_name_hash = hash::murmur2_32(node_name.c_str(), node_name.length(), 0);

	UnitActor an;
	an.name = hash::murmur2_32(actor_name.c_str(), actor_name.length(), 0);
	an.node = find_node_index(node_name_hash, node_depths);
	an.type = type_name == "STATIC" ? UnitActor::STATIC : UnitActor::DYNAMIC;
	an.shape = shape_name == "SPHERE" ? UnitActor::SPHERE :
	 			shape_name == "BOX" ? UnitActor::BOX : UnitActor::PLANE;
	an.active = active.bool_value();

	actors.push_back(an);
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
	List<UnitActor>			m_actors(default_allocator());

	// Check for nodes
	if (root.has_key("nodes"))
	{
		JSONElement nodes = root.key("nodes");
		const uint32_t num_nodes = nodes.size();

		for (uint32_t i = 0; i < num_nodes; i++)
		{
			parse_node(nodes[i], m_nodes, m_node_depths);
		}
	}

	for (uint32_t i = 0; i < m_nodes.size(); i++)
	{
		m_node_depths[i].depth = compute_link_depth(m_nodes[i], m_nodes);
	}

	std::sort(m_node_depths.begin(), m_node_depths.end(), GraphNodeDepth());

	// Check for renderable
	if (root.has_key("renderables"))
	{
		JSONElement renderables = root.key("renderables");
		uint32_t renderables_size = renderables.size();

		for (uint32_t i = 0; i < renderables_size; i++)
		{
			parse_renderable(renderables[i], m_renderables, m_node_depths);
		}
	}

	// Check for cameras
	if (root.has_key("cameras"))
	{
		JSONElement cameras = root.key("cameras");
		uint32_t num_cameras = cameras.size();

		for (uint32_t i = 0; i < num_cameras; i++)
		{
			parse_camera(cameras[i], m_cameras, m_node_depths);
		}
	}

	// check for actors
	if (root.has_key("actors"))
	{
		JSONElement actors = root.key("actors");
		uint32_t num_actors = actors.size();

		for (uint32_t i = 0; i < num_actors; i++)
		{
			parse_actor(actors[i], m_actors, m_node_depths);
		}
	}

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
	h.num_actors = m_actors.size();
	h.num_scene_graph_nodes = m_nodes.size();

	uint32_t offt = sizeof(UnitHeader);
	h.renderables_offset         = offt; offt += sizeof(UnitRenderable) * h.num_renderables;
	h.cameras_offset             = offt; offt += sizeof(UnitCamera) * h.num_cameras;
	h.actors_offset              = offt; offt += sizeof(UnitActor) * h.num_actors;
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

	// Write actors
	if (m_actors.size())
		out_file->write((char*) m_actors.begin(), sizeof(UnitActor) * h.num_actors);

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
