/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class PixbufView : Gtk.DrawingArea
{
	public static Cairo.Pattern _checker_pattern;

	public static void create_checkered_pattern()
	{
		if (_checker_pattern != null)
			return;

		int width = 16;
		int height = 16;
		Cairo.Surface checker_surface = new Cairo.ImageSurface(Cairo.Format.RGB24, width, height);

		Cairo.Context cr = new Cairo.Context(checker_surface);
		cr.set_source_rgb(0.9, 0.9, 0.9);
		cr.paint();
		cr.set_source_rgb(0.7, 0.7, 0.7);
		cr.rectangle(width / 2, 0, width / 2, height / 2);
		cr.rectangle(0, height / 2, width / 2, height / 2);
		cr.fill();

		_checker_pattern = new Cairo.Pattern.for_surface(checker_surface);
		_checker_pattern.set_filter(Cairo.Filter.NEAREST);
		_checker_pattern.set_extend(Cairo.Extend.REPEAT);
	}

	public double _zoom;
	public double _zoom_speed;
	public Gtk.EventControllerScroll _controller_scroll;
	public Gdk.Pixbuf _pixbuf;
	public Cairo.Pattern _pixbuf_pattern;
	public Cairo.Filter _filter;
	public Cairo.Extend _extend;

	public PixbufView()
	{
		create_checkered_pattern();

		_zoom = 1.0;
		_zoom_speed = 0.2;

		_controller_scroll = new Gtk.EventControllerScroll(this, Gtk.EventControllerScrollFlags.VERTICAL);
		_controller_scroll.scroll.connect(on_scroll);

		_filter = Cairo.Filter.NEAREST;
		_extend = Cairo.Extend.NONE;

		this.draw.connect(on_draw);
	}

	public void set_pixbuf(Gdk.Pixbuf pixbuf)
	{
		_pixbuf = pixbuf;
		_pixbuf_pattern = new Cairo.Pattern.for_surface(Gdk.cairo_surface_create_from_pixbuf(_pixbuf, 1, null));
		_pixbuf_pattern.set_filter(Cairo.Filter.NEAREST);
		_pixbuf_pattern.set_filter(_filter);
		_pixbuf_pattern.set_extend(_extend);
	}

	public void on_scroll(double dx, double dy)
	{
		_zoom = double.min(10.0, double.max(0.25, _zoom - dy * _zoom_speed));
		this.queue_draw();
	}

	public bool on_draw(Cairo.Context cr)
	{
		if (_pixbuf == null)
			return Gdk.EVENT_PROPAGATE;

		int allocated_width = this.get_allocated_width();
		int allocated_height = this.get_allocated_height();

		cr.set_source_rgb(0.1, 0.1, 0.1);
		cr.paint();

		cr.set_source(_checker_pattern);
		cr.rectangle(allocated_width*0.5 - _zoom*_pixbuf.width*0.5
			, allocated_height*0.5 - _zoom*_pixbuf.height*0.5
			, _zoom * _pixbuf.width
			, _zoom * _pixbuf.height
			);
		cr.clip();
		cr.paint();

		cr.translate(allocated_width * 0.5, allocated_height * 0.5);
		cr.scale(_zoom, _zoom);
		cr.translate(-_pixbuf.width * 0.5, -_pixbuf.height * 0.5);

		cr.set_source(_pixbuf_pattern);
		cr.paint();

		return Gdk.EVENT_PROPAGATE;
	}
}

} /* namespace Crown */
