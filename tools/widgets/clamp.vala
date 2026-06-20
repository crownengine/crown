/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
// Drop-in replacement (sort-of) for HdyClamp from libhandy1.
public class Clamp : Gtk.Container
{
	const int MAXIMUM_SIZE = 600;

	public Gtk.Widget _child;

	public Clamp()
	{
		base.set_has_window(false);
		base.set_can_focus(false);
		base.set_redraw_on_allocate(false);

		this._child = null;
	}

	public void set_child(Gtk.Widget widget)
	{
		if (this._child == null) {
			widget.set_parent(this);
			this._child = widget;
		}
	}

	public override void remove(Gtk.Widget widget)
	{
		if (this._child == widget) {
			widget.unparent();
			this._child = null;
			if (this.get_visible() && widget.get_visible())
				this.queue_resize_no_redraw();
		}
	}

	public override void forall_internal(bool include_internals, Gtk.Callback callback)
	{
		if (this._child != null)
			callback(this._child);
	}

	public override Gtk.SizeRequestMode get_request_mode()
	{
		return Gtk.SizeRequestMode.HEIGHT_FOR_WIDTH;
	}

	public Gtk.Widget get_child()
	{
		return this._child;
	}

	public override void size_allocate(Gtk.Allocation alloc)
	{
		this.set_allocation(alloc);

		if (this._child == null || !this._child.get_visible())
			return;

		int margin_width = this._child.margin_start + this._child.margin_end;
		int margin_height = this._child.margin_top + this._child.margin_bottom;
		int child_width = clamp_child_width(alloc.width);

		Gtk.Allocation child_alloc = {};
		// GTK subtracts child margins during allocation; keep the outer allocation large enough to
		// avoid negative inner sizes.
		child_alloc.width = int.max(child_width, margin_width);
		child_alloc.height = int.max(alloc.height, margin_height);
		child_alloc.x = alloc.x + (alloc.width - child_alloc.width) / 2;
		child_alloc.y = alloc.y;

		this._child.size_allocate_with_baseline(child_alloc, this.get_allocated_baseline());
	}

	int clamp_child_width(int clamp_width)
	{
		if (this._child == null || !this._child.get_visible())
			return 0;

		int child_min_width;
		int child_nat_width;
		this._child.get_preferred_width(out child_min_width, out child_nat_width);

		int child_max_width = int.max(child_min_width, MAXIMUM_SIZE);
		if (clamp_width < 0)
			return int.min(child_nat_width, child_max_width);

		if (clamp_width <= child_min_width)
			return clamp_width;

		return int.min(clamp_width, child_max_width);
	}

	public override void get_preferred_width(out int minimum_width, out int natural_width)
	{
		if (this._child == null || !this._child.get_visible()) {
			minimum_width = 0;
			natural_width = 0;
			return;
		}

		this._child.get_preferred_width(out minimum_width, out natural_width);
		natural_width = int.max(minimum_width, int.min(natural_width, MAXIMUM_SIZE));
	}

	public override void get_preferred_height(out int minimum_height, out int natural_height)
	{
		if (this._child == null || !this._child.get_visible()) {
			minimum_height = 0;
			natural_height = 0;
			return;
		}

		this._child.get_preferred_height(out minimum_height, out natural_height);
	}

	public override void get_preferred_height_for_width(int width, out int minimum_height, out int natural_height)
	{
		if (this._child == null || !this._child.get_visible()) {
			minimum_height = 0;
			natural_height = 0;
			return;
		}

		int child_width = clamp_child_width(width);
		this._child.get_preferred_height_for_width(child_width, out minimum_height, out natural_height);
	}
}

} /* namespace Crown */
