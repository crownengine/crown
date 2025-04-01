/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class HTML5Deployer
{
	public string? _emscripten_sdk_path;

	public HTML5Deployer()
	{
		_emscripten_sdk_path = null;

		check_config();
	}

	public int check_config()
	{
		_emscripten_sdk_path = GLib.Environment.get_variable("EMSCRIPTEN");
		if (_emscripten_sdk_path == null)
			return -1;

		return 0;
	}
}

} /* namespace Crown */
