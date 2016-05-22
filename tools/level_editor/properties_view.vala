/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
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
		public Guid _unit_id;
		public Guid _component_id;

		public ComponentView()
		{
			// Data
			_unit_id = GUID_ZERO;
			_component_id = GUID_ZERO;
		}

		public void attach_row(uint row, string label, Gtk.Widget w)
		{
			Gtk.Label l = new Label(label);
			l.width_chars = 10;
			l.set_alignment(1.0f, 0.5f);
			l.set_padding(4, 0);

			PropertyRow r = new PropertyRow(w);

			this.attach(l, 0, (int)row);
			this.attach(r, 1, (int)row);
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
			_unit_id = GUID_ZERO;
			_component_id = GUID_ZERO;

			// Widgets
			_position = new SpinButtonVector3(Vector3(0, 0, 0), Vector3(-9999.9, -9999.9, -9999.9), Vector3(9999.9, 9999.9, 9999.9));
			_rotation = new SpinButtonRotation(Vector3(0, 0, 0));
			_scale    = new SpinButtonVector3(Vector3(0, 0, 0), Vector3(0.1, 0.1, 0.1), Vector3(10.0, 10.0, 10.0));

			_position.value_changed.connect(on_value_changed);
			_rotation.value_changed.connect(on_value_changed);
			_scale.value_changed.connect(on_value_changed);

			uint row = 0;
			attach_row(row++, "Position", _position);
			attach_row(row++, "Rotation", _rotation);
			attach_row(row++, "Scale", _scale);
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
			Vector3 pos    = (Vector3)   _level.get_component_property(_unit_id, _component_id, "data.position");
			Quaternion rot = (Quaternion)_level.get_component_property(_unit_id, _component_id, "data.rotation");
			Vector3 scl    = (Vector3)   _level.get_component_property(_unit_id, _component_id, "data.scale");

			_position.value = pos;
			_rotation.value = rot;
			_scale.value    = scl;
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
		private Gtk.ColorButton _color;

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
			_color      = new ColorButton();

			_range.value_changed.connect(on_value_changed);
			_intensity.value_changed.connect(on_value_changed);
			_spot_angle.value_changed.connect(on_value_changed);
			_color.color_set.connect(on_value_changed);

			uint row = 0;
			attach_row(row++, "Type", _type);
			attach_row(row++, "Range", _range);
			attach_row(row++, "Intensity", _intensity);
			attach_row(row++, "Spot Angle", _spot_angle);
			attach_row(row++, "Color", _color);
		}

		private void on_value_changed()
		{
			// Vector3 color = Vector3((double)_color.Color.Red/(double)ushort.MaxValue
			// 	, (double)_color.Color.Green/(double)ushort.MaxValue
			// 	, (double)_color.Color.Blue/(double)ushort.MaxValue
			// 	);

			_level.set_component_property(_unit_id, _component_id, "data.type",       "directional"); // FIXME
			_level.set_component_property(_unit_id, _component_id, "data.range",      _range.value);
			_level.set_component_property(_unit_id, _component_id, "data.intensity",  _intensity.value);
			_level.set_component_property(_unit_id, _component_id, "data.spot_angle", _spot_angle.value);
			// _level.set_component_property(_unit_id, _component_id, "data.color",      color);
			_level.set_component_property(_unit_id, _component_id, "type", "light");
		}

		public override void update()
		{
			string type       = (string) _level.get_component_property(_unit_id, _component_id, "data.type");
			double range      = (double) _level.get_component_property(_unit_id, _component_id, "data.range");
			double intensity  = (double) _level.get_component_property(_unit_id, _component_id, "data.intensity");
			double spot_angle = (double) _level.get_component_property(_unit_id, _component_id, "data.spot_angle");
			Vector3 color     = (Vector3)_level.get_component_property(_unit_id, _component_id, "data.color");

			_range.value      = range;
			_intensity.value  = intensity;
			_spot_angle.value = spot_angle;
			// _color.color      = new Gdk.Color((byte)(255.0*color.x), (byte)(255.0*color.y), (byte)(255.0*color.z));
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

			uint row = 0;
			attach_row(row++, "Projection", _projection);
			attach_row(row++, "FOV", _fov);
			attach_row(row++, "Near Range", _near_range);
			attach_row(row++, "Far Range", _far_range);
		}

		public override void update()
		{
			string type       = (string)_level.get_component_property(_unit_id, _component_id, "data.projection");
			double fov        = (double)_level.get_component_property(_unit_id, _component_id, "data.fov");
			double near_range = (double)_level.get_component_property(_unit_id, _component_id, "data.near_range");
			double far_range  = (double)_level.get_component_property(_unit_id, _component_id, "data.far_range");

			_fov.value        = MathUtils.deg(fov);
			_near_range.value = near_range;
			_far_range.value  = far_range;
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

			uint row = 0;
			attach_row(row++, "Mesh", _mesh_resource);
			attach_row(row++, "Geometry", _geometry);
			attach_row(row++, "Material", _material);
			attach_row(row++, "Visible", _visible);
		}

		public override void update()
		{
			_mesh_resource.text = (string)_level.get_component_property(_unit_id, _component_id, "data.mesh_resource");
			_geometry.text      = (string)_level.get_component_property(_unit_id, _component_id, "data.geometry_name");
			_material.text      = (string)_level.get_component_property(_unit_id, _component_id, "data.material");
			_visible.active     = (bool)  _level.get_component_property(_unit_id, _component_id, "data.visible");
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
			_unit_id = GUID_ZERO;
			_component_id = GUID_ZERO;

			// Widgets
			_position = new SpinButtonVector3(Vector3(0, 0, 0), Vector3(-9999.9, -9999.9, -9999.9), Vector3(9999.9, 9999.9, 9999.9));
			_rotation = new SpinButtonRotation(Vector3(0, 0, 0));

			_position.value_changed.connect(on_value_changed);
			_rotation.value_changed.connect(on_value_changed);

			uint row = 0;
			attach_row(row++, "Position", _position);
			attach_row(row++, "Rotation", _rotation);
		}

		private void on_value_changed()
		{
			Vector3 pos    = _position.value;
			Quaternion rot = _rotation.value;

			_level.move_selected_objects(pos, rot, Vector3(1.0, 1.0, 1.0));
		}

		public override void update()
		{
			Vector3 pos    = (Vector3)   _level.get_property(_component_id, "position");
			Quaternion rot = (Quaternion)_level.get_property(_component_id, "rotation");

			_position.value = pos;
			_rotation.value = rot;
		}
	}
	public class SoundPropertiesView : ComponentView
	{
		// Data
		Level _level;

		// Widgets
		private Gtk.Entry _name;
		private SpinButtonDouble _range;
		private SpinButtonDouble _volume;
		private Gtk.CheckButton _loop;

		public SoundPropertiesView(Level level)
		{
			// Data
			_level = level;

			// Widgets
			_name   = new Gtk.Entry();
			_range  = new SpinButtonDouble(0.0, 0.001, 9999.0);
			_volume = new SpinButtonDouble(0.0, 0.0,   1.0);
			_loop   = new Gtk.CheckButton();
			_name.sensitive = false;

			_range.value_changed.connect(on_range_changed);
			_volume.value_changed.connect(on_volume_changed);
			_loop.toggled.connect(on_loop_toggled);

			uint row = 0;
			attach_row(row++, "Name", _name);
			attach_row(row++, "Range", _range);
			attach_row(row++, "Volume", _volume);
			attach_row(row++, "Loop", _loop);
		}

		private void on_range_changed()
		{
			_level.set_property(_component_id, "range", _range.value);
		}

		private void on_volume_changed()
		{
			_level.set_property(_component_id, "volume", _range.value);
		}

		private void on_loop_toggled()
		{
			_level.set_property(_component_id, "loop", _loop.active);
		}

		public override void update()
		{
			_name.text    = (string)_level.get_property(_component_id, "name");
			_range.value  = (double)_level.get_property(_component_id, "range");
			_volume.value = (double)_level.get_property(_component_id, "volume");
			_loop.active  = (bool)  _level.get_property(_component_id, "loop");
		}
	}

	public class PropertiesView : Gtk.Bin
	{
		// Data
		private Level _level;
		private HashMap<string, ComponentView> _components;

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

			_components = new HashMap<string, ComponentView>();

			// Widgets
			_components_vbox = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
			_components_vbox.margin_right = 18;

			// Unit
			add_component_view("Transform", "transform", new TransformComponentView(_level));
			add_component_view("Light", "light", new LightComponentView(_level));
			add_component_view("Camera", "camera", new CameraComponentView(_level));
			add_component_view("Mesh Renderer", "mesh_renderer", new MeshRendererComponentView(_level));

			// Sound
			add_component_view("Transform", "sound_transform", new SoundTransformView(_level));
			add_component_view("Sound Properties", "sound_properties", new SoundPropertiesView(_level));

			_nothing_to_show = new Gtk.Label("Nothing to show");

			_viewport = new Gtk.Viewport(null, null);
			_viewport.add(_components_vbox);

			_scrolled_window = new Gtk.ScrolledWindow(null, null);
			_scrolled_window.add(_viewport);

			_current_widget = null;

			set_current_widget(_nothing_to_show);

			set_size_request(470, 200);
		}

		private void add_component_view(string label, string component_type, ComponentView cv)
		{
			Gtk.Label lb = new Gtk.Label(null);
			lb.set_markup("<b>%s</b>".printf(label));
			lb.set_alignment(0.0f, 0.5f);

			Gtk.Expander expander = new Gtk.Expander("");
			expander.label_widget = lb;
			expander.child = cv;
			expander.expanded = true;

			_components[component_type] = cv;
			_components_vbox.pack_start(expander, false, true, 0);
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
			if (selection.size == 0)
			{
				set_current_widget(_nothing_to_show);
				return;
			}

			Guid id = selection[selection.size - 1];

			if (_level.is_unit(id))
			{
				set_current_widget(_scrolled_window);

				Guid unit_id = id;

				foreach (var entry in _components.entries)
				{
					ComponentView cv = entry.value;
					cv.hide();
				}
				foreach (var entry in _components.entries)
				{
					ComponentView cv = entry.value;
					Guid component_id = GUID_ZERO;
					if (_level.has_component(unit_id, entry.key, ref component_id))
					{
						cv._unit_id = unit_id;
						cv._component_id = component_id;
						cv.update();
						cv.show_all();
					}
				}
			}
			else if (_level.is_sound(id))
			{
				set_current_widget(_scrolled_window);

				foreach (var entry in _components.entries)
				{
					ComponentView cv = entry.value;
					cv.hide();
				}

				ComponentView st = _components["sound_transform"];
				ComponentView sp = _components["sound_properties"];
				st._component_id = id;
				st.update();
				st.show_all();
				sp._component_id = id;
				sp.update();
				sp.show_all();
			}
			else
			{
				set_current_widget(_nothing_to_show);
			}
		}
	}
}
