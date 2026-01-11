/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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
const string OBJECT_TYPE_FONT_GLYPH              = "font_glyph";
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
const string OBJECT_TYPE_SPRITE_FRAME            = "sprite_frame";
const string OBJECT_TYPE_SPRITE_RENDERER         = "sprite_renderer";
const string OBJECT_TYPE_STATE_MACHINE           = "state_machine";
const string OBJECT_TYPE_STATE_MACHINE_NODE      = "state_machine_node";
const string OBJECT_TYPE_STATE_MACHINE_VARIABLE  = "state_machine_variable";
const string OBJECT_TYPE_TEXTURE                 = "texture";
const string OBJECT_TYPE_TONEMAP                 = "tonemap";
const string OBJECT_TYPE_TRANSFORM               = "transform";
const string OBJECT_TYPE_UNIT                    = "unit";

// UI order reference table:
//
// spatial         500
// rendering       1000
//   global_lighti 1001
//   camera        1100
//   light         1101
//   mesh          1102
//   sprite        1103
//   fog           1104
// physics         2000
//   collider      2100
//   actor         2101
//   mover         2102
// animation       3000
//   state_machine 3100
// scripting       7000
//   lua_script    7100
// post-processing 9000
//   bloom         9100
//   tonemap       9900

public static void node_name_enum_callback(InputField enum_property, InputEnum combo, Project project)
{
	try {
		string? path = ResourceId.path(OBJECT_TYPE_MESH, (string?)enum_property.union_value());
		Mesh mesh = Mesh.load_from_path(project, path);

		combo.clear();
		foreach (var node in mesh._nodes)
			combo.append(node, node);
	} catch (JsonSyntaxError e) {
		loge(e.message);
	}
}

public static void class_enum_callback(InputField property_enum, InputEnum combo, Project project)
{
	try {
		string path = ResourceId.path("physics_config", "global");
		Hashtable global = SJSON.load_from_path(project.absolute_path(path));

		combo.clear();
		if (global.has_key("actors")) {
			Hashtable obj = (Hashtable)global["actors"];
			foreach (var e in obj)
				combo.append(e.key, e.key);
		}
	} catch (JsonSyntaxError e) {
		loge(e.message);
	}
}

public static void collision_filter_enum_callback(InputField property_enum, InputEnum combo, Project project)
{
	try {
		string path = ResourceId.path("physics_config", "global");
		Hashtable global = SJSON.load_from_path(project.absolute_path(path));

		combo.clear();
		if (global.has_key("collision_filters")) {
			Hashtable obj = (Hashtable)global["collision_filters"];
			foreach (var e in obj)
				combo.append(e.key, e.key);
		}
	} catch (JsonSyntaxError e) {
		loge(e.message);
	}
}

public static void material_enum_callback(InputField property_enum, InputEnum combo, Project project)
{
	try {
		string path = ResourceId.path("physics_config", "global");
		Hashtable global = SJSON.load_from_path(project.absolute_path(path));

		combo.clear();
		if (global.has_key("materials")) {
			Hashtable obj = (Hashtable)global["materials"];
			foreach (var e in obj)
				combo.append(e.key, e.key);
		}
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
			tooltip = "Local Position.",
		},
		PropertyDefinition()
		{
			type = PropertyType.QUATERNION,
			name = "data.rotation",
			tooltip = "Local Rotation.",
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "data.scale",
			min = VECTOR3_ZERO,
			deffault = VECTOR3_ONE,
			tooltip = "Local Scale.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 0.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_TRANSFORM
		, properties
		, 500
		, ObjectTypeFlags.UNIT_COMPONENT
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "data.mesh_resource",
			label = "Scene",
			resource_type = OBJECT_TYPE_MESH,
			deffault = "core/components/noop",
			tooltip = "Mesh resource where to read geometries from.",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.geometry_name",
			label = "Node",
			editor = PropertyEditorType.ENUM,
			enum_property = "data.mesh_resource",
			enum_callback = node_name_enum_callback,
			deffault = "Noop",
			tooltip = "Geometry to render.",
		},
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "data.material",
			resource_type = OBJECT_TYPE_MATERIAL,
			deffault = "core/components/noop",
			tooltip = "Material used to render to the geometry.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.visible",
			deffault = true,
			tooltip = "Enable geometry rendering. If disabled it skips any rendering, for example shadow casting.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.cast_shadows",
			deffault = true,
			tooltip = "Enable geometry shadow rendering.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 1.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_MESH_RENDERER
		, properties
		, 1102
		, ObjectTypeFlags.UNIT_COMPONENT
		, OBJECT_TYPE_TRANSFORM
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "data.sprite_resource",
			label = "Sprite",
			resource_type = OBJECT_TYPE_SPRITE,
			deffault = "core/components/noop",
			tooltip = "Sprite resource containing frames.",
		},
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "data.material",
			resource_type = OBJECT_TYPE_MATERIAL,
			deffault = "core/components/noop",
			tooltip = "Material used to render the sprite.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.layer",
			min = 0.0,
			max = 7.0,
			tooltip = "Sorting layer. Higher values makes the sprite appear in front.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.depth",
			min = 0.0,
			max = (double)uint32.MAX,
			tooltip = "Higher values make the sprite apper in front of other sprites in the same layer.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.visible",
			deffault = true,
			tooltip = "Enable sprite rendering.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.flip_x",
			tooltip = "Flips the sprite on the X axis.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.flip_y",
			tooltip = "Flips the sprite on the Y axis.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 1.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_SPRITE_RENDERER
		, properties
		, 1103
		, ObjectTypeFlags.UNIT_COMPONENT
		, OBJECT_TYPE_TRANSFORM
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.type",
			editor = PropertyEditorType.ENUM,
			enum_values = { "directional", "omni", "spot" },
			deffault = "omni",
			tooltip = "Light type.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name =  "data.range",
			min = 0.0,
			deffault = 15.0,
			tooltip = "Objects farther than this value won't be lit.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.intensity",
			min = 0.0,
			deffault = 500.0,
			tooltip = "Light intensity.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.spot_angle",
			editor = PropertyEditorType.ANGLE,
			min = 0.0,
			max = 90.0,
			deffault = MathUtils.rad(45.0),
			tooltip = "Angle of the light's outer cone (Spot only).",
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name =  "data.color",
			editor = PropertyEditorType.COLOR,
			min = VECTOR3_ZERO,
			max = VECTOR3_ONE,
			deffault = VECTOR3_ONE,
			tooltip = "Light color.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.shadow_bias",
			min =  0.0,
			max =  1.0,
			deffault = 0.0004,
			tooltip = "Too low values increase shadow acne. Too high values cause peter-panning.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name =  "data.cast_shadows",
			deffault = true,
			tooltip = "Enable shadow casting."
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 1.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_LIGHT
		, properties
		, 1101
		, ObjectTypeFlags.UNIT_COMPONENT
		, OBJECT_TYPE_TRANSFORM
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.projection",
			editor = PropertyEditorType.ENUM,
			enum_values = { "orthographic", "perspective" },
			deffault = "perspective",
			tooltip = "Camera projection type.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.fov",
			label = "FOV",
			editor = PropertyEditorType.ANGLE,
			min = 0.0,
			max = 90.0,
			deffault = MathUtils.rad(45.0),
			tooltip = "Vertical Field of View.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.near_range",
			deffault = 0.1,
			tooltip = "Objects closer than this value will not be rendered.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.far_range",
			deffault = 1000.0,
			tooltip = "Objects farther than this value will not be rendered.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 1.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_CAMERA
		, properties
		, 1100
		, ObjectTypeFlags.UNIT_COMPONENT
		, OBJECT_TYPE_TRANSFORM
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.source",
			editor = PropertyEditorType.ENUM,
			enum_values = { "mesh", "inline" },
			deffault = "mesh",
			tooltip = "Collider type. Inline let you specify the geometry manually.",
		},
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "data.scene",
			resource_type = OBJECT_TYPE_MESH,
			deffault = "core/components/noop",
			tooltip = "Mesh resource where to read geometries from.",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.name",
			label = "Node",
			editor = PropertyEditorType.ENUM,
			enum_property = "data.scene",
			enum_callback = node_name_enum_callback,
			deffault = "Noop",
			tooltip = "Geometry to use as collider (Mesh only).",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.shape",
			editor = PropertyEditorType.ENUM,
			enum_values = { "sphere", "capsule", "box", "convex_hull", "mesh" },
			deffault = "box",
			tooltip = "Shape to use as collider (Inline only).",
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "data.collider_data.position",
			tooltip = "Shape's local position.",
		},
		PropertyDefinition()
		{
			type = PropertyType.QUATERNION,
			name = "data.collider_data.rotation",
			tooltip = "Shape's local rotation.",
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "data.collider_data.half_extents",     // Box only.
			min = VECTOR3_ZERO,
			deffault = Vector3(0.5, 0.5, 0.5),
			tooltip = "Box radius.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.collider_data.radius",     // Sphere and capsule only.
			min = 0.0,
			deffault = 0.5,
			tooltip = "Sphere or Capsule radius.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.collider_data.height",     // Capsule only.
			min = 0.0,
			deffault = 1.0,
			tooltip = "Capsule height.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 1.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_COLLIDER
		, properties
		, 2100
		, ObjectTypeFlags.UNIT_COMPONENT
		, OBJECT_TYPE_TRANSFORM
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "_global_physics_config",
			deffault = "global",
			resource_type = "physics_config",
			hidden = true,
			not_serialized = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.class",
			editor = PropertyEditorType.ENUM,
			enum_property = "_global_physics_config",
			enum_callback = class_enum_callback,
			deffault = "static",
			tooltip = "Actor class.",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.collision_filter",
			editor = PropertyEditorType.ENUM,
			enum_property = "_global_physics_config",
			enum_callback = collision_filter_enum_callback,
			deffault = "default",
			tooltip = "Actor collision filter.",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.material",
			editor = PropertyEditorType.ENUM,
			enum_property = "_global_physics_config",
			enum_callback = material_enum_callback,
			deffault = "default",
			tooltip = "Actor physics material.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.mass",
			min = 0.0,
			deffault = 1.0,
			tooltip = "Actor physical mass.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_translation_x",
			tooltip = "Prevent the actor from moving along the X axis.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_translation_y",
			tooltip = "Prevent the actor from moving along the Y axis.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_translation_z",
			tooltip = "Prevent the actor from moving along the Z axis.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_rotation_x",
			tooltip = "Prevent the actor from rotating around the X axis.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_rotation_y",
			tooltip = "Prevent the actor from rotating around the Y axis.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.lock_rotation_z",
			tooltip = "Prevent the actor from rotating around the Z axis.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 2.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_ACTOR
		, properties
		, 2101
		, ObjectTypeFlags.UNIT_COMPONENT
		, OBJECT_TYPE_TRANSFORM
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "data.script_resource",
			label = "Script",
			resource_type = "lua",
			deffault = "core/components/noop",
			tooltip = "Script.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 1.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_SCRIPT
		, properties
		, 7100
		, ObjectTypeFlags.UNIT_COMPONENT
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "data.state_machine_resource",
			label = "State Machine",
			resource_type = OBJECT_TYPE_STATE_MACHINE,
			deffault = "core/components/noop",
			label = "State Machine.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 3.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_ANIMATION_STATE_MACHINE
		, properties
		, 3100
		, ObjectTypeFlags.UNIT_COMPONENT
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "source",
		},
	};
	database.create_object_type(OBJECT_TYPE_SOUND, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "position",
			tooltip = "Local position.",
		},
		PropertyDefinition()
		{
			type = PropertyType.QUATERNION,
			name = "rotation",
			tooltip = "Local rotation.",
		},
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "name",
			resource_type = OBJECT_TYPE_SOUND,
			deffault = "",
			tooltip = "Sound resource to play.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "range",
			min = 0.0,
			deffault = 10.0,
			tooltip = "Listeners farther than this value will not hear the sound.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "volume",
			min = 0.0,
			max = 1.0,
			deffault = 1.0,
			tooltip = "Sound volume.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "loop",
			tooltip = "Restart playback at the end.",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "group",
			deffault = "music",
			tooltip = "Mixing group this sound belongs to.",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "editor.name",
			deffault = "sound",
			hidden = true,
			tooltip = "Instance name inside the level.",
		},
	};
	StringId64 sound_source_type = database.create_object_type(OBJECT_TYPE_SOUND_SOURCE, properties, 3.0);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "frame",
			min = 0.0
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "index",
			min = 0.0
		},
	};
	database.create_object_type(OBJECT_TYPE_ANIMATION_FRAME, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "frames",
			object_type = StringId64(OBJECT_TYPE_ANIMATION_FRAME),
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "frames_per_second",
			min = 0.0,
			tooltip = "Speed at which the animation will be played.",
		},
	};
	database.create_object_type(OBJECT_TYPE_SPRITE_ANIMATION, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "index",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "name",
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "pivot",
		},
		PropertyDefinition()
		{
			type = PropertyType.QUATERNION,
			name = "region",
		},
	};
	database.create_object_type(OBJECT_TYPE_SPRITE_FRAME, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "frames",
			object_type = StringId64(OBJECT_TYPE_SPRITE_FRAME),
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "width",
			min = 0.0
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "height",
			min = 0.0
		},
	};
	database.create_object_type(OBJECT_TYPE_SPRITE, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name =  "data.color",
			editor = PropertyEditorType.COLOR,
			min = VECTOR3_ZERO,
			max = VECTOR3_ONE,
			deffault = Vector3(0.83, 0.83, 0.83),
			tooltip = "Fog color.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.density",
			min = 0.0,
			deffault = 0.02,
			tooltip = "Fog density.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name =  "data.range_min",
			min = 0.0,
			tooltip = "Distance from the camera where the fog starts.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name =  "data.range_max",
			min = 0.0,
			deffault = 1000.0,
			tooltip = "Distance from the camera where the fog ends.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.sun_blend",
			min =  0.0,
			max =  1.0,
			tooltip = "Blend sun and fog colors.",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.enabled",
			tooltip = "Enable fog effect.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 0.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_FOG
		, properties
		, 1104
		, ObjectTypeFlags.UNIT_COMPONENT
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "_global_physics_config",
			deffault = "global",
			resource_type = "physics_config",
			hidden = true,
			not_serialized = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.height",
			min = 0.0,
			deffault = 1.80,
			tooltip = "Mover capsule height.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.radius",
			min = 0.0,
			deffault = 0.50,
			tooltip = "Mover capsule radius.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.max_slope_angle",
			editor = PropertyEditorType.ANGLE,
			min = 0.0,
			max = 90.0,
			deffault = MathUtils.rad(45.0f),
			tooltip = "Mover won't climb slopes steeper than this angle.",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.collision_filter",
			editor = PropertyEditorType.ENUM,
			enum_property = "_global_physics_config",
			enum_callback = collision_filter_enum_callback,
			deffault = "default",
			tooltip = "Mover collision filter.",
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "data.center",
			tooltip = "Capsule center relative to the local position.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 2.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_MOVER
		, properties
		, 2102
		, ObjectTypeFlags.UNIT_COMPONENT
		, OBJECT_TYPE_TRANSFORM
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "data.skydome_map",
			label = "Skydome Map",
			resource_type = OBJECT_TYPE_TEXTURE,
			deffault = "core/units/skydome/skydome",
			tooltip = "Texture to use as skydome.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.skydome_intensity",
			min = 0.0,
			deffault = 1.0,
			tooltip = "Skydome intensity multiplier.",
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name =  "data.ambient_color",
			editor = PropertyEditorType.COLOR,
			min = VECTOR3_ZERO,
			max = VECTOR3_ONE,
			tooltip = "Ambient color applied to objects.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 0.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_GLOBAL_LIGHTING
		, properties
		, 1001
		, ObjectTypeFlags.UNIT_COMPONENT
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "data.enabled",
			deffault = true,
			tooltip = "Enable bloom effect.",
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
			tooltip = "Bloom weight.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "data.intensity",
			min = 0.0,
			max = 2.0,
			deffault = 0.8,
			tooltip = "Bloom intensity multiplier.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 0.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_BLOOM
		, properties
		, 9100
		, ObjectTypeFlags.UNIT_COMPONENT
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "data.type",
			editor = PropertyEditorType.ENUM,
			enum_values = { "gamma", "reinhard", "filmic", "aces" },
			deffault = "reinhard",
			tooltip = "Tonemapping operator. Gamma means no tonemapping is applied.",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "spawn_order",
			deffault = 0.0,
			hidden = true,
			not_serialized = true,
		},
	};
	database.create_object_type(OBJECT_TYPE_TONEMAP
		, properties
		, 9900
		, ObjectTypeFlags.UNIT_COMPONENT
		);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "editor.name",
			deffault = OBJECT_NAME_UNNAMED,
			tooltip = "Instance name inside the level.",
		},
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "prefab",
			resource_type = OBJECT_TYPE_UNIT,
			deffault = null,
			tooltip = "Unit prefab.",
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "children",
			object_type = StringId64(OBJECT_TYPE_UNIT),
		},
#if false
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "components",
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "deleted_components",
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "modified_components",
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "deleted_children",
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "modified_children",
		},
#endif /* if false */
	};
	StringId64 unit_type = database.create_object_type(OBJECT_TYPE_UNIT, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "source",
			resource_type = OBJECT_TYPE_MESH,
		},
	};
	database.create_object_type(OBJECT_TYPE_MESH, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "name",
			label = "Animation",
			resource_type = OBJECT_TYPE_MESH_ANIMATION,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "weight",
		}
	};
	StringId64 node_animation_type = database.create_object_type(OBJECT_TYPE_NODE_ANIMATION, properties);

	database.set_aspect(node_animation_type
		, StringId64("name")
		, (out name, database, id) => {
			string? anim_name = database.get_resource(id, "name");
			name = anim_name == null ? "(None)" : GLib.Path.get_basename(anim_name);
		});

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "event",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "mode",
			editor = PropertyEditorType.ENUM,
			enum_values = { "immediate", "wait_until_end" },
		},
		PropertyDefinition()
		{
			type = PropertyType.REFERENCE,
			object_type = StringId64(OBJECT_TYPE_STATE_MACHINE_NODE),
			name = "to",
		},
	};
	StringId64 node_transition_type = database.create_object_type(OBJECT_TYPE_NODE_TRANSITION, properties);

	database.set_aspect(node_transition_type
		, StringId64("name")
		, (out name, database, id) => {
			string node_name = "Unknown";
			Guid to_node = database.get_reference(id, "to");
			if (to_node != GUID_ZERO)
				node_name = database.get_string(to_node, "name");
			name = "To %s".printf(node_name);
		});

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "name",
			deffault = "New Node",
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "animations",
			object_type = node_animation_type,
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "loop",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "speed",
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "transitions",
			object_type = node_transition_type,
		},
	};
	StringId64 state_machine_node_type = database.create_object_type(OBJECT_TYPE_STATE_MACHINE_NODE, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "name",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "value",
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "min",
			deffault = -1.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "max",
			deffault = 1.0,
		},
	};
	StringId64 state_machine_variable_type = database.create_object_type(OBJECT_TYPE_STATE_MACHINE_VARIABLE, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.REFERENCE,
			object_type = StringId64(OBJECT_TYPE_STATE_MACHINE_NODE),
			name = "initial_state",
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "states",
			object_type = state_machine_node_type,
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "variables",
			object_type = state_machine_variable_type,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "animation_type",
			deffault = OBJECT_TYPE_SPRITE_ANIMATION,
			hidden = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "skeleton_name",
			resource_type = OBJECT_TYPE_MESH_SKELETON,
		},
	};
	database.create_object_type(OBJECT_TYPE_STATE_MACHINE, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "editor.camera.orthographic_size",
			deffault = Vector3(20, -20, 20).length(),
		},
		PropertyDefinition()
		{
			type = PropertyType.VECTOR3,
			name = "editor.camera.position",
			deffault = Vector3(20, -20, 20),
		},
		PropertyDefinition()
		{
			type = PropertyType.QUATERNION,
			name = "editor.camera.rotation",
			deffault = Quaternion.look(Vector3(-20, 20, -20).normalize(), Vector3(0.0, 0.0, 1.0)),
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "editor.camera.target_distance",
			deffault = Vector3(20, -20, 20).length(),
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "editor.camera.view_type",
			deffault = (double)CameraViewType.PERSPECTIVE,
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "sounds",
			object_type = sound_source_type,
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "units",
			object_type = unit_type,
		},
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "skydome_unit",
			resource_type = OBJECT_TYPE_UNIT,
			deffault = "core/units/skydome/skydome",
		},
	};
	database.create_object_type(OBJECT_TYPE_LEVEL, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "cp",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "width",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "height",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "x",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "x_advance",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "x_offset",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "y",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "y_offset",
			min = 0.0,
		},
	};
	database.create_object_type(OBJECT_TYPE_FONT_GLYPH, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "glyphs",
			object_type = StringId64(OBJECT_TYPE_FONT_GLYPH),
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "font_size",
			min = 0.0,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "size",
			min = 0.0,
		},
	};
	database.create_object_type(OBJECT_TYPE_FONT, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "source",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "stack_name",
		},
		PropertyDefinition()
		{
			type = PropertyType.RESOURCE,
			name = "target_skeleton",
			resource_type = OBJECT_TYPE_MESH_SKELETON,
		},
	};
	database.create_object_type(OBJECT_TYPE_MESH_ANIMATION, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "name",
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "children",
			object_type = StringId64(OBJECT_TYPE_MESH_BONE),
		},
	};
	database.create_object_type(OBJECT_TYPE_MESH_BONE, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "source",
		},
		PropertyDefinition()
		{
			type = PropertyType.OBJECTS_SET,
			name = "skeleton",
			object_type = StringId64(OBJECT_TYPE_MESH_BONE),
		},
	};
	database.create_object_type(OBJECT_TYPE_MESH_SKELETON, properties);

	properties =
	{
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "source",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "generate_mips",
			deffault = true,
			not_serialized = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "normal_map",
			not_serialized = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "output.android.format",
			editor = PropertyEditorType.ENUM,
			enum_values = { "BC1", "BC2", "BC3", "BC4", "BC5", "PTC14", "RGB8", "RGBA8" },
			deffault = "BC1",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.android.generate_mips",
			deffault = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "output.android.mip_skip_smallest",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.android.normal_map",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.android.linear",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.android.premultiply_alpha",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "output.html5.format",
			editor = PropertyEditorType.ENUM,
			enum_values = { "BC1", "BC2", "BC3", "BC4", "BC5", "PTC14", "RGB8", "RGBA8" },
			deffault = "BC1",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.html5.generate_mips",
			deffault = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "output.html5.mip_skip_smallest",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.html5.normal_map",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.html5.linear",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.html5.premultiply_alpha",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "output.linux.format",
			editor = PropertyEditorType.ENUM,
			enum_values = { "BC1", "BC2", "BC3", "BC4", "BC5", "PTC14", "RGB8", "RGBA8" },
			deffault = "BC1",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.linux.generate_mips",
			deffault = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "output.linux.mip_skip_smallest",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.linux.normal_map",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.linux.linear",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.linux.premultiply_alpha",
		},
		PropertyDefinition()
		{
			type = PropertyType.STRING,
			name = "output.windows.format",
			editor = PropertyEditorType.ENUM,
			enum_values = { "BC1", "BC2", "BC3", "BC4", "BC5", "PTC14", "RGB8", "RGBA8" },
			deffault = "BC1",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.windows.generate_mips",
			deffault = true,
		},
		PropertyDefinition()
		{
			type = PropertyType.DOUBLE,
			name = "output.windows.mip_skip_smallest",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.windows.normal_map",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.windows.linear",
		},
		PropertyDefinition()
		{
			type = PropertyType.BOOL,
			name = "output.windows.premultiply_alpha",
		},
	};
	database.create_object_type(OBJECT_TYPE_TEXTURE, properties);
}

} /* namespace Crown */
