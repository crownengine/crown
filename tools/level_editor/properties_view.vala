/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gee;

namespace Crown
{
public class PropertyRow : Gtk.Bin
{
	public PropertyRow(Gtk.Widget child)
	{
		this.hexpand = true;
		add(child);
	}
}

public class ComponentView : Gtk.Grid
{
	// Data
	public Guid _id;
	public Guid _component_id;
	public int _rows;

	public ComponentView()
	{
		// Data
		_id = GUID_ZERO;
		_component_id = GUID_ZERO;
		_rows = 0;
	}

	public void add_row(string label, Gtk.Widget w)
	{
		this.row_spacing = 6;
		this.column_spacing = 12;

		Gtk.Label l = new Label(label);
		l.width_chars = 13;
		l.set_alignment(1.0f, 0.5f);

		PropertyRow r = new PropertyRow(w);

		this.attach(l, 0, (int)_rows);
		this.attach(r, 1, (int)_rows);
		++_rows;
	}

	public virtual void update()
	{
	}
}

public class TransformComponentView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private EntryPosition _position;
	private EntryRotation _rotation;
	private EntryScale _scale;

	public TransformComponentView(Level level)
	{
		// Data
		_level = level;
		_id = GUID_ZERO;
		_component_id = GUID_ZERO;

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
		_level.move_selected_objects(_position.value
			, _rotation.value
			, _scale.value
			);
	}

	public override void update()
	{
		Unit unit = new Unit(_level._db, _id, _level._prefabs);
		_position.value = unit.get_component_property_vector3   (_component_id, "data.position");
		_rotation.value = unit.get_component_property_quaternion(_component_id, "data.rotation");
		_scale.value    = unit.get_component_property_vector3   (_component_id, "data.scale");
	}
}

public class MeshRendererComponentView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private ResourceChooserButton _mesh_resource;
	private EntryText _geometry;
	private ResourceChooserButton _material;
	private CheckBox _visible;

	public MeshRendererComponentView(Level level, ProjectStore store)
	{
		// Data
		_level = level;

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
		_level.set_mesh(_id
			, _component_id
			, _mesh_resource.value
			, _geometry.text
			, _material.value
			, _visible.value
			);
	}

	public override void update()
	{
		Unit unit = new Unit(_level._db, _id, _level._prefabs);
		_mesh_resource.value = unit.get_component_property_string(_component_id, "data.mesh_resource");
		_geometry.text       = unit.get_component_property_string(_component_id, "data.geometry_name");
		_material.value      = unit.get_component_property_string(_component_id, "data.material");
		_visible.value       = unit.get_component_property_bool  (_component_id, "data.visible");
	}
}

public class SpriteRendererComponentView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private ResourceChooserButton _sprite_resource;
	private ResourceChooserButton _material;
	private EntryDouble _layer;
	private EntryDouble _depth;
	private CheckBox _visible;

	public SpriteRendererComponentView(Level level, ProjectStore store)
	{
		// Data
		_level = level;

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
		_level.set_sprite(_id
			, _component_id
			, _layer.value
			, _depth.value
			, _material.value
			, _sprite_resource.value
			, _visible.value
			);
	}

	public override void update()
	{
		Unit unit = new Unit(_level._db, _id, _level._prefabs);
		_sprite_resource.value = unit.get_component_property_string(_component_id, "data.sprite_resource");
		_material.value        = unit.get_component_property_string(_component_id, "data.material");
		_layer.value           = unit.get_component_property_double(_component_id, "data.layer");
		_depth.value           = unit.get_component_property_double(_component_id, "data.depth");
		_visible.value         = unit.get_component_property_bool  (_component_id, "data.visible");
	}
}

public class LightComponentView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private ComboBoxMap _type;
	private EntryDouble _range;
	private EntryDouble _intensity;
	private EntryDouble _spot_angle;
	private ColorButtonVector3 _color;

	public LightComponentView(Level level)
	{
		// Data
		_level = level;

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
		_level.set_light(_id
			, _component_id
			, _type.value
			, _range.value
			, _intensity.value
			, _spot_angle.value*(Math.PI/180.0)
			, _color.value
			);
	}

	public override void update()
	{
		Unit unit = new Unit(_level._db, _id, _level._prefabs);
		string type       = unit.get_component_property_string (_component_id, "data.type");
		double range      = unit.get_component_property_double (_component_id, "data.range");
		double intensity  = unit.get_component_property_double (_component_id, "data.intensity");
		double spot_angle = unit.get_component_property_double (_component_id, "data.spot_angle");
		Vector3 color     = unit.get_component_property_vector3(_component_id, "data.color");

		_type.value       = type;
		_range.value      = range;
		_intensity.value  = intensity;
		_spot_angle.value = spot_angle*(180.0/Math.PI);
		_color.value      = color;
	}
}

public class CameraComponentView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private ComboBoxMap _projection;
	private EntryDouble _fov;
	private EntryDouble _near_range;
	private EntryDouble _far_range;

	public CameraComponentView(Level level)
	{
		// Data
		_level = level;

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
		_level.set_camera(_id
			, _component_id
			, _projection.value
			, _fov.value*(Math.PI/180.0)
			, _near_range.value
			, _far_range.value
			);
	}

	public override void update()
	{
		Unit unit = new Unit(_level._db, _id, _level._prefabs);

		_projection.value = unit.get_component_property_string(_component_id, "data.projection");
		_fov.value        = unit.get_component_property_double(_component_id, "data.fov") * (180.0/Math.PI);
		_near_range.value = unit.get_component_property_double(_component_id, "data.near_range");
		_far_range.value  = unit.get_component_property_double(_component_id, "data.far_range");
	}
}

public class ColliderComponentView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private EntryText _source;
	private EntryText _shape;
	private ResourceChooserButton _scene;
	private EntryText _name;

	public ColliderComponentView(Level level, ProjectStore store)
	{
		// Data
		_level = level;

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
		if (_source.text == "mesh")
		{
			_level.set_collider(_id
				, _component_id
				, _shape.text
				, _scene.value
				, _name.text
				);
		}
	}

	public override void update()
	{
		Unit unit = new Unit(_level._db, _id, _level._prefabs);

		Value? source = unit.get_component_property(_component_id, "data.source");
		if (source != null)
		{
			if ((string)source == "inline")
			{
				_source.text = "inline";
				_shape.text  = "";
				_scene.value = "";
				_name.text   = "";
			}
			else
			{
				_source.text = "mesh";
				_shape.text  = unit.get_component_property_string(_component_id, "data.shape");
				_scene.value = unit.get_component_property_string(_component_id, "data.scene");
				_name.text   = unit.get_component_property_string(_component_id, "data.name");
			}
		}
		else
		{
			_source.text = "mesh";
			_shape.text  = unit.get_component_property_string(_component_id, "data.shape");
			_scene.value = unit.get_component_property_string(_component_id, "data.scene");
			_name.text   = unit.get_component_property_string(_component_id, "data.name");
		}
	}
}

public class ActorComponentView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private ComboBoxMap _class;
	private EntryText _collision_filter;
	private EntryDouble _mass;
	private EntryText _material;

	public ActorComponentView(Level level)
	{
		// Data
		_level = level;

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
		_level.set_actor(_id
			, _component_id
			, _class.value
			, _collision_filter.text
			, _material.text
			, _mass.value
			);
	}

	public override void update()
	{
		Unit unit = new Unit(_level._db, _id, _level._prefabs);
		_class.value           = unit.get_component_property_string(_component_id, "data.class");
		_collision_filter.text = unit.get_component_property_string(_component_id, "data.collision_filter");
		_material.text         = unit.get_component_property_string(_component_id, "data.material");
		_mass.value            = unit.get_component_property_double(_component_id, "data.mass");
	}
}

public class ScriptComponentView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private ResourceChooserButton _script_resource;

	public ScriptComponentView(Level level, ProjectStore store)
	{
		// Data
		_level = level;

		// Widgets
		_script_resource = new ResourceChooserButton(store, "lua");
		_script_resource.value_changed.connect(on_value_changed);

		add_row("Script", _script_resource);
	}

	private void on_value_changed()
	{
		_level.set_script(_id
			, _component_id
			, _script_resource.value
			);
	}

	public override void update()
	{
		Unit unit = new Unit(_level._db, _id, _level._prefabs);
		_script_resource.value = unit.get_component_property_string(_component_id, "data.script_resource");
	}
}

public class AnimationStateMachine : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private ResourceChooserButton _state_machine_resource;

	public AnimationStateMachine(Level level, ProjectStore store)
	{
		// Data
		_level = level;

		// Widgets
		_state_machine_resource = new ResourceChooserButton(store, "state_machine");
		_state_machine_resource.value_changed.connect(on_value_changed);

		add_row("State Machine", _state_machine_resource);
	}

	private void on_value_changed()
	{
		_level.set_animation_state_machine(_id
			, _component_id
			, _state_machine_resource.value
			);
	}

	public override void update()
	{
		Unit unit = new Unit(_level._db, _id, _level._prefabs);
		_state_machine_resource.value = unit.get_component_property_string(_component_id, "data.state_machine_resource");
	}
}

public class UnitView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private EntryText _unit_name;

	public UnitView(Level level)
	{
		// Data
		_level = level;

		// Widgets
		_unit_name = new EntryText();
		_unit_name.sensitive = false;

		add_row("Name", _unit_name);
	}

	public override void update()
	{
		if (_level._db.has_property(_id, "prefab"))
			_unit_name.text = _level._db.get_property_string(_id, "prefab");
		else
			_unit_name.text = "<none>";
	}
}

public class SoundTransformView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private EntryVector3 _position;
	private EntryRotation _rotation;

	public SoundTransformView(Level level)
	{
		// Data
		_level = level;
		_id = GUID_ZERO;

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
		Vector3 pos    = _position.value;
		Quaternion rot = _rotation.value;

		_level.move_selected_objects(pos, rot, Vector3(1.0, 1.0, 1.0));
	}

	public override void update()
	{
		Vector3 pos    = _level._db.get_property_vector3   (_id, "position");
		Quaternion rot = _level._db.get_property_quaternion(_id, "rotation");

		_position.value = pos;
		_rotation.value = rot;
	}
}

public class SoundView : ComponentView
{
	// Data
	Level _level;

	// Widgets
	private ResourceChooserButton _name;
	private EntryDouble _range;
	private EntryDouble _volume;
	private CheckBox _loop;

	public SoundView(Level level, ProjectStore store)
	{
		// Data
		_level = level;

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
		_level.set_sound(_id
			, _name.value
			, _range.value
			, _volume.value
			, _loop.value
			);
	}

	public override void update()
	{
		_name.value   = _level._db.get_property_string(_id, "name");
		_range.value  = _level._db.get_property_double(_id, "range");
		_volume.value = _level._db.get_property_double(_id, "volume");
		_loop.value   = _level._db.get_property_bool  (_id, "loop");
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
	private Level _level;
	private HashMap<string, Gtk.Expander> _expanders;
	private HashMap<string, ComponentView> _components;
	private ArrayList<ComponentEntry?> _entries;

	// Widgets
	private Gtk.Label _nothing_to_show;
	private Gtk.Viewport _viewport;
	private Gtk.ScrolledWindow _scrolled_window;
	private Gtk.Box _components_vbox;
	private Gtk.Widget _current_widget;

	public PropertiesView(Level level, ProjectStore store)
	{
		// Data
		_level = level;
		_level.selection_changed.connect(on_selection_changed);

		_expanders = new HashMap<string, Gtk.Expander>();
		_components = new HashMap<string, ComponentView>();
		_entries = new ArrayList<ComponentEntry?>();

		// Widgets
		_components_vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_components_vbox.margin_bottom = 18;

		// Unit
		add_component_view("Unit",                    "name",                    0, new UnitView(_level));
		add_component_view("Transform",               "transform",               0, new TransformComponentView(_level));
		add_component_view("Light",                   "light",                   1, new LightComponentView(_level));
		add_component_view("Camera",                  "camera",                  2, new CameraComponentView(_level));
		add_component_view("Mesh Renderer",           "mesh_renderer",           3, new MeshRendererComponentView(_level, store));
		add_component_view("Sprite Renderer",         "sprite_renderer",         3, new SpriteRendererComponentView(_level, store));
		add_component_view("Collider",                "collider",                3, new ColliderComponentView(_level, store));
		add_component_view("Actor",                   "actor",                   3, new ActorComponentView(_level));
		add_component_view("Script",                  "script",                  3, new ScriptComponentView(_level, store));
		add_component_view("Animation State Machine", "animation_state_machine", 3, new AnimationStateMachine(_level, store));

		// Sound
		add_component_view("Transform", "sound_transform",  0, new SoundTransformView(_level));
		add_component_view("Sound",     "sound_properties", 1, new SoundView(_level, store));

		_entries.sort((a, b) => { return (a.position < b.position ? -1 : 1); });
		foreach (var entry in _entries)
			_components_vbox.pack_start(_expanders[entry.type], false, true, 0);

		_nothing_to_show = new Gtk.Label("Nothing to show");

		_viewport = new Gtk.Viewport(null, null);
		_viewport.add(_components_vbox);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.add(_viewport);

		_current_widget = null;

		this.get_style_context().add_class("properties-view");
		this.set_current_widget(_nothing_to_show);
	}

	private void add_component_view(string label, string component_type, int position, ComponentView cv)
	{
		Gtk.Label lb = new Gtk.Label(null);
		lb.set_markup("<b>%s</b>".printf(label));
		lb.set_alignment(0.0f, 0.5f);

		Gtk.Expander expander = new Gtk.Expander("");
		expander.label_widget = lb;
		expander.child = cv;
		expander.expanded = true;
		expander.margin_end = 12;

		_components[component_type] = cv;
		_expanders[component_type] = expander;

		_entries.add({ component_type, position });
	}

	private void set_current_widget(Gtk.Widget w)
	{
		if (_current_widget == w)
			return;

		if (_current_widget != null)
		{
			_current_widget.hide();
			remove(_current_widget);
		}

		_current_widget = w;
		_current_widget.show_all();
		add(_current_widget);
	}

	private void on_selection_changed(Gee.ArrayList<Guid?> selection)
	{
		if (selection.size != 1)
		{
			set_current_widget(_nothing_to_show);
			return;
		}

		Guid id = selection[selection.size - 1];

		if (_level.is_unit(id))
		{
			set_current_widget(_scrolled_window);

			foreach (var entry in _entries)
			{
				Gtk.Expander expander = _expanders[entry.type];

				Unit unit = new Unit(_level._db, id, _level._prefabs);
				Guid component_id;
				if (unit.has_component(out component_id, entry.type) || entry.type == "name")
				{
					ComponentView cv = _components[entry.type];
					cv._id = id;
					cv._component_id = component_id;
					cv.update();
					expander.show_all();
				}
				else
				{
					expander.hide();
				}
			}
		}
		else if (_level.is_sound(id))
		{
			set_current_widget(_scrolled_window);

			foreach (var entry in _entries)
			{
				Gtk.Expander expander = _expanders[entry.type];

				if (entry.type == "sound_transform" || entry.type == "sound_properties")
				{
					ComponentView cv = _components[entry.type];
					cv._id = id;
					cv.update();
					expander.show_all();
				}
				else
				{
					expander.hide();
				}
			}
		}
		else
		{
			set_current_widget(_nothing_to_show);
		}
	}
}

}
