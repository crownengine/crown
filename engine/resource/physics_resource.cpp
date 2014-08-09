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
#include "allocator.h"
#include "filesystem.h"
#include "string_utils.h"
#include "json_parser.h"
#include "physics_resource.h"
#include "string_utils.h"
#include "dynamic_string.h"
#include "map.h"
#include "quaternion.h"

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

	controller.name = name.to_string_id();
	controller.height = height.to_float();
	controller.radius = radius.to_float();
	controller.slope_limit = slope_limit.to_float();
	controller.step_offset = step_offset.to_float();
	controller.contact_offset = contact_offset.to_float();
	controller.collision_filter = collision_filter.to_string_id();
}

//-----------------------------------------------------------------------------
void parse_shapes(JSONElement e, Array<PhysicsShape>& shapes)
{
	Vector<DynamicString> keys(default_allocator());
	e.to_keys(keys);

	for (uint32_t k = 0; k < vector::size(keys); k++)
	{
		JSONElement shape 		= e.key(keys[k].c_str());
		JSONElement clasz		= shape.key("class");
		JSONElement type		= shape.key("type");
		JSONElement material	= shape.key("material");

		PhysicsShape ps;
		ps.name = keys[k].to_string_id();
		ps.shape_class = clasz.to_string_id();
		ps.material = material.to_string_id();
		DynamicString stype; type.to_string(stype);
		ps.type = shape_type_to_enum(stype.c_str());
		ps.position = shape.key("position").to_vector3();
		ps.rotation = shape.key("rotation").to_quaternion();

		switch (ps.type)
		{
			case PhysicsShapeType::SPHERE:
			{
				JSONElement radius = shape.key("radius");
				ps.data_0 = radius.to_float();

				break;
			}
			case PhysicsShapeType::CAPSULE:
			{
				JSONElement radius = shape.key("radius");
				JSONElement half_height = shape.key("half_height");

				ps.data_0 = radius.to_float();
				ps.data_1 = half_height.to_float();

				break;
			}
			case PhysicsShapeType::BOX:
			{
				JSONElement half_x = shape.key("half_x");
				JSONElement half_y = shape.key("half_y");
				JSONElement half_z = shape.key("half_z");

				ps.data_0 = half_x.to_float();
				ps.data_1 = half_y.to_float();
				ps.data_2 = half_z.to_float();

				break;
			}
			case PhysicsShapeType::PLANE:
			{
				JSONElement n_x = shape.key("n_x");
				JSONElement n_y = shape.key("n_y");
				JSONElement n_z = shape.key("n_z");
				JSONElement distance = shape.key("distance");

				ps.data_0 = n_x.to_float();
				ps.data_1 = n_y.to_float();
				ps.data_2 = n_z.to_float();
				ps.data_3 = distance.to_float();

				break;
			}
			case PhysicsShapeType::CONVEX_MESH:
			{
				ps.resource = shape.key("mesh").to_resource_id("mesh");

				break;
			}
		}

		array::push_back(shapes, ps);
	}
}

//-----------------------------------------------------------------------------
void parse_actors(JSONElement e, Array<PhysicsActor>& actors, Array<PhysicsShape>& actor_shapes, Array<uint32_t>& shape_indices)
{
	Vector<DynamicString> keys(default_allocator());
	e.to_keys(keys);

	for (uint32_t k = 0; k < vector::size(keys); k++)
	{
		JSONElement actor 	= e.key(keys[k].c_str());
		JSONElement node 	= actor.key("node");
		JSONElement clasz	= actor.key("class");
		JSONElement shapes	= actor.key("shapes");
		JSONElement mass	= actor.key("mass");

		PhysicsActor pa;
		pa.name = keys[k].to_string_id();
		pa.node = node.to_string_id();
		pa.actor_class = clasz.to_string_id();
		pa.mass = mass.to_float();
		pa.num_shapes = shapes.size();

		array::push_back(actors, pa);
		array::push_back(shape_indices, array::size(shape_indices));

		parse_shapes(shapes, actor_shapes);
	}
}

//-----------------------------------------------------------------------------
void parse_joints(JSONElement e, Array<PhysicsJoint>& joints)
{
	Vector<DynamicString> keys(default_allocator());
	e.to_keys(keys);

	for (uint32_t k = 0; k < vector::size(keys); k++)
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
		DynamicString jtype; type.to_string(jtype);
		pj.type = joint_type_to_enum(jtype.c_str());
		pj.actor_0 = actor_0.to_string_id();
		pj.actor_1 = actor_1.to_string_id();
		pj.anchor_0 = Vector3(anchor_0[0].to_float(), anchor_0[1].to_float(), anchor_0[2].to_float());
		pj.anchor_1 = Vector3(anchor_1[0].to_float(), anchor_1[1].to_float(), anchor_1[2].to_float());
		pj.restitution = restitution.is_nil() 	? 0.5f : restitution.to_float();
		pj.spring = spring.is_nil() 			? 100.0f : spring.to_float();
		pj.damping = damping.is_nil() 			? 0.0f : damping.to_float();
		pj.distance = distance.is_nil() 		? 1.0f : distance.to_float();
		pj.breakable = breakable.is_nil() 		? false : breakable.to_bool();
		pj.break_force = break_force.is_nil() 	? 3000.0f : break_force.to_float();
		pj.break_torque = break_torque.is_nil() ? 1000.0f : break_torque.to_float();

		switch (pj.type)
		{
			case PhysicsJointType::FIXED:
			{
				return;
			}
			case PhysicsJointType::SPHERICAL:
			{
				JSONElement y_limit_angle = joint.key_or_nil("y_limit_angle");
				JSONElement z_limit_angle = joint.key_or_nil("z_limit_angle");
				JSONElement contact_dist = joint.key_or_nil("contact_dist");

				pj.y_limit_angle = y_limit_angle.is_nil() ? math::HALF_PI : y_limit_angle.to_float();
				pj.z_limit_angle = z_limit_angle.is_nil() ? math::HALF_PI : z_limit_angle.to_float();
				pj.contact_dist = contact_dist.is_nil() ? 0.0f : contact_dist.to_float();

				break;
			}
			case PhysicsJointType::REVOLUTE:
			case PhysicsJointType::PRISMATIC:
			{
				JSONElement lower_limit = joint.key_or_nil("lower_limit");
				JSONElement upper_limit = joint.key_or_nil("upper_limit");
				JSONElement contact_dist = joint.key_or_nil("contact_dist");

				pj.lower_limit = lower_limit.is_nil() ? 0.0f : lower_limit.to_float();
				pj.upper_limit = upper_limit.is_nil() ? 0.0f : upper_limit.to_float();
				pj.contact_dist = contact_dist.is_nil() ? 0.0f : contact_dist.to_float();

				break;
			}
			case PhysicsJointType::DISTANCE:
			{
				JSONElement max_distance = joint.key_or_nil("max_distance");
				pj.max_distance = max_distance.is_nil() ? 0.0f : max_distance.to_float();

				break;
			}
			case PhysicsJointType::D6:
			{
				// Must be implemented

				break;
			}
		}

		array::push_back(joints, pj);
	}
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	JSONParser json(*file);
	fs.close(file);

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

	Array<PhysicsActor> m_actors(default_allocator());
	Array<uint32_t> m_shapes_indices(default_allocator());
	Array<PhysicsShape> m_shapes(default_allocator());
	Array<PhysicsJoint> m_joints(default_allocator());

	if (root.has_key("actors")) parse_actors(root.key("actors"), m_actors, m_shapes, m_shapes_indices);
	if (root.has_key("joints")) parse_joints(root.key("joints"), m_joints);

	PhysicsHeader h;
	h.version = 1;
	h.num_controllers = m_has_controller ? 1 : 0;
	h.num_actors = array::size(m_actors);
	h.num_shapes_indices = array::size(m_shapes_indices);
	h.num_shapes = array::size(m_shapes);
	h.num_joints = array::size(m_joints);

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

	if (array::size(m_actors))
	{
		out_file->write((char*) array::begin(m_actors), sizeof(PhysicsActor) * array::size(m_actors));
	}

	if (array::size(m_shapes_indices))
	{
		out_file->write((char*) array::begin(m_shapes_indices), sizeof(uint32_t) * array::size(m_shapes_indices));
	}

	if (array::size(m_shapes))
	{
		out_file->write((char*) array::begin(m_shapes), sizeof(PhysicsShape) * array::size(m_shapes));
	}

	if (array::size(m_joints))
	{
		out_file->write((char*) array::begin(m_joints), sizeof(PhysicsJoint) * array::size(m_joints));
	}
}
} // namespace physics_resource

namespace physics_config_resource
{
	static Map<DynamicString, uint32_t>* s_ftm = NULL;

	struct ObjectName
	{
		StringId32 name;
		uint32_t index;

		bool operator()(const ObjectName& a, const ObjectName& b)
		{
			return a.name < b.name;
		}
	};

	void parse_materials(JSONElement e, Array<ObjectName>& names, Array<PhysicsMaterial>& objects)
	{
		Vector<DynamicString> keys(default_allocator());
		e.to_keys(keys);

		for (uint32_t i = 0; i < vector::size(keys); i++)
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
			mat.static_friction = static_friction.to_float();
			mat.dynamic_friction = dynamic_friction.to_float();
			mat.restitution = restitution.to_float();

			array::push_back(names, mat_name);
			array::push_back(objects, mat);
		}
	}

	void parse_shapes(JSONElement e, Array<ObjectName>& names, Array<PhysicsShape2>& objects)
	{
		Vector<DynamicString> keys(default_allocator());
		e.to_keys(keys);

		for (uint32_t i = 0; i < vector::size(keys); i++)
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
			ps2.trigger = trigger.to_bool();
			ps2.collision_filter = collision_filter.to_string_id();

			array::push_back(names, shape_name);
			array::push_back(objects, ps2);
		}
	}

	void parse_actors(JSONElement e, Array<ObjectName>& names, Array<PhysicsActor2>& objects)
	{
		Vector<DynamicString> keys(default_allocator());
		e.to_keys(keys);

		for (uint32_t i = 0; i < vector::size(keys); i++)
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
			pa2.linear_damping = linear_damping.is_nil() ? 0.0f : linear_damping.to_float();
			pa2.angular_damping = angular_damping.is_nil() ? 0.05f : angular_damping.to_float();
			pa2.flags = 0;
			if (!dynamic.is_nil())
			{
				pa2.flags |= dynamic.to_bool() ? 1 : 0;
			}
			if (!kinematic.is_nil())
			{
				pa2.flags |= kinematic.to_bool() ? PhysicsActor2::KINEMATIC : 0;
			}
			if (!disable_gravity.is_nil())
			{
				pa2.flags |= disable_gravity.to_bool() ? PhysicsActor2::DISABLE_GRAVITY : 0;
			}

			array::push_back(names, actor_name);
			array::push_back(objects, pa2);
		}
	}

	uint32_t new_filter_mask()
	{
		static uint32_t mask = 1;
		CE_ASSERT(mask != 0x80000000u, "Too many collision filters");
		uint32_t tmp = mask;
		mask = mask << 1;
		return tmp;
	}

	uint32_t filter_to_mask(const char* f)
	{
		if (map::has(*s_ftm, DynamicString(f)))
			return map::get(*s_ftm, DynamicString(f), 0u);

		uint32_t new_filter = new_filter_mask();
		map::set(*s_ftm, DynamicString(f), new_filter);
		return new_filter;
	}

	uint32_t collides_with_to_mask(const Vector<DynamicString>& coll_with)
	{
		uint32_t mask = 0;

		for (uint32_t i = 0; i < vector::size(coll_with); i++)
		{
			mask |= filter_to_mask(coll_with[i].c_str());
		}

		return mask;
	}

	void parse_collision_filters(JSONElement e, Array<ObjectName>& names, Array<PhysicsCollisionFilter>& objects)
	{
		Vector<DynamicString> keys(default_allocator());
		e.to_keys(keys);

		for (uint32_t i = 0; i < vector::size(keys); i++)
		{
			JSONElement filter			= e.key(keys[i].c_str());
			JSONElement collides_with	= filter.key("collides_with");

			// Read filter name
			ObjectName filter_name;
			filter_name.name = keys[i].to_string_id();
			filter_name.index = i;

			// Build mask
			Vector<DynamicString> collides_with_vector(default_allocator());
			collides_with.to_array(collides_with_vector);

			PhysicsCollisionFilter pcf;
			pcf.me = filter_to_mask(keys[i].c_str());
			pcf.mask = collides_with_to_mask(collides_with_vector);

			// printf("FILTER: %s (me = %X, mask = %X\n", keys[i].c_str(), pcf.me, pcf.mask);

			array::push_back(names, filter_name);
			array::push_back(objects, pcf);
		}
	}

	void compile(Filesystem& fs, const char* resource_path, File* out_file)
	{
		File* file = fs.open(resource_path, FOM_READ);
		JSONParser json(*file);
		fs.close(file);

		JSONElement root = json.root();

		typedef Map<DynamicString, uint32_t> FilterMap;
		s_ftm = CE_NEW(default_allocator(), FilterMap)(default_allocator());

		Array<ObjectName> material_names(default_allocator());
		Array<PhysicsMaterial> material_objects(default_allocator());
		Array<ObjectName> shape_names(default_allocator());
		Array<PhysicsShape2> shape_objects(default_allocator());
		Array<ObjectName> actor_names(default_allocator());
		Array<PhysicsActor2> actor_objects(default_allocator());
		Array<ObjectName> filter_names(default_allocator());
		Array<PhysicsCollisionFilter> filter_objects(default_allocator());

		// Parse materials
		if (root.has_key("collision_filters")) parse_collision_filters(root.key("collision_filters"), filter_names, filter_objects);
		if (root.has_key("materials")) parse_materials(root.key("materials"), material_names, material_objects);
		if (root.has_key("shapes")) parse_shapes(root.key("shapes"), shape_names, shape_objects);
		if (root.has_key("actors")) parse_actors(root.key("actors"), actor_names, actor_objects);

		// Sort objects by name
		std::sort(array::begin(material_names), array::end(material_names), ObjectName());
		std::sort(array::begin(shape_names), array::end(shape_names), ObjectName());
		std::sort(array::begin(actor_names), array::end(actor_names), ObjectName());
		std::sort(array::begin(filter_names), array::end(filter_names), ObjectName());

		// Setup struct for writing
		PhysicsConfigHeader header;
		header.num_materials = array::size(material_names);
		header.num_shapes = array::size(shape_names);
		header.num_actors = array::size(actor_names);
		header.num_filters = array::size(filter_names);

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
			for (uint32_t i = 0; i < array::size(material_names); i++)
			{
				out_file->write((char*) &material_names[i].name, sizeof(StringId32));
			}

			// Write material objects
			for (uint32_t i = 0; i < array::size(material_names); i++)
			{
				out_file->write((char*) &material_objects[material_names[i].index], sizeof(PhysicsMaterial));
			}
		}

		if (header.num_shapes)
		{
			// Write shape names
			for (uint32_t i = 0; i < array::size(shape_names); i++)
			{
				out_file->write((char*) &shape_names[i].name, sizeof(StringId32));
			}

			// Write material objects
			for (uint32_t i = 0; i < array::size(shape_names); i++)
			{
				out_file->write((char*) &shape_objects[shape_names[i].index], sizeof(PhysicsShape2));
			}
		}

		if (header.num_actors)
		{
			// Write actor names
			for (uint32_t i = 0; i < array::size(actor_names); i++)
			{
				out_file->write((char*) &actor_names[i].name, sizeof(StringId32));
			}

			// Write actor objects
			for (uint32_t i = 0; i < array::size(actor_names); i++)
			{
				out_file->write((char*) &actor_objects[actor_names[i].index], sizeof(PhysicsActor2));
			}
		}

		if (header.num_filters)
		{
			// Write filter names
			for (uint32_t i = 0; i < array::size(filter_names); i++)
			{
				out_file->write((char*) &filter_names[i].name, sizeof(StringId32));
			}

			// Write filter objects
			for (uint32_t i = 0; i < array::size(filter_names); i++)
			{
				out_file->write((char*) &filter_objects[filter_names[i].index], sizeof(PhysicsCollisionFilter));
			}
		}

		CE_DELETE(default_allocator(), s_ftm);
	}

} // namespace physics_config_resource
} // namespace crown
