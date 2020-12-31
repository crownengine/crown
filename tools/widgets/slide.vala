/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class Slide : Gtk.Bin
{
	public void show_widget(Gtk.Widget slide)
	{
		if (this.get_child() != null)
			this.remove(this.get_child());
		this.add(slide);
		this.show_all();
	}
}

}
