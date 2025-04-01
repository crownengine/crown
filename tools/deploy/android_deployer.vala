/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class AndroidDeployer
{
	public string? _sdk_path;
	public string? _ndk_root_path;

	public AndroidDeployer()
	{
		_sdk_path = null;
		_ndk_root_path = null;

		check_config();
	}

	public int check_config()
	{
		_sdk_path = GLib.Environment.get_variable("ANDROID_SDK_PATH");
		if (_sdk_path == null)
			return -1;

		_ndk_root_path = GLib.Environment.get_variable("ANDROID_NDK_ROOT");
		if (_ndk_root_path == null)
			return -1;

		return 0;
	}
}

} /* namespace Crown */
