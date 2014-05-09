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
#include "StringUtils.h"
#include "JSONParser.h"
#include "Vector.h"
#include "Log.h"
#include "Matrix4x4.h"
#include "PhysicsTypes.h"
#include "Quaternion.h"
#include "Resource.h"
#include "TempAllocator.h"
#include "Types.h"
#include "Vector3.h"
#include "Camera.h"
#include "UnitResource.h"

namespace crown
{
namespace unit_resource
{

static ProjectionType::Enum projection_name_to_enum(const char* name)
{
	if (string::strcmp(name, "perspective") == 0) return ProjectionType::PERSPECTIVE;
	else if (string::strcmp(name, "orthographic") == 0) return ProjectionType::ORTHOGRAPHIC;

	CE_FATAL("Unknown projection type");
	return (ProjectionType::Enum) 0;
}

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
uint32_t compute_link_depth(const GraphNode& node, const Array<GraphNode>& nodes)
{
	if (node.parent == NO_PARENT) return 0;
	else
	{
		for (uint32_t i = 0; i < array::size(nodes); i++)
		{
			if (nodes[i].name == node.parent)
			{
				return 1 + compute_link_depth(nodes[i], nodes);
			}
		}
	}

	CE_FATAL("Node not found");
	return 0;
}

//-----------------------------------------------------------------------------
uint32_t find_node_index(StringId32 name, const Array<GraphNodeDepth>& node_depths)
{
	for (uint32_t i = 0; i < array::size(node_depths); i++)
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
int32_t find_node_parent_index(uint32_t node, const Array<GraphNode>& nodes, const Array<GraphNodeDepth>& node_depths)
{
	StringId32 parent_name = nodes[node_depths[node].index].parent;

	if (parent_name == NO_PARENT) return -1;
	for (uint32_t i = 0; i < array::size(node_depths); i++)
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
void parse_nodes(JSONElement e, Array<GraphNode>& nodes, Array<GraphNodeDepth>& node_depths)
{
	Vector<DynamicString> keys(default_allocator());
	e.to_keys(keys);

	for (uint32_t k = 0; k < vector::size(keys); k++)
	{
		const char* node_name = keys[k].c_str();
		JSONElement node = e.key(node_name);

		GraphNode gn;
		gn.name = string::murmur2_32(node_name, string::strlen(node_name));
		gn.parent = NO_PARENT;

		if (!node.key("parent").is_nil())
		{
			DynamicString parent_name;
			node.key("parent").to_string(parent_name);
			gn.parent = string::murmur2_32(parent_name.c_str(), parent_name.length(), 0);
		}

		JSONElement pos = node.key("position");
		JSONElement rot = node.key("rotation");
		gn.position = Vector3(pos[0].to_float(), pos[1].to_float(), pos[2].to_float());
		gn.rotation = Quaternion(Vector3(rot[0].to_float(), rot[1].to_float(), rot[2].to_float()), rot[3].to_float());

		GraphNodeDepth gnd;
		gnd.name = gn.name;
		gnd.index = array::size(nodes);
		gnd.depth = 0;

		array::push_back(nodes, gn);
		array::push_back(node_depths, gnd);
	}
}

//-----------------------------------------------------------------------------
void parse_cameras(JSONElement e, Array<UnitCamera>& cameras, const Array<GraphNodeDepth>& node_depths)
{
	Vector<DynamicString> keys(default_allocator());
	e.to_keys(keys);

	for (uint32_t k = 0; k < vector::size(keys); k++)
	{
		const char* camera_name = keys[k].c_str();
		JSONElement camera = e.key(camera_name);
		JSONElement node = camera.key("node");
		JSONElement type = camera.key("type");
		JSONElement fov = camera.key_or_nil("fov");
		JSONElement near = camera.key_or_nil("near_clip_distance");
		JSONElement far = camera.key_or_nil("far_clip_distance");

		DynamicString node_name;
		node.to_string(node_name);
		DynamicString camera_type;
		type.to_string(camera_type);

		StringId32 node_name_hash = string::murmur2_32(node_name.c_str(), node_name.length());

		UnitCamera cn;
		cn.name = string::murmur2_32(camera_name, string::strlen(camera_name));
		cn.node = find_node_index(node_name_hash, node_depths);
		cn.type = projection_name_to_enum(camera_type.c_str());
		cn.fov = fov.is_nil() ? 16.0 / 9.0 : fov.to_float();
		cn.near = near.is_nil() ? 0.01 : near.to_float();
		cn.far = far.is_nil() ? 1000 : far.to_float();

		array::push_back(cameras, cn);
	}
}

//-----------------------------------------------------------------------------
void parse_renderables(JSONElement e, Array<UnitRenderable>& renderables, const Array<GraphNodeDepth>& node_depths)
{
	Vector<DynamicString> keys(default_allocator());
	e.to_keys(keys);

	for (uint32_t k = 0; k < vector::size(keys); k++)
	{
		const char* renderable_name = keys[k].c_str();
		JSONElement renderable = e.key(renderable_name);

		DynamicString node_name; renderable.key("node").to_string(node_name);
		StringId32 node_name_hash = string::murmur2_32(node_name.c_str(), node_name.length(), 0);

		UnitRenderable rn;
		rn.name = string::murmur2_32(renderable_name, string::strlen(renderable_name), 0);
		rn.node = find_node_index(node_name_hash, node_depths);
		rn.visible = renderable.key("visible").to_bool();

		DynamicString res_type; renderable.key("type").to_string(res_type);
		DynamicString resource_name; renderable.key("resource").to_string(resource_name);
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
		rn.resource = ResourceId(res_name.c_str());

		array::push_back(renderables, rn);
	}
}

//-----------------------------------------------------------------------------
void parse_keys(JSONElement e, Array<Key>& generic_keys, Array<char>& values)
{
	Vector<DynamicString> keys(default_allocator());
	e.to_keys(keys);

	for (uint32_t k = 0; k < vector::size(keys); k++)
	{
		const char* key = keys[k].c_str();
		JSONElement value = e.key(key);

		Key out_key;
		out_key.name = string::murmur2_32(key, string::strlen(key));
		out_key.offset = array::size(values);

		if (value.is_bool()) out_key.type = ValueType::BOOL;
		else if (value.is_number()) out_key.type = ValueType::FLOAT;
		else if (value.is_string()) out_key.type = ValueType::STRING;
		else if (value.is_array() && value.size() == 3) out_key.type = ValueType::VECTOR3;
		else CE_FATAL("Value type not supported");

		array::push_back(generic_keys, out_key);

		switch (out_key.type)
		{
			case ValueType::BOOL:
			{
				uint32_t val = value.to_bool();
				array::push(values, (char*) &val, sizeof(uint32_t));
				break;
			}
			case ValueType::FLOAT:
			{
				float val = value.to_float();
				array::push(values, (char*) &val, sizeof(float));
				break;
			}
			case ValueType::STRING:
			{
				DynamicString val;
				value.to_string(val);
				StringId32 val_hash = string::murmur2_32(val.c_str(), val.length());
				array::push(values, (char*) &val_hash, sizeof(StringId32));
				break;
			}
			case ValueType::VECTOR3:
			{
				float val[3];
				val[0] = value[0].to_float();
				val[1] = value[1].to_float();
				val[2] = value[2].to_float();
				array::push(values, (char*) val, sizeof(float) * 3);
				break;
			}
			default:
			{
				CE_FATAL("Oops, you should not be here");
				return;
			}
		}
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
	Array<GraphNode>		m_nodes(default_allocator());
	Array<GraphNodeDepth>	m_node_depths(default_allocator());
	Array<UnitCamera>		m_cameras(default_allocator());
	Array<UnitRenderable>	m_renderables(default_allocator());
	Array<Key>				m_keys(default_allocator());
	Array<char>				m_values(default_allocator());

	// Check for nodes
	if (root.has_key("nodes")) parse_nodes(root.key("nodes"), m_nodes, m_node_depths);

	for (uint32_t i = 0; i < array::size(m_nodes); i++)
	{
		m_node_depths[i].depth = compute_link_depth(m_nodes[i], m_nodes);
	}

	std::sort(array::begin(m_node_depths), array::end(m_node_depths), GraphNodeDepth());

	if (root.has_key("renderables")) parse_renderables(root.key("renderables"), m_renderables, m_node_depths);
	if (root.has_key("cameras")) parse_cameras(root.key("cameras"), m_cameras, m_node_depths);
	if (root.has_key("keys")) parse_keys(root.key("keys"), m_keys, m_values);

	// Check if the unit has a .physics resource
	DynamicString unit_name(resource_path);
	unit_name.strip_trailing("unit");
	DynamicString physics_name = unit_name;
	physics_name += "physics";
	if (fs.is_file(physics_name.c_str()))
	{
		m_physics_resource = ResourceId(physics_name.c_str());
	}
	else
	{
		m_physics_resource = ResourceId();
	}

	// Check if the unit has a .material resource
	DynamicString material_name = unit_name;
	material_name += "material";
	if (fs.is_file(material_name.c_str()))
	{
		m_material_resource = ResourceId(material_name.c_str());
	}
	else
	{
		m_material_resource = ResourceId();
	}


	UnitHeader h;
	h.physics_resource = m_physics_resource;
	h.material_resource = m_material_resource;
	h.num_renderables = array::size(m_renderables);
	h.num_cameras = array::size(m_cameras);
	h.num_scene_graph_nodes = array::size(m_nodes);
	h.num_keys = array::size(m_keys);
	h.values_size = array::size(m_values);

	uint32_t offt = sizeof(UnitHeader);
	h.renderables_offset         = offt; offt += sizeof(UnitRenderable) * h.num_renderables;
	h.cameras_offset             = offt; offt += sizeof(UnitCamera) * h.num_cameras;
	h.scene_graph_nodes_offset   = offt; offt += sizeof(UnitNode) * h.num_scene_graph_nodes;
	h.keys_offset                = offt; offt += sizeof(Key) * h.num_keys;
	h.values_offset              = offt;

	// Write header
	out_file->write((char*) &h, sizeof(UnitHeader));

	// Write renderables
	if (array::size(m_renderables))
		out_file->write((char*) array::begin(m_renderables), sizeof(UnitRenderable) * h.num_renderables);

	// Write cameras
	if (array::size(m_cameras))
		out_file->write((char*) array::begin(m_cameras), sizeof(UnitCamera) * h.num_cameras);

	// Write node poses
	for (uint32_t i = 0; i < h.num_scene_graph_nodes; i++)
	{
		uint32_t node_index = m_node_depths[i].index;
		GraphNode& node = m_nodes[node_index];
		UnitNode un;
		un.name = node.name;
		un.parent = find_node_parent_index(i, m_nodes, m_node_depths);
		un.pose = Matrix4x4(node.rotation, node.position);
		out_file->write((char*) &un, sizeof(UnitNode));
	}

	// Write key/values
	if (array::size(m_keys))
	{
		out_file->write((char*) array::begin(m_keys), sizeof(Key) * h.num_keys);
		out_file->write((char*) array::begin(m_values), array::size(m_values));
	}
}

} // namespace unit_resource
} // namespace crown
