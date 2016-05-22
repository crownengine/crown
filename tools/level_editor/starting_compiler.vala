/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;

namespace Crown
{
	public class StartingCompiler : Gtk.Bin
	{
		public StartingCompiler()
		{
			add(new Gtk.Label("Compiling resources, please wait..."));
			set_size_request(300, 300);
			show_all();
		}
	}
}
