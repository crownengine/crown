/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "allocator.h"
#include "file.h"
#include "filesystem.h"
#include "string_utils.h"
#include "json_parser.h"
#include "vector.h"
#include "log.h"
#include "matrix4x4.h"
#include "physics_types.h"
#include "quaternion.h"
#include "compile_options.h"
#include "temp_allocator.h"
#include "types.h"
#include "vector3.h"
#include "camera.h"
#include "unit_resource.h"

namespace crown
{
namespace unit_resource
{
	struct Projection
	{
		const char* name;
		ProjectionType::Enum type;
	};

	static const Projection s_projection[] =
	{
		{ "perspective",  ProjectionType::PERSPECTIVE  },
		{ "orthographic", ProjectionType::ORTHOGRAPHIC }
	};

	static ProjectionType::Enum projection_name_to_enum(const char* name)
	{
		for (uint32_t i = 0; i < ProjectionType::COUNT; i++)
		{
			if (strcmp(name, s_projection[i].name) == 0)
				return s_projection[i].type;
		}

		CE_FATAL("Bad projection type");
		return (ProjectionType::Enum)0;
	}

	const StringId32 NO_PARENT(0xFFFFFFFF);

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

	void parse_nodes(JSONElement e, Array<GraphNode>& nodes, Array<GraphNodeDepth>& node_depths)
	{
		Vector<DynamicString> keys(default_allocator());
		e.to_keys(keys);

		for (uint32_t k = 0; k < vector::size(keys); k++)
		{
			const char* node_name = keys[k].c_str();
			JSONElement node = e.key(node_name);

			GraphNode gn;
			gn.name = StringId32(node_name);
			gn.parent = NO_PARENT;

			if (!node.key("parent").is_nil())
			{
				DynamicString parent_name;
				node.key("parent").to_string(parent_name);
				gn.parent = parent_name.to_string_id();
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

			DynamicString node_name;
			node.to_string(node_name);
			DynamicString camera_type;
			type.to_string(camera_type);

			StringId32 node_name_hash = node_name.to_string_id();

			UnitCamera cn;
			cn.name = StringId32(camera_name);
			cn.node = find_node_index(node_name_hash, node_depths);
			cn.type = projection_name_to_enum(camera_type.c_str());
			cn.fov =  camera.key_or_nil("fov").to_float(16.0f / 9.0f);
			cn.near = camera.key_or_nil("near_clip_distance").to_float(0.01f);
			cn.far =  camera.key_or_nil("far_clip_distance").to_float(1000.0f);

			array::push_back(cameras, cn);
		}
	}

	void parse_renderables(JSONElement e, Array<UnitRenderable>& renderables, const Array<GraphNodeDepth>& node_depths)
	{
		Vector<DynamicString> keys(default_allocator());
		e.to_keys(keys);

		for (uint32_t k = 0; k < vector::size(keys); k++)
		{
			const char* renderable_name = keys[k].c_str();
			JSONElement renderable = e.key(renderable_name);

			DynamicString node_name; renderable.key("node").to_string(node_name);
			StringId32 node_name_hash = node_name.to_string_id();

			UnitRenderable rn;
			rn.name = StringId32(renderable_name);
			rn.node = find_node_index(node_name_hash, node_depths);
			rn.visible = renderable.key("visible").to_bool();

			DynamicString res_type;
			renderable.key("type").to_string(res_type);

			if (res_type == "mesh")
			{
				rn.type = UnitRenderable::MESH;
				rn.resource = renderable.key("resource").to_resource_id();
			}
			else if (res_type == "sprite")
			{
				rn.type = UnitRenderable::SPRITE;
				rn.resource = renderable.key("resource").to_resource_id();
			}
			else
			{
				CE_ASSERT(false, "Oops, unknown renderable type: '%s'", res_type.c_str());
			}

			array::push_back(renderables, rn);
		}
	}

	void parse_materials(JSONElement e, Array<UnitMaterial>& materials)
	{
		for (uint32_t i = 0; i < e.size(); i++)
		{
			ResourceId mat_id = e[i].to_resource_id();
			UnitMaterial um;
			um.id = mat_id;
			array::push_back(materials, um);
		}
	}

	void compile(const char* path, CompileOptions& opts)
	{
		static const uint32_t VERSION = 1;

		Buffer buf = opts.read(path);
		JSONParser json(buf);
		JSONElement root = json.root();

		ResourceId				m_physics_resource;
		Array<GraphNode>		m_nodes(default_allocator());
		Array<GraphNodeDepth>	m_node_depths(default_allocator());
		Array<UnitCamera>		m_cameras(default_allocator());
		Array<UnitRenderable>	m_renderables(default_allocator());
		Array<UnitMaterial>		m_materials(default_allocator());

		// Check for nodes
		if (root.has_key("nodes")) parse_nodes(root.key("nodes"), m_nodes, m_node_depths);

		for (uint32_t i = 0; i < array::size(m_nodes); i++)
		{
			m_node_depths[i].depth = compute_link_depth(m_nodes[i], m_nodes);
		}

		std::sort(array::begin(m_node_depths), array::end(m_node_depths), GraphNodeDepth());

		if (root.has_key("renderables")) parse_renderables(root.key("renderables"), m_renderables, m_node_depths);
		if (root.has_key("cameras")) parse_cameras(root.key("cameras"), m_cameras, m_node_depths);
		if (root.has_key("materials")) parse_materials(root.key("materials"), m_materials);

		// Check if the unit has a .physics resource
		DynamicString unit_name(path);
		unit_name.strip_trailing(".unit");
		DynamicString physics_name = unit_name;
		physics_name += ".physics";
		if (opts._fs.exists(physics_name.c_str()))
		{
			m_physics_resource = ResourceId(unit_name.c_str());
		}
		else
		{
			m_physics_resource = ResourceId();
		}

		ResourceId sprite_anim;
		if (root.has_key("sprite_animation"))
			sprite_anim = root.key("sprite_animation").to_resource_id();

		UnitResource ur;
		ur.version = VERSION;
		ur.name = StringId64(unit_name.c_str());
		ur.physics_resource = m_physics_resource;
		ur.sprite_animation = sprite_anim;
		ur.num_renderables = array::size(m_renderables);
		ur.num_materials = array::size(m_materials);
		ur.num_cameras = array::size(m_cameras);
		ur.num_scene_graph_nodes = array::size(m_nodes);

		uint32_t offt = sizeof(UnitResource);
		ur.renderables_offset         = offt; offt += sizeof(UnitRenderable) * ur.num_renderables;
		ur.materials_offset           = offt; offt += sizeof(UnitMaterial) * ur.num_materials;
		ur.cameras_offset             = offt; offt += sizeof(UnitCamera) * ur.num_cameras;
		ur.scene_graph_nodes_offset   = offt;

		opts.write(ur.version);
		opts.write(ur._pad);
		opts.write(ur.name);
		opts.write(ur.physics_resource);
		opts.write(ur.sprite_animation);
		opts.write(ur.num_renderables);
		opts.write(ur.renderables_offset);
		opts.write(ur.num_materials);
		opts.write(ur.materials_offset);
		opts.write(ur.num_cameras);
		opts.write(ur.cameras_offset);
		opts.write(ur.num_scene_graph_nodes);
		opts.write(ur.scene_graph_nodes_offset);

		// Renderables
		for (uint32_t i = 0; i < array::size(m_renderables); i++)
		{
			opts.write(m_renderables[i].type);
			opts.write(m_renderables[i]._pad);
			opts.write(m_renderables[i].resource);
			opts.write(m_renderables[i].name);
			opts.write(m_renderables[i].node);
			opts.write(m_renderables[i].visible);
			opts.write(m_renderables[i]._pad1[0]);
			opts.write(m_renderables[i]._pad1[1]);
			opts.write(m_renderables[i]._pad1[2]);
			opts.write(m_renderables[i]._pad2[0]);
			opts.write(m_renderables[i]._pad2[1]);
			opts.write(m_renderables[i]._pad2[2]);
			opts.write(m_renderables[i]._pad2[3]);
		}

		// Materials
		for (uint32_t i = 0; i < array::size(m_materials); i++)
		{
			opts.write(m_materials[i]);
		}

		// Cameras
		for (uint32_t i = 0; i < array::size(m_cameras); i++)
		{
			opts.write(m_cameras[i].name);
			opts.write(m_cameras[i].node);
			opts.write(m_cameras[i].type);
			opts.write(m_cameras[i].fov);
			opts.write(m_cameras[i].near);
			opts.write(m_cameras[i].far);
		}

		// Write node poses
		for (uint32_t i = 0; i < ur.num_scene_graph_nodes; i++)
		{
			uint32_t node_index = m_node_depths[i].index;
			GraphNode& node = m_nodes[node_index];
			UnitNode un;
			un.name = node.name;
			un.parent = find_node_parent_index(i, m_nodes, m_node_depths);
			un.pose = Matrix4x4(node.rotation, node.position);

			opts.write(un.name);
			opts.write(un.pose);
			opts.write(un.parent);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const size_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		return res;
	}

	void online(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void offline(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	StringId64 sprite_animation(const UnitResource* ur)
	{
		return ur->sprite_animation;
	}

	StringId64 physics_resource(const UnitResource* ur)
	{
		return ur->physics_resource;
	}

	uint32_t num_renderables(const UnitResource* ur)
	{
		return ur->num_renderables;
	}

	const UnitRenderable* get_renderable(const UnitResource* ur, uint32_t i)
	{
		CE_ASSERT(i < num_renderables(ur), "Index out of bounds");

		UnitRenderable* begin = (UnitRenderable*) ((char*)ur + ur->renderables_offset);
		return &begin[i];
	}

	uint32_t num_materials(const UnitResource* ur)
	{
		return ur->num_materials;
	}

	const UnitMaterial* get_material(const UnitResource* ur, uint32_t i)
	{
		CE_ASSERT(i < num_materials(ur), "Index out of bounds");

		UnitMaterial* begin = (UnitMaterial*) ((char*)ur + ur->materials_offset);
		return &begin[i];
	}

	uint32_t num_cameras(const UnitResource* ur)
	{
		return ur->num_cameras;
	}

	const UnitCamera* get_camera(const UnitResource* ur, uint32_t i)
	{
		CE_ASSERT(i < num_cameras(ur), "Index out of bounds");

		UnitCamera* begin = (UnitCamera*) ((char*)ur + ur->cameras_offset);
		return &begin[i];
	}

	uint32_t num_scene_graph_nodes(const UnitResource* ur)
	{
		return ur->num_scene_graph_nodes;
	}

	const UnitNode* scene_graph_nodes(const UnitResource* ur)
	{
		return (UnitNode*) ((char*)ur + ur->scene_graph_nodes_offset);
	}
} // namespace unit_resource
} // namespace crown
