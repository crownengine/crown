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

	public async int create_package(Project project
		, GLib.File package_dir
		, string config_path
		, TargetConfig config
		, string app_title
		, string app_identifier
		, int app_version_code
		, string app_version_name
		, int min_sdk_version
		, int target_sdk_version
		, string keystore_path
		, string keystore_pass
		, string key_alias
		, string key_pass
		, TargetArch arch
		, string android_manifest_path
		, string apk_name
		)
	{
		if (check_config() != 0)
			return -1;

		logi("Creating Android package (%s)...".printf(arch == TargetArch.ARM ? "ARMv7-A" : "ARMv8-A"));

		var activity_name = "MainActivity";
		var package_path = package_dir.get_path();

		// Architecture-agnostic paths.
		var manifests_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "manifests");
		var java_sources_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "java");
		var app_sources_path = Path.build_path(Path.DIR_SEPARATOR_S, java_sources_path, app_identifier.replace(".", "/"));
		var assets_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "assets");
		var res_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "res");
		var bin_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "bin");
		var obj_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "obj");
		var res_layout_path = Path.build_path(Path.DIR_SEPARATOR_S, res_path, "layout");
		var res_values_path = Path.build_path(Path.DIR_SEPARATOR_S, res_path, "values");
		var res_drawable_path = Path.build_path(Path.DIR_SEPARATOR_S, res_path, "drawable");
		var manifest_xml_path = Path.build_path(Path.DIR_SEPARATOR_S, manifests_path, "AndroidManifest.xml");
		var strings_xml_path = Path.build_path(Path.DIR_SEPARATOR_S, res_path, "values", "strings.xml");
		var activity_java_path = Path.build_path(Path.DIR_SEPARATOR_S, app_sources_path, "%s.java".printf(activity_name));
		var android_jar_path = Path.build_path(Path.DIR_SEPARATOR_S, _sdk_path, "platforms", "android-" + target_sdk_version.to_string(), "android.jar");
		var libcrown_src_name = "libcrown-" + deploy_config_name(config) + ".so";
		var libcpp_name = "libc++_shared.so";
		var signed_apk = Path.build_path(Path.DIR_SEPARATOR_S, bin_path, apk_name + ".signed.apk");
		var unaligned_apk = Path.build_path(Path.DIR_SEPARATOR_S, bin_path, apk_name + ".unaligned.apk");
		var final_apk = Path.build_path(Path.DIR_SEPARATOR_S, config_path, apk_name + ".apk");

#if CROWN_PLATFORM_LINUX
		string host_platform = "linux-x86_64";
#elif CROWN_PLATFORM_WINDOWS
		string host_platform = "windows-x86_64";
#endif

		// Architecture-specific paths.
		string dc_platform = null;
		string bin_folder  = null;
		string apk_arch    = null;
		string llvm_arch   = null;
		if (arch == TargetArch.ARM) {
			dc_platform = "android";
			bin_folder  = "android-arm";
			apk_arch    = "armeabi-v7a";
			llvm_arch   = "arm-linux-androideabi";
		} else if (arch == TargetArch.ARM64) {
			dc_platform = "android-arm64";
			bin_folder  = "android-arm64";
			apk_arch    = "arm64-v8a";
			llvm_arch   = "aarch64-linux-android";
		} else {
			loge("Invalid architecture");
			return -1;
		}

		var libcrown_src_path = Path.build_path(Path.DIR_SEPARATOR_S, "..", "..", bin_folder, "bin", libcrown_src_name);
		var libcpp_src_path   = Path.build_path(Path.DIR_SEPARATOR_S
			, _ndk_root_path
			, "toolchains"
			, "llvm"
			, "prebuilt"
			, host_platform
			, "sysroot"
			, "usr"
			, "lib"
			, llvm_arch
			, libcpp_name
			);
		var lib_path_relative      = Path.build_path(Path.DIR_SEPARATOR_S, "lib", apk_arch);
		var lib_path               = Path.build_path(Path.DIR_SEPARATOR_S, package_path, lib_path_relative);
		var libcrown_path_relative = Path.build_path(Path.DIR_SEPARATOR_S, lib_path_relative, "libcrown.so");
		var libcpp_path_relative   = Path.build_path(Path.DIR_SEPARATOR_S, lib_path_relative, libcpp_name);
		var libcrown_dst_path      = Path.build_path(Path.DIR_SEPARATOR_S, lib_path, "libcrown.so");
		var libcpp_dst_path        = Path.build_path(Path.DIR_SEPARATOR_S, lib_path, "libc++_shared.so");

		if (!GLib.File.new_for_path(android_jar_path).query_exists()) {
			loge("Android platform not found: '%s'".printf(android_jar_path));
			return -1;
		}

		// Create Android project skeleton.
		try {
			GLib.File.new_for_path(manifests_path).make_directory();
			GLib.File.new_for_path(app_sources_path).make_directory_with_parents();
			GLib.File.new_for_path(lib_path).make_directory_with_parents();
			GLib.File.new_for_path(assets_path).make_directory();
			GLib.File.new_for_path(bin_path).make_directory();
			GLib.File.new_for_path(obj_path).make_directory();
			GLib.File.new_for_path(res_layout_path).make_directory_with_parents();
			GLib.File.new_for_path(res_values_path).make_directory();
			GLib.File.new_for_path(res_drawable_path).make_directory();
		} catch (Error e) {
			loge(e.message);
			return -1;
		}

		// Compile game data.
		try {
			GLib.File.new_for_path(libcrown_src_path).copy(GLib.File.new_for_path(libcrown_dst_path), GLib.FileCopyFlags.NONE);
			GLib.File.new_for_path(libcpp_src_path).copy(GLib.File.new_for_path(libcpp_dst_path), GLib.FileCopyFlags.NONE);

			string[] args;

			// Populate Android assets folder with data.
			args = new string[]
			{
				ENGINE_EXE,
				"--source-dir",
				project.source_dir(),
				"--map-source-dir",
				"core",
				project.toolchain_dir(),
				"--bundle-dir",
				assets_path,
				"--compile",
				"--bundle",
				"--platform",
				dc_platform
			};

			uint32 pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			int exit_status = yield wait_subprocess(pid);

			if (exit_status != 0) {
				loge("Failed to compile data. exit_status = %d".printf(exit_status));
				return exit_status;
			}
		} catch (Error e) {
			loge(e.message);
			return -1;
		}

		// Create Android manifest.
		if (android_manifest_path.length == 0) {
			if (AndroidDeployer.generate_manifest(manifest_xml_path
				, app_title
				, app_identifier
				, app_version_code
				, app_version_name
				, min_sdk_version
				, target_sdk_version
				) != 0) {
				return -1;
			}
		} else {
			try {
				GLib.File.new_for_path(android_manifest_path).copy(GLib.File.new_for_path(manifest_xml_path), GLib.FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				loge(e.message);
				return -1;
			}
		}

		// Create Android strings.xml.
		string android_strings = "";
		android_strings += "<resources>";
		android_strings += "\n<string name=\"activity_label\">%s</string>".printf(app_title);
		android_strings += "\n</resources>";
		android_strings += "\n";

		GLib.FileStream? fs = FileStream.open(strings_xml_path, "w");
		if (fs == null) {
			loge("Failed to open '%s'".printf(strings_xml_path));
			return -1;
		}
		fs.write(android_strings.data);
		fs.flush();

		// Create Android activity.
		if (AndroidDeployer.generate_activity(activity_java_path, app_identifier) != 0)
			return -1;

		string[] javac_args = new string[]
		{
			_javac_path,
			"-verbose",
			"-source",
			"8", // https://docs.oracle.com/javase/1.5.0/docs/relnotes/version-5.0.html
			"-target",
			"8", // https://docs.oracle.com/javase/1.5.0/docs/relnotes/version-5.0.html
			"-d",
			obj_path,
			"-classpath",
			"java",
			"-bootclasspath",
			android_jar_path,
			activity_java_path
		};

		int javac_status;
		try {
			uint32 javac = _subprocess_launcher.spawnv_async(subprocess_flags(), javac_args, package_path);
			javac_status = yield wait_subprocess(javac);
		} catch (Error e) {
			loge(e.message);
			return -1;
		}

		if (javac_status != 0) {
			loge("Failed to compile Java activity. exit_status %d".printf(javac_status));
			return javac_status;
		}

		var class_path = Path.build_path(Path.DIR_SEPARATOR_S
			, obj_path
			, app_identifier.replace(".", "/")
			, "%s.class".printf(activity_name)
			);
		var class_file = GLib.File.new_for_path(class_path);

		if (!class_file.query_exists()) {
			loge("Failed to generate .class file");
			return -1;
		}

		GLib.File? debug_keystore_file = null;

		try {
			string[] args;
			uint32 pid;
			int exit_status;

			args = new string[]
			{
				_d8_path,
				"--output",
				bin_path,
				class_path,
				"--no-desugaring"
			};

			pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			exit_status = yield wait_subprocess(pid);
			if (exit_status != 0) {
				loge("Failed to generate dex file. exit_status %d".printf(exit_status));
				return exit_status;
			}

			args = new string[]
			{
				_aapt_path,
				"package",
				"-f",
				"-m",
				"-F",
				unaligned_apk,
				"-M",
				manifest_xml_path,
				"-S",
				res_path,
				"-A",
				assets_path,
				"-I",
				android_jar_path
			};

			pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			exit_status = yield wait_subprocess(pid);
			if (exit_status != 0) {
				loge("Failed to do something with the APK. exit_status %d".printf(exit_status));
				return exit_status;
			}

			args = new string[]
			{
				_aapt_path,
				"add",
				unaligned_apk,
				"classes.dex"
			};

			pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, bin_path);
			exit_status = yield wait_subprocess(pid);
			if (exit_status != 0) {
				loge("Failed to add classes.dex to APK. exit_status %d".printf(exit_status));
				return exit_status;
			}

			args = new string[]
			{
				_aapt_path,
				"add",
				unaligned_apk,
				libcrown_path_relative.replace("\\", "/"),
				libcpp_path_relative.replace("\\", "/")
			};

			pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, package_path);
			exit_status = yield wait_subprocess(pid);
			if (exit_status != 0) {
				loge("Failed to add libs to APK. exit_status %d".printf(exit_status));
				return exit_status;
			}

			string signing_keystore_path = keystore_path;
			if (signing_keystore_path.length == 0) {
				GLib.FileIOStream debug_keystore_stream;
				debug_keystore_file = GLib.File.new_tmp("crown-debug-keystore-XXXXXX", out debug_keystore_stream);
				debug_keystore_stream.close();
				signing_keystore_path = debug_keystore_file.get_path();
				if (AndroidDeployer.write_debug_keystore(signing_keystore_path) != 0) {
					try {
						debug_keystore_file.delete();
					} catch (Error cleanup_error) {
						logw(cleanup_error.message);
					}
					debug_keystore_file = null;
					return -1;
				}
			}

			args = new string[]
			{
				_jarsigner_path,
				"-keystore",
				signing_keystore_path,
				"-storepass",
				keystore_pass,
				"-keypass",
				key_pass,
				"-signedjar",
				signed_apk,
				unaligned_apk,
				key_alias
			};

			pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			exit_status = yield wait_subprocess(pid);
			if (debug_keystore_file != null) {
				try {
					debug_keystore_file.delete();
				} catch (Error cleanup_error) {
					logw(cleanup_error.message);
				}
				debug_keystore_file = null;
			}
			if (exit_status != 0) {
				loge("Failed sign APK. exit_status %d".printf(exit_status));
				return exit_status;
			}

			args = new string[]
			{
				_zipalign_path,
				"-f",
				"4",
				signed_apk,
				final_apk
			};

			pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			exit_status = yield wait_subprocess(pid);
			if (exit_status != 0) {
				loge("Failed align APK. exit_status %d".printf(exit_status));
				return exit_status;
			}
		} catch (Error e) {
			if (debug_keystore_file != null) {
				try {
					debug_keystore_file.delete();
				} catch (Error cleanup_error) {
					logw(cleanup_error.message);
				}
			}
			loge(e.message);
			loge("Failed to deploy '%s'".printf(app_title));
			return -1;
		}

		logi("Done: #FILE(%s)".printf(config_path));
		return 0;
	}
}

} /* namespace Crown */
