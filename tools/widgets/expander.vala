/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
/// Drop-in replacement (sort-of) for Gtk.Expander with
/// ability to set a custom widget alongside Expander's label.
public class Expander : Gtk.Box
{
	private bool _expanded = false;
	private Gtk.EventBox _header_event_box;
	private Gtk.Box _header_box;
	private Gtk.Image _arrow_image;
	private Gtk.Widget _header_widget;
	private Gtk.Widget _child = null;

	public Expander(string? label = null)
	{
		Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);

		_header_event_box = new Gtk.EventBox();
		_header_event_box.button_press_event.connect(on_header_clicked);

		_header_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
		_header_box.homogeneous = false;

		_arrow_image = new Gtk.Image.from_icon_name("pan-end-symbolic", Gtk.IconSize.BUTTON);
		_header_box.pack_start(_arrow_image, false, false, 0);

		_header_widget = new Gtk.Label(label);
		_header_box.pack_start(_header_widget, true, true, 0);

		_header_event_box.add(_header_box);

		this.pack_start(_header_event_box, false, false, 0);
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
				_arrow_image.set_from_icon_name("pan-down-symbolic", Gtk.IconSize.BUTTON);
			else
				_arrow_image.set_from_icon_name("pan-end-symbolic", Gtk.IconSize.BUTTON);

			if (_child != null) {
				if (_expanded)
					_child.show();
				else
					_child.hide();
			}
		}
	}

	private bool on_header_clicked(Gtk.Widget widget, Gdk.EventButton event)
	{
		expanded = !expanded;
		return Gdk.EVENT_STOP;
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
				_header_box.pack_start(_header_widget, true, true, 0);
				_header_box.show_all();
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
			_header_box.pack_start(_header_widget, true, true, 0);
			_header_box.show_all();
		}
	}

	public override void add(Gtk.Widget widget)
	{
		assert(_child == null);

		_child = widget;
		base.add(_child);

		if (!_expanded)
			_child.hide();

		show_all();
	}

	public override void remove(Gtk.Widget widget)
	{
		if (widget == _child)
			_child = null;

		base.remove(widget);
	}
}

} /* namespace Crown */
