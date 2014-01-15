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

static uint32_t shape_type_to_enum(const char* type)
{
	const StringId32 th = hash::murmur2_32(type, string::strlen(type));

	if (string::strcmp("sphere", type) == 0) return PhysicsShapeType::SPHERE;
	else if (string::strcmp("capsule", type) == 0) return PhysicsShapeType::CAPSULE;
	else if (string::strcmp("box", type) == 0) return PhysicsShapeType::BOX;
	else if (string::strcmp("plane", type) == 0) return PhysicsShapeType::PLANE;

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
}

//-----------------------------------------------------------------------------
void parse_actor(JSONElement e, PhysicsActor& actor, List<PhysicsShape>& actor_shapes)
{
	JSONElement name = e.key("name");
	JSONElement node = e.key("node");
	JSONElement shapes = e.key("shapes");

	DynamicString actor_name;
	DynamicString actor_node;
	name.string_value(actor_name);
	node.string_value(actor_node);

	actor.name = hash::murmur2_32(actor_name.c_str(), actor_name.length());
	actor.node = hash::murmur2_32(actor_node.c_str(), actor_node.length());
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
	List<uint32_t> m_shape_index(default_allocator());
	List<PhysicsShape> m_shapes(default_allocator());
	JSONElement actors = root.key_or_nil("actors");
	if (!actors.is_nil())
	{
		for (uint32_t i = 0; i < actors.size(); i++)
		{
			if (m_shapes.size() == 0)
			{
				m_shape_index.push_back(0);
			}
			else
			{
				m_shape_index.push_back(m_shapes.size() - 1);
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

	uint32_t offt = sizeof(PhysicsHeader);
	h.controller_offset = offt; offt += sizeof(PhysicsController) * h.num_controllers;
	h.actors_offset = offt;

	out_file->write((char*) &h, sizeof(PhysicsHeader));

	if (m_has_controller)
	{
		out_file->write((char*) &m_controller, sizeof(PhysicsController));
	}

	if (m_actors.size())
	{
		out_file->write((char*) m_actors.begin(), sizeof(PhysicsActor) * m_actors.size());
	}
}

} // namespace physics_resource
} // namespace crown
