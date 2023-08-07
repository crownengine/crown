/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class EntryResourceBasename : EntryText
{
	public string _basename;
	public bool _stop_emit;

	public string value
	{
		get
		{
			return this.text;
		}
		set
		{
			_stop_emit = true;
			set_value_safe(value);
			_stop_emit = false;
		}
	}

	// Signals
	public signal void value_changed();

	public EntryResourceBasename(string basename)
	{
		this.activate.connect(on_activate);

		_stop_emit = true;
		_basename = "unset";
		set_value_safe(basename);
		_stop_emit = false;
	}

	private void on_activate()
	{
		this.select_region(0, 0);
		this.set_position(-1);
		this.set_value_safe(this.text);
	}

	private void set_value_safe(string val)
	{
		if (val.length == 0
			|| val.has_prefix(" ")
			|| val.has_suffix(" ")
			|| val.index_of_char('.') != -1
			|| val.index_of_char('<') != -1
			|| val.index_of_char('>') != -1
			|| val.index_of_char(':') != -1
			|| val.index_of_char('"') != -1
			|| val.index_of_char('/') != -1
			|| val.index_of_char('\\') != -1
			|| val.index_of_char('|') != -1
			|| val.index_of_char('?') != -1
			|| val.index_of_char('*') != -1
			|| val.is_ascii() == false
			) {
			this.text = _basename;
			return;
		}

		this.text = val;

		// Notify value changed.
		if (_basename != val) {
			_basename = val;
			if (!_stop_emit)
				value_changed();
		}
	}
}

} /* namespace Crown */
