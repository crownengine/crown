/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class AndroidDeployer
{
	public string? _java_home;
	public string _javac_path;
	public string _jarsigner_path;

	public string? _sdk_path;
	public string? _sdk_api_level;
	public string? _ndk_root_path;
	public string? _build_tools_path;
	public string _d8_path;
	public string _aapt_path;
	public string _zipalign_path;

	public AndroidDeployer()
	{
		_sdk_path = null;
		_ndk_root_path = null;
		_build_tools_path = null;

		check_config();
	}

	public int check_config()
	{
		_java_home = GLib.Environment.get_variable("JAVA_HOME");
#if CROWN_PLATFORM_WINDOWS
		// JAVA_HOME must be defined.
		if (_java_home == null) {
			loge("Set JAVA_HOME environment variable.");
			return -1;
		}

		_javac_path = Path.build_path(Path.DIR_SEPARATOR_S
			, _java_home
			, "bin"
			, "javac"
			);

		_jarsigner_path = Path.build_path(Path.DIR_SEPARATOR_S
			, _java_home
			, "bin"
			, "jarsigner"
			);
#elif CROWN_PLATFORM_LINUX
		_javac_path = "javac";
		_jarsigner_path = "jarsigner";
#endif /* if CROWN_PLATFORM_WINDOWS */

		_sdk_path = GLib.Environment.get_variable("ANDROID_SDK_PATH");
		if (_sdk_path == null) {
			loge("Set ANDROID_SDK_PATH environment variable.");
			return -1;
		}

		_sdk_api_level = GLib.Environment.get_variable("ANDROID_API_LEVEL");
		if (_sdk_api_level == null) {
			loge("Set ANDROID_API_LEVEL environment variable.");
			return -1;
		}

		_ndk_root_path = GLib.Environment.get_variable("ANDROID_NDK_ROOT");
		if (_ndk_root_path == null) {
			loge("Set ANDROID_NDK_ROOT environment variable.");
			return -1;
		}

		_build_tools_path = GLib.Environment.get_variable("ANDROID_BUILD_TOOLS");
		if (_build_tools_path == null) {
			loge("Set ANDROID_BUILD_TOOLS environment variable.");
			return -1;
		}

		_d8_path = Path.build_path(Path.DIR_SEPARATOR_S
			, _build_tools_path
			, "d8"
			);

		_aapt_path = Path.build_path(Path.DIR_SEPARATOR_S
			, _build_tools_path
			, "aapt"
			);

		_zipalign_path = Path.build_path(Path.DIR_SEPARATOR_S
			, _build_tools_path
			, "zipalign"
			);

		return 0;
	}
}

} /* namespace Crown */
