/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

extern GLib.Object gtk_color_picker_new();
extern void gtk_color_picker_pick(GLib.Object picker, GLib.AsyncReadyCallback callback);
extern Gdk.RGBA? gtk_color_picker_pick_finish(GLib.Object picker, GLib.AsyncResult result, ref GLib.Error error);

namespace Crown
{
public class ColorButton : Gtk.MenuButton
{
	public Gdk.RGBA color = { 1.0, 1.0, 1.0, 1.0 };
	public const int PADDING = 4;

	public ColorButton()
	{
		Object();
	}

	public void set_color(Gdk.RGBA c)
	{
		this.color = c;
		this.queue_draw();
	}

	public override bool draw(Cairo.Context cr)
	{
		base.draw(cr);

		Gtk.Allocation alloc;
		this.get_allocation(out alloc);

		cr.set_source_rgba(this.color.red, this.color.green, this.color.blue, this.color.alpha);
		cr.rectangle(PADDING, PADDING, alloc.width - 2 * PADDING, alloc.height - 2 * PADDING);
		cr.fill();

		return false;
	}
}

static void hsv_to_rgb(ref double r, ref double g, ref double b, double h, double s, double v)
{
	assert(h >= 0.0 && s >= 0.0 && v >= 0.0);

	if (s == 0.0) {
		r = g = b = v;
		return;
	}

	if (h >= 1.0)
		h = 0.0;

	double hf = h * 6.0;
	int i = (int)MathUtils.floor(hf);
	double f = MathUtils.fract(hf);
	double p = v * (1.0 - s);
	double q = v * (1.0 - s * f);
	double t = v * (1.0 - s * (1.0 - f));

	switch (i) {
	case 0: r = v; g = t; b = p; break;
	case 1: r = q; g = v; b = p; break;
	case 2: r = p; g = v; b = t; break;
	case 3: r = p; g = q; b = v; break;
	case 4: r = t; g = p; b = v; break;
	case 5: r = v; g = p; b = q; break;
	default: assert(false); break;
	}
}

static void rgb_to_hsv(ref double h, ref double s, ref double v, double r, double g, double b)
{
	assert(r >= 0.0 && g >= 0.0 && b >= 0.0);

	double max = double.max(double.max(r, g), b);
	double min = double.min(double.min(r, g), b);
	double delta = max - min;

	v = max;

	if (max == 0.0) {
		s = 0.0;
		h = 0.0;
		return;
	}

	s = (delta <= 0.0) ? 0.0 : (delta / max);

	if (delta <= 1e-6) {
		h = 0.0;
	} else {
		double hh;
		if (max == r) {
			hh = (g - b) / delta;
			if (hh < 0.0) hh += 6.0;
		} else if (max == g) {
			hh = (b - r) / delta + 2.0;
		} else { // max == b
			hh = (r - g) / delta + 4.0;
		}
		hh = (hh / 6.0).clamp(0.0, 1.0);
		if (hh >= 1.0)
			hh = 0.0;
		h = hh;
	}
}

const int HS_PALETTE_SEGMENTS = 24;
const Vector3 primary_colors[] =
{
	{ 1.0, 0.0, 0.0 },
	{ 1.0, 1.0, 0.0 },
	{ 0.0, 1.0, 0.0 },
	{ 0.0, 1.0, 1.0 },
	{ 0.0, 0.0, 1.0 },
	{ 1.0, 0.0, 1.0 }
};

static double norm_angle(double a)
{
	double x = Math.fmod(a, PI_TWO);
	if (x < 0)
		x += PI_TWO;
	return x;
}

static double angle_midpoint(double a, double b)
{
	double delta = b - a;
	if (delta <= -Math.PI)
		delta += PI_TWO;
	else if (delta > Math.PI)
		delta -= PI_TWO;
	return a + delta * 0.5;
}

static void color_from_anchors(out Vector3 rgb, double theta)
{
	double theta_n = norm_angle(theta);
	const double seg = Math.PI / 3.0;

	int idx = ((int)Math.floor(theta_n / seg)).clamp(0, 5);
	double anchor_a = (double)idx * seg;
	int next = (idx + 1) % 6;

	double t = ((theta_n - anchor_a) / seg).clamp(0.0, 1.0);

	rgb.x = MathUtils.lerp(primary_colors[idx].x, primary_colors[next].x, t);
	rgb.y = MathUtils.lerp(primary_colors[idx].y, primary_colors[next].y, t);
	rgb.z = MathUtils.lerp(primary_colors[idx].z, primary_colors[next].z, t);
}

static Cairo.MeshPattern create_circle_mesh(double cx, double cy, double r, double hsv_value)
{
	Cairo.MeshPattern mesh = new Cairo.MeshPattern();

	double angles[HS_PALETTE_SEGMENTS];
	double ox[HS_PALETTE_SEGMENTS];
	double oy[HS_PALETTE_SEGMENTS];
	Vector3 cols[HS_PALETTE_SEGMENTS];

	for (int i = 0; i < HS_PALETTE_SEGMENTS; ++i) {
		double theta = PI_TWO * (double)i / (double)HS_PALETTE_SEGMENTS;
		angles[i] = theta;
		ox[i] = cx + r * Math.cos(theta);
		oy[i] = cy - r * Math.sin(theta);

		Vector3 base_col;
		color_from_anchors(out base_col, theta);
		cols[i].x = base_col.x * hsv_value;
		cols[i].y = base_col.y * hsv_value;
		cols[i].z = base_col.z * hsv_value;
	}

	for (int i = 0; i < HS_PALETTE_SEGMENTS; ++i) {
		int inext = (i + 1) % HS_PALETTE_SEGMENTS;
		double angle_mid = angle_midpoint(angles[i], angles[inext]);
		double mx = cx + r * Math.cos(angle_mid);
		double my = cy - r * Math.sin(angle_mid);

		mesh.begin_patch();

		mesh.move_to(cx, cy);
		mesh.line_to(ox[i], oy[i]);
		mesh.line_to(mx, my);
		mesh.line_to(ox[inext], oy[inext]);

		mesh.set_corner_color_rgb(0
			, hsv_value
			, hsv_value
			, hsv_value
			);
		mesh.set_corner_color_rgb(1,
			cols[i].x,
			cols[i].y,
			cols[i].z
			);
		mesh.set_corner_color_rgb(2,
			0.5 * (cols[i].x + cols[inext].x),
			0.5 * (cols[i].y + cols[inext].y),
			0.5 * (cols[i].z + cols[inext].z)
			);
		mesh.set_corner_color_rgb(3,
			cols[inext].x,
			cols[inext].y,
			cols[inext].z
			);

		mesh.end_patch();
	}

	mesh.set_extend(Cairo.Extend.NONE);
	return mesh;
}

public class InputColor3 : Gtk.Box, InputField
{
	public bool _dragging;
	public int _hs_palette_radius;
	public double _hs_lens_radius_scale;
	public double _hs_lens_small_radius_scale;
	public Gtk.DrawingArea _hs_palette;
	public Gtk.Scale _hsv_v_scale;
	public InputDouble _rgb_r;
	public InputDouble _rgb_g;
	public InputDouble _rgb_b;
	public InputDouble _rgb_a;
	public InputDouble _hsv_h;
	public InputDouble _hsv_s;
	public InputDouble _hsv_v;
	public InputDouble _hsv_a;
	public PropertyGrid _rgb_grid;
	public PropertyGrid _hsv_grid;
	public InputString _color_string;
	public Gtk.Button _picker_button;
	public GLib.Object _picker;
	public Gtk.GestureMultiPress _gesture_click;
	public Gtk.EventControllerMotion _controller_motion;
	public Gtk.Box _visual_box;
	public Gtk.Box _numeric_box;
	public Gtk.Box _input_box;
	public Gtk.Box _utils_box;
	public Gtk.Box _rgb_box;
	public Gtk.Box _hsv_box;
	public Gtk.Stack _rgb_hsv_stack;
	public Gtk.StackSwitcher _rgb_hsv_switcher;
	public Gtk.Popover _popover;
	public ColorButton _color_button;

	public void set_inconsistent(bool inconsistent)
	{
	}

	public bool is_inconsistent()
	{
		return false;
	}

	public GLib.Value union_value()
	{
		return this.value;
	}

	public void set_union_value(GLib.Value v)
	{
		this.value = (Vector3)v;
	}

	public Vector3 value
	{
		get
		{
			return Vector3(_rgb_r.value, _rgb_g.value, _rgb_b.value);
		}
		set
		{
			Vector3 rgb = (Vector3)value;
			_rgb_r.value = rgb.x;
			_rgb_g.value = rgb.y;
			_rgb_b.value = rgb.z;
		}
	}

	public int palette_size_request()
	{
		double palette_size = 2.0 * _hs_palette_radius;
		double lens_size = 2.0 * (_hs_palette_radius * _hs_lens_radius_scale);
		return (int)(palette_size + lens_size);
	}

	public InputColor3()
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL);

		_dragging = false;
		_hs_palette_radius = 100;
		_hs_lens_radius_scale = 0.1;
		_hs_lens_small_radius_scale = _hs_lens_radius_scale * 0.65;

		_hs_palette = new Gtk.DrawingArea();
		_hs_palette.halign = Gtk.Align.START;
		_hs_palette.draw.connect(on_draw_circle);
		_hs_palette.size_allocate.connect(on_circle_size_allocate);
		int sr = palette_size_request();
		_hs_palette.set_size_request(sr, sr);

		_gesture_click = new Gtk.GestureMultiPress(_hs_palette);
		_gesture_click.set_button(0);
		_gesture_click.pressed.connect(on_hs_circle_button_pressed);
		_gesture_click.released.connect(on_hs_circle_button_released);

		_controller_motion = new Gtk.EventControllerMotion(_hs_palette);
		_controller_motion.motion.connect(on_hs_circle_motion);

		Gtk.Adjustment adj = new Gtk.Adjustment(0.0, 0.0, 1.0, 0.01, 0.1, 0.0);
		_hsv_v_scale = new Gtk.Scale(Gtk.Orientation.VERTICAL, adj);
		_hsv_v_scale.get_style_context().add_class("hsv-v-scale");
		_hsv_v_scale.halign = Gtk.Align.END;
		_hsv_v_scale.draw_value = false;
		_hsv_v_scale.set_digits(3);
		_hsv_v_scale.halign = Gtk.Align.START;
		_hsv_v_scale.value_changed.connect(on_hsv_v_scale_value_changed);

		_rgb_r = new InputDouble(1.0, 0.0, 1.0);
		_rgb_g = new InputDouble(1.0, 0.0, 1.0);
		_rgb_b = new InputDouble(1.0, 0.0, 1.0);
		_rgb_a = new InputDouble(1.0, 0.0, 1.0);
		_rgb_r.value_changed.connect(on_rgb_value_changed);
		_rgb_g.value_changed.connect(on_rgb_value_changed);
		_rgb_b.value_changed.connect(on_rgb_value_changed);

		_hsv_h = new InputDouble(0.0, 0.0, 1.0);
		_hsv_s = new InputDouble(0.0, 0.0, 1.0);
		_hsv_v = new InputDouble(1.0, 0.0, 1.0);
		_hsv_a = new InputDouble(_rgb_a.value, 0.0, 1.0);
		_hsv_h.value_changed.connect(on_hsv_value_changed);
		_hsv_s.value_changed.connect(on_hsv_value_changed);
		_hsv_v.value_changed.connect(on_hsv_value_changed);

		_rgb_grid = new PropertyGrid();
		_rgb_grid.row_homogeneous = false;
		_rgb_grid.label_width_chars = 8;
		_rgb_grid.add_row("Red", _rgb_r);
		_rgb_grid.add_row("Green", _rgb_g);
		_rgb_grid.add_row("Blue", _rgb_b);
		_rgb_grid.add_row("Alpha", _rgb_a);

		_hsv_grid = new PropertyGrid();
		_hsv_grid.row_homogeneous = false;
		_hsv_grid.label_width_chars = 8;
		_hsv_grid.add_row("Hue", _hsv_h);
		_hsv_grid.add_row("Saturation", _hsv_s);
		_hsv_grid.add_row("Value", _hsv_v);
		_hsv_grid.add_row("Alpha", _hsv_a);

		_color_string = new InputString();
		_color_string.set_tooltip_text("Lua color code.");
		_color_string.value_changed.connect(on_color_string_value_changed);

		_picker_button = new Gtk.Button.from_icon_name("color-select-symbolic");
		_picker_button.set_tooltip_text("Pick a color from the screen.");
		_picker_button.clicked.connect(on_picker_button_clicked);
		_picker = gtk_color_picker_new();

		_rgb_hsv_stack = new Gtk.Stack();
		_rgb_hsv_stack.add_titled(_rgb_grid, "rgb", "RGB");
		_rgb_hsv_stack.add_titled(_hsv_grid, "hsv", "HSV");
		_rgb_hsv_stack.map.connect(() => {
				_rgb_hsv_stack.set_visible_child_name("hsv");
			});
		_rgb_hsv_switcher = new Gtk.StackSwitcher();
		_rgb_hsv_switcher.set_stack(_rgb_hsv_stack);

		_visual_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_visual_box.margin = 0;
		_visual_box.pack_start(_hs_palette, true, true, 0);
		_visual_box.pack_start(_hsv_v_scale, false, false, 0);

		_numeric_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 8);
		_numeric_box.pack_start(_rgb_hsv_switcher);
		_numeric_box.pack_start(_rgb_hsv_stack);

		_utils_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 4);
		_utils_box.pack_start(_color_string);
		_utils_box.pack_start(_picker_button, false);

		_input_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 16 /* Avoid slider overlapping buttons. */);
		_input_box.margin = 12;
		_input_box.pack_start(_visual_box);
		_input_box.pack_start(_numeric_box);
		_input_box.pack_start(_utils_box);

		_popover = new Gtk.Popover(null);
		_popover.add(_input_box);
		// _popover.has_arrow = false;
		_input_box.show_all();

		_color_button = new ColorButton();
		_color_button.set_tooltip_text("Choose a color.");
		_color_button.can_focus = false;
		_color_button.set_popover(_popover);

		this.value_changed.connect(on_value_changed);
		this.pack_start(_color_button);

		on_value_changed();
	}

	public void palette_radius(out double full_radius, out double radius)
	{
		double w = (double)this._hs_palette.get_allocated_width();
		double h = (double)this._hs_palette.get_allocated_height();

		full_radius = Math.fmin(w, h) * 0.5;
		radius = full_radius - full_radius * _hs_lens_radius_scale;
	}

	public bool on_draw_circle(Cairo.Context cr)
	{
		double w = (double)this._hs_palette.get_allocated_width();
		double h = (double)this._hs_palette.get_allocated_height();

		double cx = w / 2.0;
		double cy = h / 2.0;
		double full_radius;
		double radius;
		palette_radius(out full_radius, out radius);

		Cairo.MeshPattern mesh = create_circle_mesh(cx, cy, radius, _hsv_v.value);

		cr.save();
		cr.arc(cx, cy, radius, 0, PI_TWO);
		cr.clip();

		cr.set_source(mesh);
		cr.paint();

		cr.restore();

		// Draw outline.
		cr.set_line_width(1.0);
		cr.arc(cx, cy, radius - 0.5, 0, PI_TWO);
		cr.set_source_rgb(0.5, 0.5, 0.5);
		cr.stroke();

		double scale = _dragging ? _hs_lens_radius_scale : _hs_lens_small_radius_scale;
		draw_lens(cr, full_radius * scale);

		return false;
	}

	public bool draw_lens(Cairo.Context cr, double radius)
	{
		Vector2 p = xy_from_hs({ _hsv_h.value, _hsv_s.value });

		cr.save();
		cr.arc(p.x, p.y, radius, 0, PI_TWO);
		cr.clip();

		cr.set_source_rgb(_rgb_r.value, _rgb_g.value, _rgb_b.value);
		cr.paint();

		cr.restore();

		// Draw outline.
		cr.set_line_width(1.0);
		cr.arc(p.x, p.y, radius - 0.5, 0, PI_TWO);
		cr.set_source_rgb(0.5, 0.5, 0.5);
		cr.stroke();

		return false;
	}

	public void on_circle_size_allocate(Gtk.Allocation allocation)
	{
		int dia = (int)Math.fmax(1.0, Math.fmin ((double)allocation.width, (double)allocation.height));
		_hsv_v_scale.set_size_request(-1, dia);
	}

	public void on_hsv_v_scale_value_changed()
	{
		double v = this._hsv_v_scale.get_value();
		if (v < 0.0) v = 0.0;
		if (v > 1.0) v = 1.0;
		_hsv_v.value = 1.0 - v;
		_hs_palette.queue_draw();
	}

	public void on_rgb_value_changed()
	{
		double h = 0.0;
		double s = 0.0;
		double v = 1.0;

		disconnect_hsv();
		rgb_to_hsv(ref h
			, ref s
			, ref v
			, _rgb_r.value
			, _rgb_g.value
			, _rgb_b.value
			);
		_hsv_h.value = h;
		_hsv_s.value = s;
		_hsv_v.value = v;
		_hsv_v_scale.set_value(1.0 - _hsv_v.value);
		connect_hsv();

		_hs_palette.queue_draw();

		value_changed(this);
	}

	public void on_hsv_value_changed()
	{
		double r = 1.0;
		double g = 1.0;
		double b = 1.0;

		disconnect_rgb();
		hsv_to_rgb(ref r
			, ref g
			, ref b
			, _hsv_h.value
			, _hsv_s.value
			, _hsv_v.value
			);
		_rgb_r.value = r;
		_rgb_g.value = g;
		_rgb_b.value = b;
		disconnect_hsv();
		_hsv_v_scale.set_value(1.0 - _hsv_v.value);
		connect_hsv();
		connect_rgb();

		_hs_palette.queue_draw();

		value_changed(this);
	}

	public void disconnect_rgb()
	{
		_rgb_r.value_changed.disconnect(on_rgb_value_changed);
		_rgb_g.value_changed.disconnect(on_rgb_value_changed);
		_rgb_b.value_changed.disconnect(on_rgb_value_changed);
	}

	public void connect_rgb()
	{
		_rgb_r.value_changed.connect(on_rgb_value_changed);
		_rgb_g.value_changed.connect(on_rgb_value_changed);
		_rgb_b.value_changed.connect(on_rgb_value_changed);
	}

	public void disconnect_hsv()
	{
		_hsv_h.value_changed.disconnect(on_hsv_value_changed);
		_hsv_s.value_changed.disconnect(on_hsv_value_changed);
		_hsv_v.value_changed.disconnect(on_hsv_value_changed);
	}

	public void connect_hsv()
	{
		_hsv_h.value_changed.connect(on_hsv_value_changed);
		_hsv_s.value_changed.connect(on_hsv_value_changed);
		_hsv_v.value_changed.connect(on_hsv_value_changed);
	}

	public void on_value_changed()
	{
		Vector3 c = this.value;
		_color_button.set_color({ c.x, c.y, c.z, 1.0 });
		_color_string.value_changed.disconnect(on_color_string_value_changed);
		_color_string.value = "Color4(%d, %d, %d, %d)".printf((int)(_rgb_r.value * 255.0)
			, (int)(_rgb_g.value * 255.0)
			, (int)(_rgb_b.value * 255.0)
			, (int)(_rgb_a.value * 255.0)
			);
		_color_string.value_changed.connect(on_color_string_value_changed);
	}

	// Returns the hue (x) and saturation (y) of the specified point @a p on a HS circle.
	public Vector2 hs_from_xy(Vector2 p)
	{
		double w = (double)this._hs_palette.get_allocated_width();
		double h = (double)this._hs_palette.get_allocated_height();

		double radius = w * 0.5;
		double cx = (p.x - radius) / w;
		double cy = (h - p.y - radius) / h;

		Vector2 hs = { 0.0, 0.0 };

		hs.x = Math.atan2(cy, cx);
		if (hs.x < 0)
			hs.x += PI_TWO;
		hs.x /= PI_TWO;

		hs.y = Vector2(cx, cy).length() * 2.0;
		return hs;
	}

	public Vector2 xy_from_hs(Vector2 hs)
	{
		double full_radius;
		double radius;
		palette_radius(out full_radius, out radius);

		double cx = Math.cos(PI_TWO * hs.x) * hs.y * radius;
		double cy = Math.sin(PI_TWO * hs.x) * hs.y * radius;

		cx = cx + full_radius;
		cy = full_radius - cy;
		return { cx, cy };
	}

	public void on_hs_circle_button_pressed(int n_press, double x, double y)
	{
		Vector2 hs = hs_from_xy({ x, y });
		_hsv_h.value = hs.x;
		_hsv_s.value = hs.y;

		_dragging = true;
		_hs_palette.get_window().set_cursor(new Gdk.Cursor.from_name(Gdk.Display.get_default(), "none"));
		_hs_palette.queue_draw();
	}

	public void on_hs_circle_button_released(int n_press, double x, double y)
	{
		_dragging = false;
		_hs_palette.get_window().set_cursor(new Gdk.Cursor.from_name(Gdk.Display.get_default(), "default"));
		_hs_palette.queue_draw();
	}

	public void on_hs_circle_motion(double x, double y)
	{
		Vector2 hs = hs_from_xy({ x, y });
		_hsv_h.value = hs.x;
		_hsv_s.value = hs.y;
	}

	public void on_color_picked(GLib.Object? object, GLib.AsyncResult result)
	{
		GLib.Error error = null;
		Gdk.RGBA? rgba = gtk_color_picker_pick_finish(_picker, result, ref error);
		if (rgba != null)
			this.value = Vector3(rgba.red, rgba.green, rgba.blue);
	}

	public void on_picker_button_clicked()
	{
		gtk_color_picker_pick(_picker, on_color_picked);
	}

	public void on_color_string_value_changed()
	{
		int rgba[4];

		if (_color_string.value.scanf("Color4(%d, %d, %d, %d)"
			, out rgba[0]
			, out rgba[1]
			, out rgba[2]
			, out rgba[3]
			) == 4) {
			_rgb_r.value = rgba[0] / 255.0;
			_rgb_g.value = rgba[1] / 255.0;
			_rgb_b.value = rgba[2] / 255.0;
			_rgb_a.value = rgba[3] / 255.0;
		}
	}
}

} /* namespace Crown */
