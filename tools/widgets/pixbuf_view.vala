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

#if CROWN_GTK3
		_controller_scroll = new Gtk.EventControllerScroll(this, Gtk.EventControllerScrollFlags.VERTICAL);
		this.draw.connect(on_draw);
#else
		_controller_scroll = new Gtk.EventControllerScroll(Gtk.EventControllerScrollFlags.VERTICAL);
		this.add_controller(_controller_scroll);
#endif
		_controller_scroll.scroll.connect(on_scroll);

		_filter = Cairo.Filter.NEAREST;
		_extend = Cairo.Extend.NONE;
	}

	public void set_pixbuf(Gdk.Pixbuf pixbuf)
	{
		_pixbuf = pixbuf;
#if CROWN_GTK3
		_pixbuf_pattern = new Cairo.Pattern.for_surface(Gdk.cairo_surface_create_from_pixbuf(_pixbuf, 1, null));
#else
		int width = _pixbuf.get_width();
		int height = _pixbuf.get_height();
		var surface = new Cairo.ImageSurface(Cairo.Format.ARGB32, width, height);
		var cr = new Cairo.Context(surface);
		Gdk.cairo_set_source_pixbuf(cr, _pixbuf, 0, 0);
		cr.paint();
		_pixbuf_pattern = new Cairo.Pattern.for_surface(surface);
#endif
		_pixbuf_pattern.set_filter(Cairo.Filter.NEAREST);
		_pixbuf_pattern.set_filter(_filter);
		_pixbuf_pattern.set_extend(_extend);
	}

#if CROWN_GTK3
	public void on_scroll(double dx, double dy)
#else
	public bool on_scroll(double dx, double dy)
#endif
	{
		_zoom = double.min(10.0, double.max(0.25, _zoom - dy * _zoom_speed));
		this.queue_draw();
#if !CROWN_GTK3
		return Gdk.EVENT_PROPAGATE;
#endif
	}

#if CROWN_GTK3
	public bool on_draw(Cairo.Context cr)
#else
	public override void snapshot(Gtk.Snapshot snapshot)
#endif
	{
#if CROWN_GTK3
		if (_pixbuf == null)
			return Gdk.EVENT_PROPAGATE;
#else
		if (_pixbuf == null) {
			base.snapshot(snapshot);
			return;
		}
#endif

		int allocated_width = this.get_allocated_width();
		int allocated_height = this.get_allocated_height();
#if !CROWN_GTK3
		var cr = snapshot.append_cairo(Graphene.Rect()
			{
				origin = { 0, 0 },
				size = { allocated_width, allocated_height }
			});
#endif

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

#if CROWN_GTK3
		return Gdk.EVENT_PROPAGATE;
#else
		base.snapshot(snapshot);
#endif
	}
}

} /* namespace Crown */
