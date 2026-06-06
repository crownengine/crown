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
	public string? _ndk_root_path;
	public string _d8_path;
	public string _aapt_path;
	public string _zipalign_path;

	public AndroidDeployer()
	{
		_java_home = null;
		_sdk_path = null;
		_ndk_root_path = null;

		check_config();
	}

	public int check_config()
	{
		_java_home = GLib.Environment.get_variable("JAVA_HOME");
#if CROWN_PLATFORM_WINDOWS
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
		if (_java_home != null) {
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
		} else {
			_javac_path = "javac";
			_jarsigner_path = "jarsigner";
		}
#endif /* if CROWN_PLATFORM_WINDOWS */

		_sdk_path = GLib.Environment.get_variable("ANDROID_SDK_PATH");
		if (_sdk_path == null) {
			loge("Set ANDROID_SDK_PATH environment variable.");
			return -1;
		}

		_ndk_root_path = GLib.Environment.get_variable("ANDROID_NDK_ROOT");
		if (_ndk_root_path == null) {
			loge("Set ANDROID_NDK_ROOT environment variable.");
			return -1;
		}

		string build_tools_path = Path.build_path(Path.DIR_SEPARATOR_S, _sdk_path, "build-tools", "34.0.0");
		if (!GLib.File.new_for_path(build_tools_path).query_exists()) {
			loge("Android build tools not found: '%s'".printf(build_tools_path));
			return -1;
		}

		_d8_path = Path.build_path(Path.DIR_SEPARATOR_S
			, build_tools_path
			, "d8"
			);

		_aapt_path = Path.build_path(Path.DIR_SEPARATOR_S
			, build_tools_path
			, "aapt"
			);

		_zipalign_path = Path.build_path(Path.DIR_SEPARATOR_S
			, build_tools_path
			, "zipalign"
			);

		return 0;
	}
}

} /* namespace Crown */
