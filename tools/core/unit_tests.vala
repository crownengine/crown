/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
private static void test_string()
{
	stdout.printf("test_print_max_decimals\n");

	char buffer[PRINT_MAX_DECIMALS_BUFFER_SIZE];
	assert(print_max_decimals(buffer, 0.0, 4) == "0");
	assert(print_max_decimals(buffer, 1.0, 4) == "1");
	assert(print_max_decimals(buffer, 1.2, 4) == "1.2");
	assert(print_max_decimals(buffer, 1.23456, 4) == "1.2346");
	assert(print_max_decimals(buffer, -1.23456, 4) == "-1.2346");
	assert(print_max_decimals(buffer, 1.6, 0) == "2");
	assert(print_max_decimals(buffer, 0.00001, 4) == "0");
	assert(print_max_decimals(buffer, double.MAX, 5).length == 309);
}

private static bool contains_guid(Guid?[] ids, Guid id)
{
	foreach (unowned Guid? member in ids) {
		if (Guid.equal_func(member, id))
			return true;
	}
	return false;
}

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
	// Read a bool property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		assert(db.get_bool(id, "b") == true);
	}

	// Read a double property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		assert(db.get_double(id, "d") == 1.0);
	}

	// Read a string property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		assert(db.get_string(id, "s") == "a");
	}

	// Read a vector property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		assert(Vector3.equal_func(db.get_vector3(id, "v"), Vector3(1.0, 2.0, 3.0)));
	}

	// Read a quaternion property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		assert(Quaternion.equal_func(db.get_quaternion(id, "q"), Quaternion(1.0, 2.0, 3.0, 4.0)));
	}

	// Read a resource property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		assert(db.get_resource(id, "r") == "a");
	}

	// Read a reference property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		assert(Guid.equal_func(db.get_reference(id, "ref"), GUID_ZERO));
	}

	// Read an objects set property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		assert(db.get_set(id, "set").length == 0);
	}

	// Write each property type.
	// Write a bool property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		db.set_bool(id, "b", false);
		assert(db.get_bool(id, "b") == false);
	}

	// Write a double property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		db.set_double(id, "d", 2.0);
		assert(db.get_double(id, "d") == 2.0);
	}

	// Write a string property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		db.set_string(id, "s", "b");
		assert(db.get_string(id, "s") == "b");
	}

	// Write a vector property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		db.set_vector3(id, "v", Vector3(4.0, 5.0, 6.0));
		assert(Vector3.equal_func(db.get_vector3(id, "v"), Vector3(4.0, 5.0, 6.0)));
	}

	// Write a quaternion property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		db.set_quaternion(id, "q", Quaternion(5.0, 6.0, 7.0, 8.0));
		assert(Quaternion.equal_func(db.get_quaternion(id, "q"), Quaternion(5.0, 6.0, 7.0, 8.0)));
	}

	// Write a resource property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		db.set_resource(id, "r", "b");
		assert(db.get_resource(id, "r") == "b");
	}

	// Write a reference property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");
		Guid to = Guid.new_guid();
		db.create(to, "object");

		db.set_reference(id, "ref", to);
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

	// Replace a string-backed resource value.
	{
		UndoRedo undo_redo = new UndoRedo();
		Database db = new Database(p, undo_redo);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

		UndoRedo? ur = db.disable_undo();
		db.set_string(id, "r", "a");
		db.restore_undo(ur);
		undo_redo.reset();

		db.set_resource(id, "r", "b");
		db.add_restore_point(ActionType.CHANGE_OBJECTS, { id });
		assert(db.get_resource(id, "r") == "b");

		db.undo();
		assert(db.get_resource(id, "r") == "a");

		db.redo();
		assert(db.get_resource(id, "r") == "b");
	}

	// Add and remove an object from a set.
	// Add an object to a set.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");

		db.add_to_set(root, "set", child);

		Guid?[] ids = db.get_set(root, "set");
		assert(ids.length == 1);
		assert(Guid.equal_func(ids[0], child));
		assert(Guid.equal_func(db.owner(child), root));
	}

	// Remove an object from a set.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		db.add_to_set(root, "set", child);

		db.remove_from_set(root, "set", child);

		assert(db.get_set(root, "set").length == 0);
		assert(Guid.equal_func(db.owner(child), GUID_ZERO));
	}

	// Ensure sets are unique.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, "object");
		Guid child = Guid.new_guid();
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
		db.create(root, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		db.add_to_set(root, "set", child);

		db.destroy(child);

		assert(db.get_set(root, "set").length == 0);
	}

	// Destroy an object.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");

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
		db.create(root, "object");
		Guid child = Guid.new_guid();
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
		db.create(a, "object");
		Guid b = Guid.new_guid();
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
		db.create(root, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		Guid dead = Guid.new_guid();
		db.create(dead, "object");
		Guid copy = Guid.new_guid();
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
		db.create(root, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		Guid copy = Guid.new_guid();
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
		db.create(id, "object");
		Guid to = Guid.new_guid();
		db.create(to, "object");
		Guid copy = Guid.new_guid();
		db.set_reference(id, "ref", to);

		db.duplicate_one(id, copy);

		assert(Guid.equal_func(db.get_reference(copy, "ref"), to));
	}

	// Remap references between duplicated roots.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid a = Guid.new_guid();
		db.create(a, "object");
		Guid b = Guid.new_guid();
		db.create(b, "object");
		Guid ca = Guid.new_guid();
		Guid cb = Guid.new_guid();
		db.set_reference(a, "ref", b);
		db.set_reference(b, "ref", a);

		db.duplicate({ a, b }, { ca, cb });

		assert(Guid.equal_func(db.get_reference(ca, "ref"), cb));
		assert(Guid.equal_func(db.get_reference(cb, "ref"), ca));
	}

	// Add duplicated roots to their set.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, "object");
		Guid a = Guid.new_guid();
		db.create(a, "object");
		Guid b = Guid.new_guid();
		db.create(b, "object");
		Guid ca = Guid.new_guid();
		Guid cb = Guid.new_guid();
		db.add_to_set(root, "set", a);
		db.add_to_set(root, "set", b);

		db.duplicate_and_add_to_set({ a, b }, { ca, cb });

		Guid?[] ids = db.get_set(root, "set");
		assert(ids.length == 4);
		assert(contains_guid(ids, ca));
		assert(contains_guid(ids, cb));
	}

	// Keep overlapping selections in the duplicated hierarchy, in either order.
	for (int depth = 1; depth <= 2; ++depth) {
		for (int reverse = 0; reverse < 2; ++reverse) {
			Database db = new Database(p);
			db.create_object_type("object", props);
			Guid root = Guid.new_guid();
			db.create(root, "object");
			Guid a = Guid.new_guid();
			db.create(a, "object");
			db.add_to_set(root, "set", a);
			Guid parent = a;
			if (depth == 2) {
				parent = Guid.new_guid();
				db.create(parent, "object");
				db.add_to_set(a, "set", parent);
			}
			Guid b = Guid.new_guid();
			db.create(b, "object");
			db.add_to_set(parent, "set", b);
			Guid ca = Guid.new_guid();
			Guid cb = Guid.new_guid();

			if (reverse == 0)
				db.duplicate_and_add_to_set({ a, b }, { ca, cb });
			else
				db.duplicate_and_add_to_set({ b, a }, { cb, ca });

			assert(db.get_set(root, "set").length == 2);
			assert(contains_guid(db.get_set(root, "set"), a));
			assert(contains_guid(db.get_set(root, "set"), ca));
			assert(db.get_set(parent, "set").length == 1);
			assert(contains_guid(db.get_set(parent, "set"), b));
			assert(Guid.equal_func(db.owner(b), parent));
			assert(db.get_set(ca, "set").length == 1);
			Guid copied_parent = ca;
			if (depth == 2) {
				assert(db.get_set(a, "set").length == 1);
				assert(contains_guid(db.get_set(a, "set"), parent));
				copied_parent = db.get_set(ca, "set")[0];
				assert(!Guid.equal_func(copied_parent, parent));
				assert(Guid.equal_func(db.owner(copied_parent), ca));
			}
			assert(db.get_set(copied_parent, "set").length == 1);
			assert(contains_guid(db.get_set(copied_parent, "set"), cb));
			assert(Guid.equal_func(db.owner(cb), copied_parent));
			assert(Guid.equal_func(db.owner(ca), root));
			assert(db._data.size() == 2 + 2 * (depth + 1));
		}
	}

	// Copy property values when duplicating.
	// Copy a bool property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");
		Guid copy = Guid.new_guid();
		db.set_bool(id, "b", false);

		db.duplicate_one(id, copy);

		assert(db.get_bool(copy, "b") == false);
	}

	// Copy a double property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");
		Guid copy = Guid.new_guid();
		db.set_double(id, "d", 2.0);

		db.duplicate_one(id, copy);

		assert(db.get_double(copy, "d") == 2.0);
	}

	// Copy a string property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");
		Guid copy = Guid.new_guid();
		db.set_string(id, "s", "b");

		db.duplicate_one(id, copy);

		assert(db.get_string(copy, "s") == "b");
	}

	// Copy a vector property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");
		Guid copy = Guid.new_guid();
		db.set_vector3(id, "v", Vector3(4.0, 5.0, 6.0));

		db.duplicate_one(id, copy);

		assert(Vector3.equal_func(db.get_vector3(copy, "v"), Vector3(4.0, 5.0, 6.0)));
	}

	// Copy a quaternion property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");
		Guid copy = Guid.new_guid();
		db.set_quaternion(id, "q", Quaternion(5.0, 6.0, 7.0, 8.0));

		db.duplicate_one(id, copy);

		assert(Quaternion.equal_func(db.get_quaternion(copy, "q"), Quaternion(5.0, 6.0, 7.0, 8.0)));
	}

	// Copy a resource property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, "object");
		Guid copy = Guid.new_guid();
		db.set_resource(id, "r", "b");

		db.duplicate_one(id, copy);

		assert(db.get_resource(copy, "r") == "b");
	}

	// Reuse supplied IDs when duplicated roots overlap.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		Guid copy = Guid.new_guid();
		Guid cc = Guid.new_guid();
		db.add_to_set(root, "set", child);

		db.duplicate({ root, child }, { copy, cc });

		Guid?[] ids = db.get_set(copy, "set");
		assert(db._data.size() == 5);
		assert(ids.length == 1);
		assert(Guid.equal_func(ids[0], cc));
	}

	// Decode unit children only once.
	{
		Database db = new Database(p);
		create_object_types(db);
		Guid root = Guid.new_guid();
		db.create(root, OBJECT_TYPE_UNIT);

		GLib.HashTable<string, Value?> child = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		child["_type"] = OBJECT_TYPE_UNIT;
		child["children"] = new GLib.GenericArray<Value?>();

		GLib.GenericArray<Value?> children = new GLib.GenericArray<Value?>();
		children.add(child);
		GLib.HashTable<string, Value?> json = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		json["_type"] = OBJECT_TYPE_UNIT;
		json["children"] = children;

		db.decode_object(root, GUID_ZERO, "", json);

		assert(db.get_set(root, "children").length == 1);
		assert(db._data.size() == 3);
	}

	// Instances inherit ordinary properties and merge object sets by prefab ID.
	// Inherit an ordinary property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid inst = Guid.new_guid();
		db.create_from_prefab(inst, src);

		assert(!db.get_data(inst).contains("s"));
		db.set_string(src, "s", "from source");
		assert(db.get_string(inst, "s") == "from source");
	}

	// Preserve a local property override.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid inst = Guid.new_guid();
		db.set_string(src, "s", "source before");
		db.create_from_prefab(inst, src);

		db.set_string(inst, "s", "local");
		db.set_string(src, "s", "source after");

		assert(db.get_string(inst, "s") == "local");
	}

	// Clear a local property override.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid inst = Guid.new_guid();
		db.set_bool(src, "b", false);
		db.create_from_prefab(inst, src);
		db.set_bool(inst, "b", true);

		db.set_null(inst, "b");

		assert(db.get_bool(inst, "b") == false);
	}

	// Preserve inheritance when duplicating an instance.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid inst = Guid.new_guid();
		Guid copy = Guid.new_guid();
		db.create_from_prefab(inst, src);
		db.set_string(src, "s", "before duplication");

		db.duplicate_one(inst, copy);

		assert(!db.get_data(copy).contains("s"));
		db.set_string(src, "s", "after duplication");
		assert(db.get_string(copy, "s") == "after duplication");
	}

	// Inherit object set members.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid a = Guid.new_guid();
		db.create(a, "object");
		Guid b = Guid.new_guid();
		db.create(b, "object");
		Guid inst = Guid.new_guid();
		db.add_to_set(src, "set", a);
		db.add_to_set(src, "set", b);

		db.create_from_prefab(inst, src);

		Guid?[] members = db.get_set(inst, "set");
		assert(members.length == 2);
		assert(contains_guid(members, a));
		assert(contains_guid(members, b));
	}

	// Replace an inherited set member by prefab ID.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		Guid other = Guid.new_guid();
		db.create(other, "object");
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.add_to_set(src, "set", other);
		db.create_from_prefab(inst, src);
		db.create_from_prefab(repl, child);

		db.add_to_set(inst, "set", repl);

		Guid?[] members = db.get_set(inst, "set");
		assert(members.length == 2);
		assert(contains_guid(members, repl));
		assert(contains_guid(members, other));
		assert(!contains_guid(members, child));
	}

	// Serialize an inherited set member replacement.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		Guid other = Guid.new_guid();
		db.create(other, "object");
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.add_to_set(src, "set", other);
		db.create_from_prefab(inst, src);
		db.create_from_prefab(repl, child);
		db.add_to_set(inst, "set", repl);

		GLib.HashTable<string, Value?> encoded = db.encode_object(inst, db.get_data(inst));

		Database loaded = new Database(p);
		loaded.create_object_type("object", props);
		loaded.create(src, "object");
		loaded.create(child, "object");
		loaded.create(other, "object");
		loaded.add_to_set(src, "set", child);
		loaded.add_to_set(src, "set", other);
		loaded.create_from_prefab(inst, src);
		loaded.decode_object(inst, GUID_ZERO, "", encoded);

		Guid?[] members = loaded.get_set(inst, "set");
		assert(members.length == 2);
		assert(contains_guid(members, repl));
		assert(contains_guid(members, other));
		assert(!contains_guid(members, child));
	}

	// Serialize a local property override on an instance member.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.create_from_prefab(inst, src);
		db.create_from_prefab(repl, child);
		db.set_double(repl, "d", 7.0);
		db.add_to_set(inst, "set", repl);

		GLib.HashTable<string, Value?> encoded = db.encode_object(inst, db.get_data(inst));

		Database loaded = new Database(p);
		loaded.create_object_type("object", props);
		loaded.create(src, "object");
		loaded.create(child, "object");
		loaded.add_to_set(src, "set", child);
		loaded.create_from_prefab(inst, src);
		loaded.decode_object(inst, GUID_ZERO, "", encoded);

		assert(loaded.get_double(repl, "d") == 7.0);
	}

	// Undo and redo an inherited set member replacement.
	{
		UndoRedo undo_redo = new UndoRedo();
		Database db = new Database(p, undo_redo);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		Guid other = Guid.new_guid();
		db.create(other, "object");
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.add_to_set(src, "set", other);
		db.create_from_prefab(inst, src);

		undo_redo.reset();
		db.create_from_prefab(repl, child);
		db.add_to_set(inst, "set", repl);
		db.set_double(repl, "d", 7.0);
		db.add_restore_point(ActionType.CHANGE_OBJECTS, { inst });

		// Undo removes the local instance and reveals its source member again.
		db.undo();
		Guid?[] members = db.get_set(inst, "set");
		assert(contains_guid(members, child));
		assert(!contains_guid(members, repl));

		db.redo();
		members = db.get_set(inst, "set");
		assert(contains_guid(members, repl));
		assert(!contains_guid(members, child));
	}

	// Inherit members added to the source set.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		Guid inst = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.create_from_prefab(inst, src);

		Guid added = Guid.new_guid();
		db.create(added, "object");
		db.add_to_set(src, "set", added);

		Guid?[] members = db.get_set(inst, "set");
		assert(members.length == 2);
		assert(contains_guid(members, child));
		assert(contains_guid(members, added));
	}

	// Remove an instance member when its source member is removed.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, "object");
		Guid child = Guid.new_guid();
		db.create(child, "object");
		Guid other = Guid.new_guid();
		db.create(other, "object");
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.add_to_set(src, "set", other);
		db.create_from_prefab(inst, src);
		db.create_from_prefab(repl, child);
		db.add_to_set(inst, "set", repl);

		db.remove_from_set(src, "set", child);

		Guid?[] members = db.get_set(inst, "set");
		assert(members.length == 1);
		assert(contains_guid(members, other));
		assert(!contains_guid(members, child));
		assert(!contains_guid(members, repl));
	}
}

private static GLib.HashTable<string, Value?> legacy_mesh_renderer_unit_json(Guid component_id)
{
	GLib.HashTable<string, Value?> data = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
	data["material"] = "materials/source";

	GLib.HashTable<string, Value?> component = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
	component["_guid"] = component_id.to_string();
	component["_type"] = OBJECT_TYPE_MESH_RENDERER;
	component["data"] = data;

	GLib.GenericArray<Value?> components = new GLib.GenericArray<Value?>();
	components.add(component);
	GLib.HashTable<string, Value?> unit = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
	unit["_type"] = OBJECT_TYPE_UNIT;
	unit["components"] = components;
	return unit;
}

private static void test_mesh_resource()
{
	stdout.printf("test_mesh_resource\n");

	// Set mesh material slots.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid comp = Guid.new_guid();
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);

		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		MeshResource.set_material_slot(db, comp, "Mb", "materials/Mb");

		Guid?[] members = db.get_set(comp, "data.materials");
		assert(members.length == 2);
		Guid ma = db.get_string(members[0], "data.slot") == "Ma" ? members[0] : members[1];
		Guid mb = Guid.equal_func(ma, members[0]) ? members[1] : members[0];
		assert(db.get_resource(ma, "data.material") == "materials/Ma");
		assert(db.get_resource(mb, "data.material") == "materials/Mb");
	}

	// Update an existing mesh material slot without changing its identity.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid comp = Guid.new_guid();
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		Guid binding = db.get_set(comp, "data.materials")[0];

		MeshResource.set_material_slot(db, comp, "Ma", "materials/Mc");

		Guid?[] members = db.get_set(comp, "data.materials");
		assert(members.length == 1);
		assert(Guid.equal_func(members[0], binding));
		assert(db.get_resource(binding, "data.material") == "materials/Mc");
	}

	// Undo and redo a mesh material slot update.
	{
		Project p = new Project();
		UndoRedo undo_redo = new UndoRedo();
		Database db = new Database(p, undo_redo);
		create_object_types(db);
		Guid comp = Guid.new_guid();
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		Guid binding = db.get_set(comp, "data.materials")[0];
		undo_redo.reset();

		MeshResource.set_material_slot(db, comp, "Ma", "materials/Mc");
		db.add_restore_point(ActionType.CHANGE_OBJECTS, { binding });
		assert(db.get_resource(binding, "data.material") == "materials/Mc");

		db.undo();
		assert(db.get_resource(binding, "data.material") == "materials/Ma");

		db.redo();
		assert(db.get_resource(binding, "data.material") == "materials/Mc");
	}

	// Generate mesh setup commands before material assignment commands.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid unit = Guid.new_guid();
		Guid comp = Guid.new_guid();
		db.create(unit, OBJECT_TYPE_UNIT);
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		db.add_to_set(unit, "components", comp);
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		MeshResource.set_material_slot(db, comp, "Mb", "materials/Mb");

		StringBuilder commands = new StringBuilder();
		Unit.generate_set_component_commands(commands, unit, comp, db);

		assert(commands.str.index_of(":set_mesh(") < commands.str.index_of(":set_mesh_material("));
		assert(commands.str.contains("materials/Ma"));
		assert(commands.str.contains("materials/Mb"));
	}

	// Round-trip mesh material slots.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid unit = Guid.new_guid();
		Guid comp = Guid.new_guid();
		db.create(unit, OBJECT_TYPE_UNIT);
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		db.add_to_set(unit, "components", comp);
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		MeshResource.set_material_slot(db, comp, "Mb", "materials/Mb");
		Guid?[] members = db.get_set(comp, "data.materials");
		Guid ma = db.get_string(members[0], "data.slot") == "Ma" ? members[0] : members[1];

		Database loaded = new Database(p);
		create_object_types(loaded);
		loaded.create(unit, OBJECT_TYPE_UNIT);
		loaded.decode_object(unit, GUID_ZERO, "", db.encode_object(unit, db.get_data(unit)));

		assert(loaded.get_set(comp, "data.materials").length == 2);
		assert(loaded.get_resource(ma, "data.material") == "materials/Ma");
	}

	// Omit a destroyed mesh material slot from destroy commands.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid unit = Guid.new_guid();
		Guid comp = Guid.new_guid();
		db.create(unit, OBJECT_TYPE_UNIT);
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		db.add_to_set(unit, "components", comp);
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		MeshResource.set_material_slot(db, comp, "Mb", "materials/Mb");
		Guid?[] members = db.get_set(comp, "data.materials");
		Guid ma = db.get_string(members[0], "data.slot") == "Ma" ? members[0] : members[1];

		db.destroy(ma);
		StringBuilder commands = new StringBuilder();
		assert(Unit.generate_destroy_commands(commands, { ma }, db) == 1);

		assert(!commands.str.contains("materials/Ma"));
		assert(commands.str.contains("materials/Mb"));
	}

	// Undo and redo destruction of a mesh material slot.
	{
		Project p = new Project();
		UndoRedo undo_redo = new UndoRedo();
		Database db = new Database(p, undo_redo);
		create_object_types(db);
		Guid comp = Guid.new_guid();
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		MeshResource.set_material_slot(db, comp, "Mb", "materials/Mb");
		Guid binding = db.get_set(comp, "data.materials")[0];
		undo_redo.reset();

		db.destroy(binding);
		db.add_restore_point(ActionType.DESTROY_OBJECTS, { binding });
		assert(db.get_set(comp, "data.materials").length == 1);

		db.undo();
		assert(db.get_set(comp, "data.materials").length == 2);

		db.redo();
		assert(db.get_set(comp, "data.materials").length == 1);
	}

	// Round-trip mesh material overrides.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid unit = Guid.new_guid();
		Guid comp = Guid.new_guid();
		Guid binding = Guid.new_guid();
		string key = "modified_components.#" + comp.to_string() + ".data.materials";
		db.create(unit, OBJECT_TYPE_UNIT);
		db.set_resource(unit, "prefab", "units/prefab");
		db.create(binding, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(binding, "data.slot", "Ma");
		db.set_resource(binding, "data.material", "materials/Ma");
		db.add_to_set(unit, key, binding);

		Database loaded = new Database(p);
		create_object_types(loaded);
		loaded.create(unit, OBJECT_TYPE_UNIT);
		loaded.decode_object(unit, GUID_ZERO, "", db.encode_object(unit, db.get_data(unit)));

		assert(!loaded.has_property(unit, "materials"));
		assert(loaded.has_property(unit, key));
		Guid?[] members = loaded.get_set(unit, key);
		assert(members.length == 1);
		assert(Guid.equal_func(members[0], binding));
		assert(loaded.get_resource(binding, "data.material") == "materials/Ma");
	}

	// Give legacy whole-mesh materials a stable binding identity.
	{
		Project p = new Project();
		Guid src = Guid.parse("0cc80c3d-089a-43ad-b145-77c504e2ef50");
		Guid comp = Guid.parse("49753fff-34ea-4aff-8bcf-5116f1c1519a");
		Database first = new Database(p);
		create_object_types(first);
		first.create(src, OBJECT_TYPE_UNIT);
		first.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		Guid a = first.get_set(comp, "data.materials")[0];

		Database second = new Database(p);
		create_object_types(second);
		second.create(src, OBJECT_TYPE_UNIT);
		second.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		Guid b = second.get_set(comp, "data.materials")[0];

		assert(Guid.equal_func(b, a));
	}

	// Migrate a sparse legacy whole-mesh material override.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.parse("0cc80c3d-089a-43ad-b145-77c504e2ef50");
		Guid comp = Guid.parse("49753fff-34ea-4aff-8bcf-5116f1c1519a");
		db.create(src, OBJECT_TYPE_UNIT);
		db.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		Guid child = db.get_set(comp, "data.materials")[0];
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		Guid inst = Guid.new_guid();
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "units/prefab");
		db.set_resource(inst, "modified_components.#" + comp.to_string() + ".data.material", "materials/instance");

		Unit(db, inst).prune_stale_overrides();

		GLib.GenericSet<Guid?> members = (GLib.GenericSet<Guid?>)Unit(db, inst).get_component_property(comp, "data.materials");
		int count = 0;
		Guid repl = GUID_ZERO;
		foreach (unowned Guid? id in members) {
			++count;
			repl = id;
		}
		assert(count == 1);
		assert(!Guid.equal_func(repl, child));
		assert(Guid.equal_func(db.get_reference(repl, "_prefab"), child));
		assert(db.get_string(repl, "data.slot") == "default");
		assert(db.get_resource(repl, "data.material") == "materials/instance");
	}

	// Collapse duplicate legacy material overrides onto their source binding.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.parse("0cc80c3d-089a-43ad-b145-77c504e2ef50");
		Guid comp = Guid.parse("49753fff-34ea-4aff-8bcf-5116f1c1519a");
		db.create(src, OBJECT_TYPE_UNIT);
		db.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		Guid child = db.get_set(comp, "data.materials")[0];
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		Guid inst = Guid.parse("0444b593-d55c-49c2-86aa-eea58388eea6");
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "units/prefab");
		string key = "modified_components.#" + comp.to_string() + ".data.materials";
		Guid repl = Guid.parse("4edacc7b-8622-4979-aa7d-90e79befe109");
		db.create_from_prefab(repl, child);
		db.set_reference(repl, "_prefab", Guid.parse("9ef9f820-82b9-487c-821e-1a6e9df77d1a"));
		db.set_resource(repl, "data.material", "units/beach/materials/M_MountainBike_Variant02");
		db.add_to_set(inst, key, repl);
		Guid other = Guid.parse("736f5e9f-a90b-47f8-b386-d34cdabfe077");
		db.create_from_prefab(other, child);
		db.set_reference(other, "_prefab", Guid.parse("6c652871-5c4a-4e80-9489-bf7706cb97c9"));
		db.set_resource(other, "data.material", "units/beach/materials/M_MountainBike_Variant02");
		db.add_to_set(inst, key, other);

		GLib.GenericSet<Guid?> members = (GLib.GenericSet<Guid?>)Unit(db, inst).get_component_property(comp, "data.materials");

		int count = 0;
		foreach (unowned Guid? id in members)
			++count;
		assert(count == 1);
		assert(members.contains(repl) || members.contains(other));
		assert(!members.contains(child));
		assert(Guid.equal_func(db.get_reference(repl, "_prefab"), child));
		assert(Guid.equal_func(db.get_reference(other, "_prefab"), child));
		foreach (unowned Guid? id in members) {
			assert(db.get_string(id, "data.slot") == "default");
			assert(db.get_resource(id, "data.material") == "units/beach/materials/M_MountainBike_Variant02");
		}
	}

	// Ignore unsupported full material overrides without _prefab.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.parse("0cc80c3d-089a-43ad-b145-77c504e2ef50");
		Guid comp = Guid.parse("49753fff-34ea-4aff-8bcf-5116f1c1519a");
		db.create(src, OBJECT_TYPE_UNIT);
		db.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		Guid child = db.get_set(comp, "data.materials")[0];
		db.set_reference(GUID_ZERO, "core/units/primitives/cube.unit", src);
		Guid inst = Guid.new_guid();
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "core/units/primitives/cube");
		Guid binding = Guid.new_guid();
		db.create(binding, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(binding, "data.slot", "default");
		db.set_resource(binding, "data.material", "units/water/water2");
		string key = "modified_components.#" + comp.to_string() + ".data.materials";
		db.add_to_set(inst, key, binding);

		GLib.GenericSet<Guid?> members = (GLib.GenericSet<Guid?>)Unit(db, inst).get_component_property(comp, "data.materials");

		int count = 0;
		foreach (unowned Guid? id in members)
			++count;
		assert(count == 1);
		assert(!members.contains(binding));
		assert(members.contains(child));
		assert(Guid.equal_func(db.get_reference(binding, "_prefab"), GUID_ZERO));
	}

	// Exclude unsupported full material overrides from generated commands.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.parse("0cc80c3d-089a-43ad-b145-77c504e2ef50");
		Guid comp = Guid.parse("49753fff-34ea-4aff-8bcf-5116f1c1519a");
		db.create(src, OBJECT_TYPE_UNIT);
		db.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		db.set_reference(GUID_ZERO, "core/units/primitives/cube.unit", src);
		Guid inst = Guid.new_guid();
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "core/units/primitives/cube");
		Guid binding = Guid.new_guid();
		db.create(binding, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(binding, "data.slot", "default");
		db.set_resource(binding, "data.material", "units/water/water2");
		string key = "modified_components.#" + comp.to_string() + ".data.materials";
		db.add_to_set(inst, key, binding);

		StringBuilder commands = new StringBuilder();
		Unit.generate_mesh_material_commands(commands, inst, comp, db);

		assert(!commands.str.contains("units/water/water2"));
		assert(commands.str.contains("materials/source"));
	}

	// Replace one inherited material binding while preserving other source bindings.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.new_guid();
		Guid comp = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid other = Guid.new_guid();
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.create(src, OBJECT_TYPE_UNIT);
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		db.add_to_set(src, "components", comp);
		db.create(child, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(child, "data.slot", "Ma");
		db.set_resource(child, "data.material", "materials/source");
		db.add_to_set(comp, "data.materials", child);
		db.create(other, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(other, "data.slot", "Mb");
		db.set_resource(other, "data.material", "materials/other");
		db.add_to_set(comp, "data.materials", other);
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "units/prefab");
		db.create_from_prefab(repl, child);
		db.set_resource(repl, "data.material", "materials/instance");
		string key = "modified_components.#" + comp.to_string() + ".data.materials";
		db.add_to_set(inst, key, repl);

		GLib.GenericSet<Guid?> members = (GLib.GenericSet<Guid?>)Unit(db, inst).get_component_property(comp, "data.materials");

		int count = 0;
		foreach (unowned Guid? id in members)
			++count;
		assert(count == 2);
		assert(members.contains(repl));
		assert(members.contains(other));
		assert(!members.contains(child));
	}

	// Inherit material bindings added to a source component.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.new_guid();
		Guid comp = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid inst = Guid.new_guid();
		db.create(src, OBJECT_TYPE_UNIT);
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		db.add_to_set(src, "components", comp);
		db.create(child, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(child, "data.slot", "Ma");
		db.add_to_set(comp, "data.materials", child);
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "units/prefab");

		Guid added = Guid.new_guid();
		db.create(added, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(added, "data.slot", "Mb");
		db.add_to_set(comp, "data.materials", added);

		GLib.GenericSet<Guid?> members = (GLib.GenericSet<Guid?>)Unit(db, inst).get_component_property(comp, "data.materials");
		assert(members.contains(child));
		assert(members.contains(added));
	}

	// Generate commands from the effective material override.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.new_guid();
		Guid comp = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.create(src, OBJECT_TYPE_UNIT);
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		db.add_to_set(src, "components", comp);
		db.create(child, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(child, "data.slot", "Ma");
		db.set_resource(child, "data.material", "materials/source");
		db.add_to_set(comp, "data.materials", child);
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "units/prefab");
		db.create_from_prefab(repl, child);
		db.set_resource(repl, "data.material", "materials/instance");
		string key = "modified_components.#" + comp.to_string() + ".data.materials";
		db.add_to_set(inst, key, repl);

		StringBuilder commands = new StringBuilder();
		Unit.generate_mesh_material_commands(commands, inst, comp, db);

		assert(commands.str.contains("Ma"));
		assert(commands.str.contains("materials/instance"));
		assert(!commands.str.contains("materials/source"));
	}

	// Round-trip an inherited mesh material override.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.new_guid();
		Guid comp = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.create(src, OBJECT_TYPE_UNIT);
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		db.add_to_set(src, "components", comp);
		db.create(child, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(child, "data.slot", "Ma");
		db.set_resource(child, "data.material", "materials/source");
		db.add_to_set(comp, "data.materials", child);
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "units/prefab");
		db.create_from_prefab(repl, child);
		db.set_resource(repl, "data.material", "materials/instance");
		string key = "modified_components.#" + comp.to_string() + ".data.materials";
		db.add_to_set(inst, key, repl);

		Database loaded = new Database(p);
		create_object_types(loaded);
		loaded.create(src, OBJECT_TYPE_UNIT);
		loaded.decode_object(src, GUID_ZERO, "", db.encode_object(src, db.get_data(src)));
		loaded.set_reference(GUID_ZERO, "units/prefab.unit", src);
		loaded.create(inst, OBJECT_TYPE_UNIT);
		loaded.decode_object(inst, GUID_ZERO, "", db.encode_object(inst, db.get_data(inst)));

		assert(loaded.get_resource(repl, "data.material") == "materials/instance");
		GLib.GenericSet<Guid?> members = (GLib.GenericSet<Guid?>)Unit(loaded, inst).get_component_property(comp, "data.materials");
		assert(members.contains(repl));
		assert(!members.contains(child));
	}

	// Remove an inherited material replacement when its source binding is removed.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.new_guid();
		Guid comp = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.create(src, OBJECT_TYPE_UNIT);
		db.create(comp, OBJECT_TYPE_MESH_RENDERER);
		db.add_to_set(src, "components", comp);
		db.create(child, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(child, "data.slot", "Ma");
		db.add_to_set(comp, "data.materials", child);
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "units/prefab");
		db.create_from_prefab(repl, child);
		string key = "modified_components.#" + comp.to_string() + ".data.materials";
		db.add_to_set(inst, key, repl);

		db.remove_from_set(comp, "data.materials", child);

		GLib.GenericSet<Guid?> members = (GLib.GenericSet<Guid?>)Unit(db, inst).get_component_property(comp, "data.materials");
		assert(!members.contains(repl));
	}
}

private static void test_inherited_lod_levels()
{
	stdout.printf("test_inherited_lod_levels\n");

	// Generate LOD commands with a local screen-size override.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.new_guid();
		Guid comp = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		Guid mesh = Guid.new_guid();
		db.create(src, OBJECT_TYPE_UNIT);
		db.create(comp, OBJECT_TYPE_LOD_GROUP);
		db.add_to_set(src, "components", comp);
		db.create(child, OBJECT_TYPE_LOD_LEVEL);
		db.set_double(child, "data.screen_size", 0.5);
		db.set_reference(child, "data.mesh_renderer", mesh);
		db.add_to_set(comp, "data.lod_levels", child);
		db.set_reference(GUID_ZERO, "units/lod-prefab.unit", src);
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "units/lod-prefab");
		db.create_from_prefab(repl, child);
		db.set_double(repl, "data.screen_size", 0.25);
		string key = "modified_components.#" + comp.to_string() + ".data.lod_levels";
		db.add_to_set(inst, key, repl);

		StringBuilder commands = new StringBuilder();
		Unit.generate_add_component_commands(commands, inst, comp, db);

		assert(commands.str.contains("0.25"));
		assert(!commands.str.contains("0.5,"));
	}

	// Generate LOD commands with an inherited mesh renderer reference.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.new_guid();
		Guid comp = Guid.new_guid();
		Guid child = Guid.new_guid();
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		Guid mesh = Guid.new_guid();
		db.create(src, OBJECT_TYPE_UNIT);
		db.create(comp, OBJECT_TYPE_LOD_GROUP);
		db.add_to_set(src, "components", comp);
		db.create(child, OBJECT_TYPE_LOD_LEVEL);
		db.set_double(child, "data.screen_size", 0.5);
		db.add_to_set(comp, "data.lod_levels", child);
		db.set_reference(GUID_ZERO, "units/lod-prefab.unit", src);
		db.create(inst, OBJECT_TYPE_UNIT);
		db.set_resource(inst, "prefab", "units/lod-prefab");
		db.create_from_prefab(repl, child);
		string key = "modified_components.#" + comp.to_string() + ".data.lod_levels";
		db.add_to_set(inst, key, repl);

		db.set_reference(child, "data.mesh_renderer", mesh);
		StringBuilder commands = new StringBuilder();
		Unit.generate_add_component_commands(commands, inst, comp, db);

		assert(commands.str.contains(mesh.to_string()));
	}
}

public static int main_unit_tests()
{
	test_string();
	test_database();
	test_mesh_resource();
	test_inherited_lod_levels();
	return 0;
}

} /* namespace Crown */
