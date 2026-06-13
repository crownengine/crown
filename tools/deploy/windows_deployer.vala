/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class WindowsDeployer
{
	public async int create_package(Project project
		, GLib.File package_dir
		, TargetConfig config
		, string app_title
		, string exe_name
		)
	{
		logi("Creating Windows package");

		string package_path = package_dir.get_path();

		// Create data bundle.
		try {
			string args[] =
			{
				ENGINE_EXE,
				"--source-dir",
				project.source_dir(),
				"--map-source-dir",
				"core",
				project.toolchain_dir(),
				"--bundle-dir",
				package_path,
				"--compile",
				"--bundle",
				"--platform",
				"windows"
			};

			uint32 compiler = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			int exit_status = yield wait_subprocess(compiler);

			if (exit_status != 0) {
				loge("Failed to compile data. exit_status = %d".printf(exit_status));
				return exit_status;
			}

			GLib.File engine_exe_src = File.new_for_path(EXE_PREFIX + "crown-%s".printf(deploy_config_name(config)) + EXE_SUFFIX);
			GLib.File engine_exe_dst = File.new_for_path(Path.build_filename(package_path, exe_name + EXE_SUFFIX));
			engine_exe_src.copy(engine_exe_dst, FileCopyFlags.OVERWRITE);

			string openal_name = "openal-release.dll";
			GLib.File openal_dll_src = File.new_for_path(openal_name);
			GLib.File openal_dll_dst = File.new_for_path(Path.build_filename(package_path, openal_name));
			openal_dll_src.copy(openal_dll_dst, FileCopyFlags.OVERWRITE);

			try {
				string lua_name = "lua51.dll";
				GLib.File lua_dll_src = File.new_for_path(lua_name);
				GLib.File lua_dll_dst = File.new_for_path(Path.build_filename(package_path, lua_name));
				lua_dll_src.copy(lua_dll_dst, FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				// Ignore: runtime's MinGW builds statically link luajit.
			}
		} catch (Error e) {
			loge("%s".printf(e.message));
			loge("Failed to deploy '%s'".printf(app_title));
			return -1;
		}

		logi("Done: #FILE(%s)".printf(package_path));
		return 0;
	}
}

} /* namespace Crown */
