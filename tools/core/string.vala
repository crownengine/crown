/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public const int PRINT_MAX_DECIMALS_BUFFER_SIZE = 512;

public static unowned string print_max_decimals(char[] buffer, double num, int max_decimals)
{
	assert(max_decimals >= 0);
	assert(max_decimals <= buffer.length - double.MAX_10_EXP - 4);

	char format[16];
	int num_digits = 1;
	for (int precision = max_decimals; precision >= 10; precision /= 10)
		++num_digits;

	format[0] = '%';
	format[1] = '.';
	format[2 + num_digits] = 'f';
	format[3 + num_digits] = '\0';

	int precision = max_decimals;
	for (int ii = 1 + num_digits; ii >= 2; --ii) {
		format[ii] = (char)('0' + precision % 10);
		precision /= 10;
	}

	unowned string formatted = num.format(buffer, (string)format);
	int len = formatted.length;

	if (max_decimals > 0) {
		// Trim trailing zeroes.
		while (len > 0 && formatted[len - 1] == '0')
			len--;

		// Remove trailing decimal point, if any.
		if (len > 0 && formatted[len - 1] == '.')
			len--;
	}

	buffer[len] = '\0';
	return formatted;
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
