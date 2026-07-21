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
	Database db = p._files;
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
	db.create_object_type("object", props);

	// Read defaults and write each property type.
	{
		Guid id = Guid.new_guid();
		Guid to = Guid.new_guid();
		db.create(id, "object");
		db.create(to, "object");

		assert(db.get_bool(id, "b") == true);
		assert(db.get_double(id, "d") == 1.0);
		assert(db.get_string(id, "s") == "a");
		assert(Vector3.equal_func(db.get_vector3(id, "v"), Vector3(1.0, 2.0, 3.0)));
		assert(Quaternion.equal_func(db.get_quaternion(id, "q"), Quaternion(1.0, 2.0, 3.0, 4.0)));
		assert(db.get_resource(id, "r") == "a");
		assert(Guid.equal_func(db.get_reference(id, "ref"), GUID_ZERO));
		assert(db.get_set(id, "set").length == 0);

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

		db.set_resource(id, "r", null);
		assert(db.get_resource(id, "r") == null);
	}

	// Ensure sets are unique, skip dead objects and update ownership.
	{
		Guid root = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid dead = Guid.new_guid();
		db.create(root, "object");
		db.create(child, "object");
		db.create(dead, "object");

		db.add_to_set(root, "set", child);
		db.add_to_set(root, "set", child);
		db.add_to_set(root, "set", dead);
		db.destroy(dead);
		Guid?[] ids = db.get_set(root, "set");
		assert(ids.length == 1);
		assert(Guid.equal_func(ids[0], child));
		assert(Guid.equal_func(db.owner(child), root));

		db.remove_from_set(root, "set", child);
		assert(db.get_set(root, "set").length == 0);
		assert(Guid.equal_func(db.owner(child), GUID_ZERO));
	}

	// Destroy an object recursively and reset the database.
	{
		Guid id = Guid.new_guid();
		Guid child = Guid.new_guid();
		db.create(id, "object");
		db.create(child, "object");
		db.add_to_set(id, "set", child);
		assert(db.has_object(id));
		assert(db.is_alive(id));
		assert(db.object_type(id) == "object");

		db.destroy(id);
		assert(db.has_object(id));
		assert(db.has_object(child));
		assert(!db.is_alive(id));
		assert(!db.is_alive(child));
		assert(db.object_type(id) == "object");

		db.reset();
		assert(!db.has_object(id));
		assert(!db.has_object(child));
		assert(db._data.size() == 1);
	}

	// Skip dead descendants and remap internal but not external references.
	{
		Guid root = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid dead = Guid.new_guid();
		Guid ext = Guid.new_guid();
		Guid copy = Guid.new_guid();
		db.create(root, "object");
		db.create(child, "object");
		db.create(dead, "object");
		db.create(ext, "object");
		db.add_to_set(root, "set", child);
		db.add_to_set(root, "set", dead);
		db.set_reference(root, "ref", child);
		db.set_reference(child, "ref", ext);
		db.destroy(dead);

		db.duplicate_one(root, copy);

		Guid?[] ids = db.get_set(copy, "set");
		assert(db._data.size() == 7);
		assert(ids.length == 1);
		assert(Guid.equal_func(db.get_reference(copy, "ref"), ids[0]));
		assert(Guid.equal_func(db.get_reference(ids[0], "ref"), ext));
	}

	// Remap references between roots and add the copies to their set.
	{
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
		db.set_reference(a, "ref", b);
		db.set_reference(b, "ref", a);

		db.duplicate_and_add_to_set({ a, b }, { copy_a, copy_b });

		assert(db.get_set(root, "set").length == 4);
		assert(Guid.equal_func(db.get_reference(copy_a, "ref"), copy_b));
		assert(Guid.equal_func(db.get_reference(copy_b, "ref"), copy_a));
	}
}

public static int main_unit_tests()
{
	test_database();
	return 0;
}

} /* namespace Crown */
