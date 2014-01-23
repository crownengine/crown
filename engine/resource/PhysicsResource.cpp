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

//-----------------------------------------------------------------------------
static uint32_t actor_type_to_enum(const char* type)
{
	if (string::strcmp("static", type) == 0) return PhysicsActorType::STATIC;
	else if (string::strcmp("dynamic_physical", type) == 0) return PhysicsActorType::DYNAMIC_PHYSICAL;
	else if (string::strcmp("dynamic_kinematic", type) == 0) return PhysicsActorType::DYNAMIC_KINEMATIC;

	CE_FATAL("Bad actor type");
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
static uint32_t shape_type_to_enum(const char* type)
{
	if (string::strcmp("sphere", type) == 0) 		return PhysicsShapeType::SPHERE;
	else if (string::strcmp("capsule", type) == 0) 	return PhysicsShapeType::CAPSULE;
	else if (string::strcmp("box", type) == 0) 		return PhysicsShapeType::BOX;
	else if (string::strcmp("plane", type) == 0) 	return PhysicsShapeType::PLANE;

	CE_FATAL("Bad shape type");
}

//-----------------------------------------------------------------------------
static uint32_t joint_type_to_enum(const char* type)
{
	if (string::strcmp("fixed", type) == 0) 			return PhysicsJointType::FIXED;
	else if (string::strcmp("spherical", type) == 0) 	return PhysicsJointType::SPHERICAL;
	else if (string::strcmp("revolute", type) == 0) 	return PhysicsJointType::REVOLUTE;
	else if (string::strcmp("prismatic", type) == 0) 	return PhysicsJointType::PRISMATIC;
	else if (string::strcmp("distance", type) == 0) 	return PhysicsJointType::DISTANCE;
	else if (string::strcmp("d6", type) == 0) 			return PhysicsJointType::D6;	

	CE_FATAL("Bad joint type");
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
	JSONElement trigger = e.key("trigger");

	DynamicString shape_name;
	DynamicString shape_type;
	name.string_value(shape_name);
	type.string_value(shape_type);

	shape.name = hash::murmur2_32(shape_name.c_str(), shape_name.length());
	shape.type = shape_type_to_enum(shape_type.c_str());
	shape.trigger = trigger.bool_value();

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
	JSONElement mask = e.key("mask");
	JSONElement shapes = e.key("shapes");

	DynamicString actor_name;
	DynamicString actor_node;
	DynamicString actor_type;
	List<float> actor_material(default_allocator());
	DynamicString actor_group;
	Vector<DynamicString> actor_mask(default_allocator());
	name.string_value(actor_name);
	node.string_value(actor_node);
	type.string_value(actor_type);
	material.array_value(actor_material);
	group.string_value(actor_group);
	mask.array_value(actor_mask);

	memset(&actor, 0, sizeof(PhysicsActor));
	actor.name = hash::murmur2_32(actor_name.c_str(), actor_name.length());
	actor.node = hash::murmur2_32(actor_node.c_str(), actor_node.length());
	actor.type = actor_type_to_enum(actor_type.c_str());
	actor.static_friction = actor_material[0];
	actor.dynamic_friction = actor_material[1];
	actor.restitution = actor_material[2];
	actor.group = actor_group_to_enum(actor_group.c_str());
	for (uint32_t i = 0; i < actor_mask.size(); i++)
	{
		actor.mask |= actor_group_to_enum(actor_mask[i].c_str());
	}

	actor.num_shapes = shapes.size();

	for (uint32_t i = 0; i < actor.num_shapes; i++)
	{
		PhysicsShape ps;
		parse_shape(shapes[i], ps);
		actor_shapes.push_back(ps);
	}
}

//-----------------------------------------------------------------------------
void parse_joint(JSONElement e, PhysicsJoint& joint)
{
	JSONElement name = e.key("name");
	JSONElement type = e.key("type");
	JSONElement actor_0 = e.key("actor_0");
	JSONElement actor_1 = e.key("actor_1");
	JSONElement anchor_0 = e.key("anchor_0");
	JSONElement anchor_1 = e.key("anchor_1");
	JSONElement restitution = e.key("restitution");
	JSONElement spring = e.key("spring");
	JSONElement damping = e.key("damping");
	JSONElement distance = e.key("distance");
	JSONElement breakable = e.key("breakable");
	JSONElement break_force = e.key("break_force");
	JSONElement break_torque = e.key("break_torque");

	DynamicString joint_name;
	DynamicString joint_type;
	DynamicString joint_actor_0;
	DynamicString joint_actor_1;
	List<float> joint_anchor_0(default_allocator());
	List<float> joint_anchor_1(default_allocator());
	
	name.string_value(joint_name);
	type.string_value(joint_type);
	actor_0.string_value(joint_actor_0);
	actor_1.string_value(joint_actor_1);
	anchor_0.array_value(joint_anchor_0);
	anchor_1.array_value(joint_anchor_1);

	joint.name = hash::murmur2_32(joint_name.c_str(), joint_name.length());
	joint.type = joint_type_to_enum(joint_type.c_str());
	joint.actor_0 = hash::murmur2_32(joint_actor_0.c_str(), joint_actor_0.length());
	joint.actor_1 = hash::murmur2_32(joint_actor_1.c_str(), joint_actor_1.length());
	joint.anchor_0 = Vector3(joint_anchor_0.begin());
	joint.anchor_1 = Vector3(joint_anchor_1.begin());
	joint.restitution = restitution.float_value();
	joint.spring = spring.float_value();
	joint.damping = damping.float_value();
	joint.distance = distance.float_value();
	joint.breakable = breakable.bool_value();
	joint.break_force = break_force.float_value();
	joint.break_torque = break_torque.float_value();
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

	// Read joints
	List<PhysicsJoint> m_joints(default_allocator());
	JSONElement joints = root.key_or_nil("joints");
	if (!joints.is_nil())
	{
		for (uint32_t i = 0; i < joints.size(); i++)
		{
			PhysicsJoint j;
			parse_joint(joints[i], j);
			m_joints.push_back(j);
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
	h.num_joints = m_joints.size();

	uint32_t offt = sizeof(PhysicsHeader);
	h.controller_offset = offt; offt += sizeof(PhysicsController) * h.num_controllers;
	h.actors_offset = offt; offt += sizeof(PhysicsActor) * h.num_actors;
	h.shapes_indices_offset = offt; offt += sizeof(uint32_t) * h.num_shapes_indices;
	h.shapes_offset = offt; offt += sizeof(PhysicsShape) * h.num_shapes;
	h.joints_offset = offt;

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

	if (m_joints.size())
	{
		out_file->write((char*) m_joints.begin(), sizeof(PhysicsJoint) * m_joints.size());
	}
}

} // namespace physics_resource
} // namespace crown
