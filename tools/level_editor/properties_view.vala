/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
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
		private SpinButtonVector3 _position;
		private SpinButtonRotation _rotation;
		private SpinButtonVector3 _scale;

		public TransformComponentView(Level level)
		{
			// Data
			_level = level;
			_id = GUID_ZERO;
			_component_id = GUID_ZERO;

			// Widgets
			_position = new SpinButtonVector3(Vector3(0, 0, 0), Vector3(-9999.9, -9999.9, -9999.9), Vector3(9999.9, 9999.9, 9999.9));
			_position.value_changed.connect(on_value_changed);
			_rotation = new SpinButtonRotation(Vector3(0, 0, 0));
			_rotation.value_changed.connect(on_value_changed);
			_scale = new SpinButtonVector3(Vector3(0, 0, 0), Vector3(0.1, 0.1, 0.1), Vector3(10.0, 10.0, 10.0));
			_scale.value_changed.connect(on_value_changed);

			add_row("Position", _position);
			add_row("Rotation", _rotation);
			add_row("Scale", _scale);
		}

		private void on_value_changed()
		{
			Vector3 pos    = _position.value;
			Quaternion rot = _rotation.value;
			Vector3 scl    = _scale.value;

			_level.move_selected_objects(pos, rot, scl);
		}

		public override void update()
		{
			Unit unit = new Unit(_level._db, _id, _level._prefabs);
			Vector3 pos    = unit.get_component_property_vector3   (_component_id, "data.position");
			Quaternion rot = unit.get_component_property_quaternion(_component_id, "data.rotation");
			Vector3 scl    = unit.get_component_property_vector3   (_component_id, "data.scale");

			_position.value = pos;
			_rotation.value = rot;
			_scale.value    = scl;
		}
	}

	public class MeshRendererComponentView : ComponentView
	{
		// Data
		Level _level;

		// Widgets
		private Gtk.Entry _mesh_resource;
		private Gtk.Entry _geometry;
		private Gtk.Entry _material;
		private CheckBox _visible;

		public MeshRendererComponentView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_mesh_resource = new Gtk.Entry();
			_mesh_resource.sensitive = false;
			_geometry = new Gtk.Entry();
			_geometry.sensitive = false;
			_material = new Gtk.Entry();
			_material.sensitive = false;
			_visible = new CheckBox();

			add_row("Mesh", _mesh_resource);
			add_row("Geometry", _geometry);
			add_row("Material", _material);
			add_row("Visible", _visible);
		}

		public override void update()
		{
			Unit unit = new Unit(_level._db, _id, _level._prefabs);
			_mesh_resource.text = unit.get_component_property_string(_component_id, "data.mesh_resource");
			_geometry.text      = unit.get_component_property_string(_component_id, "data.geometry_name");
			_material.text      = unit.get_component_property_string(_component_id, "data.material");
			_visible.value      = unit.get_component_property_bool  (_component_id, "data.visible");
		}
	}

	public class SpriteRendererComponentView : ComponentView
	{
		// Data
		Level _level;

		// Widgets
		private Gtk.Entry _sprite_resource;
		private Gtk.Entry _material;
		private SpinButtonDouble _layer;
		private SpinButtonDouble _depth;
		private CheckBox _visible;

		public SpriteRendererComponentView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_sprite_resource = new Gtk.Entry();
			_material = new Gtk.Entry();
			_layer = new SpinButtonDouble(0.0, 0.0, 7.0);
			_layer.value_changed.connect(on_value_changed);
			_depth = new SpinButtonDouble(0.0, 0.0, 9999.0);
			_depth.value_changed.connect(on_value_changed);
			_visible = new CheckBox();
			_visible.value_changed.connect(on_value_changed);
			_sprite_resource.sensitive = false;
			_material.sensitive = false;

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
				, _material.text
				, _sprite_resource.text
				, _visible.value
				);
		}

		public override void update()
		{
			Unit unit = new Unit(_level._db, _id, _level._prefabs);
			_sprite_resource.text = unit.get_component_property_string(_component_id, "data.sprite_resource");
			_material.text        = unit.get_component_property_string(_component_id, "data.material");
			_layer.value          = unit.get_component_property_double(_component_id, "data.layer");
			_depth.value          = unit.get_component_property_double(_component_id, "data.depth");
			_visible.value        = unit.get_component_property_bool  (_component_id, "data.visible");
		}
	}

	public class LightComponentView : ComponentView
	{
		// Data
		Level _level;

		// Widgets
		private ComboBoxMap _type;
		private SpinButtonDouble _range;
		private SpinButtonDouble _intensity;
		private SpinButtonDouble _spot_angle;
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
			_range = new SpinButtonDouble(0.0, 0.0, 999.0);
			_range.value_changed.connect(on_value_changed);
			_intensity = new SpinButtonDouble(0.0, 0.0,  10.0);
			_intensity.value_changed.connect(on_value_changed);
			_spot_angle = new SpinButtonDouble(0.0, 0.0,  90.0);
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
		private SpinButtonDouble _fov;
		private SpinButtonDouble _near_range;
		private SpinButtonDouble _far_range;

		public CameraComponentView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_projection = new ComboBoxMap();
			_projection.append("orthographic", "Orthographic");
			_projection.append("perspective", "Perspective");
			_projection.value_changed.connect(on_value_changed);
			_fov = new SpinButtonDouble(0.0, 1.0,   90.0);
			_fov.value_changed.connect(on_value_changed);
			_near_range = new SpinButtonDouble(0.0, 0.001, 9999.0);
			_near_range.value_changed.connect(on_value_changed);
			_far_range  = new SpinButtonDouble(0.0, 0.001, 9999.0);
			_far_range.value_changed.connect(on_value_changed);

			add_row("Projection", _projection);
			add_row("FOV", _fov);
			add_row("Near Range", _near_range);
			add_row("Far Range", _far_range);
		}

		private void on_value_changed()
		{
			Unit unit = new Unit(_level._db, _id, _level._prefabs);
			unit.set_component_property_string(_component_id, "data.projection", _projection.value);
			unit.set_component_property_double(_component_id, "data.fov",        _fov.value*(Math.PI/180.0));
			unit.set_component_property_double(_component_id, "data.near_range", _near_range.value);
			unit.set_component_property_double(_component_id, "data.far_range",  _far_range.value);
			unit.set_component_property_string(_component_id, "type", "camera");
		}

		public override void update()
		{
			Unit unit = new Unit(_level._db, _id, _level._prefabs);
			string type       = unit.get_component_property_string(_component_id, "data.projection");
			double fov        = unit.get_component_property_double(_component_id, "data.fov");
			double near_range = unit.get_component_property_double(_component_id, "data.near_range");
			double far_range  = unit.get_component_property_double(_component_id, "data.far_range");

			_projection.value = type;
			_fov.value        = fov*(180.0/Math.PI);
			_near_range.value = near_range;
			_far_range.value  = far_range;
		}
	}

	public class ColliderComponentView : ComponentView
	{
		// Data
		Level _level;

		// Widgets
		private Gtk.Entry _shape;
		private Gtk.Entry _scene;
		private Gtk.Entry _name;
		private Gtk.Entry _material;

		public ColliderComponentView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_shape = new Gtk.Entry();
			_shape.sensitive = false;
			_scene = new Gtk.Entry();
			_scene.sensitive = false;
			_name = new Gtk.Entry();
			_name.sensitive = false;
			_material = new Gtk.Entry();
			_material.sensitive = false;

			add_row("Shape", _shape);
			add_row("Scene", _scene);
			add_row("Name", _name);
			add_row("Material", _material);
		}

		public override void update()
		{
			Unit unit = new Unit(_level._db, _id, _level._prefabs);
			_shape.text    = unit.get_component_property_string(_component_id, "data.shape");
			_scene.text    = unit.get_component_property_string(_component_id, "data.scene");
			_name.text     = unit.get_component_property_string(_component_id, "data.name");
			_material.text = unit.get_component_property_string(_component_id, "data.material");
		}
	}

	public class ActorComponentView : ComponentView
	{
		// Data
		Level _level;

		// Widgets
		private Gtk.Entry _class;
		private Gtk.Entry _collision_filter;
		private SpinButtonDouble _mass;
		private Gtk.Entry _material;

		public ActorComponentView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_class = new Gtk.Entry();
			_collision_filter = new Gtk.Entry();
			_material = new Gtk.Entry();
			_mass = new SpinButtonDouble(1.0, 0.0, 9999.0);
			_mass.value_changed.connect(on_value_changed);
			_class.sensitive = false;
			_collision_filter.sensitive = false;
			_material.sensitive = false;

			add_row("Class", _class);
			add_row("Collision Filter", _collision_filter);
			add_row("Material", _material);
			add_row("Mass", _mass);
		}

		private void on_value_changed()
		{
			Unit unit = new Unit(_level._db, _id, _level._prefabs);
			unit.set_component_property_string(_component_id, "data.class", _class.text);
			unit.set_component_property_string(_component_id, "data.collision_filter", _collision_filter.text);
			unit.set_component_property_string(_component_id, "data.material", _material.text);
			unit.set_component_property_double(_component_id, "data.mass", _mass.value);
			unit.set_component_property_string(_component_id, "type", "actor");
		}

		public override void update()
		{
			Unit unit = new Unit(_level._db, _id, _level._prefabs);
			_class.text            = unit.get_component_property_string(_component_id, "data.class");
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
		private Gtk.Entry _script_resource;

		public ScriptComponentView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_script_resource = new Gtk.Entry();
			_script_resource.sensitive = false;

			add_row("Script", _script_resource);
		}

		public override void update()
		{
			Unit unit = new Unit(_level._db, _id, _level._prefabs);
			_script_resource.text = unit.get_component_property_string(_component_id, "data.script_resource");
		}
	}

	public class AnimationStateMachine : ComponentView
	{
		// Data
		Level _level;

		// Widgets
		private Gtk.Entry _state_machine_resource;

		public AnimationStateMachine(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_state_machine_resource = new Gtk.Entry();
			_state_machine_resource.sensitive = false;

			add_row("State Machine", _state_machine_resource);
		}

		public override void update()
		{
			Unit unit = new Unit(_level._db, _id, _level._prefabs);
			_state_machine_resource.text = unit.get_component_property_string(_component_id, "data.state_machine_resource");
		}
	}

	public class UnitView : ComponentView
	{
		// Data
		Level _level;

		// Widgets
		private Gtk.Entry _unit_name;

		public UnitView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_unit_name = new Gtk.Entry();
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
		private SpinButtonVector3 _position;
		private SpinButtonRotation _rotation;

		public SoundTransformView(Level level)
		{
			// Data
			_level = level;
			_id = GUID_ZERO;

			// Widgets
			_position = new SpinButtonVector3(Vector3(0, 0, 0), Vector3(-9999.9, -9999.9, -9999.9), Vector3(9999.9, 9999.9, 9999.9));
			_rotation = new SpinButtonRotation(Vector3(0, 0, 0));

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
		private Gtk.Entry _name;
		private SpinButtonDouble _range;
		private SpinButtonDouble _volume;
		private CheckBox _loop;

		public SoundView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_name   = new Gtk.Entry();
			_range  = new SpinButtonDouble(0.0, 0.0, 9999.0);
			_volume = new SpinButtonDouble(0.0, 0.0,    1.0);
			_loop   = new CheckBox();
			_name.sensitive = false;

			_range.value_changed.connect(on_value_changed);
			_volume.value_changed.connect(on_value_changed);
			_loop.value_changed.connect(on_value_changed);

			add_row("Name", _name);
			add_row("Range", _range);
			add_row("Volume", _volume);
			add_row("Loop", _loop);
		}

		private void on_value_changed()
		{
			_level.set_sound(_id
				, _name.text
				, _range.value
				, _volume.value
				, _loop.value
				);
		}

		public override void update()
		{
			_name.text    = _level._db.get_property_string(_id, "name");
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

		public PropertiesView(Level level)
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
			add_component_view("Mesh Renderer",           "mesh_renderer",           3, new MeshRendererComponentView(_level));
			add_component_view("Sprite Renderer",         "sprite_renderer",         3, new SpriteRendererComponentView(_level));
			add_component_view("Collider",                "collider",                3, new ColliderComponentView(_level));
			add_component_view("Actor",                   "actor",                   3, new ActorComponentView(_level));
			add_component_view("Script",                  "script",                  3, new ScriptComponentView(_level));
			add_component_view("Animation State Machine", "animation_state_machine", 3, new AnimationStateMachine(_level));

			// Sound
			add_component_view("Transform", "sound_transform",  0, new SoundTransformView(_level));
			add_component_view("Sound",     "sound_properties", 1, new SoundView(_level));

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

			_components[component_type] = cv;
			_expanders[component_type] = expander;

			_entries.add({ component_type, position });
		}

		private void set_current_widget(Gtk.Widget w)
		{
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
					expander.hide();

					Guid component_id = GUID_ZERO;
					Unit unit = new Unit(_level._db, id, _level._prefabs);
					if (unit.has_component(entry.type, ref component_id) || entry.type == "name")
					{
						ComponentView cv = _components[entry.type];
						cv._id = id;
						cv._component_id = component_id;
						cv.update();
						expander.show_all();
					}
				}
			}
			else if (_level.is_sound(id))
			{
				set_current_widget(_scrolled_window);

				foreach (var entry in _entries)
				{
					Gtk.Expander expander = _expanders[entry.type];
					expander.hide();

					if (entry.type == "sound_transform" || entry.type == "sound_properties")
					{
						ComponentView cv = _components[entry.type];
						cv._id = id;
						cv.update();
						expander.show_all();
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
