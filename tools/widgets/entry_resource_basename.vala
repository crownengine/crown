/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class EntryResourceBasename : EntryText
{
	public EntryResourceBasename(string basename)
	{
		_stop_emit = true;
		_value = "unset";
		set_value_safe(basename);
		_stop_emit = false;
	}

	public override void set_value_safe(string val)
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
			this.text = _value;
			return;
		}

		base.set_value_safe(val);
	}
}

} /* namespace Crown */
