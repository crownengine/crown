/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
[CCode (cname = "android_debug_keystore")]
extern uint8* android_debug_keystore;

[CCode (cname = "android_debug_keystore_size")]
extern size_t android_debug_keystore_size;

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

	public static int generate_manifest(string path
		, string app_title
		, string app_identifier
		, int app_version_code
		, string app_version_name
		, int min_sdk_version
		, int target_sdk_version
		)
	{
		var activity_name = "MainActivity";
		string android_manifest = "";
		android_manifest += "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
		android_manifest += "\n<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"";
		android_manifest += "\n  package=\"%s\"".printf(app_identifier);
		android_manifest += "\n  android:versionCode=\"%u\"".printf(app_version_code);
		android_manifest += "\n  android:versionName=\"%s\">".printf(app_version_name);
		android_manifest += "\n";
		android_manifest += "\n  <uses-sdk";
		android_manifest += "\n    android:minSdkVersion=\"%d\"".printf(min_sdk_version);
		android_manifest += "\n    android:targetSdkVersion=\"%d\" />".printf(target_sdk_version);
		android_manifest += "\n";
		android_manifest += "\n  <!-- For ConsoleServer -->";
		android_manifest += "\n  <uses-permission android:name=\"android.permission.INTERNET\" />";
		android_manifest += "\n";
		android_manifest += "\n  <!-- For mouse input -->";
		android_manifest += "\n  <uses-feature";
		android_manifest += "\n    android:name=\"android.hardware.type.pc\"";
		android_manifest += "\n    android:required=\"false\" />";
		android_manifest += "\n";
		android_manifest += "\n  <application";
		android_manifest += "\n    android:hasCode=\"true\"";
		android_manifest += "\n    android:label=\"%s\">".printf(app_title);
		android_manifest += "\n    <activity";
		android_manifest += "\n      android:name=\"%s.%s\"".printf(app_identifier, activity_name);
		android_manifest += "\n      android:configChanges=\"orientation|keyboardHidden\"";
		android_manifest += "\n      android:label=\"@string/activity_label\"";
		android_manifest += "\n      android:screenOrientation=\"landscape\"";
		android_manifest += "\n      android:theme=\"@android:style/Theme.NoTitleBar.Fullscreen\">";
		android_manifest += "\n";
		android_manifest += "\n      <!-- Tell NativeActivity the name of our .so -->";
		android_manifest += "\n      <meta-data";
		android_manifest += "\n        android:name=\"android.app.lib_name\"";
		android_manifest += "\n        android:value=\"crown\" />";
		android_manifest += "\n";
		android_manifest += "\n      <intent-filter>";
		android_manifest += "\n        <action android:name=\"android.intent.action.MAIN\" />";
		android_manifest += "\n        <action android:name=\"android.intent.action.VIEW\" />";
		android_manifest += "\n        <category android:name=\"android.intent.category.LAUNCHER\" />";
		android_manifest += "\n      </intent-filter>";
		android_manifest += "\n    </activity>";
		android_manifest += "\n  </application>";
		android_manifest += "\n</manifest>";
		android_manifest += "\n";

		GLib.FileStream? fs = FileStream.open(path, "w");
		if (fs == null) {
			loge("Failed to open '%s'".printf(path));
			return -1;
		}
		fs.write(android_manifest.data);
		fs.flush();

		return 0;
	}

	public static int write_debug_keystore(string path)
	{
		GLib.FileStream? fs = FileStream.open(path, "wb");
		if (fs == null) {
			loge("Failed to open '%s'".printf(path));
			return -1;
		}

		unowned uint8[] data = (uint8[])android_debug_keystore;
		data.length = (int)android_debug_keystore_size;

		if (fs.write(data) != android_debug_keystore_size) {
			loge("Failed to write '%s'".printf(path));
			return -1;
		}
		fs.flush();

		return 0;
	}

	public static int generate_activity(string path, string app_identifier)
	{
		var activity_name = "MainActivity";
		string android_activity = "";
		android_activity += "package %s;".printf(app_identifier);
		android_activity += "\n";
		android_activity += "\nimport android.app.NativeActivity;";
		android_activity += "\nimport android.os.Bundle;";
		android_activity += "\nimport android.view.View;";
		android_activity += "\n";
		android_activity += "\npublic class %s extends NativeActivity".printf(activity_name);
		android_activity += "\n{";
		android_activity += "\n    static";
		android_activity += "\n    {";
		android_activity += "\n        System.loadLibrary(\"crown\");";
		android_activity += "\n    }";
		android_activity += "\n";
		android_activity += "\n    @Override";
		android_activity += "\n    public void onCreate(Bundle savedInstanceState) {";
		android_activity += "\n        super.onCreate(savedInstanceState);";
		android_activity += "\n        // Init additional stuff here (Ads etc.)";
		android_activity += "\n    }";
		android_activity += "\n";
		android_activity += "\n    @Override";
		android_activity += "\n    public void onDestroy() {";
		android_activity += "\n        // Destroy additional stuff here (Ads etc)";
		android_activity += "\n        super.onDestroy();";
		android_activity += "\n    }";
		android_activity += "\n";
		android_activity += "\n    @Override";
		android_activity += "\n    public void onWindowFocusChanged(boolean hasFocus) {";
		android_activity += "\n        super.onWindowFocusChanged(hasFocus);";
		android_activity += "\n        if (hasFocus) {";
		android_activity += "\n            hideSystemUI();";
		android_activity += "\n        }";
		android_activity += "\n    }";
		android_activity += "\n";
		android_activity += "\n    public void hideSystemUI() {";
		android_activity += "\n        // Enables regular immersive mode.";
		android_activity += "\n        // For \"lean back\" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.";
		android_activity += "\n        // Or for \"sticky immersive,\" replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY";
		android_activity += "\n        View decorView = getWindow().getDecorView();";
		android_activity += "\n        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE";
		android_activity += "\n            // Set the content to appear under the system bars so that the";
		android_activity += "\n            // content doesn't resize when the system bars hide and show.";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_LAYOUT_STABLE";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN";
		android_activity += "\n            // Hide the nav bar and status bar";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_FULLSCREEN";
		android_activity += "\n            );";
		android_activity += "\n    }";
		android_activity += "\n";
		android_activity += "\n    public void showSystemUI() {";
		android_activity += "\n        // Shows the system bars by removing all the flags";
		android_activity += "\n        // except for the ones that make the content appear under the system bars.";
		android_activity += "\n        View decorView = getWindow().getDecorView();";
		android_activity += "\n        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION";
		android_activity += "\n            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN";
		android_activity += "\n            );";
		android_activity += "\n    }";
		android_activity += "\n}";
		android_activity += "\n";

		GLib.FileStream? fs = FileStream.open(path, "w");
		if (fs == null) {
			loge("Failed to open '%s'".printf(path));
			return -1;
		}
		fs.write(android_activity.data);
		fs.flush();

		return 0;
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
