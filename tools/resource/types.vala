/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
const string OBJECT_TYPE_ACTOR                   = "actor";
const string OBJECT_TYPE_ANIMATION_FRAME         = "animation_frame";
const string OBJECT_TYPE_ANIMATION_STATE_MACHINE = "animation_state_machine";
const string OBJECT_TYPE_BLOOM                   = "bloom";
const string OBJECT_TYPE_CAMERA                  = "camera";
const string OBJECT_TYPE_COLLIDER                = "collider";
const string OBJECT_TYPE_FILE                    = "file";
const string OBJECT_TYPE_FOG                     = "fog";
const string OBJECT_TYPE_FONT                    = "font";
const string OBJECT_TYPE_GLOBAL_LIGHTING         = "global_lighting";
const string OBJECT_TYPE_LEVEL                   = "level";
const string OBJECT_TYPE_LIGHT                   = "light";
const string OBJECT_TYPE_MATERIAL                = "material";
const string OBJECT_TYPE_MESH                    = "mesh";
const string OBJECT_TYPE_MESH_ANIMATION          = "mesh_animation";
const string OBJECT_TYPE_MESH_BONE               = "mesh_bone";
const string OBJECT_TYPE_MESH_RENDERER           = "mesh_renderer";
const string OBJECT_TYPE_MESH_SKELETON           = "mesh_skeleton";
const string OBJECT_TYPE_MOVER                   = "mover";
const string OBJECT_TYPE_NODE_ANIMATION          = "node_animation";
const string OBJECT_TYPE_NODE_TRANSITION         = "node_transition";
const string OBJECT_TYPE_SCRIPT                  = "script";
const string OBJECT_TYPE_SOUND                   = "sound";
const string OBJECT_TYPE_SOUND_SOURCE            = "sound_source";
const string OBJECT_TYPE_SPRITE                  = "sprite";
const string OBJECT_TYPE_SPRITE_ANIMATION        = "sprite_animation";
const string OBJECT_TYPE_SPRITE_RENDERER         = "sprite_renderer";
const string OBJECT_TYPE_STATE_MACHINE           = "state_machine";
const string OBJECT_TYPE_STATE_MACHINE_NODE      = "state_machine_node";
const string OBJECT_TYPE_STATE_MACHINE_VARIABLE  = "state_machine_variable";
const string OBJECT_TYPE_TEXTURE                 = "texture";
const string OBJECT_TYPE_TONEMAP                 = "tonemap";
const string OBJECT_TYPE_TRANSFORM               = "transform";
const string OBJECT_TYPE_UNIT                    = "unit";

public static void node_name_enum_callback(InputField enum_property, InputEnum combo, Project project)
{
	try {
		string path = ResourceId.path(OBJECT_TYPE_MESH, (string)enum_property.union_value());
		Mesh mesh = Mesh.load_from_path(project, path);

		combo.clear();
		foreach (var node in mesh._nodes)
			combo.append(node, node);

		combo.value = combo.any_valid_id();
	} catch (JsonSyntaxError e) {
		loge(e.message);
	}
}

public static void scene_resource_callback(InputField enum_property, InputResource chooser, Project project)
{
	if (enum_property.union_value() == "mesh")
		chooser.set_union_value("core/units/primitives/cube");
}

public static void shape_resource_callback(InputField enum_property, InputEnum combo, Project project)
{
	if (enum_property.union_value() == "inline")
		combo.set_union_value("box");
}

public static void class_enum_callback(InputField property_enum, InputEnum combo, Project project)
{
	try {
		string path = ResourceId.path("physics_config", "global");
		Hashtable global = SJSON.load_from_path(project.absolute_path(path));

		string prev_enum = combo.value;
		combo.clear();
		if (global.has_key("actors")) {
			Hashtable obj = (Hashtable)global["actors"];
			foreach (var e in obj)
				combo.append(e.key, e.key);
		}
		combo.value = prev_enum;
	} catch (JsonSyntaxError e) {
		loge(e.message);
	}
}

public static void collision_filter_enum_callback(InputField property_enum, InputEnum combo, Project project)
{
	try {
		string path = ResourceId.path("physics_config", "global");
		Hashtable global = SJSON.load_from_path(project.absolute_path(path));

		string prev_enum = combo.value;
		combo.clear();
		if (global.has_key("collision_filters")) {
			Hashtable obj = (Hashtable)global["collision_filters"];
			foreach (var e in obj)
				combo.append(e.key, e.key);
		}
		combo.value = prev_enum;
	} catch (JsonSyntaxError e) {
		loge(e.message);
	}
}

public static void material_enum_callback(InputField property_enum, InputEnum combo, Project project)
{
	try {
		string path = ResourceId.path("physics_config", "global");
		Hashtable global = SJSON.load_from_path(project.absolute_path(path));

		string prev_enum = combo.value;
		combo.clear();
		if (global.has_key("materials")) {
			Hashtable obj = (Hashtable)global["materials"];
			foreach (var e in obj)
				combo.append(e.key, e.key);
		}
		combo.value = prev_enum;
	} catch (JsonSyntaxError e) {
		loge(e.message);
	}
}

public static void create_object_types(Database database)
{
	PropertyDefinition[] properties;

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "data.position",
		},
		PropertyDefinition()
		{
			type = PropertyType.QUATERNION,
			name = "data.rotation",
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "data.scale",
			min = VECTOR3_ZERO,
			deffault = VECTOR3_ONE,
		},
	};
	database.create_object_type(OBJECT_TYPE_TRANSFORM, properties, 0.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.mesh_resource",
			label = "Scene",
			editor = PropertyEditorType.RESOURCE,
			resource_type = OBJECT_TYPE_MESH,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.geometry_name",
			label = "Node",
			editor = PropertyEditorType.ENUM,
			enum_property = "data.mesh_resource",
			enum_callback = node_name_enum_callback,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.material",
			editor = PropertyEditorType.RESOURCE,
			resource_type = OBJECT_TYPE_MATERIAL,
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.visible",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.cast_shadows",
			deffault = true
		},
	};
	database.create_object_type(OBJECT_TYPE_MESH_RENDERER, properties, 3.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.sprite_resource",
			label = "Sprite",
			editor = PropertyEditorType.RESOURCE,
			resource_type = OBJECT_TYPE_SPRITE,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.material",
			editor = PropertyEditorType.RESOURCE,
			resource_type = OBJECT_TYPE_MATERIAL,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.layer",
			min = 0.0,
			max = 7.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.depth",
			min = 0.0,
			max = (double)uint32.MAX,
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.visible",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.flip_x",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.flip_y",
		},
	};
	database.create_object_type(OBJECT_TYPE_SPRITE_RENDERER, properties, 3.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.type",
			editor = PropertyEditorType.ENUM,
			enum_values = { "directional", "omni", "spot" },
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name =  "data.range",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.intensity",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.spot_angle",
			editor = PropertyEditorType.ANGLE,
			min = 0.0,
			max = 90.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name =  "data.color",
			editor = PropertyEditorType.COLOR,
			min = VECTOR3_ZERO,
			max = VECTOR3_ONE,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.shadow_bias",
			min =  0.0,
			max =  1.0,
			deffault = 0.0001,
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name =  "data.cast_shadows",
			deffault = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_LIGHT, properties, 1.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.projection",
			editor = PropertyEditorType.ENUM,
			enum_values = { "orthographic", "perspective" },
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.fov",
			label = "FOV",
			editor = PropertyEditorType.ANGLE,
			min = 0.0,
			max = 90.0
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.near_range",
			deffault = 0.1,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.far_range",
			deffault = 1000.0,
		},
	};
	database.create_object_type(OBJECT_TYPE_CAMERA, properties, 2.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.source",
			editor = PropertyEditorType.ENUM,
			enum_values = { "mesh", "inline" },
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.scene",
			editor = PropertyEditorType.RESOURCE,
			resource_type = OBJECT_TYPE_MESH,
			enum_property = "data.source",
			resource_callback = scene_resource_callback
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.name",
			label = "Node",
			editor = PropertyEditorType.ENUM,
			enum_property = "data.scene",
			enum_callback = node_name_enum_callback,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.shape",
			editor = PropertyEditorType.ENUM,
			enum_values = { "sphere", "capsule", "box", "convex_hull", "mesh" },
			enum_property = "data.source",
			enum_callback = shape_resource_callback,
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "data.collider_data.position",
		},
		PropertyDefinition()
		{
			type = PropertyType.QUATERNION,
			name = "data.collider_data.rotation",
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "data.collider_data.half_extents",     // Box only.
			min = VECTOR3_ZERO,
			deffault = Vector3(0.5, 0.5, 0.5),
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.collider_data.radius",     // Sphere and capsule only.
			min = 0.0,
			deffault = 0.5,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.collider_data.height",     // Capsule only.
			min = 0.0,
			deffault = 1.0,
		},
	};
	database.create_object_type(OBJECT_TYPE_COLLIDER, properties, 3.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "_global_physics_config",
			deffault = "global",
			editor = PropertyEditorType.RESOURCE,
			resource_type = "physics_config",
			hidden = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.class",
			editor = PropertyEditorType.ENUM,
			enum_property = "_global_physics_config",
			enum_callback = class_enum_callback
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.collision_filter",
			editor = PropertyEditorType.ENUM,
			enum_property = "_global_physics_config",
			enum_callback = collision_filter_enum_callback
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.material",
			editor = PropertyEditorType.ENUM,
			enum_property = "_global_physics_config",
			enum_callback = material_enum_callback
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.mass",
			min = 0.0,
			deffault = 1.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_translation_x",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_translation_y",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_translation_z",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_rotation_x",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_rotation_y",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_rotation_z",
		},
	};
	database.create_object_type(OBJECT_TYPE_ACTOR, properties, 3.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.script_resource",
			label = "Script",
			editor = PropertyEditorType.RESOURCE,
			resource_type = "lua"
		},
	};
	database.create_object_type(OBJECT_TYPE_SCRIPT, properties, 3.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.state_machine_resource",
			label = "State Machine",
			editor = PropertyEditorType.RESOURCE,
			resource_type = OBJECT_TYPE_STATE_MACHINE
		},
	};
	database.create_object_type(OBJECT_TYPE_ANIMATION_STATE_MACHINE, properties, 3.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "position",
		},
		PropertyDefinition()
		{
			type = PropertyType.QUATERNION,
			name = "rotation",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "name",
			editor = PropertyEditorType.RESOURCE,
			resource_type = OBJECT_TYPE_SOUND,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "range",
			min = 0.0,
			deffault = 10.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "volume",
			min = 0.0,
			max = 1.0,
			deffault = 1.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "loop",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "group",
			deffault = "music",
		},
	};
	database.create_object_type(OBJECT_TYPE_SOUND_SOURCE, properties, 3.0);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name =  "data.color",
			editor = PropertyEditorType.COLOR,
			min = VECTOR3_ZERO,
			max = VECTOR3_ONE,
			deffault = VECTOR3_ONE,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.density",
			min = 0.0,
			deffault = 0.02
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name =  "data.range_min",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name =  "data.range_max",
			min = 0.0,
			deffault = 1000.0
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.sun_blend",
			min =  0.0,
			max =  1.0,
			deffault = 0.0
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.enabled",
			deffault = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_FOG, properties, 0.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "_global_physics_config",
			deffault = "global",
			editor = PropertyEditorType.RESOURCE,
			resource_type = "physics_config",
			hidden = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.height",
			min = 0.0,
			deffault = 1.80,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.radius",
			min = 0.0,
			deffault = 0.50,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.max_slope_angle",
			editor = PropertyEditorType.ANGLE,
			min = 0.0,
			max = 90.0,
			deffault = 45.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.collision_filter",
			editor = PropertyEditorType.ENUM,
			enum_property = "_global_physics_config",
			enum_callback = collision_filter_enum_callback
		},
	};
	database.create_object_type(OBJECT_TYPE_MOVER, properties, 3.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.skydome_map",
			label = "Skydome Map",
			editor = PropertyEditorType.RESOURCE,
			resource_type = OBJECT_TYPE_TEXTURE,
			deffault = "core/units/skydome/skydome"
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.skydome_intensity",
			min = 0.0,
			deffault = 1.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name =  "data.ambient_color",
			editor = PropertyEditorType.COLOR,
			min = VECTOR3_ZERO,
			max = VECTOR3_ONE,
		},
	};
	database.create_object_type(OBJECT_TYPE_GLOBAL_LIGHTING, properties, 0.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.enabled",
			deffault = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.threshold",
			min = 0.0,
			hidden = true
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.weight",
			min = 0.0,
			max = 1.0,
			deffault = 0.05,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.intensity",
			min = 0.0,
			max = 2.0,
			deffault = 0.8,
		},
	};
	database.create_object_type(OBJECT_TYPE_BLOOM, properties, 0.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.type",
			editor = PropertyEditorType.ENUM,
			enum_values = { "gamma", "reinhard", "filmic", "aces" },
		},
	};
	database.create_object_type(OBJECT_TYPE_TONEMAP, properties, 0.0, ObjectTypeFlags.UNIT_COMPONENT);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "editor.name",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "prefab",
			editor = PropertyEditorType.RESOURCE,
			resource_type = OBJECT_TYPE_UNIT,
		},
	};
	database.create_object_type(OBJECT_TYPE_UNIT, properties);
}

} /* namespace Crown */
