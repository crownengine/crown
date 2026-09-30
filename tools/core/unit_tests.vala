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

	StringId64 unit_id = STRING_ID_64("unit", 0xe0a48d0be9a7453f);
	assert(unit_id == StringId64("unit"));
	assert(unit_id.to_string() == "e0a48d0be9a7453f");
}

private static void test_sjson()
{
	stdout.printf("test_sjson\n");
	string sjson = "name = \"plain\"\nescaped = \"a\\\"b\"\n";
	try {
		for (int i = 0; i < 2; ++i) {
			GLib.HashTable<string, Value?> parsed = SJSON.decode(sjson.data[0 : sjson.length]);
			assert((string)parsed["name"] == "plain");
			assert((string)parsed["escaped"] == "a\"b");
		}
	} catch (JsonSyntaxError e) {
		assert_not_reached();
	}
}

private static void test_json()
{
	stdout.printf("test_json\n");
	string json = "{\"name\":\"plain\",\"escaped\":\"a\\\"b\"}";
	try {
		for (int i = 0; i < 2; ++i) {
			GLib.HashTable<string, Value?> parsed = (GLib.HashTable<string, Value?>)JSON.decode(json.data[0 : json.length]);
			assert((string)parsed["name"] == "plain");
			assert((string)parsed["escaped"] == "a\"b");
		}
	} catch (JsonSyntaxError e) {
		assert_not_reached();
	}
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
	{
		Database files = p.files();
		assert(files.has_type(STRING_ID_64(OBJECT_TYPE_DATABASE, 0x6d90dd26dff90856)));
		assert(files.object_definition(STRING_ID_64(OBJECT_TYPE_DATABASE, 0x6d90dd26dff90856)).length == 0);
		p.add_file("example.unit", 1, 2);
		Guid?[] file_ids = files.get_set(GUID_ZERO, files.property_index(GUID_ZERO, STRING_ID_64("data", 0x8fd0d44d20650b68)));
		assert(file_ids.length == 1);
		assert(files.object_type(file_ids[0]) == STRING_ID_64(OBJECT_TYPE_FILE, 0x63d525adf27fd749));
		assert(files.get_string(file_ids[0], files.property_index(file_ids[0], STRING_ID_64("path", 0xae70259f6415b584))) == "example.unit");
	}
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

	// Declared and undeclared keys share the object type's schema.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		PropertyDefinition[] other_props =
		{
			PropertyDefinition()
			{
				type = PropertyType.STRING, name = "name"
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL, name = "b"
			},
		};
		db.create_object_type("other", other_props);
		Guid a = Guid.new_guid();
		Guid b = Guid.new_guid();
		Guid other = Guid.new_guid();
		db.create(a, StringId64("object"));
		db.create(b, StringId64("object"));
		db.create(other, StringId64("other"));
		assert(db._data[a][0].holds(typeof(StringId64)));
		assert(db.object_type(a) == StringId64("object"));
		Value? type_value = db.get_property(a, "_type");
		assert(type_value.holds(typeof(StringId64)));
		StringId64 type_id = (StringId64)type_value;
		assert(type_id == StringId64("object"));
		GLib.HashTable<string, Value?> encoded = db.encode_object(a);
		assert((string)encoded["_type"] == "object");
		Database loaded = new Database(p);
		loaded.create_object_type("object", props);
		GLib.GenericArray<Value?> objects = new GLib.GenericArray<Value?>();
		objects.add(encoded);
		loaded.decode_set(GUID_ZERO, "objects", objects);
		assert(loaded.object_type(a) == StringId64("object"));
		type_value = loaded.get_property(a, "_type");
		type_id = (StringId64)type_value;
		assert(type_id == StringId64("object"));
		uint32 bool_index = 0;
		assert(db.find_property(ref bool_index, StringId64("object"), PropertyType.BOOL, "b"));
		assert(db.property_index(a, STRING_ID_64("b", 0xea8bfc7d922a2a37)) == bool_index);
		assert(db.property_index(other, STRING_ID_64("b", 0xea8bfc7d922a2a37)) == 5u);
		assert(db.property_index(a, STRING_ID_64("_prefab", 0xeb91306c1265f913)) == 3u);
		unowned ObjectTypeInfo? object_info = db.type_info(StringId64("object"));
		int num_properties = object_info.property_name_ids.length;
		string missing_key = "deleted_components.#" + Guid.new_guid().to_string();
		assert(db.property_index(a, StringId64(missing_key)) == uint32.MAX);
		assert(db.get_property(a, missing_key) == null);
		assert(!db.has_property(b, missing_key));
		assert((string)db.get_property(a, missing_key, "fallback") == "fallback");
		assert(db.get_string(a, db.property_index(a, StringId64(missing_key)), "fallback") == "fallback");
		assert(object_info.property_name_ids.length == num_properties);

		string key_a = "modified_components.#" + Guid.new_guid().to_string() + ".name";
		string key_b = "modified_components.#" + Guid.new_guid().to_string() + ".name";
		db.set_string(a, key_a, "a");
		assert(db.get_string(b, db.property_index(b, StringId64(key_a)), "missing") == "missing");
		db.set_string(b, key_b, "b");
		assert(db.property_index(a, StringId64(key_a)) == db.property_index(b, StringId64(key_a)));
		assert(db.property_index(a, StringId64(key_a)) != db.property_index(b, StringId64(key_b)));
		assert(db.get_string(a, db.property_index(a, StringId64(key_a))) == "a");
		assert(db.get_string(b, db.property_index(b, StringId64(key_b))) == "b");
		assert(db.get_string(b, db.property_index(b, StringId64(key_a)), "missing") == "missing");
		int num_properties_before_read = object_info.property_name_ids.length;
		assert(db.property_index(b, STRING_ID_64("unused", 0x67b793809e581fd6)) == uint32.MAX);
		assert(!db.has_property(b, "unused"));
		assert(db.get_set(b, db.property_index(b, STRING_ID_64("unused", 0x67b793809e581fd6))).length == 0);
		assert(object_info.property_name_ids.length == num_properties_before_read);
		db.set_null(b, "unused");
		assert(db._data[b].length > db.property_index(b, STRING_ID_64("unused", 0x67b793809e581fd6)));
		assert(!db.has_property(b, "unused"));

		int num_properties_before_prefab = object_info.property_name_ids.length;
		Guid instance = Guid.new_guid();
		db.create_from_prefab(instance, a);
		assert(object_info.property_name_ids.length == num_properties_before_prefab);
		assert(db.get_string(instance, db.property_index(instance, StringId64(key_a))) == "a");
		assert(db.property_index(instance, StringId64(key_a)) == db.property_index(a, StringId64(key_a)));

		Database dest = new Database(p);
		dest.create_object_type("object", props);
		Guid copy = Guid.new_guid();
		db.duplicate_one(a, copy, dest);
		assert(dest.get_string(copy, dest.property_index(copy, StringId64(key_a))) == "a");
		assert(dest.object_type(copy) == db.object_type(a));
		type_value = dest.get_property(copy, "_type");
		type_id = (StringId64)type_value;
		assert(type_id == StringId64("object"));
		Database deep = new Database(p);
		deep.create_object_type("object", props);
		db.copy_deep(deep, a, "");
		assert(deep.object_type(a) == db.object_type(a));

		Guid cleared = Guid.new_guid();
		Guid cleared_copy = Guid.new_guid();
		db.create(cleared, StringId64("object"));
		db.set_null(cleared, "s");
		db.duplicate_one(cleared, cleared_copy, dest);
		assert(!dest.has_property(cleared_copy, "s"));
		assert(dest.get_property(cleared_copy, "set") != null);
		assert(dest.get_set(cleared_copy, dest.property_index(cleared_copy, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 0);
	}
	{
		Database db = new Database(p);
		PropertyDefinition[] late_props =
		{
			PropertyDefinition()
			{
				type = PropertyType.BOOL, name = "b"
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING, name = "name"
			},
		};
		StringId64 late_type = db.create_object_type("late", late_props);
		Guid id = Guid.new_guid();
		db.create(id, late_type);
		db.set_string(id, "dynamic", "value");
		db.set_bool(id, "b", true);
		assert(db.object_type(id) == StringId64("late"));
		assert(db.type_name(db.object_type(id)) == "late");
		assert(db.property_index(id, STRING_ID_64("dynamic", 0x3995d0559a810e3a)) == 6u);
		assert(db.property_index(id, STRING_ID_64("b", 0xea8bfc7d922a2a37)) == 4u);
		assert(db.get_bool(id, 4u));
		assert(db.get_string(id, db.property_index(id, STRING_ID_64("dynamic", 0x3995d0559a810e3a))) == "value");
		unowned ObjectTypeInfo? info = db.type_info(StringId64("late"));
		assert(info.property_names.length == info.property_name_ids.length);
		assert(info.property_names.length == info.property_definitions.length);
		assert(info.property_name_ids[2] == StringId64("dynamic"));
		assert(info.property_names[2] == "dynamic" && info.property_definitions[2].declaration_order == -1);
		assert(info.property_names[0] == "b" && info.property_definitions[0].type == PropertyType.BOOL);
		uint32 dynamic_index = 0;
		assert(!db.find_property(ref dynamic_index, StringId64("late"), PropertyType.BOOL, "dynamic"));
		unowned PropertyDefinition[] declared = db.object_definition(StringId64("late"));
		assert(declared.length == 2);
		assert(declared[0].name == "b" && declared[1].name == "name");
	}
	{
		Database db = new Database(p);
		PropertyDefinition[] unit_props =
		{
			PropertyDefinition()
			{
				type = PropertyType.OBJECTS_SET, name = "children", object_type = StringId64("unit")
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING, name = "name"
			},
		};
		db.create_object_type("unit", unit_props);
		Guid root_id = Guid.new_guid();
		Guid child_id = Guid.new_guid();
		GLib.HashTable<string, Value?> child = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		child["_guid"] = child_id.to_string();
		child["_type"] = "unit";
		child["dynamic"] = "child";
		GLib.GenericArray<Value?> children = new GLib.GenericArray<Value?>();
		children.add(child);
		GLib.HashTable<string, Value?> root = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		root["_guid"] = root_id.to_string();
		root["_type"] = "unit";
		root["children"] = children;
		root["name"] = "root";
		GLib.GenericArray<Value?> roots = new GLib.GenericArray<Value?>();
		roots.add(root);
		db.decode_set(GUID_ZERO, "objects", roots);
		assert(db.get_string(root_id, db.property_index(root_id, STRING_ID_64("name", 0xd4c943cba60c270b))) == "root");
		assert(db.get_string(child_id, db.property_index(child_id, STRING_ID_64("dynamic", 0x3995d0559a810e3a))) == "child");
	}
	{
		UndoRedo undo_redo = new UndoRedo();
		Database db = new Database(p, undo_redo);
		db.create_object_type("object", props);
		db.create_object_type("other", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));
		undo_redo.reset();

		db.set_string(id, "dynamic", "value");
		db.add_restore_point(ActionType.CHANGE_OBJECTS, { id });
		uint32 dynamic_property = db.property_index(id, STRING_ID_64("dynamic", 0x3995d0559a810e3a));
		db.set_string(id, "_type", "other");
		db.add_restore_point(ActionType.CHANGE_OBJECTS, { id });
		assert(db.object_type(id) == StringId64("other"));
		Value? type_value = db.get_property(id, "_type");
		StringId64 type_id = (StringId64)type_value;
		assert(type_id == StringId64("other"));
		db.undo();
		assert(db.object_type(id) == StringId64("object"));
		db.undo();
		assert(db.get_property(id, "dynamic") == null);
		db.redo();
		assert(db.get_string(id, dynamic_property) == "value");
		db.redo();
		assert(db.object_type(id) == StringId64("other"));
	}
	{
		UndoRedo undo_redo = new UndoRedo();
		Database db = new Database(p, undo_redo);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		Guid child = Guid.new_guid();
		db.create(id, StringId64("object"));
		db.create(child, StringId64("object"));
		undo_redo.reset();

		db.add_to_set(id, "dynamic_set", child);
		db.add_restore_point(ActionType.CHANGE_OBJECTS, { id, child });
		uint32 property = db.property_index(id, StringId64("dynamic_set"));
		assert(db.get_set(id, property).length == 1);
		db.undo();
		assert(db.get_set(id, property).length == 0);
		db.redo();
		assert(db.get_set(id, property).length == 1);

		db.remove_from_set(id, "dynamic_set", child);
		db.add_restore_point(ActionType.CHANGE_OBJECTS, { id, child });
		assert(db.get_set(id, property).length == 0);
		db.undo();
		assert(db.get_set(id, property).length == 1);
		db.redo();
		assert(db.get_set(id, property).length == 0);
	}
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		Guid child = Guid.new_guid();
		db.create(id, StringId64("object"));
		db.create(child, StringId64("object"));
		db.add_to_set(id, "set", child);
		Value? set_value = db.get_property(id, "set");
		((GLib.GenericSet<Guid?>)set_value).remove(child);
		assert(db.get_set(id, db.property_index(id, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 0);
	}

	// Read property defaults.
	// Read a bool property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		assert(db.get_bool(id, db.property_index(id, STRING_ID_64("b", 0xea8bfc7d922a2a37))) == true);
	}

	// Read a double property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		assert(db.get_double(id, db.property_index(id, STRING_ID_64("d", 0x17dffbc5a8f17839))) == 1.0);
	}

	// Read a string property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		assert(db.get_string(id, db.property_index(id, STRING_ID_64("s", 0xe5db19474a903141))) == "a");
	}

	// Read a vector property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		assert(Vector3.equal_func(db.get_vector3(id, db.property_index(id, STRING_ID_64("v", 0x039e135b4a2b31d2))), Vector3(1.0, 2.0, 3.0)));
	}

	// Read a quaternion property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		assert(Quaternion.equal_func(db.get_quaternion(id, db.property_index(id, STRING_ID_64("q", 0x7af8d99b413c664f))), Quaternion(1.0, 2.0, 3.0, 4.0)));
	}

	// Read a resource property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		assert(db.get_resource(id, db.property_index(id, STRING_ID_64("r", 0xeb9e71988f8c8e3d))) == "a");
	}

	// Read a reference property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		assert(Guid.equal_func(db.get_reference(id, db.property_index(id, STRING_ID_64("ref", 0x83fe77400dff8939))), GUID_ZERO));
	}

	// Read an objects set property default.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		assert(db.get_set(id, db.property_index(id, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 0);
	}

	// Write each property type.
	// Write a bool property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		db.set_bool(id, "b", false);
		assert(db.get_bool(id, db.property_index(id, STRING_ID_64("b", 0xea8bfc7d922a2a37))) == false);
	}

	// Write a double property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		db.set_double(id, "d", 2.0);
		assert(db.get_double(id, db.property_index(id, STRING_ID_64("d", 0x17dffbc5a8f17839))) == 2.0);
	}

	// Write a string property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		db.set_string(id, "s", "b");
		assert(db.get_string(id, db.property_index(id, STRING_ID_64("s", 0xe5db19474a903141))) == "b");
	}

	// Write a vector property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		db.set_vector3(id, "v", Vector3(4.0, 5.0, 6.0));
		assert(Vector3.equal_func(db.get_vector3(id, db.property_index(id, STRING_ID_64("v", 0x039e135b4a2b31d2))), Vector3(4.0, 5.0, 6.0)));
	}

	// Write a quaternion property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		db.set_quaternion(id, "q", Quaternion(5.0, 6.0, 7.0, 8.0));
		assert(Quaternion.equal_func(db.get_quaternion(id, db.property_index(id, STRING_ID_64("q", 0x7af8d99b413c664f))), Quaternion(5.0, 6.0, 7.0, 8.0)));
	}

	// Write a resource property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		db.set_resource(id, "r", "b");
		assert(db.get_resource(id, db.property_index(id, STRING_ID_64("r", 0xeb9e71988f8c8e3d))) == "b");
	}

	// Write a reference property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));
		Guid to = Guid.new_guid();
		db.create(to, StringId64("object"));

		db.set_reference(id, "ref", to);
		assert(Guid.equal_func(db.get_reference(id, db.property_index(id, STRING_ID_64("ref", 0x83fe77400dff8939))), to));
	}

	// Write a null resource.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		db.set_resource(id, "r", null);
		assert(db.get_resource(id, db.property_index(id, STRING_ID_64("r", 0xeb9e71988f8c8e3d))) == null);
	}

	// Replace a string-backed resource value.
	{
		UndoRedo undo_redo = new UndoRedo();
		Database db = new Database(p, undo_redo);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		UndoRedo? ur = db.disable_undo();
		db.set_string(id, "r", "a");
		db.restore_undo(ur);
		undo_redo.reset();

		db.set_resource(id, "r", "b");
		db.add_restore_point(ActionType.CHANGE_OBJECTS, { id });
		assert(db.get_resource(id, db.property_index(id, STRING_ID_64("r", 0xeb9e71988f8c8e3d))) == "b");

		db.undo();
		assert(db.get_resource(id, db.property_index(id, STRING_ID_64("r", 0xeb9e71988f8c8e3d))) == "a");

		db.redo();
		assert(db.get_resource(id, db.property_index(id, STRING_ID_64("r", 0xeb9e71988f8c8e3d))) == "b");
	}

	// Add and remove an object from a set.
	// Add an object to a set.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));

		db.add_to_set(root, "set", child);

		Guid?[] ids = db.get_set(root, db.property_index(root, STRING_ID_64("set", 0x237afba9ce4e06bf)));
		assert(ids.length == 1);
		assert(Guid.equal_func(ids[0], child));
		assert(Guid.equal_func(db.owner(child), root));
	}

	// Remove an object from a set.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		db.add_to_set(root, "set", child);

		db.remove_from_set(root, "set", child);

		assert(db.get_set(root, db.property_index(root, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 0);
		assert(Guid.equal_func(db.owner(child), GUID_ZERO));
	}

	// Ensure sets are unique.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));

		db.add_to_set(root, "set", child);
		db.add_to_set(root, "set", child);

		assert(db.get_set(root, db.property_index(root, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 1);
	}

	// Skip dead objects in sets.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		db.add_to_set(root, "set", child);

		db.destroy(child);

		assert(db.get_set(root, db.property_index(root, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 0);
	}

	// Destroy an object.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));

		db.destroy(id);

		assert(db.has_object(id));
		assert(!db.is_alive(id));
		assert(db.object_type(id) == StringId64("object"));
	}

	// Destroy descendants recursively.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
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
		db.create(a, StringId64("object"));
		Guid b = Guid.new_guid();
		db.create(b, StringId64("object"));

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
		db.create(root, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		Guid dead = Guid.new_guid();
		db.create(dead, StringId64("object"));
		Guid copy = Guid.new_guid();
		db.add_to_set(root, "set", child);
		db.add_to_set(root, "set", dead);
		db.destroy(dead);

		db.duplicate_one(root, copy);

		assert(db._data.size() == 6);
		assert(db.get_set(copy, db.property_index(copy, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 1);
	}

	// Remap internal references when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		Guid copy = Guid.new_guid();
		db.add_to_set(root, "set", child);
		db.set_reference(root, "ref", child);

		db.duplicate_one(root, copy);

		Guid?[] ids = db.get_set(copy, db.property_index(copy, STRING_ID_64("set", 0x237afba9ce4e06bf)));
		assert(ids.length == 1);
		assert(Guid.equal_func(db.get_reference(copy, db.property_index(copy, STRING_ID_64("ref", 0x83fe77400dff8939))), ids[0]));
	}

	// Preserve external references when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));
		Guid to = Guid.new_guid();
		db.create(to, StringId64("object"));
		Guid copy = Guid.new_guid();
		db.set_reference(id, "ref", to);

		db.duplicate_one(id, copy);

		assert(Guid.equal_func(db.get_reference(copy, db.property_index(copy, STRING_ID_64("ref", 0x83fe77400dff8939))), to));
	}

	// Remap references between duplicated roots.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid a = Guid.new_guid();
		db.create(a, StringId64("object"));
		Guid b = Guid.new_guid();
		db.create(b, StringId64("object"));
		Guid ca = Guid.new_guid();
		Guid cb = Guid.new_guid();
		db.set_reference(a, "ref", b);
		db.set_reference(b, "ref", a);

		db.duplicate({ a, b }, { ca, cb });

		assert(Guid.equal_func(db.get_reference(ca, db.property_index(ca, STRING_ID_64("ref", 0x83fe77400dff8939))), cb));
		assert(Guid.equal_func(db.get_reference(cb, db.property_index(cb, STRING_ID_64("ref", 0x83fe77400dff8939))), ca));
	}

	// Add duplicated roots to their set.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, StringId64("object"));
		Guid a = Guid.new_guid();
		db.create(a, StringId64("object"));
		Guid b = Guid.new_guid();
		db.create(b, StringId64("object"));
		Guid ca = Guid.new_guid();
		Guid cb = Guid.new_guid();
		db.add_to_set(root, "set", a);
		db.add_to_set(root, "set", b);

		db.duplicate_and_add_to_set({ a, b }, { ca, cb });

		Guid?[] ids = db.get_set(root, db.property_index(root, STRING_ID_64("set", 0x237afba9ce4e06bf)));
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
			db.create(root, StringId64("object"));
			Guid a = Guid.new_guid();
			db.create(a, StringId64("object"));
			db.add_to_set(root, "set", a);
			Guid parent = a;
			if (depth == 2) {
				parent = Guid.new_guid();
				db.create(parent, StringId64("object"));
				db.add_to_set(a, "set", parent);
			}
			Guid b = Guid.new_guid();
			db.create(b, StringId64("object"));
			db.add_to_set(parent, "set", b);
			Guid ca = Guid.new_guid();
			Guid cb = Guid.new_guid();

			if (reverse == 0)
				db.duplicate_and_add_to_set({ a, b }, { ca, cb });
			else
				db.duplicate_and_add_to_set({ b, a }, { cb, ca });

			assert(db.get_set(root, db.property_index(root, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 2);
			assert(contains_guid(db.get_set(root, db.property_index(root, STRING_ID_64("set", 0x237afba9ce4e06bf))), a));
			assert(contains_guid(db.get_set(root, db.property_index(root, STRING_ID_64("set", 0x237afba9ce4e06bf))), ca));
			assert(db.get_set(parent, db.property_index(parent, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 1);
			assert(contains_guid(db.get_set(parent, db.property_index(parent, STRING_ID_64("set", 0x237afba9ce4e06bf))), b));
			assert(Guid.equal_func(db.owner(b), parent));
			assert(db.get_set(ca, db.property_index(ca, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 1);
			Guid copied_parent = ca;
			if (depth == 2) {
				assert(db.get_set(a, db.property_index(a, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 1);
				assert(contains_guid(db.get_set(a, db.property_index(a, STRING_ID_64("set", 0x237afba9ce4e06bf))), parent));
				copied_parent = db.get_set(ca, db.property_index(ca, STRING_ID_64("set", 0x237afba9ce4e06bf)))[0];
				assert(!Guid.equal_func(copied_parent, parent));
				assert(Guid.equal_func(db.owner(copied_parent), ca));
			}
			assert(db.get_set(copied_parent, db.property_index(copied_parent, STRING_ID_64("set", 0x237afba9ce4e06bf))).length == 1);
			assert(contains_guid(db.get_set(copied_parent, db.property_index(copied_parent, STRING_ID_64("set", 0x237afba9ce4e06bf))), cb));
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
		db.create(id, StringId64("object"));
		Guid copy = Guid.new_guid();
		db.set_bool(id, "b", false);

		db.duplicate_one(id, copy);

		assert(db.get_bool(copy, db.property_index(copy, STRING_ID_64("b", 0xea8bfc7d922a2a37))) == false);
	}

	// Copy a double property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));
		Guid copy = Guid.new_guid();
		db.set_double(id, "d", 2.0);

		db.duplicate_one(id, copy);

		assert(db.get_double(copy, db.property_index(copy, STRING_ID_64("d", 0x17dffbc5a8f17839))) == 2.0);
	}

	// Copy a string property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));
		Guid copy = Guid.new_guid();
		db.set_string(id, "s", "b");

		db.duplicate_one(id, copy);

		assert(db.get_string(copy, db.property_index(copy, STRING_ID_64("s", 0xe5db19474a903141))) == "b");
	}

	// Copy a vector property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));
		Guid copy = Guid.new_guid();
		db.set_vector3(id, "v", Vector3(4.0, 5.0, 6.0));

		db.duplicate_one(id, copy);

		assert(Vector3.equal_func(db.get_vector3(copy, db.property_index(copy, STRING_ID_64("v", 0x039e135b4a2b31d2))), Vector3(4.0, 5.0, 6.0)));
	}

	// Copy a quaternion property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));
		Guid copy = Guid.new_guid();
		db.set_quaternion(id, "q", Quaternion(5.0, 6.0, 7.0, 8.0));

		db.duplicate_one(id, copy);

		assert(Quaternion.equal_func(db.get_quaternion(copy, db.property_index(copy, STRING_ID_64("q", 0x7af8d99b413c664f))), Quaternion(5.0, 6.0, 7.0, 8.0)));
	}

	// Copy a resource property when duplicating.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid id = Guid.new_guid();
		db.create(id, StringId64("object"));
		Guid copy = Guid.new_guid();
		db.set_resource(id, "r", "b");

		db.duplicate_one(id, copy);

		assert(db.get_resource(copy, db.property_index(copy, STRING_ID_64("r", 0xeb9e71988f8c8e3d))) == "b");
	}

	// Reuse supplied IDs when duplicated roots overlap.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid root = Guid.new_guid();
		db.create(root, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		Guid copy = Guid.new_guid();
		Guid cc = Guid.new_guid();
		db.add_to_set(root, "set", child);

		db.duplicate({ root, child }, { copy, cc });

		Guid?[] ids = db.get_set(copy, db.property_index(copy, STRING_ID_64("set", 0x237afba9ce4e06bf)));
		assert(db._data.size() == 5);
		assert(ids.length == 1);
		assert(Guid.equal_func(ids[0], cc));
	}

	// Decode unit children only once.
	{
		Database db = new Database(p);
		create_object_types(db);
		Guid root = Guid.new_guid();
		db.create(root, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));

		GLib.HashTable<string, Value?> child = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		child["_type"] = OBJECT_TYPE_UNIT;
		child["children"] = new GLib.GenericArray<Value?>();

		GLib.GenericArray<Value?> children = new GLib.GenericArray<Value?>();
		children.add(child);
		GLib.HashTable<string, Value?> json = new GLib.HashTable<string, Value?>(GLib.str_hash, GLib.str_equal);
		json["_type"] = OBJECT_TYPE_UNIT;
		json["children"] = children;

		db.decode_object(root, GUID_ZERO, "", json);

		assert(db.get_set(root, db.property_index(root, STRING_ID_64("children", 0x6fbb13de0e1dce0d))).length == 1);
		assert(db._data.size() == 3);
	}

	// Instances inherit ordinary properties and merge object sets by prefab ID.
	// Inherit an ordinary property.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, StringId64("object"));
		Guid inst = Guid.new_guid();
		db.create_from_prefab(inst, src);

		assert(db._data[inst].length <= db.property_index(inst, STRING_ID_64("s", 0xe5db19474a903141)) || db._data[inst][db.property_index(inst, STRING_ID_64("s", 0xe5db19474a903141))] == null);
		db.set_string(src, "s", "from source");
		assert(db.get_string(inst, db.property_index(inst, STRING_ID_64("s", 0xe5db19474a903141))) == "from source");
	}

	// Preserve a local property override.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, StringId64("object"));
		Guid inst = Guid.new_guid();
		db.set_string(src, "s", "source before");
		db.create_from_prefab(inst, src);

		db.set_string(inst, "s", "local");
		db.set_string(src, "s", "source after");

		assert(db.get_string(inst, db.property_index(inst, STRING_ID_64("s", 0xe5db19474a903141))) == "local");
	}

	// Clear a local property override.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, StringId64("object"));
		Guid inst = Guid.new_guid();
		db.set_bool(src, "b", false);
		db.create_from_prefab(inst, src);
		db.set_bool(inst, "b", true);

		db.set_null(inst, "b");

		assert(db.get_bool(inst, db.property_index(inst, STRING_ID_64("b", 0xea8bfc7d922a2a37))) == false);
	}

	// Preserve inheritance when duplicating an instance.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, StringId64("object"));
		Guid inst = Guid.new_guid();
		Guid copy = Guid.new_guid();
		db.create_from_prefab(inst, src);
		db.set_string(src, "s", "before duplication");

		db.duplicate_one(inst, copy);

		assert(db._data[copy].length <= db.property_index(copy, STRING_ID_64("s", 0xe5db19474a903141)) || db._data[copy][db.property_index(copy, STRING_ID_64("s", 0xe5db19474a903141))] == null);
		db.set_string(src, "s", "after duplication");
		assert(db.get_string(copy, db.property_index(copy, STRING_ID_64("s", 0xe5db19474a903141))) == "after duplication");
	}

	// Inherit object set members.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, StringId64("object"));
		Guid a = Guid.new_guid();
		db.create(a, StringId64("object"));
		Guid b = Guid.new_guid();
		db.create(b, StringId64("object"));
		Guid inst = Guid.new_guid();
		db.add_to_set(src, "set", a);
		db.add_to_set(src, "set", b);

		db.create_from_prefab(inst, src);

		Guid?[] members = db.get_set(inst, db.property_index(inst, STRING_ID_64("set", 0x237afba9ce4e06bf)));
		assert(members.length == 2);
		assert(contains_guid(members, a));
		assert(contains_guid(members, b));
	}

	// Replace an inherited set member by prefab ID.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		Guid other = Guid.new_guid();
		db.create(other, StringId64("object"));
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.add_to_set(src, "set", other);
		db.create_from_prefab(inst, src);
		db.create_from_prefab(repl, child);

		db.add_to_set(inst, "set", repl);

		Guid?[] members = db.get_set(inst, db.property_index(inst, STRING_ID_64("set", 0x237afba9ce4e06bf)));
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
		db.create(src, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		Guid other = Guid.new_guid();
		db.create(other, StringId64("object"));
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.add_to_set(src, "set", other);
		db.create_from_prefab(inst, src);
		db.create_from_prefab(repl, child);
		db.add_to_set(inst, "set", repl);

		GLib.HashTable<string, Value?> encoded = db.encode_object(inst);

		Database loaded = new Database(p);
		loaded.create_object_type("object", props);
		loaded.create(src, StringId64("object"));
		loaded.create(child, StringId64("object"));
		loaded.create(other, StringId64("object"));
		loaded.add_to_set(src, "set", child);
		loaded.add_to_set(src, "set", other);
		loaded.create_from_prefab(inst, src);
		loaded.decode_object(inst, GUID_ZERO, "", encoded);

		Guid?[] members = loaded.get_set(inst, loaded.property_index(inst, STRING_ID_64("set", 0x237afba9ce4e06bf)));
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
		db.create(src, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.create_from_prefab(inst, src);
		db.create_from_prefab(repl, child);
		db.set_double(repl, "d", 7.0);
		db.add_to_set(inst, "set", repl);

		GLib.HashTable<string, Value?> encoded = db.encode_object(inst);

		Database loaded = new Database(p);
		loaded.create_object_type("object", props);
		loaded.create(src, StringId64("object"));
		loaded.create(child, StringId64("object"));
		loaded.add_to_set(src, "set", child);
		loaded.create_from_prefab(inst, src);
		loaded.decode_object(inst, GUID_ZERO, "", encoded);

		assert(loaded.get_double(repl, loaded.property_index(repl, STRING_ID_64("d", 0x17dffbc5a8f17839))) == 7.0);
	}

	// Undo and redo an inherited set member replacement.
	{
		UndoRedo undo_redo = new UndoRedo();
		Database db = new Database(p, undo_redo);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		Guid other = Guid.new_guid();
		db.create(other, StringId64("object"));
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
		Guid?[] members = db.get_set(inst, db.property_index(inst, STRING_ID_64("set", 0x237afba9ce4e06bf)));
		assert(contains_guid(members, child));
		assert(!contains_guid(members, repl));

		db.redo();
		members = db.get_set(inst, db.property_index(inst, STRING_ID_64("set", 0x237afba9ce4e06bf)));
		assert(contains_guid(members, repl));
		assert(!contains_guid(members, child));
	}

	// Inherit members added to the source set.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		Guid inst = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.create_from_prefab(inst, src);

		Guid added = Guid.new_guid();
		db.create(added, StringId64("object"));
		db.add_to_set(src, "set", added);

		Guid?[] members = db.get_set(inst, db.property_index(inst, STRING_ID_64("set", 0x237afba9ce4e06bf)));
		assert(members.length == 2);
		assert(contains_guid(members, child));
		assert(contains_guid(members, added));
	}

	// Remove an instance member when its source member is removed.
	{
		Database db = new Database(p);
		db.create_object_type("object", props);
		Guid src = Guid.new_guid();
		db.create(src, StringId64("object"));
		Guid child = Guid.new_guid();
		db.create(child, StringId64("object"));
		Guid other = Guid.new_guid();
		db.create(other, StringId64("object"));
		Guid inst = Guid.new_guid();
		Guid repl = Guid.new_guid();
		db.add_to_set(src, "set", child);
		db.add_to_set(src, "set", other);
		db.create_from_prefab(inst, src);
		db.create_from_prefab(repl, child);
		db.add_to_set(inst, "set", repl);

		db.remove_from_set(src, "set", child);

		Guid?[] members = db.get_set(inst, db.property_index(inst, STRING_ID_64("set", 0x237afba9ce4e06bf)));
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

private static void test_duplicate_unit_tree()
{
	stdout.printf("test_duplicate_unit_tree\n");

	for (int reverse = 0; reverse < 2; ++reverse) {
		Database db = new Database(new Project());
		Level level = new Level(db, new RuntimeInstance("test", null));
		create_object_types(db);
		DatabaseEditor editor = new DatabaseEditor(1024 * 1024, db);
		Guid a = Guid.new_guid();
		db.create(a, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		Guid middle = Guid.new_guid();
		db.create(middle, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.add_to_set(a, "children", middle);
		Guid b = Guid.new_guid();
		db.create(b, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.add_to_set(middle, "children", b);
		Guid sound = Guid.new_guid();
		db.create(sound, STRING_ID_64(OBJECT_TYPE_SOUND_SOURCE, 0xbe0fa879e7a28684));
		db.add_restore_point((int)ActionType.CREATE_OBJECTS, { a, sound });

		StringBuilder commands = new StringBuilder();
		Guid?[] created = {};
		db.objects_created.connect((ids, flags) => {
				created = ids;
				level.generate_spawn_objects(commands, ids);
			});
		if (reverse == 0)
			editor.selection_set({ a, sound, b });
		else
			editor.selection_set({ b, sound, a });
		editor._action_group.activate_action("duplicate", null);

		Guid ca = editor._selection[reverse == 0 ? 0 : 2];
		Guid cb = editor._selection[reverse == 0 ? 2 : 0];
		Guid cs = editor._selection[1];
		Guid cm = db.get_set(ca, db.property_index(ca, STRING_ID_64("children", 0x6fbb13de0e1dce0d)))[0];
		assert(editor._selection.length == 3);
		assert(created.length == 3);
		assert(contains_guid(created, ca));
		assert(contains_guid(created, cb));
		assert(contains_guid(created, cs));
		foreach (Guid id in new Guid[] { ca, cm, cb }) {
			string spawn = LevelEditorApi.spawn_empty_unit(id);
			assert(commands.str.split(spawn).length == 2);
		}
		assert(commands.str.index_of(LevelEditorApi.spawn_empty_unit(ca))
			< commands.str.index_of(LevelEditorApi.spawn_empty_unit(cb)));

		commands.truncate(0);
		level.generate_spawn_objects(commands, { cb });
		assert(commands.str.split(LevelEditorApi.spawn_empty_unit(cb)).length == 2);
		assert(!commands.str.contains(LevelEditorApi.spawn_empty_unit(ca)));

		assert(db.undo() == (int)ActionType.CREATE_OBJECTS);
		assert(!db.is_alive(ca) && !db.is_alive(cm) && !db.is_alive(cb));
		assert(db.is_alive(a) && db.is_alive(middle) && db.is_alive(b));
		commands.truncate(0);
		created = {};
		assert(db.redo() == (int)ActionType.CREATE_OBJECTS);
		assert(created.length == 3);
		assert(contains_guid(created, ca));
		assert(contains_guid(created, cb));
		assert(contains_guid(created, cs));
		assert(db.is_alive(ca) && db.is_alive(cm) && db.is_alive(cb));
		foreach (Guid id in new Guid[] { ca, cm, cb })
			assert(commands.str.split(LevelEditorApi.spawn_empty_unit(id)).length == 2);
	}
}

private static void test_component_type_dependencies()
{
	Database db = new Database(new Project());
	create_object_types(db);
	GLib.HashTable<StringId64?, GLib.GenericArray<StringId64?>> previous_registry = Unit._component_registry;
	Unit._component_registry = null;
	StringId64 collider_type = STRING_ID_64(OBJECT_TYPE_COLLIDER, 0x9a9fe4362129d74e);
	StringId64 actor_type = STRING_ID_64(OBJECT_TYPE_ACTOR, 0xac2b738a374cf583);
	Unit.register_component_type(collider_type, "");
	Unit.register_component_type(actor_type, OBJECT_TYPE_COLLIDER);
	Guid id = Guid.new_guid();
	db.create(id, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
	Unit unit = Unit(db, id);
	assert(unit.add_component_type_dependencies(actor_type));
	Guid component_id;
	assert(unit.has_component(out component_id, collider_type));
	assert(unit.has_component(out component_id, actor_type));
	assert(!unit.add_component_type_dependencies(actor_type));
	Unit._component_registry = previous_registry;
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
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));

		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		MeshResource.set_material_slot(db, comp, "Mb", "materials/Mb");

		Guid?[] members = db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)));
		assert(members.length == 2);
		Guid ma = db.get_string(members[0], db.property_index(members[0], STRING_ID_64("data.slot", 0x0de060e1cd2f27fe))) == "Ma" ? members[0] : members[1];
		Guid mb = Guid.equal_func(ma, members[0]) ? members[1] : members[0];
		assert(db.get_resource(ma, db.property_index(ma, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "materials/Ma");
		assert(db.get_resource(mb, db.property_index(mb, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "materials/Mb");
	}

	// Update an existing mesh material slot without changing its identity.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid comp = Guid.new_guid();
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		Guid binding = db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)))[0];

		MeshResource.set_material_slot(db, comp, "Ma", "materials/Mc");

		Guid?[] members = db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)));
		assert(members.length == 1);
		assert(Guid.equal_func(members[0], binding));
		assert(db.get_resource(binding, db.property_index(binding, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "materials/Mc");
	}

	// Undo and redo a mesh material slot update.
	{
		Project p = new Project();
		UndoRedo undo_redo = new UndoRedo();
		Database db = new Database(p, undo_redo);
		create_object_types(db);
		Guid comp = Guid.new_guid();
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		Guid binding = db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)))[0];
		undo_redo.reset();

		MeshResource.set_material_slot(db, comp, "Ma", "materials/Mc");
		db.add_restore_point(ActionType.CHANGE_OBJECTS, { binding });
		assert(db.get_resource(binding, db.property_index(binding, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "materials/Mc");

		db.undo();
		assert(db.get_resource(binding, db.property_index(binding, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "materials/Ma");

		db.redo();
		assert(db.get_resource(binding, db.property_index(binding, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "materials/Mc");
	}

	// Generate mesh setup commands before material assignment commands.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid unit = Guid.new_guid();
		Guid comp = Guid.new_guid();
		db.create(unit, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
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
		db.create(unit, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
		db.add_to_set(unit, "components", comp);
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		MeshResource.set_material_slot(db, comp, "Mb", "materials/Mb");
		Guid?[] members = db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)));
		Guid ma = db.get_string(members[0], db.property_index(members[0], STRING_ID_64("data.slot", 0x0de060e1cd2f27fe))) == "Ma" ? members[0] : members[1];

		Database loaded = new Database(p);
		create_object_types(loaded);
		loaded.create(unit, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		loaded.decode_object(unit, GUID_ZERO, "", db.encode_object(unit));

		assert(loaded.get_set(comp, loaded.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d))).length == 2);
		assert(loaded.get_resource(ma, loaded.property_index(ma, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "materials/Ma");
	}

	// Omit a destroyed mesh material slot from destroy commands.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid unit = Guid.new_guid();
		Guid comp = Guid.new_guid();
		db.create(unit, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
		db.add_to_set(unit, "components", comp);
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		MeshResource.set_material_slot(db, comp, "Mb", "materials/Mb");
		Guid?[] members = db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)));
		Guid ma = db.get_string(members[0], db.property_index(members[0], STRING_ID_64("data.slot", 0x0de060e1cd2f27fe))) == "Ma" ? members[0] : members[1];

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
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
		MeshResource.set_material_slot(db, comp, "Ma", "materials/Ma");
		MeshResource.set_material_slot(db, comp, "Mb", "materials/Mb");
		Guid binding = db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)))[0];
		undo_redo.reset();

		db.destroy(binding);
		db.add_restore_point(ActionType.DESTROY_OBJECTS, { binding });
		assert(db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d))).length == 1);

		db.undo();
		assert(db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d))).length == 2);

		db.redo();
		assert(db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d))).length == 1);
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
		db.create(unit, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.set_resource(unit, "prefab", "units/prefab");
		db.create(binding, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
		db.set_string(binding, "data.slot", "Ma");
		db.set_resource(binding, "data.material", "materials/Ma");
		db.add_to_set(unit, key, binding);

		Database loaded = new Database(p);
		create_object_types(loaded);
		loaded.create(unit, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		loaded.decode_object(unit, GUID_ZERO, "", db.encode_object(unit));

		assert(!loaded.has_property(unit, "materials"));
		assert(loaded.has_property(unit, key));
		Guid?[] members = loaded.get_set(unit, loaded.property_index(unit, StringId64(key)));
		assert(members.length == 1);
		assert(Guid.equal_func(members[0], binding));
		assert(loaded.get_resource(binding, loaded.property_index(binding, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "materials/Ma");
	}

	// Give legacy whole-mesh materials a stable binding identity.
	{
		Project p = new Project();
		Guid src = Guid.parse("0cc80c3d-089a-43ad-b145-77c504e2ef50");
		Guid comp = Guid.parse("49753fff-34ea-4aff-8bcf-5116f1c1519a");
		Database first = new Database(p);
		create_object_types(first);
		first.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		first.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		Guid a = first.get_set(comp, first.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)))[0];

		Database second = new Database(p);
		create_object_types(second);
		second.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		second.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		Guid b = second.get_set(comp, second.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)))[0];

		assert(Guid.equal_func(b, a));
	}

	// Migrate a sparse legacy whole-mesh material override.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.parse("0cc80c3d-089a-43ad-b145-77c504e2ef50");
		Guid comp = Guid.parse("49753fff-34ea-4aff-8bcf-5116f1c1519a");
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		Guid child = db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)))[0];
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		Guid inst = Guid.new_guid();
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
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
		assert(Guid.equal_func(db.get_reference(repl, db.property_index(repl, STRING_ID_64("_prefab", 0xeb91306c1265f913))), child));
		assert(db.get_string(repl, db.property_index(repl, STRING_ID_64("data.slot", 0x0de060e1cd2f27fe))) == "default");
		assert(db.get_resource(repl, db.property_index(repl, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "materials/instance");
	}

	// Collapse duplicate legacy material overrides onto their source binding.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.parse("0cc80c3d-089a-43ad-b145-77c504e2ef50");
		Guid comp = Guid.parse("49753fff-34ea-4aff-8bcf-5116f1c1519a");
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		Guid child = db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)))[0];
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		Guid inst = Guid.parse("0444b593-d55c-49c2-86aa-eea58388eea6");
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
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
		assert(Guid.equal_func(db.get_reference(repl, db.property_index(repl, STRING_ID_64("_prefab", 0xeb91306c1265f913))), child));
		assert(Guid.equal_func(db.get_reference(other, db.property_index(other, STRING_ID_64("_prefab", 0xeb91306c1265f913))), child));
		foreach (unowned Guid? id in members) {
			assert(db.get_string(id, db.property_index(id, STRING_ID_64("data.slot", 0x0de060e1cd2f27fe))) == "default");
			assert(db.get_resource(id, db.property_index(id, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "units/beach/materials/M_MountainBike_Variant02");
		}
	}

	// Ignore unsupported full material overrides without _prefab.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.parse("0cc80c3d-089a-43ad-b145-77c504e2ef50");
		Guid comp = Guid.parse("49753fff-34ea-4aff-8bcf-5116f1c1519a");
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		Guid child = db.get_set(comp, db.property_index(comp, STRING_ID_64("data.materials", 0xb4c01840c957402d)))[0];
		db.set_reference(GUID_ZERO, "core/units/primitives/cube.unit", src);
		Guid inst = Guid.new_guid();
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.set_resource(inst, "prefab", "core/units/primitives/cube");
		Guid binding = Guid.new_guid();
		db.create(binding, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
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
		assert(Guid.equal_func(db.get_reference(binding, db.property_index(binding, STRING_ID_64("_prefab", 0xeb91306c1265f913))), GUID_ZERO));
	}

	// Exclude unsupported full material overrides from generated commands.
	{
		Project p = new Project();
		Database db = new Database(p);
		create_object_types(db);
		Guid src = Guid.parse("0cc80c3d-089a-43ad-b145-77c504e2ef50");
		Guid comp = Guid.parse("49753fff-34ea-4aff-8bcf-5116f1c1519a");
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.decode_object(src, GUID_ZERO, "", legacy_mesh_renderer_unit_json(comp));
		db.set_reference(GUID_ZERO, "core/units/primitives/cube.unit", src);
		Guid inst = Guid.new_guid();
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.set_resource(inst, "prefab", "core/units/primitives/cube");
		Guid binding = Guid.new_guid();
		db.create(binding, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
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
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
		db.add_to_set(src, "components", comp);
		db.create(child, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
		db.set_string(child, "data.slot", "Ma");
		db.set_resource(child, "data.material", "materials/source");
		db.add_to_set(comp, "data.materials", child);
		db.create(other, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
		db.set_string(other, "data.slot", "Mb");
		db.set_resource(other, "data.material", "materials/other");
		db.add_to_set(comp, "data.materials", other);
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
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
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
		db.add_to_set(src, "components", comp);
		db.create(child, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
		db.set_string(child, "data.slot", "Ma");
		db.add_to_set(comp, "data.materials", child);
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.set_resource(inst, "prefab", "units/prefab");

		Guid added = Guid.new_guid();
		db.create(added, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
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
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
		db.add_to_set(src, "components", comp);
		db.create(child, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
		db.set_string(child, "data.slot", "Ma");
		db.set_resource(child, "data.material", "materials/source");
		db.add_to_set(comp, "data.materials", child);
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
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
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
		db.add_to_set(src, "components", comp);
		db.create(child, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
		db.set_string(child, "data.slot", "Ma");
		db.set_resource(child, "data.material", "materials/source");
		db.add_to_set(comp, "data.materials", child);
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.set_resource(inst, "prefab", "units/prefab");
		db.create_from_prefab(repl, child);
		db.set_resource(repl, "data.material", "materials/instance");
		string key = "modified_components.#" + comp.to_string() + ".data.materials";
		db.add_to_set(inst, key, repl);

		Database loaded = new Database(p);
		create_object_types(loaded);
		loaded.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		loaded.decode_object(src, GUID_ZERO, "", db.encode_object(src));
		loaded.set_reference(GUID_ZERO, "units/prefab.unit", src);
		loaded.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		loaded.decode_object(inst, GUID_ZERO, "", db.encode_object(inst));

		assert(loaded.get_resource(repl, loaded.property_index(repl, STRING_ID_64("data.material", 0xf014ddbddc53c116))) == "materials/instance");
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
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.create(comp, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
		db.add_to_set(src, "components", comp);
		db.create(child, STRING_ID_64(OBJECT_TYPE_MESH_MATERIAL, 0x6701353384dd782d));
		db.set_string(child, "data.slot", "Ma");
		db.add_to_set(comp, "data.materials", child);
		db.set_reference(GUID_ZERO, "units/prefab.unit", src);
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
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
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.create(comp, STRING_ID_64(OBJECT_TYPE_LOD_GROUP, 0x97993ef522a1a9f0));
		db.add_to_set(src, "components", comp);
		db.create(child, STRING_ID_64(OBJECT_TYPE_LOD_LEVEL, 0x5aeafa4cb5acd79a));
		db.set_double(child, "data.screen_size", 0.5);
		db.set_reference(child, "data.mesh_renderer", mesh);
		db.add_to_set(comp, "data.lod_levels", child);
		db.set_reference(GUID_ZERO, "units/lod-prefab.unit", src);
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
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
		db.create(src, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
		db.create(comp, STRING_ID_64(OBJECT_TYPE_LOD_GROUP, 0x97993ef522a1a9f0));
		db.add_to_set(src, "components", comp);
		db.create(child, STRING_ID_64(OBJECT_TYPE_LOD_LEVEL, 0x5aeafa4cb5acd79a));
		db.set_double(child, "data.screen_size", 0.5);
		db.add_to_set(comp, "data.lod_levels", child);
		db.set_reference(GUID_ZERO, "units/lod-prefab.unit", src);
		db.create(inst, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));
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
	test_sjson();
	test_json();
	test_database();
	test_duplicate_unit_tree();
	test_component_type_dependencies();
	test_mesh_resource();
	test_inherited_lod_levels();
	return 0;
}

} /* namespace Crown */
