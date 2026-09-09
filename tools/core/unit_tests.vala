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
}

private static void test_mesh_resource()
{
	stdout.printf("test_mesh_resource\n");

	// Set mesh material slots.
	{
		Project project = new Project();
		UndoRedo undo_redo = new UndoRedo();
		Database db = new Database(project, undo_redo);
		create_object_types(db);
		Guid unit_id = Guid.new_guid();
		Guid component_id = Guid.new_guid();
		db.create(unit_id, OBJECT_TYPE_UNIT);
		db.create(component_id, OBJECT_TYPE_MESH_RENDERER);
		db.add_to_set(unit_id, "components", component_id);
		MeshResource.set_material_slot(db, component_id, "Ma", "materials/Ma");
		MeshResource.set_material_slot(db, component_id, "Mb", "materials/Mb");
		Guid?[] bindings = db.get_set(component_id, "data.materials");
		assert(bindings.length == 2);
		Guid binding = db.get_string(bindings[0], "data.slot") == "Ma" ? bindings[0] : bindings[1];

		// Reimport updates the existing assignment instead of changing its identity.
		undo_redo.reset();
		MeshResource.set_material_slot(db, component_id, "Ma", "materials/Mc");
		db.add_restore_point(ActionType.CHANGE_OBJECTS, { binding });
		assert(db.get_set(component_id, "data.materials").length == 2);
		assert(db.get_resource(binding, "data.material") == "materials/Mc");
		db.undo();
		assert(db.get_resource(binding, "data.material") == "materials/Ma");
		db.redo();
		assert(db.get_resource(binding, "data.material") == "materials/Mc");

		StringBuilder commands = new StringBuilder();
		Unit.generate_set_component_commands(commands, unit_id, component_id, db);
		assert(commands.str.index_of(":set_mesh(") < commands.str.index_of(":set_mesh_material("));
		assert(commands.str.contains("materials/Mc"));
		assert(commands.str.contains("materials/Mb"));

		// Saving and loading preserves the nested assignments and their resource types.
		Database loaded = new Database(project);
		create_object_types(loaded);
		loaded.create(unit_id, OBJECT_TYPE_UNIT);
		loaded.decode_object(unit_id, GUID_ZERO, "", db.encode_object(unit_id, db.get_data(unit_id)));
		assert(loaded.get_set(component_id, "data.materials").length == 2);
		assert(loaded.get_resource(binding, "data.material") == "materials/Mc");

		undo_redo.reset();
		db.destroy(binding);
		db.add_restore_point(ActionType.DESTROY_OBJECTS, { binding });
		commands.truncate(0);
		assert(Unit.generate_destroy_commands(commands, { binding }, db) == 1);
		assert(!commands.str.contains("materials/Mc"));
		assert(commands.str.contains("materials/Mb"));
		db.undo();
		assert(db.get_set(component_id, "data.materials").length == 2);
		db.redo();
		assert(db.get_set(component_id, "data.materials").length == 1);
	}

	// Round-trip mesh material overrides.
	{
		Project project = new Project();
		Database db = new Database(project);
		create_object_types(db);
		Guid unit_id = Guid.new_guid();
		Guid component_id = Guid.new_guid();
		Guid binding_id = Guid.new_guid();
		string materials_key = "modified_components.#" + component_id.to_string() + ".data.materials";
		db.create(unit_id, OBJECT_TYPE_UNIT);
		db.set_resource(unit_id, "prefab", "units/prefab");
		db.create(binding_id, OBJECT_TYPE_MESH_MATERIAL);
		db.set_string(binding_id, "data.slot", "Ma");
		db.set_resource(binding_id, "data.material", "materials/Ma");
		db.add_to_set(unit_id, materials_key, binding_id);

		Database loaded = new Database(project);
		create_object_types(loaded);
		loaded.create(unit_id, OBJECT_TYPE_UNIT);
		loaded.decode_object(unit_id, GUID_ZERO, "", db.encode_object(unit_id, db.get_data(unit_id)));

		assert(!loaded.has_property(unit_id, "materials"));
		assert(loaded.has_property(unit_id, materials_key));
		Guid?[] bindings = loaded.get_set(unit_id, materials_key);
		assert(bindings.length == 1);
		assert(Guid.equal_func(bindings[0], binding_id));
		assert(loaded.get_resource(binding_id, "data.material") == "materials/Ma");
	}
}

public static int main_unit_tests()
{
	test_string();
	test_database();
	test_mesh_resource();
	return 0;
}

} /* namespace Crown */
