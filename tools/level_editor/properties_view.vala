/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class TransformPropertyGrid : PropertyGrid
{
	public TransformPropertyGrid(Database db)
	{
		base(db);

		PropertyDefinition[] properties =
		{
			PropertyDefinition()
			{
				type = PropertyType.VECTOR3,
				name = "data.position",
				label = "Position",
			},
			PropertyDefinition()
			{
				type = PropertyType.QUATERNION,
				name = "data.rotation",
				label = "Rotation",
			},
			PropertyDefinition()
			{
				type = PropertyType.VECTOR3,
				name = "data.scale",
				label = "Scale",
				min = VECTOR3_ZERO,
				deffault = VECTOR3_ONE,
			},
		};

		add_object_type(properties);
	}
}

public class MeshRendererPropertyGrid : PropertyGrid
{
	public MeshRendererPropertyGrid(Database db, ProjectStore store)
	{
		base(db, store);

		PropertyDefinition[] properties =
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
				label = "Material",
				editor = PropertyEditorType.RESOURCE,
				resource_type = OBJECT_TYPE_MATERIAL,
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name = "data.visible",
				label = "Visible",
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name = "data.cast_shadows",
				label = "Cast Shadows",
			},
		};

		add_object_type(properties);
	}
}

public void node_name_enum_callback(InputField enum_property, InputEnum combo, Project project)
{
	try {
		string path = ResourceId.path(OBJECT_TYPE_MESH, (string)enum_property.union_value());
		Hashtable mesh_resource = SJSON.load_from_path(project.absolute_path(path));

		combo.clear();
		Mesh mesh = Mesh();
		mesh.decode(mesh_resource);
		foreach (var node in mesh._nodes)
			combo.append(node, node);

		combo.value = combo.any_valid_id();
	} catch (JsonSyntaxError e) {
		loge(e.message);
	}
}

public class SpriteRendererPropertyGrid : PropertyGrid
{
	public SpriteRendererPropertyGrid(Database db, ProjectStore store)
	{
		base(db, store);

		PropertyDefinition[] properties =
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
				label = "Material",
				editor = PropertyEditorType.RESOURCE,
				resource_type = OBJECT_TYPE_MATERIAL,
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "data.layer",
				label = "Layer",
				min = 0.0,
				max = 7.0,
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "data.layer",
				label = "Depth",
				min = 0.0,
				max = (double)uint32.MAX,
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name = "data.visible",
				label = "Visible",
			},
		};

		add_object_type(properties);
	}
}

public class LightPropertyGrid : PropertyGrid
{
	public LightPropertyGrid(Database db)
	{
		base(db);

		PropertyDefinition[] properties =
		{
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "data.type",
				label = "Type",
				editor = PropertyEditorType.ENUM,
				enum_values = { "directional", "omni", "spot" },
				enum_labels = { "Directional", "Omni", "Spot" },
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name =  "data.range",
				label =  "Range",
				min = 0.0,
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "data.intensity",
				label = "Intensity",
				min = 0.0,
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "data.spot_angle",
				label =  "Spot Angle",
				editor = PropertyEditorType.ANGLE,
				min = 0.0,
				max = 90.0,
			},
			PropertyDefinition()
			{
				type = PropertyType.VECTOR3,
				name =  "data.color",
				label =  "Color",
				editor = PropertyEditorType.COLOR,
				min = VECTOR3_ZERO,
				max = VECTOR3_ONE,
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "data.shadow_bias",
				label =  "Shadow Bias",
				min =  0.0,
				max =  1.0,
				deffault = 0.0001,
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name =  "data.cast_shadows",
				label =  "Cast Shadows",
				deffault = true,
			},
		};

		add_object_type(properties);
	}
}

public class CameraPropertyGrid : PropertyGrid
{
	public CameraPropertyGrid(Database db)
	{
		base(db);

		PropertyDefinition[] properties =
		{
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "data.projection",
				label = "Projection",
				editor = PropertyEditorType.ENUM,
				enum_values = { "orthographic", "perspective" },
				enum_labels = { "Orthographic", "Perspective" },
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
				label = "Near Range",
				deffault = 0.1,
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "data.far_range",
				label = "Far Range",
				deffault = 1000.0,
			},
		};

		add_object_type(properties);
	}
}

public class ColliderPropertyGrid : PropertyGrid
{
	public ColliderPropertyGrid(Database db, ProjectStore store)
	{
		base(db, store);

		PropertyDefinition[] properties =
		{
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "data.source",
				label = "Source",
				editor = PropertyEditorType.ENUM,
				enum_values = { "mesh", "inline" },
				enum_labels = { "Mesh", "Inline" },
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "data.scene",
				label = "Scene",
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
				label = "Shape",
				editor = PropertyEditorType.ENUM,
				enum_values = { "sphere", "capsule", "box", "convex_hull", "mesh" },
				enum_labels = { "Sphere", "Capsule", "Box", "Convex Hull", "Mesh" },
				enum_property = "data.source",
				enum_callback = shape_resource_callback,
			},
			PropertyDefinition()
			{
				type = PropertyType.VECTOR3,
				name = "data.collider_data.position",
				label = "Position",
			},
			PropertyDefinition()
			{
				type = PropertyType.QUATERNION,
				name = "data.collider_data.rotation",
				label = "Rotation",
			},
			PropertyDefinition()
			{
				type = PropertyType.VECTOR3,
				name = "data.collider_data.half_extents", // Box only.
				label = "Half Extents",
				min = VECTOR3_ZERO,
				deffault = Vector3(0.5, 0.5, 0.5),
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "data.collider_data.radius", // Sphere and capsule only.
				label = "Radius",
				min = 0.0,
				deffault = 0.5,
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "data.collider_data.height", // Capsule only.
				label = "Radius",
				min = 0.0,
				deffault = 1.0,
			},
		};

		add_object_type(properties);
	}

	public void scene_resource_callback(InputField enum_property, InputResource chooser, Project project)
	{
		if (enum_property.union_value() == "mesh")
			chooser.set_union_value("core/units/primitives/cube");
	}

	public void shape_resource_callback(InputField enum_property, InputEnum combo, Project project)
	{
		if (enum_property.union_value() == "inline")
			combo.set_union_value("box");
	}
}

public class ActorPropertyGrid : PropertyGrid
{
	public ActorPropertyGrid(Database db, ProjectStore store)
	{
		base(db, store);

		PropertyDefinition[] properties =
		{
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "_global_physics_config",
				label = "Physics Resource",
				deffault = "global",
				editor = PropertyEditorType.RESOURCE,
				resource_type = "physics_config",
				hidden = true,
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "data.class",
				label = "Class",
				editor = PropertyEditorType.ENUM,
				enum_property = "_global_physics_config",
				enum_callback = class_enum_callback
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "data.collision_filter",
				label = "Collision Filter",
				editor = PropertyEditorType.ENUM,
				enum_property = "_global_physics_config",
				enum_callback = collision_filter_enum_callback
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "data.material",
				label = "Material",
				editor = PropertyEditorType.ENUM,
				enum_property = "_global_physics_config",
				enum_callback = material_enum_callback
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "data.mass",
				label = "Mass",
				min = 0.0,
				deffault = 1.0,
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name = "data.lock_translation_x",
				label = "Lock Translation X",
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name = "data.lock_translation_y",
				label = "Lock Translation Y",
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name = "data.lock_translation_z",
				label = "Lock Translation Z",
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name = "data.lock_rotation_x",
				label = "Lock Rotation X",
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name = "data.lock_rotation_y",
				label = "Lock Rotation Y",
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name = "data.lock_rotation_z",
				label = "Lock Rotation Z",
			},
		};

		add_object_type(properties);
	}

	private void class_enum_callback(InputField property_enum, InputEnum combo, Project project)
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

	private void collision_filter_enum_callback(InputField property_enum, InputEnum combo, Project project)
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

	private void material_enum_callback(InputField property_enum, InputEnum combo, Project project)
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
}

public class ScriptPropertyGrid : PropertyGrid
{
	public ScriptPropertyGrid(Database db, ProjectStore store)
	{
		base(db, store);

		PropertyDefinition[] properties =
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

		add_object_type(properties);
	}
}

public class AnimationStateMachine : PropertyGrid
{
	public AnimationStateMachine(Database db, ProjectStore store)
	{
		base(db, store);

		PropertyDefinition[] properties =
		{
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "data.state_machine_resource",
				label = "State Machine",
				editor = PropertyEditorType.RESOURCE,
				resource_type = OBJECT_TYPE_ANIMATION_STATE_MACHINE
			},
		};

		add_object_type(properties);
	}
}

public class UnitView : PropertyGrid
{
	// Widgets
	private InputResource _prefab;
	private Gtk.MenuButton _component_add;
	private Gtk.Box _components;
	private Gtk.Popover _add_popover;

	private void on_add_component_clicked(Gtk.Button button)
	{
		Gtk.Application app = ((Gtk.Window)this.get_toplevel()).application;
		app.activate_action("unit-add-component", new GLib.Variant.string(button.label));

		_add_popover.hide();
	}

	public static Gtk.Menu component_menu(string object_type)
	{
		Gtk.Menu menu = new Gtk.Menu();
		Gtk.MenuItem mi;

		mi = new Gtk.MenuItem.with_label("Remove Component");
		mi.activate.connect(() => {
				GLib.Application.get_default().activate_action("unit-remove-component", new GLib.Variant.string(object_type));
			});
		menu.add(mi);

		return menu;
	}

	public UnitView(Database db, ProjectStore store)
	{
		base(db, store);

		// Widgets
		_prefab = new InputResource(store, "unit");
		_prefab._selector.sensitive = false;

		// List of component types.
		const string components[] =
		{
			OBJECT_TYPE_TRANSFORM,
			OBJECT_TYPE_LIGHT,
			OBJECT_TYPE_CAMERA,
			OBJECT_TYPE_MESH_RENDERER,
			OBJECT_TYPE_SPRITE_RENDERER,
			OBJECT_TYPE_COLLIDER,
			OBJECT_TYPE_ACTOR,
			OBJECT_TYPE_SCRIPT,
			OBJECT_TYPE_ANIMATION_STATE_MACHINE
		};

		Gtk.Box add_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		for (int cc = 0; cc < components.length; ++cc) {
			Gtk.Button mb;
			mb = new Gtk.Button.with_label(components[cc]);
			mb.clicked.connect(on_add_component_clicked);
			add_box.pack_start(mb);
		}
		add_box.show_all();
		_add_popover = new Gtk.Popover(null);
		_add_popover.add(add_box);

		_component_add = new Gtk.MenuButton();
		_component_add.label = "Add Component";
		_component_add.set_popover(_add_popover);

		_components = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		_components.homogeneous = true;
		_components.pack_start(_component_add);

		add_row("Prefab", _prefab);
		add_row("Components", _components);
	}

	public override void update()
	{
		if (_db.has_property(_id, "prefab")) {
			_prefab.value = _db.get_property_string(_id, "prefab");
		} else {
			_prefab.value = "<none>";
		}
	}
}

public class SoundSourcePropertyGrid : PropertyGrid
{
	public SoundSourcePropertyGrid(Database db, ProjectStore store)
	{
		base(db, store);

		PropertyDefinition[] properties =
		{
			PropertyDefinition()
			{
				type = PropertyType.VECTOR3,
				name = "position",
				label = "Position",
			},
			PropertyDefinition()
			{
				type = PropertyType.QUATERNION,
				name = "rotation",
				label = "Rotation",
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "name",
				label = "Name",
				editor = PropertyEditorType.RESOURCE,
				resource_type = OBJECT_TYPE_SOUND,
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "range",
				label = "Range",
				min = 0.0,
				deffault = 10.0,
			},
			PropertyDefinition()
			{
				type = PropertyType.DOUBLE,
				name = "volume",
				label = "Volume",
				min = 0.0,
				max = 1.0,
				deffault = 1.0,
			},
			PropertyDefinition()
			{
				type = PropertyType.BOOL,
				name = "loop",
				label = "Loop",
			},
			PropertyDefinition()
			{
				type = PropertyType.STRING,
				name = "group",
				label = "Group",
				deffault = "music",
			},
		};

		add_object_type(properties);
	}
}

public class PropertiesView : Gtk.Bin
{
	public struct ComponentEntry
	{
		string type;
		int position;
	}

	// Data
	private Database _db;
	private Gee.HashMap<string, Expander> _expanders;
	private Gee.HashMap<string, bool> _expander_states;
	private Gee.HashMap<string, PropertyGrid> _objects;
	private Gee.ArrayList<ComponentEntry?> _entries;
	private Gee.ArrayList<Guid?>? _selection;

	// Widgets
	private Gtk.Label _nothing_to_show;
	private Gtk.Label _unknown_object_type;
	private Gtk.Viewport _viewport;
	private Gtk.ScrolledWindow _scrolled_window;
	private PropertyGridSet _object_view;
	private Gtk.Stack _stack;

	[CCode (has_target = false)]
	public delegate Gtk.Menu ContextMenu(string object_type);

	public PropertiesView(Database db, ProjectStore store)
	{
		// Data
		_db = db;

		_expanders = new Gee.HashMap<string, Expander>();
		_expander_states = new Gee.HashMap<string, bool>();
		_objects = new Gee.HashMap<string, PropertyGrid>();
		_entries = new Gee.ArrayList<ComponentEntry?>();
		_selection = null;

		// Widgets
		_object_view = new PropertyGridSet();
		_object_view.border_width = 6;

		// Unit
		register_object_type("Unit",                    "name",                              0, new UnitView(_db, store));
		register_object_type("Transform",               OBJECT_TYPE_TRANSFORM,               0, new TransformPropertyGrid(_db),             UnitView.component_menu);
		register_object_type("Light",                   OBJECT_TYPE_LIGHT,                   1, new LightPropertyGrid(_db),                 UnitView.component_menu);
		register_object_type("Camera",                  OBJECT_TYPE_CAMERA,                  2, new CameraPropertyGrid(_db),                UnitView.component_menu);
		register_object_type("Mesh Renderer",           OBJECT_TYPE_MESH_RENDERER,           3, new MeshRendererPropertyGrid(_db, store),   UnitView.component_menu);
		register_object_type("Sprite Renderer",         OBJECT_TYPE_SPRITE_RENDERER,         3, new SpriteRendererPropertyGrid(_db, store), UnitView.component_menu);
		register_object_type("Collider",                OBJECT_TYPE_COLLIDER,                3, new ColliderPropertyGrid(_db, store),       UnitView.component_menu);
		register_object_type("Actor",                   OBJECT_TYPE_ACTOR,                   3, new ActorPropertyGrid(_db, store),          UnitView.component_menu);
		register_object_type("Script",                  OBJECT_TYPE_SCRIPT,                  3, new ScriptPropertyGrid(_db, store),         UnitView.component_menu);
		register_object_type("Animation State Machine", OBJECT_TYPE_ANIMATION_STATE_MACHINE, 3, new AnimationStateMachine(_db, store),      UnitView.component_menu);
		register_object_type("Sound",                   OBJECT_TYPE_SOUND_SOURCE,            0, new SoundSourcePropertyGrid(_db, store));

		_nothing_to_show = new Gtk.Label("Select an object to start editing");
		_unknown_object_type = new Gtk.Label("Unknown object type");

		_viewport = new Gtk.Viewport(null, null);
		_viewport.add(_object_view);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_viewport);

		_stack = new Gtk.Stack();
		_stack.add(_nothing_to_show);
		_stack.add(_scrolled_window);
		_stack.add(_unknown_object_type);

		this.add(_stack);
		this.get_style_context().add_class("properties-view");

		store._project.project_reset.connect(on_project_reset);
	}

	private void register_object_type(string label, string object_type, int position, PropertyGrid cv, ContextMenu? action = null)
	{
		Expander expander = _object_view.add_property_grid(cv, label);
		if (action != null) {
			expander.button_release_event.connect((ev) => {
					if (ev.button == Gdk.BUTTON_SECONDARY) {
						Gtk.Menu menu = action(object_type);
						menu.show_all();
						menu.popup_at_pointer(ev);
						return Gdk.EVENT_STOP;
					}

					return Gdk.EVENT_PROPAGATE;
				});
		}

		_objects[object_type] = cv;
		_expanders[object_type] = expander;
		_entries.add({ object_type, position });
	}

	public void show_unit(Guid id)
	{
		foreach (var entry in _entries) {
			Expander expander = _expanders[entry.type];
			_expander_states[entry.type] = expander.expanded;
		}
		_stack.set_visible_child(_scrolled_window);

		foreach (var entry in _entries) {
			Expander expander = _expanders[entry.type];
			bool was_expanded = _expander_states.has_key(entry.type) ? _expander_states[entry.type] : false;

			Unit unit = Unit(_db, id);
			Guid component_id;
			Guid owner_id;
			if (unit.has_component_with_owner(out component_id, out owner_id, entry.type) || entry.type == "name") {
				PropertyGrid cv = _objects[entry.type];
				cv._id = id;
				cv._component_id = component_id;
				cv.update();

				if (id == owner_id)
					expander.get_style_context().remove_class("inherited");
				else
					expander.get_style_context().add_class("inherited");

				expander.show();
				expander.expanded = was_expanded;
			} else {
				expander.hide();
			}
		}
	}

	public void show_sound_source(Guid id)
	{
		foreach (var entry in _entries) {
			Expander expander = _expanders[entry.type];
			_expander_states[entry.type] = expander.expanded;
		}

		_stack.set_visible_child(_scrolled_window);

		foreach (var entry in _entries) {
			Expander expander = _expanders[entry.type];

			if (entry.type == OBJECT_TYPE_SOUND_SOURCE) {
				bool was_expanded = _expander_states.has_key(entry.type) ? _expander_states[entry.type] : false;

				PropertyGrid cv = _objects[entry.type];
				cv._id = id;
				cv.update();

				expander.show();
				expander.expanded = was_expanded;
			} else {
				expander.hide();
			}
		}
	}

	public void show_or_hide_properties()
	{
		if (_selection == null || _selection.size != 1) {
			_stack.set_visible_child(_nothing_to_show);
			return;
		}

		Guid id = _selection[_selection.size - 1];
		if (!_db.has_object(id))
			return;

		if (_db.object_type(id) == OBJECT_TYPE_UNIT)
			show_unit(id);
		else if (_db.object_type(id) == OBJECT_TYPE_SOUND_SOURCE)
			show_sound_source(id);
		else
			_stack.set_visible_child(_unknown_object_type);
	}

	public void on_selection_changed(Gee.ArrayList<Guid?> selection)
	{
		_selection = selection;
		show_or_hide_properties();
	}

	public override void map()
	{
		base.map();
		show_or_hide_properties();
	}

	public void on_project_reset()
	{
		foreach (var obj in _objects) {
			PropertyGrid cv = obj.value;
			cv._id = GUID_ZERO;
			cv._component_id = GUID_ZERO;
		}
	}
}

} /* namespace Crown */
