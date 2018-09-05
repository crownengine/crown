/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
	/// <summary>
	/// Show one slide (widget) at a time.
	/// </summary>
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
