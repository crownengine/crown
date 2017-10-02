/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
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
			l.width_chars = 10;
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
			_rotation = new SpinButtonRotation(Vector3(0, 0, 0));
			_scale    = new SpinButtonVector3(Vector3(0, 0, 0), Vector3(0.1, 0.1, 0.1), Vector3(10.0, 10.0, 10.0));

			_position.value_changed.connect(on_value_changed);
			_rotation.value_changed.connect(on_value_changed);
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
			Vector3 pos    = (Vector3)   _level.get_component_property(_id, _component_id, "data.position");
			Quaternion rot = (Quaternion)_level.get_component_property(_id, _component_id, "data.rotation");
			Vector3 scl    = (Vector3)   _level.get_component_property(_id, _component_id, "data.scale");

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
		private Gtk.CheckButton _visible;

		public MeshRendererComponentView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_mesh_resource = new Gtk.Entry();
			_geometry = new Gtk.Entry();
			_material = new Gtk.Entry();
			_visible = new Gtk.CheckButton();
			_mesh_resource.sensitive = false;
			_geometry.sensitive = false;
			_material.sensitive = false;

			add_row("Mesh", _mesh_resource);
			add_row("Geometry", _geometry);
			add_row("Material", _material);
			add_row("Visible", _visible);
		}

		public override void update()
		{
			_mesh_resource.text = (string)_level.get_component_property(_id, _component_id, "data.mesh_resource");
			_geometry.text      = (string)_level.get_component_property(_id, _component_id, "data.geometry_name");
			_material.text      = (string)_level.get_component_property(_id, _component_id, "data.material");
			_visible.active     = (bool)  _level.get_component_property(_id, _component_id, "data.visible");
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
		private Gtk.CheckButton _visible;

		public SpriteRendererComponentView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_sprite_resource = new Gtk.Entry();
			_material = new Gtk.Entry();
			_layer = new SpinButtonDouble(0.0, 0.0, 7.0);
			_depth = new SpinButtonDouble(0.0, 0.0, 9999.0);
			_visible = new Gtk.CheckButton();
			_sprite_resource.sensitive = false;
			_material.sensitive = false;

			_layer.value_changed.connect(on_value_changed);
			_depth.value_changed.connect(on_value_changed);

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
				);
		}

		public override void update()
		{
			_sprite_resource.text = (string)_level.get_component_property(_id, _component_id, "data.sprite_resource");
			_material.text        = (string)_level.get_component_property(_id, _component_id, "data.material");
			_layer.value          = (double)_level.get_component_property(_id, _component_id, "data.layer");
			_depth.value          = (double)_level.get_component_property(_id, _component_id, "data.depth");
			_visible.active       = (bool)  _level.get_component_property(_id, _component_id, "data.visible");
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
			_type       = new ComboBoxMap();
			_type.append("directional", "Directional");
			_type.append("omni", "Omni");
			_type.append("spot", "Spot");
			_range      = new SpinButtonDouble(0.0, 0.0, 999.0);
			_intensity  = new SpinButtonDouble(0.0, 0.0,  10.0);
			_spot_angle = new SpinButtonDouble(0.0, 0.0,  90.0);
			_color      = new ColorButtonVector3();

			_type.value_changed.connect(on_value_changed);
			_range.value_changed.connect(on_value_changed);
			_intensity.value_changed.connect(on_value_changed);
			_spot_angle.value_changed.connect(on_value_changed);
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
			string type       = (string) _level.get_component_property(_id, _component_id, "data.type");
			double range      = (double) _level.get_component_property(_id, _component_id, "data.range");
			double intensity  = (double) _level.get_component_property(_id, _component_id, "data.intensity");
			double spot_angle = (double) _level.get_component_property(_id, _component_id, "data.spot_angle");
			Vector3 color     = (Vector3)_level.get_component_property(_id, _component_id, "data.color");

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
			_fov        = new SpinButtonDouble(0.0, 1.0,   90.0);
			_near_range = new SpinButtonDouble(0.0, 0.001, 9999.0);
			_far_range  = new SpinButtonDouble(0.0, 0.001, 9999.0);

			_projection.value_changed.connect(on_value_changed);
			_fov.value_changed.connect(on_value_changed);
			_near_range.value_changed.connect(on_value_changed);
			_far_range.value_changed.connect(on_value_changed);

			add_row("Projection", _projection);
			add_row("FOV", _fov);
			add_row("Near Range", _near_range);
			add_row("Far Range", _far_range);
		}

		private void on_value_changed()
		{
			_level.set_component_property(_id, _component_id, "data.projection", _projection.value);
			_level.set_component_property(_id, _component_id, "data.fov",        _fov.value*(Math.PI/180.0));
			_level.set_component_property(_id, _component_id, "data.near_range", _near_range.value);
			_level.set_component_property(_id, _component_id, "data.far_range",  _far_range.value);
			_level.set_component_property(_id, _component_id, "type", "camera");
		}

		public override void update()
		{
			string type       = (string)_level.get_component_property(_id, _component_id, "data.projection");
			double fov        = (double)_level.get_component_property(_id, _component_id, "data.fov");
			double near_range = (double)_level.get_component_property(_id, _component_id, "data.near_range");
			double far_range  = (double)_level.get_component_property(_id, _component_id, "data.far_range");

			_projection.value = type;
			_fov.value        = fov*(180.0/Math.PI);
			_near_range.value = near_range;
			_far_range.value  = far_range;
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
			_script_resource.text = (string)_level.get_component_property(_id, _component_id, "data.script_resource");
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
			_state_machine_resource.text = (string)_level.get_component_property(_id, _component_id, "data.state_machine_resource");
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
			Value? val = _level.get_property(_id, "prefab");
			_unit_name.text = val == null ? "<none>" : (string)val;
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
			Vector3 pos    = (Vector3)   _level.get_property(_id, "position");
			Quaternion rot = (Quaternion)_level.get_property(_id, "rotation");

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
		private Gtk.CheckButton _loop;

		public SoundView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_name   = new Gtk.Entry();
			_range  = new SpinButtonDouble(0.0, 0.0, 9999.0);
			_volume = new SpinButtonDouble(0.0, 0.0,    1.0);
			_loop   = new Gtk.CheckButton();
			_name.sensitive = false;

			_range.value_changed.connect(on_value_changed);
			_volume.value_changed.connect(on_value_changed);
			_loop.toggled.connect(on_value_changed);

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
				, _loop.active
				);
		}

		public override void update()
		{
			_name.text    = (string)_level.get_property(_id, "name");
			_range.value  = (double)_level.get_property(_id, "range");
			_volume.value = (double)_level.get_property(_id, "volume");
			_loop.active  = (bool)  _level.get_property(_id, "loop");
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
			_components_vbox.margin_right = 18;

			// Unit
			add_component_view("Unit",                    "name",                    0, new UnitView(_level));
			add_component_view("Transform",               "transform",               0, new TransformComponentView(_level));
			add_component_view("Light",                   "light",                   1, new LightComponentView(_level));
			add_component_view("Camera",                  "camera",                  2, new CameraComponentView(_level));
			add_component_view("Mesh Renderer",           "mesh_renderer",           3, new MeshRendererComponentView(_level));
			add_component_view("Sprite Renderer",         "sprite_renderer",         3, new SpriteRendererComponentView(_level));
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

			set_current_widget(_nothing_to_show);

			set_size_request(300, 200);
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
					if (_level.has_component(id, entry.type, ref component_id) || entry.type == "name")
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
