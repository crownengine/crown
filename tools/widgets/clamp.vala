/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class Clamp : Gtk.Bin
{
	// Drop-in replacement (sort-of) for HdyClamp from libhandy1.
	public Clamp()
	{
	}

	protected override void size_allocate(Gtk.Allocation alloc)
	{
		Gtk.Widget? child = this.get_child();
		if (child == null || !child.is_visible())
			return;

		int child_min_width;
		child.get_preferred_width(out child_min_width, null);

		Gtk.Allocation child_alloc = {};
		child_alloc.width = 600;
		child_alloc.height = alloc.height;
		child_alloc.x = alloc.x + (alloc.width - child_alloc.width) / 2;
		child_alloc.y = alloc.y;

		child.size_allocate_with_baseline(child_alloc, this.get_allocated_baseline());
	}
}

}
