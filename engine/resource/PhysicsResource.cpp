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

#include <algorithm>
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
static uint32_t shape_type_to_enum(const char* type)
{
	if (string::strcmp("sphere", type) == 0) 		return PhysicsShapeType::SPHERE;
	else if (string::strcmp("capsule", type) == 0) 	return PhysicsShapeType::CAPSULE;
	else if (string::strcmp("box", type) == 0) 		return PhysicsShapeType::BOX;
	else if (string::strcmp("plane", type) == 0) 	return PhysicsShapeType::PLANE;

	CE_FATAL("Bad shape type");
	return 0;
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
	return 0;
}

//-----------------------------------------------------------------------------
void parse_controller(JSONElement e, PhysicsController& controller)
{
	JSONElement name 				= e.key("name");
	JSONElement height 				= e.key("height");
	JSONElement radius 				= e.key("radius");
	JSONElement slope_limit 		= e.key("slope_limit");
	JSONElement step_offset 		= e.key("step_offset");
	JSONElement contact_offset 		= e.key("contact_offset");
	JSONElement collision_filter 	= e.key("collision_filter");

	controller.name = name.string_id_value();
	controller.height = height.float_value();
	controller.radius = radius.float_value();
	controller.slope_limit = slope_limit.float_value();
	controller.step_offset = step_offset.float_value();
	controller.contact_offset = contact_offset.float_value();
	controller.collision_filter = collision_filter.string_id_value();
}

//-----------------------------------------------------------------------------
void parse_shapes(JSONElement e, List<PhysicsShape>& shapes)
{
	Vector<DynamicString> keys(default_allocator());
	e.key_value(keys);

	for (uint32_t k = 0; k < keys.size(); k++)
	{
		JSONElement shape 		= e.key(keys[k].c_str());
		JSONElement clasz		= shape.key("class");
		JSONElement type		= shape.key("type");
		JSONElement material	= shape.key("material");

		PhysicsShape ps;
		ps.name = keys[k].to_string_id();
		ps.shape_class = clasz.string_id_value();
		ps.material = material.string_id_value();
		DynamicString stype; type.string_value(stype);
		ps.type = shape_type_to_enum(stype.c_str());

		switch (ps.type)
		{
			case PhysicsShapeType::SPHERE:
			{
				JSONElement radius = shape.key("radius");
				ps.data_0 = radius.float_value();
				break;
			}
			case PhysicsShapeType::CAPSULE:
			{
				// TODO
				break;
			}
			case PhysicsShapeType::BOX:
			{
				JSONElement half_x = shape.key("half_x");
				JSONElement half_y = shape.key("half_y");
				JSONElement half_z = shape.key("half_z");

				ps.data_0 = half_x.float_value();
				ps.data_1 = half_y.float_value();
				ps.data_2 = half_z.float_value();

				break;
			}
			case PhysicsShapeType::PLANE:
			{
				// TODO
				break;
			}
		}

		shapes.push_back(ps);
	}
}

//-----------------------------------------------------------------------------
void parse_actors(JSONElement e, List<PhysicsActor>& actors, List<PhysicsShape>& actor_shapes, List<uint32_t>& shape_indices)
{
	Vector<DynamicString> keys(default_allocator());
	e.key_value(keys);

	for (uint32_t k = 0; k < keys.size(); k++)
	{
		JSONElement actor 	= e.key(keys[k].c_str());
		JSONElement node 	= actor.key("node");
		JSONElement clasz	= actor.key("class");
		JSONElement shapes	= actor.key("shapes");

		PhysicsActor pa;
		pa.name = keys[k].to_string_id();
		pa.node = node.string_id_value();
		pa.actor_class = clasz.string_id_value();
		pa.num_shapes = shapes.size();

		actors.push_back(pa);
		shape_indices.push_back(shape_indices.size());

		parse_shapes(shapes, actor_shapes);
	}
}

//-----------------------------------------------------------------------------
void parse_joints(JSONElement e, List<PhysicsJoint>& joints)
{
	Vector<DynamicString> keys(default_allocator());
	e.key_value(keys);

	for (uint32_t k = 0; k < keys.size(); k++)
	{
		JSONElement joint			= e.key(keys[k].c_str());
		JSONElement type 			= joint.key("type");
		JSONElement actor_0 		= joint.key("actor_0");
		JSONElement actor_1 		= joint.key("actor_1");
		JSONElement anchor_0 		= joint.key("anchor_0");
		JSONElement anchor_1 		= joint.key("anchor_1");
		JSONElement restitution 	= joint.key_or_nil("restitution");
		JSONElement spring 			= joint.key_or_nil("spring");
		JSONElement damping 		= joint.key_or_nil("damping");
		JSONElement distance 		= joint.key_or_nil("distance");
		JSONElement breakable 		= joint.key_or_nil("breakable");
		JSONElement break_force 	= joint.key_or_nil("break_force");
		JSONElement break_torque	= joint.key_or_nil("break_torque");

		PhysicsJoint pj;
		pj.name = keys[k].to_string_id();
		DynamicString jtype; type.string_value(jtype);
		pj.type = joint_type_to_enum(jtype.c_str());
		pj.actor_0 = actor_0.string_id_value();
		pj.actor_1 = actor_1.string_id_value();
		pj.anchor_0 = Vector3(anchor_0[0].float_value(), anchor_0[1].float_value(), anchor_0[2].float_value());
		pj.anchor_1 = Vector3(anchor_1[0].float_value(), anchor_1[1].float_value(), anchor_1[2].float_value());
		pj.restitution = restitution.is_nil() 	? 0.5 : restitution.float_value();
		pj.spring = spring.is_nil() 			? 100.0 : spring.float_value();
		pj.damping = damping.is_nil() 			? 0.0 : damping.float_value();
		pj.distance = distance.is_nil() 		? 1.0 : distance.float_value();
		pj.breakable = breakable.is_nil() 		? false : breakable.bool_value();
		pj.break_force = break_force.is_nil() 	? 3000.0 : break_force.float_value();
		pj.break_torque = break_torque.is_nil() ? 1000.0 : break_torque.float_value();

		joints.push_back(pj);
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

	List<PhysicsActor> m_actors(default_allocator());
	List<uint32_t> m_shapes_indices(default_allocator());
	List<PhysicsShape> m_shapes(default_allocator());
	List<PhysicsJoint> m_joints(default_allocator());

	if (root.has_key("actors")) parse_actors(root.key("actors"), m_actors, m_shapes, m_shapes_indices);
	if (root.has_key("joints")) parse_joints(root.key("joints"), m_joints);

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

namespace physics_config_resource
{
	struct ObjectName
	{
		StringId32 name;
		uint32_t index;

		bool operator()(const ObjectName& a, const ObjectName& b)
		{
			return a.name < b.name;
		}
	};

	struct NameToMask
	{
		StringId32 name;
		uint32_t mask;
	};

	uint32_t collides_with_to_mask(Vector<DynamicString>& collides_with, List<NameToMask>& name_to_mask)
	{
		uint32_t mask = 0;

		for (uint32_t i = 0; i < collides_with.size(); i++)
		{
			StringId32 cur_name = collides_with[i].to_string_id();
			for (uint32_t j = 0; j < name_to_mask.size(); j++)
			{
				if (cur_name == name_to_mask[j].name) mask |= name_to_mask[j].mask;
			}
		}

		return mask;
	}

	uint32_t collision_filter_to_mask(const char* filter, List<NameToMask> name_to_mask)
	{
		StringId32 filter_hash = hash::murmur2_32(filter, string::strlen(filter));
		for (uint32_t i = 0; i < name_to_mask.size(); i++)
		{
			if (name_to_mask[i].name == filter_hash) return name_to_mask[i].mask;
		}

		CE_ASSERT(false, "Collision filter '%s' not found", filter);
		return 0;
	}

	void parse_materials(JSONElement e, List<ObjectName>& names, List<PhysicsMaterial>& objects)
	{
		Vector<DynamicString> keys(default_allocator());
		e.key_value(keys);

		for (uint32_t i = 0; i < keys.size(); i++)
		{
			JSONElement material 			= e.key(keys[i].c_str());
			JSONElement static_friction 	= material.key("static_friction");
			JSONElement dynamic_friction	= material.key("dynamic_friction");
			JSONElement restitution			= material.key("restitution");

			// Read material name
			ObjectName mat_name;
			mat_name.name = keys[i].to_string_id();
			mat_name.index = i;

			// Read material object
			PhysicsMaterial mat;
			mat.static_friction = static_friction.float_value();
			mat.dynamic_friction = dynamic_friction.float_value();
			mat.restitution = restitution.float_value();

			names.push_back(mat_name);
			objects.push_back(mat);
		}
	}

	void parse_shapes(JSONElement e, List<NameToMask>& name_to_mask, List<ObjectName>& names, List<PhysicsShape2>& objects)
	{
		Vector<DynamicString> keys(default_allocator());
		e.key_value(keys);

		for (uint32_t i = 0; i < keys.size(); i++)
		{
			JSONElement shape				= e.key(keys[i].c_str());
			JSONElement collision_filter 	= shape.key("collision_filter");
			JSONElement trigger				= shape.key("trigger");

			// Read shape name
			ObjectName shape_name;
			shape_name.name = keys[i].to_string_id();
			shape_name.index = i;

			// Read shape object
			PhysicsShape2 ps2;
			ps2.trigger = trigger.bool_value();
			DynamicString cfilter; collision_filter.string_value(cfilter);
			ps2.collision_filter = collision_filter_to_mask(cfilter.c_str(), name_to_mask);

			names.push_back(shape_name);
			objects.push_back(ps2);
		}
	}

	void parse_actors(JSONElement e, List<ObjectName>& names, List<PhysicsActor2>& objects)
	{
		Vector<DynamicString> keys(default_allocator());
		e.key_value(keys);

		for (uint32_t i = 0; i < keys.size(); i++)
		{
			JSONElement actor			= e.key(keys[i].c_str());
			JSONElement linear_damping	= actor.key_or_nil("linear_damping");
			JSONElement angular_damping	= actor.key_or_nil("angular_damping");
			JSONElement dynamic			= actor.key_or_nil("dynamic");
			JSONElement kinematic		= actor.key_or_nil("kinematic");
			JSONElement disable_gravity	= actor.key_or_nil("disable_gravity");

			// Read actor name
			ObjectName actor_name;
			actor_name.name = keys[i].to_string_id();
			actor_name.index = i;

			// Read actor object
			PhysicsActor2 pa2;
			//actor.collision_filter = coll_filter.to_string_id();
			pa2.linear_damping = linear_damping.is_nil() ? 0.0 : linear_damping.float_value();
			pa2.angular_damping = angular_damping.is_nil() ? 0.05 : angular_damping.float_value();
			pa2.flags = 0;
			if (!dynamic.is_nil())
			{
				pa2.flags |= dynamic.bool_value() ? : 0;
			}
			if (!kinematic.is_nil())
			{
				pa2.flags |= kinematic.bool_value() ? PhysicsActor2::KINEMATIC : 0;
			}
			if (!disable_gravity.is_nil())
			{
				pa2.flags |= disable_gravity.bool_value() ? PhysicsActor2::DISABLE_GRAVITY : 0;
			}

			names.push_back(actor_name);
			objects.push_back(pa2);
		}
	}

	void parse_collision_filters(JSONElement e, List<ObjectName>& names, List<PhysicsCollisionFilter>& objects, List<NameToMask>& name_to_mask)
	{
		Vector<DynamicString> keys(default_allocator());
		e.key_value(keys);

		// Assign a unique mask to each collision filter
		for (uint32_t i = 0; i < keys.size(); i++)
		{
			NameToMask ntm;
			ntm.name = keys[i].to_string_id();
			ntm.mask = 1 << i;
			name_to_mask.push_back(ntm);
		}

		for (uint32_t i = 0; i < keys.size(); i++)
		{
			JSONElement filter			= e.key(keys[i].c_str());
			JSONElement collides_with	= filter.key("collides_with");

			// Read filter name
			ObjectName filter_name;
			filter_name.name = keys[i].to_string_id();
			filter_name.index = i;

			// Build mask
			Vector<DynamicString> collides_with_vector(default_allocator());
			collides_with.array_value(collides_with_vector);

			PhysicsCollisionFilter pcf;
			pcf.mask = collides_with_to_mask(collides_with_vector, name_to_mask);

			printf("FILTER: %s, mask = %X\n", keys[i].c_str(), pcf.mask);

			names.push_back(filter_name);
			objects.push_back(pcf);
		}		
	}

	void compile(Filesystem& fs, const char* resource_path, File* out_file)
	{
		File* file = fs.open(resource_path, FOM_READ);
		char* buf = (char*)default_allocator().allocate(file->size());
		file->read(buf, file->size());

		JSONParser json(buf);
		JSONElement root = json.root();

		List<ObjectName> material_names(default_allocator());
		List<PhysicsMaterial> material_objects(default_allocator());
		List<ObjectName> shape_names(default_allocator());
		List<PhysicsShape2> shape_objects(default_allocator());
		List<ObjectName> actor_names(default_allocator());
		List<PhysicsActor2> actor_objects(default_allocator());
		List<ObjectName> filter_names(default_allocator());
		List<PhysicsCollisionFilter> filter_objects(default_allocator());
		List<NameToMask> name_to_mask(default_allocator());

		// Parse materials
		if (root.has_key("collision_filters")) parse_collision_filters(root.key("collision_filters"), filter_names, filter_objects, name_to_mask);
		if (root.has_key("materials")) parse_materials(root.key("materials"), material_names, material_objects);
		if (root.has_key("shapes")) parse_shapes(root.key("shapes"), name_to_mask, shape_names, shape_objects);
		if (root.has_key("actors")) parse_actors(root.key("actors"), actor_names, actor_objects);

		default_allocator().deallocate(buf);
		fs.close(file);

		// Sort objects by name
		std::sort(material_names.begin(), material_names.end(), ObjectName());
		std::sort(shape_names.begin(), shape_names.end(), ObjectName());
		std::sort(actor_names.begin(), actor_names.end(), ObjectName());
		std::sort(filter_names.begin(), filter_names.end(), ObjectName());

		// Setup struct for writing
		PhysicsConfigHeader header;
		header.num_materials = material_names.size();
		header.num_shapes = shape_names.size();
		header.num_actors = actor_names.size();
		header.num_filters = filter_names.size();

		uint32_t offt = sizeof(PhysicsConfigHeader);
		header.materials_offset = offt;
		offt += sizeof(StringId32) * header.num_materials;
		offt += sizeof(PhysicsMaterial) * header.num_materials;

		header.shapes_offset = offt;
		offt += sizeof(StringId32) * header.num_shapes;
		offt += sizeof(PhysicsShape2) * header.num_shapes;

		header.actors_offset = offt;
		offt += sizeof(StringId32) * header.num_actors;
		offt += sizeof(PhysicsActor2) * header.num_actors;

		header.filters_offset = offt;
		offt += sizeof(StringId32) * header.num_filters;
		offt += sizeof(PhysicsCollisionFilter) * header.num_filters;

		// Write all
		out_file->write((char*) &header, sizeof(PhysicsConfigHeader));

		if (header.num_materials)
		{
			// Write material names
			for (uint32_t i = 0; i < material_names.size(); i++)
			{
				out_file->write((char*) &material_names[i].name, sizeof(StringId32));
			}

			// Write material objects
			for (uint32_t i = 0; i < material_names.size(); i++)
			{
				out_file->write((char*) &material_objects[material_names[i].index], sizeof(PhysicsMaterial));
			}
		}

		if (header.num_shapes)
		{
			// Write shape names
			for (uint32_t i = 0; i < shape_names.size(); i++)
			{
				out_file->write((char*) &shape_names[i].name, sizeof(StringId32));
			}

			// Write material objects
			for (uint32_t i = 0; i < shape_names.size(); i++)
			{
				out_file->write((char*) &shape_objects[shape_names[i].index], sizeof(PhysicsShape2));
			}
		}

		if (header.num_actors)
		{
			// Write shape names
			for (uint32_t i = 0; i < actor_names.size(); i++)
			{
				out_file->write((char*) &actor_names[i].name, sizeof(StringId32));
			}

			// Write material objects
			for (uint32_t i = 0; i < actor_names.size(); i++)
			{
				out_file->write((char*) &actor_objects[actor_names[i].index], sizeof(PhysicsActor2));
			}
		}

		if (header.num_filters)
		{
			// Write shape names
			for (uint32_t i = 0; i < filter_names.size(); i++)
			{
				out_file->write((char*) &filter_names[i].name, sizeof(StringId32));
			}

			// Write material objects
			for (uint32_t i = 0; i < filter_names.size(); i++)
			{
				out_file->write((char*) &filter_objects[filter_names[i].index], sizeof(PhysicsCollisionFilter));
			}
		}
	}

} // namespace physics_config_resource
} // namespace crown
