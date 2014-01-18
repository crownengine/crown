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
#include "Filesystem.h"
#include "Hash.h"
#include "JSONParser.h"
#include "PhysicsResource.h"
#include "StringUtils.h"
#include "DynamicString.h"

namespace crown
{
namespace physics_resource
{

static uint32_t actor_type_to_enum(const char* type)
{
	if (string::strcmp("static", type) == 0) return PhysicsActorType::STATIC;
	else if (string::strcmp("dynamic_physical", type) == 0) return PhysicsActorType::DYNAMIC_PHYSICAL;
	else if (string::strcmp("dynamic_kinematic", type) == 0) return PhysicsActorType::DYNAMIC_KINEMATIC;

	CE_FATAL("Bad actor type");
}

static uint32_t actor_group_to_enum(const char* group)
{
	if (string::strcmp("GROUP_0", group) == 0) 	return PhysicsActorGroup::GROUP_0;
	if (string::strcmp("GROUP_1", group) == 0) 	return PhysicsActorGroup::GROUP_1;
	if (string::strcmp("GROUP_2", group) == 0) 	return PhysicsActorGroup::GROUP_2;
	if (string::strcmp("GROUP_3", group) == 0) 	return PhysicsActorGroup::GROUP_3;
	if (string::strcmp("GROUP_4", group) == 0) 	return PhysicsActorGroup::GROUP_4;
	if (string::strcmp("GROUP_5", group) == 0) 	return PhysicsActorGroup::GROUP_5;
	if (string::strcmp("GROUP_6", group) == 0) 	return PhysicsActorGroup::GROUP_6;
	if (string::strcmp("GROUP_7", group) == 0) 	return PhysicsActorGroup::GROUP_7;
	if (string::strcmp("GROUP_8", group) == 0) 	return PhysicsActorGroup::GROUP_8;
	if (string::strcmp("GROUP_9", group) == 0) 	return PhysicsActorGroup::GROUP_9;
	if (string::strcmp("GROUP_10", group) == 0) return PhysicsActorGroup::GROUP_10;
	if (string::strcmp("GROUP_11", group) == 0) return PhysicsActorGroup::GROUP_11;
	if (string::strcmp("GROUP_12", group) == 0) return PhysicsActorGroup::GROUP_12;
	if (string::strcmp("GROUP_13", group) == 0) return PhysicsActorGroup::GROUP_13;
	if (string::strcmp("GROUP_14", group) == 0) return PhysicsActorGroup::GROUP_14;
	if (string::strcmp("GROUP_15", group) == 0) return PhysicsActorGroup::GROUP_15;
	if (string::strcmp("GROUP_16", group) == 0) return PhysicsActorGroup::GROUP_16;
	if (string::strcmp("GROUP_17", group) == 0) return PhysicsActorGroup::GROUP_17;
	if (string::strcmp("GROUP_18", group) == 0) return PhysicsActorGroup::GROUP_18;
	if (string::strcmp("GROUP_19", group) == 0) return PhysicsActorGroup::GROUP_19;
	if (string::strcmp("GROUP_20", group) == 0) return PhysicsActorGroup::GROUP_20;
	if (string::strcmp("GROUP_21", group) == 0) return PhysicsActorGroup::GROUP_21;
	if (string::strcmp("GROUP_22", group) == 0) return PhysicsActorGroup::GROUP_22;
	if (string::strcmp("GROUP_23", group) == 0) return PhysicsActorGroup::GROUP_23;
	if (string::strcmp("GROUP_24", group) == 0) return PhysicsActorGroup::GROUP_24;
	if (string::strcmp("GROUP_25", group) == 0) return PhysicsActorGroup::GROUP_25;
	if (string::strcmp("GROUP_26", group) == 0) return PhysicsActorGroup::GROUP_26;
	if (string::strcmp("GROUP_27", group) == 0) return PhysicsActorGroup::GROUP_27;
	if (string::strcmp("GROUP_28", group) == 0) return PhysicsActorGroup::GROUP_28;
	if (string::strcmp("GROUP_29", group) == 0) return PhysicsActorGroup::GROUP_29;
	if (string::strcmp("GROUP_30", group) == 0) return PhysicsActorGroup::GROUP_30;
	if (string::strcmp("GROUP_31", group) == 0) return PhysicsActorGroup::GROUP_31;

	CE_FATAL("Bad actor group");
}

static uint32_t shape_type_to_enum(const char* type)
{
	if (string::strcmp("sphere", type) == 0) 		return PhysicsShapeType::SPHERE;
	else if (string::strcmp("capsule", type) == 0) 	return PhysicsShapeType::CAPSULE;
	else if (string::strcmp("box", type) == 0) 		return PhysicsShapeType::BOX;
	else if (string::strcmp("plane", type) == 0) 	return PhysicsShapeType::PLANE;

	CE_FATAL("Bad shape type");
}

//-----------------------------------------------------------------------------
void parse_controller(JSONElement e, PhysicsController& controller)
{
	JSONElement name = e.key("name");
	JSONElement height = e.key("height");
	JSONElement radius = e.key("radius");
	JSONElement slope_limit = e.key("slope_limit");
	JSONElement step_offset = e.key("step_offset");
	JSONElement contact_offset = e.key("contact_offset");

	DynamicString contr_name;
	name.string_value(contr_name);
	controller.name = hash::murmur2_32(contr_name.c_str(), contr_name.length());
	controller.height = height.float_value();
	controller.radius = radius.float_value();
	controller.slope_limit = slope_limit.float_value();
	controller.step_offset = step_offset.float_value();
	controller.contact_offset = contact_offset.float_value();
}

//-----------------------------------------------------------------------------
void parse_shape(JSONElement e, PhysicsShape& shape)
{
	JSONElement name = e.key("name");
	JSONElement type = e.key("type");

	DynamicString shape_name;
	DynamicString shape_type;
	name.string_value(shape_name);
	type.string_value(shape_type);

	shape.name = hash::murmur2_32(shape_name.c_str(), shape_name.length());
	shape.type = shape_type_to_enum(shape_type.c_str());

	switch (shape.type)
	{
		case PhysicsShapeType::SPHERE:
		{
			JSONElement radius = e.key("radius");
			shape.data_0 = radius.float_value();
			break;
		}
		case PhysicsShapeType::CAPSULE:
		{
			// TODO
			break;
		}
		case PhysicsShapeType::BOX:
		{
			JSONElement half_x = e.key("half_x");
			JSONElement half_y = e.key("half_y");
			JSONElement half_z = e.key("half_z");

			shape.data_0 = half_x.float_value();
			shape.data_1 = half_y.float_value();
			shape.data_2 = half_z.float_value();

			break;
		}
		case PhysicsShapeType::PLANE:
		{
			// TODO
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void parse_actor(JSONElement e, PhysicsActor& actor, List<PhysicsShape>& actor_shapes)
{
	JSONElement name = e.key("name");
	JSONElement node = e.key("node");
	JSONElement type = e.key("type");
	JSONElement material = e.key("material");
	JSONElement group = e.key("group");
	JSONElement shapes = e.key("shapes");

	DynamicString actor_name;
	DynamicString actor_node;
	DynamicString actor_type;
	DynamicString actor_group;
	List<float> actor_material(default_allocator());
	name.string_value(actor_name);
	node.string_value(actor_node);
	type.string_value(actor_type);
	material.array_value(actor_material);
	group.string_value(actor_group);

	actor.name = hash::murmur2_32(actor_name.c_str(), actor_name.length());
	actor.node = hash::murmur2_32(actor_node.c_str(), actor_node.length());
	actor.type = actor_type_to_enum(actor_type.c_str());
	Log::i("s:%f d:%f r: %f", actor_material[0], actor_material[1], actor_material[2]);
	actor.static_friction = actor_material[0];
	actor.dynamic_friction = actor_material[1];
	actor.restitution = actor_material[2];
	actor.group = actor_group_to_enum(actor_group.c_str());

	actor.num_shapes = shapes.size();

	for (uint32_t i = 0; i < actor.num_shapes; i++)
	{
		PhysicsShape ps;
		parse_shape(shapes[i], ps);
		actor_shapes.push_back(ps);
	}
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	JSONParser json(buf);
	JSONElement root = json.root();

	bool m_has_controller = false;
	PhysicsController m_controller;

	// Read controller
	JSONElement controller = root.key_or_nil("controller");
	if (controller.is_nil())
	{
		m_has_controller = false;
	}
	else
	{
		parse_controller(controller, m_controller);
		m_has_controller = true;
	}

	// Read actors
	List<PhysicsActor> m_actors(default_allocator());
	List<uint32_t> m_shapes_indices(default_allocator());
	List<PhysicsShape> m_shapes(default_allocator());
	JSONElement actors = root.key_or_nil("actors");
	if (!actors.is_nil())
	{
		for (uint32_t i = 0; i < actors.size(); i++)
		{
			if (m_shapes.size() == 0)
			{
				m_shapes_indices.push_back(0);
			}
			else
			{
				m_shapes_indices.push_back(m_shapes.size());
			}

			PhysicsActor a;
			parse_actor(actors[i], a, m_shapes);
			m_actors.push_back(a);
		}
	}

	fs.close(file);
	default_allocator().deallocate(buf);

	PhysicsHeader h;
	h.version = 1;
	h.num_controllers = m_has_controller ? 1 : 0;
	h.num_actors = m_actors.size();
	h.num_shapes_indices = m_shapes_indices.size();
	h.num_shapes = m_shapes.size();

	uint32_t offt = sizeof(PhysicsHeader);
	h.controller_offset = offt; offt += sizeof(PhysicsController) * h.num_controllers;
	h.actors_offset = offt; offt += sizeof(PhysicsActor) * h.num_actors;
	h.shapes_indices_offset = offt; offt += sizeof(uint32_t) * h.num_shapes_indices;
	h.shapes_offset = offt;

	out_file->write((char*) &h, sizeof(PhysicsHeader));

	if (m_has_controller)
	{
		out_file->write((char*) &m_controller, sizeof(PhysicsController));
	}

	if (m_actors.size())
	{
		out_file->write((char*) m_actors.begin(), sizeof(PhysicsActor) * m_actors.size());
	}

	if (m_shapes_indices.size())
	{
		out_file->write((char*) m_shapes_indices.begin(), sizeof(uint32_t) * m_shapes_indices.size());
	}

	if (m_shapes.size())
	{
		out_file->write((char*) m_shapes.begin(), sizeof(PhysicsShape) * m_shapes.size());
	}
}

} // namespace physics_resource
} // namespace crown
