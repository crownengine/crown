/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;

namespace Crown
{
	/// <summary>
	/// Map combo box.
	/// </summary>
	public class ComboBoxMap : Gtk.ComboBoxText
	{
		public ComboBoxMap()
		{
			this.scroll_event.connect(on_scroll);
		}

		private bool on_scroll(Gdk.EventScroll ev)
		{
			GLib.Signal.stop_emission_by_name(this, "scroll-event");
			return false;
		}
	}
}
