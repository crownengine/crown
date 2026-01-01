/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputResourceBasename : InputString
{
	public InputResourceBasename(string basename)
	{
		_value = "unset";
		set_value_safe(basename);
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
