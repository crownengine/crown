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
#include "UnitCompiler.h"
#include "TempAllocator.h"
#include "Log.h"
#include "PhysicsTypes.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "Matrix4x4.h"

namespace crown
{

const StringId32 NO_PARENT = 0xFFFFFFFF;

//-----------------------------------------------------------------------------
UnitCompiler::UnitCompiler()
	: m_nodes(default_allocator())
	, m_node_depths(default_allocator())
	, m_cameras(default_allocator())
	, m_renderables(default_allocator())
	, m_actors(default_allocator())
{
}

//-----------------------------------------------------------------------------
size_t UnitCompiler::compile_impl(Filesystem& fs, const char* resource_path)
{
	File* file = fs.open(resource_path, FOM_READ);

	char file_buf[4096];
	file->read(file_buf, file->size());
	fs.close(file);

	JSONParser json(file_buf);
	JSONElement root = json.root();

	// Check for nodes
	if (root.has_key("nodes"))
	{
		JSONElement nodes = root.key("nodes");
		const uint32_t num_nodes = nodes.size();

		for (uint32_t i = 0; i < num_nodes; i++)
		{
			parse_node(nodes[i]);
		}
	}

	for (uint32_t i = 0; i < m_nodes.size(); i++)
	{
		m_node_depths[i].depth = compute_link_depth(m_nodes[i]);
	}

	std::sort(m_node_depths.begin(), m_node_depths.end(), GraphNodeDepth());

	// Check for renderable
	if (root.has_key("renderables"))
	{
		JSONElement renderables = root.key("renderables");
		uint32_t renderables_size = renderables.size();

		for (uint32_t i = 0; i < renderables_size; i++)
		{
			parse_renderable(renderables[i]);
		}
	}

	// Check for cameras
	if (root.has_key("cameras"))
	{
		JSONElement cameras = root.key("cameras");
		uint32_t num_cameras = cameras.size();

		for (uint32_t i = 0; i < num_cameras; i++)
		{
			parse_camera(cameras[i]);
		}
	}

	// check for actors
	if (root.has_key("actors"))
	{
		JSONElement actors = root.key("actors");
		uint32_t num_actors = actors.size();

		for (uint32_t i = 0; i < num_actors; i++)
		{
			parse_actor(actors[i]);
		}
	}

	return 1;
}

//-----------------------------------------------------------------------------
void UnitCompiler::parse_node(JSONElement e)
{
	JSONElement name = e.key("name");
	JSONElement parent = e.key("parent");
	JSONElement pos = e.key("position");
	JSONElement rot = e.key("rotation");

	GraphNode gn;
	gn.name = hash::murmur2_32(name.string_value(), name.size(), 0);
	gn.parent = parent.is_nil() ? NO_PARENT : hash::murmur2_32(parent.string_value(), parent.size(), 0);
	gn.position = Vector3(pos[0].float_value(), pos[1].float_value(), pos[2].float_value());
	gn.rotation = Quaternion(Vector3(rot[0].float_value(), rot[1].float_value(), rot[2].float_value()), rot[3].float_value());

	GraphNodeDepth gnd;
	gnd.name = gn.name;
	gnd.index = m_nodes.size();
	gnd.depth = 0;

	m_nodes.push_back(gn);
	m_node_depths.push_back(gnd);
}

//-----------------------------------------------------------------------------
void UnitCompiler::parse_camera(JSONElement e)
{
	JSONElement name = e.key("name");
	JSONElement node = e.key("node");

	StringId32 node_name = hash::murmur2_32(node.string_value(), node.size(), 0);

	UnitCamera cn;
	cn.name = hash::murmur2_32(name.string_value(), name.size(), 0);
	cn.node = find_node_index(node_name);

	m_cameras.push_back(cn);
}

//-----------------------------------------------------------------------------
void UnitCompiler::parse_renderable(JSONElement e)
{
	JSONElement name = e.key("name");
	JSONElement node = e.key("node");
	JSONElement type = e.key("type");
	JSONElement res = e.key("resource");
	JSONElement vis = e.key("visible");

	StringId32 node_name = hash::murmur2_32(node.string_value(), node.size(), 0);

	UnitRenderable rn;
	rn.name = hash::murmur2_32(name.string_value(), name.size(), 0);
	rn.node = find_node_index(node_name);
	rn.visible = vis.bool_value();

	const char* res_type = type.string_value();
	DynamicString res_name;

	if (string::strcmp(res_type, "mesh") == 0)
	{
		rn.type = UnitRenderable::MESH;
		res_name += res.string_value();
		res_name += ".mesh";
	}
	else if (string::strcmp(res_type, "sprite") == 0)
	{
		rn.type = UnitRenderable::SPRITE;
		res_name += res.string_value();
		res_name += ".sprite";
	}
	else
	{
		CE_ASSERT(false, "Oops, unknown renderable type: '%s'", res_type);
	}
	rn.resource.id = hash::murmur2_64(res_name.c_str(), string::strlen(res_name.c_str()), 0);

	m_renderables.push_back(rn);
}

//-----------------------------------------------------------------------------
void UnitCompiler::parse_actor(JSONElement e)
{
	JSONElement name = e.key("name");
	JSONElement node = e.key("node");
	JSONElement type = e.key("type");
	JSONElement shape = e.key("shape");
	JSONElement active = e.key("active");

	StringId32 node_name = hash::murmur2_32(node.string_value(), node.size(), 0);

	UnitActor an;
	an.name = hash::murmur2_32(name.string_value(), name.size(), 0);
	an.node = find_node_index(node_name);
	an.type = string::strcmp(type.string_value(), "STATIC") == 0 ? UnitActor::STATIC : UnitActor::DYNAMIC;
	an.shape = string::strcmp(shape.string_value(), "SPHERE") == 0 ? UnitActor::SPHERE :
	 			string::strcmp(shape.string_value(), "BOX") == 0 ? UnitActor::BOX : UnitActor::PLANE;
	an.active = active.bool_value();

	m_actors.push_back(an);
}

//-----------------------------------------------------------------------------
uint32_t UnitCompiler::compute_link_depth(GraphNode& node)
{
	if (node.parent == NO_PARENT) return 0;
	else
	{
		for (uint32_t i = 0; i < m_nodes.size(); i++)
		{
			if (m_nodes[i].name == node.parent)
			{
				return 1 + compute_link_depth(m_nodes[i]);
			}
		}
	}

	CE_FATAL("Node not found");
}

//-----------------------------------------------------------------------------
uint32_t UnitCompiler::find_node_index(StringId32 name)
{
	for (uint32_t i = 0; i < m_node_depths.size(); i++)
	{
		if (m_node_depths[i].name == name)
		{
			return i;
		}
	}

	CE_FATAL("Node not found");
}

//-----------------------------------------------------------------------------
int32_t UnitCompiler::find_node_parent_index(uint32_t node)
{
	StringId32 parent_name = m_nodes[m_node_depths[node].index].parent;

	if (parent_name == NO_PARENT) return -1;
	for (uint32_t i = 0; i < m_node_depths.size(); i++)
	{
		if (parent_name == m_node_depths[i].name)
		{
			return i;
		}
	}

	CE_FATAL("Node not found");
}

//-----------------------------------------------------------------------------
void UnitCompiler::write_impl(File* out_file)
{
	UnitHeader h;
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

	Log::d("num renderables = %d", m_renderables.size());
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
		printf("|%.1f|%.1f|%.1f|%.1f|\n", pose.m[0], pose.m[4], pose.m[8], pose.m[12]);
		printf("|%.1f|%.1f|%.1f|%.1f|\n", pose.m[1], pose.m[5], pose.m[9], pose.m[13]);
		printf("|%.1f|%.1f|%.1f|%.1f|\n", pose.m[2], pose.m[6], pose.m[10], pose.m[14]);
		printf("|%.1f|%.1f|%.1f|%.1f|\n", pose.m[3], pose.m[7], pose.m[11], pose.m[15]);
		out_file->write((char*) pose.to_float_ptr(), sizeof(float) * 16);
	}

	// Write parent hierarchy
	for (uint32_t i = 0; i < h.num_scene_graph_nodes; i++)
	{
		int32_t parent = find_node_parent_index(i);
		out_file->write((char*) &parent, sizeof(int32_t));
	}

	m_nodes.clear();
	m_node_depths.clear();
	m_renderables.clear();
	m_cameras.clear();
	m_actors.clear();
	(void)out_file;
}

} // namespace crown
