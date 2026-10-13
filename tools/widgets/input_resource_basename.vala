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

	protected override void commit_value(string val)
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
			this.value = _value;
			return;
		}

		base.commit_value(val);
	}
}

} /* namespace Crown */
