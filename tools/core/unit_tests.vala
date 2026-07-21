/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
private static void test_database()
{
	stdout.printf("test_database\n");

	Project p = new Project();
	PropertyDefinition[] props =
	{
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "b",
			deffault = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "d",
			deffault = 1.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "s",
			deffault = "a",
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "v",
			deffault = Vector3(1.0, 2.0, 3.0),
		},
		PropertyDefinition()
		{
			type = PropertyType.QUATERNION,
			name = "q",
			deffault = Quaternion(1.0, 2.0, 3.0, 4.0),
		},
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "r",
			resource_type = "r",
			deffault = "a",
		},
		PropertyDefinition()
		{
			type = PropertyType.REFERENCE,
			name = "ref",
			object_type = StringId64("object"),
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "set",
			object_type = StringId64("object"),
		},
	};

	// Read property defaults.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		assert(db.get_bool(id, "b") == true);
		assert(db.get_double(id, "d") == 1.0);
		assert(db.get_string(id, "s") == "a");
		assert(Vector3.equal_func(db.get_vector3(id, "v"), Vector3(1.0, 2.0, 3.0)));
		assert(Quaternion.equal_func(db.get_quaternion(id, "q"), Quaternion(1.0, 2.0, 3.0, 4.0)));
		assert(db.get_resource(id, "r") == "a");
		assert(Guid.equal_func(db.get_reference(id, "ref"), GUID_ZERO));
		assert(db.get_set(id, "set").length == 0);
	}

	// Write each property type.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		Guid to = Guid.new_guid();
		db.create(id, "object");
		db.create(to, "object");

		db.set_bool(id, "b", false);
		db.set_double(id, "d", 2.0);
		db.set_string(id, "s", "b");
		db.set_vector3(id, "v", Vector3(4.0, 5.0, 6.0));
		db.set_quaternion(id, "q", Quaternion(5.0, 6.0, 7.0, 8.0));
		db.set_resource(id, "r", "b");
		db.set_reference(id, "ref", to);

		assert(db.get_bool(id, "b") == false);
		assert(db.get_double(id, "d") == 2.0);
		assert(db.get_string(id, "s") == "b");
		assert(Vector3.equal_func(db.get_vector3(id, "v"), Vector3(4.0, 5.0, 6.0)));
		assert(Quaternion.equal_func(db.get_quaternion(id, "q"), Quaternion(5.0, 6.0, 7.0, 8.0)));
		assert(db.get_resource(id, "r") == "b");
		assert(Guid.equal_func(db.get_reference(id, "ref"), to));
	}

	// Write a null resource.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		db.set_resource(id, "r", null);
		assert(db.get_resource(id, "r") == null);
	}

	// Add and remove an object from a set.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		Guid child = Guid.new_guid();
		db.create(root, "object");
		db.create(child, "object");

		db.add_to_set(root, "set", child);
		Guid?[] ids = db.get_set(root, "set");
		assert(ids.length == 1);
		assert(Guid.equal_func(ids[0], child));
		assert(Guid.equal_func(db.owner(child), root));

		db.remove_from_set(root, "set", child);
		assert(db.get_set(root, "set").length == 0);
		assert(Guid.equal_func(db.owner(child), GUID_ZERO));
	}

	// Ensure sets are unique.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		Guid child = Guid.new_guid();
		db.create(root, "object");
		db.create(child, "object");

		db.add_to_set(root, "set", child);
		db.add_to_set(root, "set", child);
		assert(db.get_set(root, "set").length == 1);
	}

	// Skip dead objects in sets.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		Guid dead = Guid.new_guid();
		db.create(root, "object");
		db.create(dead, "object");
		db.add_to_set(root, "set", dead);
		db.destroy(dead);

		assert(db.get_set(root, "set").length == 0);
	}

	// Destroy an object.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");
		assert(db.has_object(id));
		assert(db.is_alive(id));
		assert(db.object_type(id) == "object");

		db.destroy(id);
		assert(db.has_object(id));
		assert(!db.is_alive(id));
		assert(db.object_type(id) == "object");
	}

	// Destroy descendants recursively.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		Guid child = Guid.new_guid();
		db.create(root, "object");
		db.create(child, "object");
		db.add_to_set(root, "set", child);

		db.destroy(root);
		assert(db.has_object(root));
		assert(db.has_object(child));
		assert(!db.is_alive(root));
		assert(!db.is_alive(child));
	}

	// Reset the database.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid a = Guid.new_guid();
		Guid b = Guid.new_guid();
		db.create(a, "object");
		db.create(b, "object");

		db.reset();
		assert(!db.has_object(a));
		assert(!db.has_object(b));
		assert(db._data.size() == 1);
	}

	// Skip dead descendants when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid dead = Guid.new_guid();
		Guid copy = Guid.new_guid();
		db.create(root, "object");
		db.create(child, "object");
		db.create(dead, "object");
		db.add_to_set(root, "set", child);
		db.add_to_set(root, "set", dead);
		db.destroy(dead);

		db.duplicate_one(root, copy);

		assert(db._data.size() == 6);
		assert(db.get_set(copy, "set").length == 1);
	}

	// Remap internal references when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid copy = Guid.new_guid();
		db.create(root, "object");
		db.create(child, "object");
		db.add_to_set(root, "set", child);
		db.set_reference(root, "ref", child);

		db.duplicate_one(root, copy);

		Guid?[] ids = db.get_set(copy, "set");
		assert(ids.length == 1);
		assert(Guid.equal_func(db.get_reference(copy, "ref"), ids[0]));
	}

	// Preserve external references when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		Guid to = Guid.new_guid();
		Guid copy = Guid.new_guid();
		db.create(id, "object");
		db.create(to, "object");
		db.set_reference(id, "ref", to);

		db.duplicate_one(id, copy);

		assert(Guid.equal_func(db.get_reference(copy, "ref"), to));
	}

	// Remap references between duplicated roots.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid a = Guid.new_guid();
		Guid b = Guid.new_guid();
		Guid copy_a = Guid.new_guid();
		Guid copy_b = Guid.new_guid();
		db.create(a, "object");
		db.create(b, "object");
		db.set_reference(a, "ref", b);
		db.set_reference(b, "ref", a);

		db.duplicate({ a, b }, { copy_a, copy_b });

		assert(Guid.equal_func(db.get_reference(copy_a, "ref"), copy_b));
		assert(Guid.equal_func(db.get_reference(copy_b, "ref"), copy_a));
	}

	// Add duplicated roots to their set.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		Guid a = Guid.new_guid();
		Guid b = Guid.new_guid();
		Guid copy_a = Guid.new_guid();
		Guid copy_b = Guid.new_guid();
		db.create(root, "object");
		db.create(a, "object");
		db.create(b, "object");
		db.add_to_set(root, "set", a);
		db.add_to_set(root, "set", b);

		db.duplicate_and_add_to_set({ a, b }, { copy_a, copy_b });

		assert(db.get_set(root, "set").length == 4);
	}

	// Copy property values when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		Guid copy = Guid.new_guid();
		db.create(id, "object");
		db.set_bool(id, "b", false);
		db.set_double(id, "d", 2.0);
		db.set_string(id, "s", "b");
		db.set_vector3(id, "v", Vector3(4.0, 5.0, 6.0));
		db.set_quaternion(id, "q", Quaternion(5.0, 6.0, 7.0, 8.0));
		db.set_resource(id, "r", "b");

		db.duplicate_one(id, copy);

		assert(db.get_bool(copy, "b") == false);
		assert(db.get_double(copy, "d") == 2.0);
		assert(db.get_string(copy, "s") == "b");
		assert(Vector3.equal_func(db.get_vector3(copy, "v"), Vector3(4.0, 5.0, 6.0)));
		assert(Quaternion.equal_func(db.get_quaternion(copy, "q"), Quaternion(5.0, 6.0, 7.0, 8.0)));
		assert(db.get_resource(copy, "r") == "b");
	}

	// Reuse supplied IDs when duplicated roots overlap.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid copy = Guid.new_guid();
		Guid copy_child = Guid.new_guid();
		db.create(root, "object");
		db.create(child, "object");
		db.add_to_set(root, "set", child);

		db.duplicate({ root, child }, { copy, copy_child });

		Guid?[] ids = db.get_set(copy, "set");
		assert(db._data.size() == 5);
		assert(ids.length == 1);
		assert(Guid.equal_func(ids[0], copy_child));
	}
}

public static int main_unit_tests()
{
	test_database();
	return 0;
}

} /* namespace Crown */
