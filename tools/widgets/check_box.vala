/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;
using Gdk;

namespace Crown
{
	/// <summary>
	/// Check box.
	/// </summary>
	public class CheckBox : Gtk.Bin
	{
		// Data
		private bool _stop_emit;

		// Widgets
		private Gtk.CheckButton _check_button;

		public bool value
		{
			get
			{
				return _check_button.active;
			}
			set
			{
				_stop_emit = true;
				_check_button.active = value;
				_stop_emit = false;
			}
		}

		// Signals
		public signal void value_changed();

		public CheckBox()
		{
			this.hexpand = true;

			// Data
			_stop_emit = false;

			// Widgets
			_check_button = new CheckButton();

			_check_button.toggled.connect(on_value_changed);

			add(_check_button);
		}

		private void on_value_changed()
		{
			if (!_stop_emit)
				value_changed();
		}
	}
}
