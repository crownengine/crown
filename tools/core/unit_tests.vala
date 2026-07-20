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

	// Skip dead descendants and remap references to live copies.
	{
		Guid root = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid dead = Guid.new_guid();
		Guid copy = Guid.new_guid();
		db.create(root, "object");
		db.create(child, "object");
		db.create(dead, "object");
		db.add_to_set(root, "set", child);
		db.add_to_set(root, "set", dead);
		db.set_reference(root, "ref", child);
		db.destroy(dead);

		db.duplicate_one(root, copy);

		Guid?[] ids = db.get_set(copy, "set");
		assert(db._data.size() == 6);
		assert(ids.length == 1);
		assert(Guid.equal_func(db.get_reference(copy, "ref"), ids[0]));
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
