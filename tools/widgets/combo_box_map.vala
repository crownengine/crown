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
	public class ComboBoxMap : Gtk.Bin
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private Gtk.ComboBoxText _combo_box;

		// Signals
		public signal void value_changed();

		public string value
		{
			get
			{
				return _combo_box.get_active_id();
			}
			set
			{
				_stop_emit = true;
				_combo_box.set_active_id((string)value);
				_stop_emit = false;
			}
		}

		public ComboBoxMap()
		{
			// Data
			_stop_emit = true;

			// Widgets
			_combo_box = new Gtk.ComboBoxText();
			_combo_box.changed.connect(on_changed);
			_combo_box.scroll_event.connect(on_scroll);

			add(_combo_box);
		}

		public void append(string? id, string text)
		{
			_combo_box.append(id, text);
		}

		private void on_changed()
		{
			if (!_stop_emit)
				value_changed();
		}

		private bool on_scroll(Gdk.EventScroll ev)
		{
			GLib.Signal.stop_emission_by_name(_combo_box, "scroll-event");
			return false;
		}
	}
}
