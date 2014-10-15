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
	struct Shape
	{
		const char* name;
		PhysicsShapeType::Enum type;
	};

	static const Shape s_shape[PhysicsShapeType::COUNT] =
	{
		{ "sphere",  PhysicsShapeType::SPHERE  },
		{ "capsule", PhysicsShapeType::CAPSULE },
		{ "box",     PhysicsShapeType::BOX     },
		{ "plane",   PhysicsShapeType::PLANE   }
	};

	struct Joint
	{
		const char* name;
		PhysicsJointType::Enum type;
	};

	static const Joint s_joint[PhysicsJointType::COUNT] =
	{
		{ "fixed",     PhysicsJointType::FIXED     },
		{ "spherical", PhysicsJointType::SPHERICAL },
		{ "revolute",  PhysicsJointType::REVOLUTE  },
		{ "prismatic", PhysicsJointType::PRISMATIC },
		{ "distance",  PhysicsJointType::DISTANCE  },
		{ "d6",        PhysicsJointType::D6        }
	};

	static uint32_t shape_type_to_enum(const char* type)
	{
		for (uint32_t i = 0; i < PhysicsShapeType::COUNT; i++)
		{
			if (string::strcmp(type, s_shape[i].name) == 0)
				return s_shape[i].type;
		}

		CE_FATAL("Bad shape type");
		return 0;
	}

	static uint32_t joint_type_to_enum(const char* type)
	{
		for (uint32_t i = 0; i < PhysicsJointType::COUNT; i++)
		{
			if (string::strcmp(type, s_joint[i].name) == 0)
				return s_joint[i].type;
		}

		CE_FATAL("Bad joint type");
		return 0;
	}

	void parse_controller(JSONElement e, PhysicsController& controller)
	{
		controller.name =             e.key("name").to_string_id();
		controller.height =           e.key("height").to_float();
		controller.radius =           e.key("radius").to_float();
		controller.slope_limit =      e.key("slope_limit").to_float();
		controller.step_offset =      e.key("step_offset").to_float();
		controller.contact_offset =   e.key("contact_offset").to_float();
		controller.collision_filter = e.key("collision_filter").to_string_id();
	}

	void parse_shapes(JSONElement e, Array<PhysicsShape>& shapes)
	{
		Vector<DynamicString> keys(default_allocator());
		e.to_keys(keys);

		for (uint32_t k = 0; k < vector::size(keys); k++)
		{
			JSONElement shape 		= e.key(keys[k].c_str());

			PhysicsShape ps;
			ps.name =        keys[k].to_string_id();
			ps.shape_class = shape.key("class").to_string_id();
			ps.material =    shape.key("material").to_string_id();
			ps.position =    shape.key("position").to_vector3();
			ps.rotation =    shape.key("rotation").to_quaternion();

			DynamicString stype; shape.key("type").to_string(stype);
			ps.type = shape_type_to_enum(stype.c_str());

			switch (ps.type)
			{
				case PhysicsShapeType::SPHERE:
				{
					ps.data_0 = shape.key("radius").to_float();
					break;
				}
				case PhysicsShapeType::CAPSULE:
				{
					ps.data_0 = shape.key("radius").to_float();
					ps.data_1 = shape.key("half_height").to_float();
					break;
				}
				case PhysicsShapeType::BOX:
				{
					ps.data_0 = shape.key("half_x").to_float();
					ps.data_1 = shape.key("half_y").to_float();
					ps.data_2 = shape.key("half_z").to_float();
					break;
				}
				case PhysicsShapeType::PLANE:
				{
					ps.data_0 = shape.key("n_x").to_float();
					ps.data_1 = shape.key("n_y").to_float();
					ps.data_2 = shape.key("n_z").to_float();
					ps.data_3 = shape.key("distance").to_float();
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

	void parse_actors(JSONElement e, Array<PhysicsActor>& actors, Array<PhysicsShape>& actor_shapes, Array<uint32_t>& shape_indices)
	{
		Vector<DynamicString> keys(default_allocator());
		e.to_keys(keys);

		for (uint32_t k = 0; k < vector::size(keys); k++)
		{
			JSONElement actor 	= e.key(keys[k].c_str());

			PhysicsActor pa;
			pa.name =        keys[k].to_string_id();
			pa.node =        actor.key("node").to_string_id();
			pa.actor_class = actor.key("class").to_string_id();
			pa.mass =        actor.key("mass").to_float();
			pa.num_shapes =  actor.key("shapes").size();

			array::push_back(actors, pa);
			array::push_back(shape_indices, array::size(shape_indices));

			parse_shapes(actor.key("shapes"), actor_shapes);
		}
	}

	void parse_joints(JSONElement e, Array<PhysicsJoint>& joints)
	{
		Vector<DynamicString> keys(default_allocator());
		e.to_keys(keys);

		for (uint32_t k = 0; k < vector::size(keys); k++)
		{
			JSONElement joint			= e.key(keys[k].c_str());
			JSONElement type 			= joint.key("type");

			PhysicsJoint pj;
			pj.name = keys[k].to_string_id();
			DynamicString jtype; type.to_string(jtype);
			pj.type         = joint_type_to_enum(jtype.c_str());
			pj.actor_0      = joint.key("actor_0").to_string_id();
			pj.actor_1      = joint.key("actor_1").to_string_id();
			pj.anchor_0     = joint.key("anchor_0").to_vector3();
			pj.anchor_1     = joint.key("anchor_1").to_vector3();
			pj.restitution  = joint.key_or_nil("restitution").to_float(0.5f);
			pj.spring       = joint.key_or_nil("spring").to_float(100.0f);
			pj.damping      = joint.key_or_nil("damping").to_float(0.0f);
			pj.distance     = joint.key_or_nil("distance").to_float(1.0f);
			pj.breakable    = joint.key_or_nil("breakable").to_bool(false);
			pj.break_force  = joint.key_or_nil("break_force").to_float(3000.0f);
			pj.break_torque = joint.key_or_nil("break_torque").to_float(1000.0f);

			switch (pj.type)
			{
				case PhysicsJointType::FIXED:
				{
					return;
				}
				case PhysicsJointType::SPHERICAL:
				{
					pj.y_limit_angle = joint.key_or_nil("y_limit_angle").to_float(math::HALF_PI);
					pj.z_limit_angle = joint.key_or_nil("z_limit_angle").to_float(math::HALF_PI);
					pj.contact_dist =  joint.key_or_nil("contact_dist").to_float(0.0f);
					break;
				}
				case PhysicsJointType::REVOLUTE:
				case PhysicsJointType::PRISMATIC:
				{
					pj.lower_limit =  joint.key_or_nil("lower_limit").to_float(0.0f);
					pj.upper_limit =  joint.key_or_nil("upper_limit").to_float(0.0f);
					pj.contact_dist = joint.key_or_nil("contact_dist").to_float(0.0f);
					break;
				}
				case PhysicsJointType::DISTANCE:
				{
					pj.max_distance = joint.key_or_nil("max_distance").to_float(0.0f);
					break;
				}
				case PhysicsJointType::D6:
				{
					CE_FATAL("Not implemented");
					break;
				}
			}

			array::push_back(joints, pj);
		}
	}

	void compile(const char* path, CompileOptions& opts)
	{
		static const uint32_t VERSION = 1;

		Buffer buf = opts.read(path);
		JSONParser json(array::begin(buf));
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

		PhysicsResource pr;
		pr.version = VERSION;
		pr.num_controllers = m_has_controller ? 1 : 0;
		pr.num_actors = array::size(m_actors);
		pr.num_shapes_indices = array::size(m_shapes_indices);
		pr.num_shapes = array::size(m_shapes);
		pr.num_joints = array::size(m_joints);

		uint32_t offt = sizeof(PhysicsResource);
		pr.controller_offset = offt; offt += sizeof(PhysicsController) * pr.num_controllers;
		pr.actors_offset = offt; offt += sizeof(PhysicsActor) * pr.num_actors;
		pr.shapes_indices_offset = offt; offt += sizeof(uint32_t) * pr.num_shapes_indices;
		pr.shapes_offset = offt; offt += sizeof(PhysicsShape) * pr.num_shapes;
		pr.joints_offset = offt;

		// Write all
		opts.write(pr.version);
		opts.write(pr.num_controllers);
		opts.write(pr.controller_offset);
		opts.write(pr.num_actors);
		opts.write(pr.actors_offset);
		opts.write(pr.num_shapes_indices);
		opts.write(pr.shapes_indices_offset);
		opts.write(pr.num_shapes);
		opts.write(pr.shapes_offset);
		opts.write(pr.num_joints);
		opts.write(pr.joints_offset);

		if (m_has_controller)
		{
			opts.write(m_controller.name);
			opts.write(m_controller.height);
			opts.write(m_controller.radius);
			opts.write(m_controller.slope_limit);
			opts.write(m_controller.step_offset);
			opts.write(m_controller.contact_offset);
			opts.write(m_controller.collision_filter);
		}

		for (uint32_t i = 0; i < array::size(m_actors); i++)
		{
			opts.write(m_actors[i].name);
			opts.write(m_actors[i].node);
			opts.write(m_actors[i].actor_class);
			opts.write(m_actors[i].mass);
			opts.write(m_actors[i].num_shapes);
		}

		for (uint32_t i = 0; i < array::size(m_shapes_indices); i++)
		{
			opts.write(m_shapes_indices[i]);
		}

		for (uint32_t i = 0; i < array::size(m_shapes); i++)
		{
			opts.write(m_shapes[i].name);
			opts.write(m_shapes[i].shape_class);
			opts.write(m_shapes[i].type);
			opts.write(m_shapes[i].material);
			opts.write(m_shapes[i].resource);
			opts.write(m_shapes[i].position);
			opts.write(m_shapes[i].rotation);
			opts.write(m_shapes[i].data_0);
			opts.write(m_shapes[i].data_1);
			opts.write(m_shapes[i].data_2);
			opts.write(m_shapes[i].data_3);
		}

		for (uint32_t i = 0; i < array::size(m_joints); i++)
		{
			opts.write(m_joints[i].name);
			opts.write(m_joints[i].type);
			opts.write(m_joints[i].actor_0);
			opts.write(m_joints[i].actor_1);
			opts.write(m_joints[i].anchor_0);
			opts.write(m_joints[i].anchor_1);
			opts.write(m_joints[i].breakable);
			opts.write(m_joints[i]._pad[0]);
			opts.write(m_joints[i]._pad[1]);
			opts.write(m_joints[i]._pad[2]);
			opts.write(m_joints[i].break_force);
			opts.write(m_joints[i].break_torque);
			opts.write(m_joints[i].lower_limit);
			opts.write(m_joints[i].upper_limit);
			opts.write(m_joints[i].y_limit_angle);
			opts.write(m_joints[i].z_limit_angle);
			opts.write(m_joints[i].max_distance);
			opts.write(m_joints[i].contact_dist);
			opts.write(m_joints[i].restitution);
			opts.write(m_joints[i].spring);
			opts.write(m_joints[i].damping);
			opts.write(m_joints[i].distance);
		}
	}

	void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

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

	bool has_controller(const PhysicsResource* pr)
	{
		return pr->num_controllers == 1;
	}

	const PhysicsController* controller(const PhysicsResource* pr)
	{
		CE_ASSERT(has_controller(pr), "Controller does not exist");
		PhysicsController* controller = (PhysicsController*) ((char*)pr + pr->controller_offset);
		return controller;
	}

	uint32_t num_actors(const PhysicsResource* pr)
	{
		return pr->num_actors;
	}

	const PhysicsActor* actor(const PhysicsResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_actors(pr), "Index out of bounds");
		PhysicsActor* actor = (PhysicsActor*) ((char*)pr + pr->actors_offset);
		return &actor[i];
	}

	uint32_t num_shapes_indices(const PhysicsResource* pr)
	{
		return pr->num_shapes_indices;
	}

	uint32_t shape_index(const PhysicsResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_shapes_indices(pr), "Index out of bounds");
		uint32_t* index = (uint32_t*) ((char*)pr + pr->shapes_indices_offset);
		return index[i];
	}

	uint32_t num_shapes(const PhysicsResource* pr)
	{
		return pr->num_shapes;
	}

	const PhysicsShape* shape(const PhysicsResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_shapes(pr), "Index out of bounds");
		PhysicsShape* shape = (PhysicsShape*) ((char*)pr + pr->shapes_offset);
		return &shape[i];
	}

	uint32_t num_joints(const PhysicsResource* pr)
	{
		return pr->num_joints;
	}

	const PhysicsJoint* joint(const PhysicsResource* pr, uint32_t i)
	{
		CE_ASSERT(i < num_joints(pr), "Index out of bounds");
		PhysicsJoint* joint = (PhysicsJoint*) ((char*)pr + pr->joints_offset);
		return &joint[i];
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
			JSONElement material = e.key(keys[i].c_str());

			// Read material name
			ObjectName mat_name;
			mat_name.name = keys[i].to_string_id();
			mat_name.index = i;

			// Read material object
			PhysicsMaterial mat;
			mat.static_friction =  material.key("static_friction").to_float();
			mat.dynamic_friction = material.key("dynamic_friction").to_float();
			mat.restitution =      material.key("restitution").to_float();

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
			JSONElement shape = e.key(keys[i].c_str());

			// Read shape name
			ObjectName shape_name;
			shape_name.name = keys[i].to_string_id();
			shape_name.index = i;

			// Read shape object
			PhysicsShape2 ps2;
			ps2.trigger =          shape.key("trigger").to_bool();
			ps2.collision_filter = shape.key("collision_filter").to_string_id();

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

			// Read actor name
			ObjectName actor_name;
			actor_name.name = keys[i].to_string_id();
			actor_name.index = i;

			// Read actor object
			PhysicsActor2 pa2;
			pa2.linear_damping =  actor.key_or_nil("linear_damping").to_float(0.0f);
			pa2.angular_damping = actor.key_or_nil("angular_damping").to_float(0.05f);

			JSONElement dynamic			= actor.key_or_nil("dynamic");
			JSONElement kinematic		= actor.key_or_nil("kinematic");
			JSONElement disable_gravity	= actor.key_or_nil("disable_gravity");

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

	void compile(const char* path, CompileOptions& opts)
	{
		static const uint32_t VERSION = 1;

		Buffer buf = opts.read(path);
		JSONParser json(array::begin(buf));
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
		PhysicsConfigResource pcr;
		pcr.version = VERSION;
		pcr.num_materials = array::size(material_names);
		pcr.num_shapes = array::size(shape_names);
		pcr.num_actors = array::size(actor_names);
		pcr.num_filters = array::size(filter_names);

		uint32_t offt = sizeof(PhysicsConfigResource);
		pcr.materials_offset = offt;
		offt += sizeof(StringId32) * pcr.num_materials;
		offt += sizeof(PhysicsMaterial) * pcr.num_materials;

		pcr.shapes_offset = offt;
		offt += sizeof(StringId32) * pcr.num_shapes;
		offt += sizeof(PhysicsShape2) * pcr.num_shapes;

		pcr.actors_offset = offt;
		offt += sizeof(StringId32) * pcr.num_actors;
		offt += sizeof(PhysicsActor2) * pcr.num_actors;

		pcr.filters_offset = offt;
		offt += sizeof(StringId32) * pcr.num_filters;
		offt += sizeof(PhysicsCollisionFilter) * pcr.num_filters;

		// Write all
		opts.write(pcr.version);
		opts.write(pcr.num_materials);
		opts.write(pcr.materials_offset);
		opts.write(pcr.num_shapes);
		opts.write(pcr.shapes_offset);
		opts.write(pcr.num_actors);
		opts.write(pcr.actors_offset);
		opts.write(pcr.num_filters);
		opts.write(pcr.filters_offset);

		// Write material names
		for (uint32_t i = 0; i < pcr.num_materials; i++)
		{
			opts.write(material_names[i].name);
		}

		// Write material objects
		for (uint32_t i = 0; i < pcr.num_materials; i++)
		{
			opts.write(material_objects[material_names[i].index].static_friction);
			opts.write(material_objects[material_names[i].index].dynamic_friction);
			opts.write(material_objects[material_names[i].index].restitution);
		}

		// Write shape names
		for (uint32_t i = 0; i < pcr.num_shapes; i++)
		{
			opts.write(shape_names[i].name);
		}

		// Write material objects
		for (uint32_t i = 0; i < pcr.num_shapes; i++)
		{
			opts.write(shape_objects[shape_names[i].index].collision_filter);
			opts.write(shape_objects[shape_names[i].index].trigger);
			opts.write(shape_objects[shape_names[i].index]._pad[0]);
			opts.write(shape_objects[shape_names[i].index]._pad[1]);
			opts.write(shape_objects[shape_names[i].index]._pad[2]);
		}

		// Write actor names
		for (uint32_t i = 0; i < pcr.num_actors; i++)
		{
			opts.write(actor_names[i].name);
		}

		// Write actor objects
		for (uint32_t i = 0; i < pcr.num_actors; i++)
		{
			opts.write(actor_objects[actor_names[i].index].linear_damping);
			opts.write(actor_objects[actor_names[i].index].angular_damping);
			opts.write(actor_objects[actor_names[i].index].flags);
		}

		// Write filter names
		for (uint32_t i = 0; i < pcr.num_filters; i++)
		{
			opts.write(filter_names[i].name);
		}

		// Write filter objects
		for (uint32_t i = 0; i < pcr.num_filters; i++)
		{
			opts.write(filter_objects[filter_names[i].index].me);
			opts.write(filter_objects[filter_names[i].index].mask);
		}

		CE_DELETE(default_allocator(), s_ftm);
	}

	void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

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

	uint32_t num_materials(const PhysicsConfigResource* pcr)
	{
		return pcr->num_materials;
	}

	/// Returns the material with the given @a name
	const PhysicsMaterial* material(const PhysicsConfigResource* pcr, StringId32 name)
	{
		StringId32* begin = (StringId32*) ((char*)pcr + pcr->materials_offset);
		StringId32* end = begin + num_materials(pcr);
		StringId32* id = std::find(begin, end, name);
		CE_ASSERT(id != end, "Material not found");
		return material_by_index(pcr, id - begin);
	}

	const PhysicsMaterial* material_by_index(const PhysicsConfigResource* pcr, uint32_t i)
	{
		CE_ASSERT(i < num_materials(pcr), "Index out of bounds");
		const PhysicsMaterial* base = (PhysicsMaterial*) ((char*)pcr + pcr->materials_offset + sizeof(StringId32) * num_materials(pcr));
		return &base[i];
	}

	uint32_t num_shapes(const PhysicsConfigResource* pcr)
	{
		return pcr->num_shapes;
	}

	const PhysicsShape2* shape(const PhysicsConfigResource* pcr, StringId32 name)
	{
		StringId32* begin = (StringId32*) ((char*)pcr + pcr->shapes_offset);
		StringId32* end = begin + num_shapes(pcr);
		StringId32* id = std::find(begin, end, name);
		CE_ASSERT(id != end, "Shape not found");
		return shape_by_index(pcr, id - begin);
	}

	const PhysicsShape2* shape_by_index(const PhysicsConfigResource* pcr, uint32_t i)
	{
		CE_ASSERT(i < num_shapes(pcr), "Index out of bounds");
		const PhysicsShape2* base = (PhysicsShape2*) ((char*)pcr + pcr->shapes_offset + sizeof(StringId32) * num_shapes(pcr));
		return &base[i];
	}

	uint32_t num_actors(const PhysicsConfigResource* pcr)
	{
		return pcr->num_actors;
	}

	/// Returns the actor with the given @a name
	const PhysicsActor2* actor(const PhysicsConfigResource* pcr, StringId32 name)
	{
		StringId32* begin = (StringId32*) ((char*)pcr + pcr->actors_offset);
		StringId32* end = begin + num_actors(pcr);
		StringId32* id = std::find(begin, end, name);
		CE_ASSERT(id != end, "Actor not found");
		return actor_by_index(pcr, id - begin);
	}

	const PhysicsActor2* actor_by_index(const PhysicsConfigResource* pcr, uint32_t i)
	{
		CE_ASSERT(i < num_actors(pcr), "Index out of bounds");
		const PhysicsActor2* base = (PhysicsActor2*) ((char*)pcr + pcr->actors_offset + sizeof(StringId32) * num_actors(pcr));
		return &base[i];
	}

	uint32_t num_filters(const PhysicsConfigResource* pcr)
	{
		return pcr->num_filters;
	}

	const PhysicsCollisionFilter* filter(const PhysicsConfigResource* pcr, StringId32 name)
	{
		StringId32* begin = (StringId32*) ((char*)pcr + pcr->filters_offset);
		StringId32* end = begin + num_filters(pcr);
		StringId32* id = std::find(begin, end, name);
		CE_ASSERT(id != end, "Filter not found");
		return filter_by_index(pcr, id - begin);
	}

	const PhysicsCollisionFilter* filter_by_index(const PhysicsConfigResource* pcr, uint32_t i)
	{
		CE_ASSERT(i < num_filters(pcr), "Index out of bounds");
		const PhysicsCollisionFilter* base = (PhysicsCollisionFilter*) ((char*)pcr + pcr->filters_offset + sizeof(StringId32) * num_filters(pcr));
		return &base[i];
	}
} // namespace physics_config_resource
} // namespace crown
