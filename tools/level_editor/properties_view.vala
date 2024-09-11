/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;
using Gee;

namespace Crown
{
public class PropertyGrid : Gtk.Grid
{
	// Data
	public Database? _db;
	public Guid _id;
	public Guid _component_id;
	public int _rows;

	public PropertyGrid(Database? db = null)
	{
		this.row_spacing = 4;
		this.row_homogeneous = true;
		this.column_spacing = 12;

		// Data
		_db = db;
		_id = GUID_ZERO;
		_component_id = GUID_ZERO;
		_rows = 0;
	}

	public Gtk.Widget add_row(string label, Gtk.Widget w)
	{
		Gtk.Label l = new Label(label);
		l.width_chars = 13;
		l.set_alignment(1.0f, 0.5f);

		w.hexpand = true;

		this.attach(l, 0, (int)_rows);
		this.attach(w, 1, (int)_rows);
		++_rows;

		return l;
	}

	public virtual void update()
	{
	}
}

public class PropertyGridSet : Gtk.Box
{
	public PropertyGridSet()
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);
	}

	public Gtk.Expander add_property_grid(PropertyGrid cv, string label)
	{
		Gtk.Label lb = new Gtk.Label(null);
		lb.set_markup("<b>%s</b>".printf(label));
		lb.set_alignment(0.0f, 0.5f);

		Gtk.Expander expander = new Gtk.Expander("");
		expander.label_widget = lb;
		expander.expanded = true;
		expander.add(cv);
		this.pack_start(expander, false, true, 0);

		return expander;
	}
}

public class TransformPropertyGrid : PropertyGrid
{
	// Widgets
	private EntryPosition _position;
	private EntryRotation _rotation;
	private EntryScale _scale;

	public TransformPropertyGrid(Database db)
	{
		base(db);

		// Widgets
		_position = new EntryPosition();
		_position.value_changed.connect(on_value_changed);
		_rotation = new EntryRotation();
		_rotation.value_changed.connect(on_value_changed);
		_scale = new EntryScale();
		_scale.value_changed.connect(on_value_changed);

		add_row("Position", _position);
		add_row("Rotation", _rotation);
		add_row("Scale", _scale);
	}

	private void on_value_changed()
	{
		Unit unit = new Unit(_db, _id);
		unit.set_component_property_vector3   (_component_id, "data.position", _position.value);
		unit.set_component_property_quaternion(_component_id, "data.rotation", _rotation.value);
		unit.set_component_property_vector3   (_component_id, "data.scale", _scale.value);

		_db.add_restore_point((int)ActionType.SET_TRANSFORM, new Guid?[] { _id, _component_id });
	}

	public override void update()
	{
		Unit unit = new Unit(_db, _id);
		_position.value = unit.get_component_property_vector3   (_component_id, "data.position");
		_rotation.value = unit.get_component_property_quaternion(_component_id, "data.rotation");
		_scale.value    = unit.get_component_property_vector3   (_component_id, "data.scale");
	}
}

public class MeshRendererPropertyGrid : PropertyGrid
{
	// Widgets
	private ResourceChooserButton _mesh_resource;
	private EntryText _geometry;
	private ResourceChooserButton _material;
	private CheckBox _visible;

	public MeshRendererPropertyGrid(Database db, ProjectStore store)
	{
		base(db);

		// Widgets
		_mesh_resource = new ResourceChooserButton(store, "mesh");
		_mesh_resource.value_changed.connect(on_value_changed);
		_geometry = new EntryText();
		_geometry.sensitive = false;
		_material = new ResourceChooserButton(store, "material");
		_material.value_changed.connect(on_value_changed);
		_visible = new CheckBox();
		_visible.value_changed.connect(on_value_changed);

		add_row("Mesh", _mesh_resource);
		add_row("Geometry", _geometry);
		add_row("Material", _material);
		add_row("Visible", _visible);
	}

	private void on_value_changed()
	{
		Unit unit = new Unit(_db, _id);
		unit.set_component_property_string(_component_id, "data.mesh_resource", _mesh_resource.value);
		unit.set_component_property_string(_component_id, "data.geometry_name", _geometry.text);
		unit.set_component_property_string(_component_id, "data.material", _material.value);
		unit.set_component_property_bool  (_component_id, "data.visible", _visible.value);

		_db.add_restore_point((int)ActionType.SET_MESH, new Guid?[] { _id, _component_id });
	}

	public override void update()
	{
		Unit unit = new Unit(_db, _id);
		_mesh_resource.value = unit.get_component_property_string(_component_id, "data.mesh_resource");
		_geometry.text       = unit.get_component_property_string(_component_id, "data.geometry_name");
		_material.value      = unit.get_component_property_string(_component_id, "data.material");
		_visible.value       = unit.get_component_property_bool  (_component_id, "data.visible");
	}
}

public class SpriteRendererPropertyGrid : PropertyGrid
{
	// Widgets
	private ResourceChooserButton _sprite_resource;
	private ResourceChooserButton _material;
	private EntryDouble _layer;
	private EntryDouble _depth;
	private CheckBox _visible;

	public SpriteRendererPropertyGrid(Database db, ProjectStore store)
	{
		base(db);

		// Widgets
		_sprite_resource = new ResourceChooserButton(store, "sprite");
		_sprite_resource.value_changed.connect(on_value_changed);
		_material = new ResourceChooserButton(store, "material");
		_material.value_changed.connect(on_value_changed);
		_layer = new EntryDouble(0.0, 0.0, 7.0);
		_layer.value_changed.connect(on_value_changed);
		_depth = new EntryDouble(0.0, 0.0, (double)uint32.MAX);
		_depth.value_changed.connect(on_value_changed);
		_visible = new CheckBox();
		_visible.value_changed.connect(on_value_changed);

		add_row("Sprite", _sprite_resource);
		add_row("Material", _material);
		add_row("Layer", _layer);
		add_row("Depth", _depth);
		add_row("Visible", _visible);
	}

	private void on_value_changed()
	{
		Unit unit = new Unit(_db, _id);
		unit.set_component_property_string(_component_id, "data.sprite_resource", _sprite_resource.value);
		unit.set_component_property_string(_component_id, "data.material", _material.value);
		unit.set_component_property_double(_component_id, "data.layer", _layer.value);
		unit.set_component_property_double(_component_id, "data.depth", _depth.value);
		unit.set_component_property_bool  (_component_id, "data.visible", _visible.value);

		_db.add_restore_point((int)ActionType.SET_SPRITE, new Guid?[] { _id, _component_id });
	}

	public override void update()
	{
		Unit unit = new Unit(_db, _id);
		_sprite_resource.value = unit.get_component_property_string(_component_id, "data.sprite_resource");
		_material.value        = unit.get_component_property_string(_component_id, "data.material");
		_layer.value           = unit.get_component_property_double(_component_id, "data.layer");
		_depth.value           = unit.get_component_property_double(_component_id, "data.depth");
		_visible.value         = unit.get_component_property_bool  (_component_id, "data.visible");
	}
}

public class LightPropertyGrid : PropertyGrid
{
	// Widgets
	private ComboBoxMap _type;
	private EntryDouble _range;
	private EntryDouble _intensity;
	private EntryDouble _spot_angle;
	private ColorButtonVector3 _color;

	public LightPropertyGrid(Database db)
	{
		base(db);

		// Widgets
		_type = new ComboBoxMap();
		_type.value_changed.connect(on_value_changed);
		_type.append("directional", "Directional");
		_type.append("omni", "Omni");
		_type.append("spot", "Spot");
		_range = new EntryDouble(0.0, 0.0, double.MAX);
		_range.value_changed.connect(on_value_changed);
		_intensity = new EntryDouble(0.0, 0.0,  double.MAX);
		_intensity.value_changed.connect(on_value_changed);
		_spot_angle = new EntryDouble(0.0, 0.0,  90.0);
		_spot_angle.value_changed.connect(on_value_changed);
		_color = new ColorButtonVector3();
		_color.value_changed.connect(on_value_changed);

		add_row("Type", _type);
		add_row("Range", _range);
		add_row("Intensity", _intensity);
		add_row("Spot Angle", _spot_angle);
		add_row("Color", _color);
	}

	private void on_value_changed()
	{
		Unit unit = new Unit(_db, _id);
		unit.set_component_property_string (_component_id, "data.type",       _type.value);
		unit.set_component_property_double (_component_id, "data.range",      _range.value);
		unit.set_component_property_double (_component_id, "data.intensity",  _intensity.value);
		unit.set_component_property_double (_component_id, "data.spot_angle", _spot_angle.value * (Math.PI/180.0));
		unit.set_component_property_vector3(_component_id, "data.color",      _color.value);

		_db.add_restore_point((int)ActionType.SET_LIGHT, new Guid?[] { _id, _component_id });
	}

	public override void update()
	{
		Unit unit = new Unit(_db, _id);
		_type.value       = unit.get_component_property_string (_component_id, "data.type");
		_range.value      = unit.get_component_property_double (_component_id, "data.range");
		_intensity.value  = unit.get_component_property_double (_component_id, "data.intensity");
		_spot_angle.value = unit.get_component_property_double (_component_id, "data.spot_angle") * (180.0/Math.PI);
		_color.value      = unit.get_component_property_vector3(_component_id, "data.color");
	}
}

public class CameraPropertyGrid : PropertyGrid
{
	// Widgets
	private ComboBoxMap _projection;
	private EntryDouble _fov;
	private EntryDouble _near_range;
	private EntryDouble _far_range;

	public CameraPropertyGrid(Database db)
	{
		base(db);

		// Widgets
		_projection = new ComboBoxMap();
		_projection.append("orthographic", "Orthographic");
		_projection.append("perspective", "Perspective");
		_projection.value_changed.connect(on_value_changed);
		_fov = new EntryDouble(0.0, 1.0,   90.0);
		_fov.value_changed.connect(on_value_changed);
		_near_range = new EntryDouble(0.001, double.MIN, double.MAX);
		_near_range.value_changed.connect(on_value_changed);
		_far_range  = new EntryDouble(1000.000, double.MIN, double.MAX);
		_far_range.value_changed.connect(on_value_changed);

		add_row("Projection", _projection);
		add_row("FOV", _fov);
		add_row("Near Range", _near_range);
		add_row("Far Range", _far_range);
	}

	private void on_value_changed()
	{
		Unit unit = new Unit(_db, _id);
		unit.set_component_property_string(_component_id, "data.projection", _projection.value);
		unit.set_component_property_double(_component_id, "data.fov", _fov.value * (Math.PI/180.0));
		unit.set_component_property_double(_component_id, "data.near_range", _near_range.value);
		unit.set_component_property_double(_component_id, "data.far_range", _far_range.value);

		_db.add_restore_point((int)ActionType.SET_CAMERA, new Guid?[] { _id, _component_id });
	}

	public override void update()
	{
		Unit unit = new Unit(_db, _id);
		_projection.value = unit.get_component_property_string(_component_id, "data.projection");
		_fov.value        = unit.get_component_property_double(_component_id, "data.fov") * (180.0/Math.PI);
		_near_range.value = unit.get_component_property_double(_component_id, "data.near_range");
		_far_range.value  = unit.get_component_property_double(_component_id, "data.far_range");
	}
}

public class ColliderPropertyGrid : PropertyGrid
{
	// Widgets
	private EntryText _source;
	private EntryText _shape;
	private ResourceChooserButton _scene;
	private EntryText _name;

	public ColliderPropertyGrid(Database db, ProjectStore store)
	{
		base(db);

		// Widgets
		_source = new EntryText();
		_source.sensitive = false;
		_shape = new EntryText();
		_shape.sensitive = false;
		_scene = new ResourceChooserButton(store, "mesh");
		_scene.sensitive = false;
		_scene.value_changed.connect(on_value_changed);
		_scene.sensitive = false;
		_name = new EntryText();
		_name.sensitive = false;

		add_row("Source", _source);
		add_row("Shape", _shape);
		add_row("Scene", _scene);
		add_row("Name", _name);
	}

	private void on_value_changed()
	{
		Unit unit = new Unit(_db, _id);
		unit.set_component_property_string(_component_id, "data.shape", _shape.text);
		unit.set_component_property_string(_component_id, "data.scene", _scene.value);
		unit.set_component_property_string(_component_id, "data.name", _name.text);

		_db.add_restore_point((int)ActionType.SET_COLLIDER, new Guid?[] { _id, _component_id });
	}

	public override void update()
	{
		Unit unit = new Unit(_db, _id);

		Value? source = unit.get_component_property(_component_id, "data.source");
		if (source != null) {
			if ((string)source == "inline") {
				_source.text = "inline";
				_shape.text  = "";
				_scene.value = "";
				_name.text   = "";
			} else {
				_source.text = "mesh";
				_shape.text  = unit.get_component_property_string(_component_id, "data.shape");
				_scene.value = unit.get_component_property_string(_component_id, "data.scene");
				_name.text   = unit.get_component_property_string(_component_id, "data.name");
			}
		} else {
			_source.text = "mesh";
			_shape.text  = unit.get_component_property_string(_component_id, "data.shape");
			_scene.value = unit.get_component_property_string(_component_id, "data.scene");
			_name.text   = unit.get_component_property_string(_component_id, "data.name");
		}
	}
}

public class ActorPropertyGrid : PropertyGrid
{
	// Widgets
	private ComboBoxMap _class;
	private EntryText _collision_filter;
	private EntryDouble _mass;
	private EntryText _material;

	public ActorPropertyGrid(Database db)
	{
		base(db);

		// Widgets
		_class = new ComboBoxMap();
		_class.append("static", "static");
		_class.append("dynamic", "dynamic");
		_class.append("keyframed", "keyframed");
		_class.append("trigger", "trigger");
		_class.value_changed.connect(on_value_changed);
		_collision_filter = new EntryText();
		_collision_filter.sensitive = false;
		_material = new EntryText();
		_material.sensitive = false;
		_mass = new EntryDouble(1.0, 0.0, double.MAX);
		_mass.value_changed.connect(on_value_changed);

		add_row("Class", _class);
		add_row("Collision Filter", _collision_filter);
		add_row("Material", _material);
		add_row("Mass", _mass);
	}

	private void on_value_changed()
	{
		Unit unit = new Unit(_db, _id);
		unit.set_component_property_string(_component_id, "data.class", _class.value);
		unit.set_component_property_string(_component_id, "data.collision_filter", _collision_filter.text);
		unit.set_component_property_string(_component_id, "data.material", _material.text);
		unit.set_component_property_double(_component_id, "data.mass", _mass.value);
		unit.set_component_property_bool  (_component_id, "data.lock_rotation_x", (bool)unit.get_component_property_bool(_component_id, "data.lock_rotation_x"));
		unit.set_component_property_bool  (_component_id, "data.lock_rotation_y", (bool)unit.get_component_property_bool(_component_id, "data.lock_rotation_y"));
		unit.set_component_property_bool  (_component_id, "data.lock_rotation_z", (bool)unit.get_component_property_bool(_component_id, "data.lock_rotation_z"));
		unit.set_component_property_bool  (_component_id, "data.lock_translation_x", (bool)unit.get_component_property_bool(_component_id, "data.lock_translation_x"));
		unit.set_component_property_bool  (_component_id, "data.lock_translation_y", (bool)unit.get_component_property_bool(_component_id, "data.lock_translation_y"));
		unit.set_component_property_bool  (_component_id, "data.lock_translation_z", (bool)unit.get_component_property_bool(_component_id, "data.lock_translation_z"));

		_db.add_restore_point((int)ActionType.SET_ACTOR, new Guid?[] { _id, _component_id });
	}

	public override void update()
	{
		Unit unit = new Unit(_db, _id);
		_class.value           = unit.get_component_property_string(_component_id, "data.class");
		_collision_filter.text = unit.get_component_property_string(_component_id, "data.collision_filter");
		_material.text         = unit.get_component_property_string(_component_id, "data.material");
		_mass.value            = unit.get_component_property_double(_component_id, "data.mass");
	}
}

public class ScriptPropertyGrid : PropertyGrid
{
	// Widgets
	private ResourceChooserButton _script_resource;

	public ScriptPropertyGrid(Database db, ProjectStore store)
	{
		base(db);

		// Widgets
		_script_resource = new ResourceChooserButton(store, "lua");
		_script_resource.value_changed.connect(on_value_changed);

		add_row("Script", _script_resource);
	}

	private void on_value_changed()
	{
		Unit unit = new Unit(_db, _id);
		unit.set_component_property_string(_component_id, "data.script_resource", _script_resource.value);

		_db.add_restore_point((int)ActionType.SET_SCRIPT, new Guid?[] { _id, _component_id });
	}

	public override void update()
	{
		Unit unit = new Unit(_db, _id);
		_script_resource.value = unit.get_component_property_string(_component_id, "data.script_resource");
	}
}

public class AnimationStateMachine : PropertyGrid
{
	// Widgets
	private ResourceChooserButton _state_machine_resource;

	public AnimationStateMachine(Database db, ProjectStore store)
	{
		base(db);

		// Widgets
		_state_machine_resource = new ResourceChooserButton(store, "state_machine");
		_state_machine_resource.value_changed.connect(on_value_changed);

		add_row("State Machine", _state_machine_resource);
	}

	private void on_value_changed()
	{
		Unit unit = new Unit(_db, _id);
		unit.set_component_property_string(_component_id, "data.state_machine_resource", _state_machine_resource.value);

		_db.add_restore_point((int)ActionType.SET_ANIMATION_STATE_MACHINE, new Guid?[] { _id, _component_id });
	}

	public override void update()
	{
		Unit unit = new Unit(_db, _id);
		_state_machine_resource.value = unit.get_component_property_string(_component_id, "data.state_machine_resource");
	}
}

public class UnitView : PropertyGrid
{
	// Widgets
	private ResourceChooserButton _prefab;

	public UnitView(Database db, ProjectStore store)
	{
		base(db);

		// Widgets
		_prefab = new ResourceChooserButton(store, "unit");
		_prefab._selector.sensitive = false;

		add_row("Prefab", _prefab);
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

public class SoundTransformView : PropertyGrid
{
	// Widgets
	private EntryVector3 _position;
	private EntryRotation _rotation;

	public SoundTransformView(Database db)
	{
		base(db);

		// Widgets
		_position = new EntryPosition();
		_rotation = new EntryRotation();

		_position.value_changed.connect(on_value_changed);
		_rotation.value_changed.connect(on_value_changed);

		add_row("Position", _position);
		add_row("Rotation", _rotation);
	}

	private void on_value_changed()
	{
		_db.set_property_vector3   (_id, "position", _position.value);
		_db.set_property_quaternion(_id, "rotation", _rotation.value);

		_db.add_restore_point((int)ActionType.SET_SOUND, new Guid?[] { _id });
	}

	public override void update()
	{
		Vector3 pos    = _db.get_property_vector3   (_id, "position");
		Quaternion rot = _db.get_property_quaternion(_id, "rotation");

		_position.value = pos;
		_rotation.value = rot;
	}
}

public class SoundView : PropertyGrid
{
	// Widgets
	private ResourceChooserButton _name;
	private EntryDouble _range;
	private EntryDouble _volume;
	private CheckBox _loop;

	public SoundView(Database db, ProjectStore store)
	{
		base(db);

		// Widgets
		_name   = new ResourceChooserButton(store, "sound");
		_name.value_changed.connect(on_value_changed);
		_range  = new EntryDouble(1.0, 0.0, double.MAX);
		_range.value_changed.connect(on_value_changed);
		_volume = new EntryDouble(1.0, 0.0, 1.0);
		_volume.value_changed.connect(on_value_changed);
		_loop   = new CheckBox();
		_loop.value_changed.connect(on_value_changed);

		add_row("Name", _name);
		add_row("Range", _range);
		add_row("Volume", _volume);
		add_row("Loop", _loop);
	}

	private void on_value_changed()
	{
		_db.set_property_string(_id, "name", _name.value);
		_db.set_property_double(_id, "range", _range.value);
		_db.set_property_double(_id, "volume", _volume.value);
		_db.set_property_bool  (_id, "loop", _loop.value);

		_db.add_restore_point((int)ActionType.SET_SOUND, new Guid?[] { _id });
	}

	public override void update()
	{
		_name.value   = _db.get_property_string(_id, "name");
		_range.value  = _db.get_property_double(_id, "range");
		_volume.value = _db.get_property_double(_id, "volume");
		_loop.value   = _db.get_property_bool  (_id, "loop");
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
	private HashMap<string, Gtk.Expander> _expanders;
	private HashMap<string, PropertyGrid> _objects;
	private ArrayList<ComponentEntry?> _entries;
	private Gee.ArrayList<Guid?>? _selection;

	// Widgets
	private Gtk.Label _nothing_to_show;
	private Gtk.Label _unknown_object_type;
	private Gtk.Viewport _viewport;
	private Gtk.ScrolledWindow _scrolled_window;
	private PropertyGridSet _object_view;
	private Gtk.Stack _stack;

	public PropertiesView(Database db, ProjectStore store)
	{
		// Data
		_db = db;

		_expanders = new HashMap<string, Gtk.Expander>();
		_objects = new HashMap<string, PropertyGrid>();
		_entries = new ArrayList<ComponentEntry?>();
		_selection = null;

		// Widgets
		_object_view = new PropertyGridSet();
		_object_view.border_width = 6;

		// Unit
		register_object_type("Unit",                    "name",                              0, new UnitView(_db, store));
		register_object_type("Transform",               OBJECT_TYPE_TRANSFORM,               0, new TransformPropertyGrid(_db));
		register_object_type("Light",                   OBJECT_TYPE_LIGHT,                   1, new LightPropertyGrid(_db));
		register_object_type("Camera",                  OBJECT_TYPE_CAMERA,                  2, new CameraPropertyGrid(_db));
		register_object_type("Mesh Renderer",           OBJECT_TYPE_MESH_RENDERER,           3, new MeshRendererPropertyGrid(_db, store));
		register_object_type("Sprite Renderer",         OBJECT_TYPE_SPRITE_RENDERER,         3, new SpriteRendererPropertyGrid(_db, store));
		register_object_type("Collider",                OBJECT_TYPE_COLLIDER,                3, new ColliderPropertyGrid(_db, store));
		register_object_type("Actor",                   OBJECT_TYPE_ACTOR,                   3, new ActorPropertyGrid(_db));
		register_object_type("Script",                  OBJECT_TYPE_SCRIPT,                  3, new ScriptPropertyGrid(_db, store));
		register_object_type("Animation State Machine", OBJECT_TYPE_ANIMATION_STATE_MACHINE, 3, new AnimationStateMachine(_db, store));

		// Sound
		register_object_type("Transform", "sound_transform",  0, new SoundTransformView(_db));
		register_object_type("Sound",     "sound_properties", 1, new SoundView(_db, store));

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
	}

	private void register_object_type(string label, string object_type, int position, PropertyGrid cv)
	{
		Gtk.Expander expander = _object_view.add_property_grid(cv, label);
		_objects[object_type] = cv;
		_expanders[object_type] = expander;
		_entries.add({ object_type, position });
	}

	public void show_unit(Guid id)
	{
		_stack.set_visible_child(_scrolled_window);

		foreach (var entry in _entries) {
			Gtk.Expander expander = _expanders[entry.type];

			Unit unit = new Unit(_db, id);
			Guid component_id;
			if (unit.has_component(out component_id, entry.type) || entry.type == "name") {
				PropertyGrid cv = _objects[entry.type];
				cv._id = id;
				cv._component_id = component_id;
				cv.update();
				expander.show_all();
			} else {
				expander.hide();
			}
		}
	}

	public void show_sound_source(Guid id)
	{
		_stack.set_visible_child(_scrolled_window);

		foreach (var entry in _entries) {
			Gtk.Expander expander = _expanders[entry.type];

			if (entry.type == "sound_transform" || entry.type == "sound_properties") {
				PropertyGrid cv = _objects[entry.type];
				cv._id = id;
				cv.update();
				expander.show_all();
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
}

} /* namespace Crown */
