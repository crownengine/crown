/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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

	public static int generate_index(string path, string runtime_name)
	{
		string index_html = "";
		index_html += "<!doctype html>";
		index_html += "\n<html lang=\"en-us\">";
		index_html += "\n<head>";
		index_html += "\n<meta charset=\"utf-8\">";
		index_html += "\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
		index_html += "\n<style>";
		index_html += "\n  body {";
		index_html += "\n    margin: 0px;";
		index_html += "\n    background-color: black;";
		index_html += "\n  }";
		index_html += "\n  .app {";
		index_html += "\n    display: block;";
		index_html += "\n    border: 0px;";
		index_html += "\n    margin: 0px;";
		index_html += "\n    position: absolute;";
		index_html += "\n    top: 0;";
		index_html += "\n    left: 0;";
		index_html += "\n    width: 100%;";
		index_html += "\n    height: 100%;";
		index_html += "\n    background-color: black;";
		index_html += "\n  }";
		index_html += "\n</style>";
		index_html += "\n</head>";
		index_html += "\n<body>";
		index_html += "\n<canvas class=\"app\" id=\"canvas\" oncontextmenu=\"event.preventDefault()\" tabindex=-1></canvas>";
		index_html += "\n<script type='text/javascript'>";
		index_html += "\n  var Module = {";
		index_html += "\n    preRun: [],";
		index_html += "\n    postRun: [],";
		index_html += "\n    canvas: (() => {";
		index_html += "\n      var canvas = document.getElementById('canvas');";
		index_html += "\n      canvas.addEventListener(\"webglcontextlost\", (e) => {";
		index_html += "\n        alert('WebGL context lost. You will need to reload the page.');";
		index_html += "\n        e.preventDefault();";
		index_html += "\n      }, false);";
		index_html += "\n      return canvas;";
		index_html += "\n    })(),";
		index_html += "\n    setStatus: (text) => { },";
		index_html += "\n    monitorRunDependencies: (left) => { }";
		index_html += "\n  };";
		index_html += "\n  window.onerror = (event) => {";
		index_html += "\n    console.error('onerror: ' + event.message);";
		index_html += "\n  };";
		index_html += "\n</script>";
		index_html += "\n<script async type=\"text/javascript\" src=\"data.js\"></script>";
		index_html += "\n<script async type=\"text/javascript\" src=\"%s.js\"></script>".printf(runtime_name);
		index_html += "\n</body>";
		index_html += "\n</html>";
		index_html += "\n";

		GLib.FileStream? fs = FileStream.open(path, "w");
		if (fs == null) {
			loge("Failed to open '%s'".printf(path));
			return -1;
		}
		fs.write(index_html.data);
		fs.flush();

		return 0;
	}

	public int check_config()
	{
		_emscripten_sdk_path = GLib.Environment.get_variable("EMSCRIPTEN");
		if (_emscripten_sdk_path == null)
			return -1;

		return 0;
	}

	public async int create_package(Project project
		, GLib.File package_dir
		, TargetConfig config
		, string app_title
		, string html5_index_path
		, string exe_name
		)
	{
		check_config();

		logi("Creating HTML5 package...");

		string package_path = package_dir.get_path();

		// Create data bundle.
		try {
			string[] args;
			string tmp_bundle_dir = GLib.DirUtils.make_tmp("XXXXXX");

			args = new string[]
			{
				ENGINE_EXE,
				"--source-dir",
				project.source_dir(),
				"--map-source-dir",
				"core",
				project.toolchain_dir(),
				"--bundle-dir",
				tmp_bundle_dir,
				"--compile",
				"--bundle",
				"--platform",
				"html5"
			};

			var pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			var exit_status = yield wait_subprocess(pid);

			if (exit_status != 0) {
				loge("Failed to compile data. exit_status = %d".printf(exit_status));
				return exit_status;
			}

			// Copy runtime executables to package folder.
			var runtime_name_src = "crown-%s".printf(deploy_config_name(config));
			var runtime_path_src = Path.build_path(Path.DIR_SEPARATOR_S, "..", "..", "wasm", "bin", runtime_name_src);
			var runtime_name_dst = Path.build_filename(package_path, runtime_name_src);

			var src = File.new_for_path(runtime_path_src + ".js");
			var dst = File.new_for_path(runtime_name_dst + ".js");
			src.copy(dst, FileCopyFlags.OVERWRITE);

			try {
				src = File.new_for_path(runtime_path_src + ".worker.js");
				dst = File.new_for_path(runtime_name_dst + ".worker.js");
				src.copy(dst, FileCopyFlags.OVERWRITE);
			} catch (GLib.Error e) {
				// NOOP: newer emscripten versions embed .worker.js into main .js.
			}

			src = File.new_for_path(runtime_path_src + ".wasm");
			dst = File.new_for_path(runtime_name_dst + ".wasm");
			src.copy(dst, FileCopyFlags.OVERWRITE);

			// Package bundle data with emscripten's file_packager.
			args = new string[]
			{
				Path.build_path(Path.DIR_SEPARATOR_S, _emscripten_sdk_path, "tools", "file_packager"),
				Path.build_path(Path.DIR_SEPARATOR_S, package_path, "data.bin"),
				"--preload",
				"./data",
				"--js-output=" + Path.build_path(Path.DIR_SEPARATOR_S, package_path, "data.js")
			};

			pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, tmp_bundle_dir);
			exit_status = yield wait_subprocess(pid);
			project.delete_tree(GLib.File.new_for_path(tmp_bundle_dir));

			if (exit_status != 0) {
				loge("Failed to package data.js. exit_status %d".printf(exit_status));
				return exit_status;
			}

			// Generate index.html.
			var index_html_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "index.html");

			if (html5_index_path.length == 0) {
				if (HTML5Deployer.generate_index(index_html_path, runtime_name_src) != 0)
					return -1;
			} else {
				GLib.File.new_for_path(html5_index_path).copy(GLib.File.new_for_path(index_html_path), GLib.FileCopyFlags.OVERWRITE);
			}
		} catch (Error e) {
			loge(e.message);
			loge("Failed to deploy '%s'".printf(app_title));
			return -1;
		}

		logi("Done: #FILE(%s)".printf(package_path));
		return 0;
	}
}

} /* namespace Crown */
