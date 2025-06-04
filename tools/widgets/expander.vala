/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
/// Drop-in replacement (sort-of) for Gtk.Expander with
/// ability to set a custom widget alongside Expander's label.
public class Expander : Gtk.Box
{
	public bool _expanded = false;
#if CROWN_GTK3
	public Gtk.EventBox _header_event_box;
	public Gtk.GestureMultiPress _gesture_click;
#else
	public Gtk.GestureClick _gesture_click;
#endif
	public Gtk.Box _header_box;
	public Gtk.Image _arrow_image;
	public Gtk.Widget _header_widget;
	public Gtk.Widget _child = null;

	public Expander(string? label = null)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);
		this.name = "expander2";

#if CROWN_GTK3
		_header_event_box = new Gtk.EventBox();

		_gesture_click = new Gtk.GestureMultiPress(_header_event_box);
#else
		_gesture_click = new Gtk.GestureClick();
#endif
		_gesture_click.pressed.connect(on_header_button_pressed);

		_header_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		_header_box.homogeneous = false;
#if !CROWN_GTK3
		_header_box.add_controller(_gesture_click);
#endif

#if CROWN_GTK3
		_arrow_image = new Gtk.Image.from_icon_name("pan-end-symbolic", Gtk.IconSize.BUTTON);
		_header_box.pack_start(_arrow_image, false, false, 0);
#else
		_arrow_image = new Gtk.Image.from_icon_name("pan-end-symbolic");
		_header_box.append(_arrow_image);
#endif

		_header_widget = new Gtk.Label(label);
#if CROWN_GTK3
		_header_box.pack_start(_header_widget, true, true, 0);
		_header_box.get_style_context().add_class("header");
		_header_event_box.add(_header_box);

		this.pack_start(_header_event_box, false, false, 0);
#else
		_header_box.prepend(_header_widget);
		_header_box.add_css_class("header");

		this.append(_header_box);
#endif
	}

	public bool expanded
	{
		get
		{
			return _expanded;
		}
		set
		{
			if (_expanded == value)
				return;

			_expanded = value;

			if (_expanded)
#if CROWN_GTK3
				_arrow_image.set_from_icon_name("pan-down-symbolic", Gtk.IconSize.BUTTON);
#else
				_arrow_image.set_from_icon_name("pan-down-symbolic");
#endif
			else
#if CROWN_GTK3
				_arrow_image.set_from_icon_name("pan-end-symbolic", Gtk.IconSize.BUTTON);
#else
				_arrow_image.set_from_icon_name("pan-end-symbolic");
#endif

			if (_child != null) {
				if (_expanded)
					_child.show();
				else
					_child.hide();
			}
		}
	}

	public void on_header_button_pressed(int n_press, double x, double y)
	{
		uint button = _gesture_click.get_current_button();

		if (button == Gdk.BUTTON_PRIMARY)
			expanded = !expanded;
	}

	public string label
	{
		get
		{
			if (_header_widget is Gtk.Label)
				return ((Gtk.Label)_header_widget).label;
			else
				return "";
		}
		set
		{
			if (_header_widget is Gtk.Label) {
				((Gtk.Label)_header_widget).label = value;
			} else {
				_header_box.remove(_header_widget);
				_header_widget = new Gtk.Label(value);
#if CROWN_GTK3
				_header_box.pack_start(_header_widget, true, true, 0);
				_header_box.show_all();
#else
				_header_box.append(_header_widget);
#endif
			}
		}
	}

	public Gtk.Widget custom_header
	{
		get
		{
			return _header_widget;
		}
		set
		{
			if (_header_widget != null)
				_header_box.remove(_header_widget);

			_header_widget = value;
#if CROWN_GTK3
			_header_box.pack_start(_header_widget, true, true, 0);
			_header_box.show_all();
#else
			_header_box.append(_header_widget);
#endif
		}
	}

#if CROWN_GTK3
	public override void add(Gtk.Widget widget)
#else
	public void add(Gtk.Widget widget)
#endif
	{
		assert(_child == null);

		_child = widget;
#if CROWN_GTK3
		base.add(_child);
#else
		base.append(_child);
#endif

		if (!_expanded)
			_child.hide();
#if CROWN_GTK3
		show_all();
#endif
	}

#if CROWN_GTK3
	public override void remove(Gtk.Widget widget)
#else
	public void remove(Gtk.Widget widget)
#endif
	{
		if (widget == _child)
			_child = null;

		base.remove(widget);
	}

	public override void map()
	{
		if (_child != null) {
			if (!_expanded)
				_child.hide();
		}

		base.map();
	}
}

} /* namespace Crown */
