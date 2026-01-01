/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public static string print_max_decimals(double num, int max_decimals)
{
	string formatted = "%.*f".printf(max_decimals, num);
	int len = formatted.length;

	if (max_decimals > 0) {
		// Trim trailing zeroes.
		while (len > 0 && formatted[len - 1] == '0')
			len--;

		// Remove trailing decimal point, if any.
		if (len > 0 && formatted[len - 1] == '.')
			len--;

		return formatted.substring(0, len);
	}

	// Strip the decimal point and anything after.
	int dot = formatted.index_of_char('.');
	return (dot >= 0) ? formatted.substring(0, dot) : formatted;
}

public static string camel_case(string str)
{
	int len = str.length;
	GLib.StringBuilder sb = new GLib.StringBuilder.sized(len);
	bool capitalize = true;

	for (int i = 0; i < len; i++) {
		char c = str[i];

		if (c.isalnum()) {
			sb.append_c(capitalize ? c.toupper() : c.tolower());
			capitalize = false;
		} else {
			if (sb.len > 0 && sb.str[sb.len - 1] != ' ')
				sb.append_c(' ');
			capitalize = true;
		}
	}

	return sb.str;
}

} /* namespace Crown */
