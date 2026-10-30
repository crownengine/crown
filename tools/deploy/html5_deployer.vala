/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
[CCode (cname = "splash_logo_svg_base64")]
extern unowned string splash_logo_svg_base64;

public class HTML5Deployer
{
	public string? _emscripten_sdk_path;

	public HTML5Deployer()
	{
		_emscripten_sdk_path = null;

		check_config();
	}

	public static int generate_index(string path, string runtime_name, string bundle_url = "")
	{
		string index_html = "";
		index_html += "<!doctype html>";
		index_html += "\n<html lang=\"en-us\">";
		index_html += "\n<head>";
		index_html += "\n<meta charset=\"utf-8\">";
		index_html += "\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
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
		index_html += "\n  #loading {";
		index_html += "\n    position: absolute;";
		index_html += "\n    inset: 0;";
		index_html += "\n    display: flex;";
		index_html += "\n    flex-direction: column;";
		index_html += "\n    align-items: center;";
		index_html += "\n    justify-content: center;";
		index_html += "\n    gap: 24px;";
		index_html += "\n    color: #f0f0f0;";
		index_html += "\n    font: 14px sans-serif;";
		index_html += "\n    text-align: center;";
		index_html += "\n  }";
		index_html += "\n  #loading_progress {";
		index_html += "\n    width: min(480px, 80vw);";
		index_html += "\n    height: 2px;";
		index_html += "\n    background-color: #101010;";
		index_html += "\n    overflow: hidden;";
		index_html += "\n  }";
		index_html += "\n  #loading_progress > div {";
		index_html += "\n    width: var(--progress, 0%);";
		index_html += "\n    height: 100%;";
		index_html += "\n    background-color: #f0f0f0;";
		index_html += "\n  }";
		index_html += "\n  #loading_progress:not([aria-valuenow]) > div {";
		index_html += "\n    width: 2px;";
		index_html += "\n    animation: loading-pulse 1s ease-in-out infinite alternate;";
		index_html += "\n  }";
		index_html += "\n  @keyframes loading-pulse {";
		index_html += "\n    from { opacity: 0.25; }";
		index_html += "\n    to { opacity: 1; }";
		index_html += "\n  }";
		index_html += "\n</style>";
		index_html += "\n</head>";
		index_html += "\n<body>";
		index_html += "\n<canvas class=\"app\" id=\"canvas\" oncontextmenu=\"event.preventDefault()\" tabindex=-1></canvas>";
		index_html += "\n<div id=\"loading\">";
		index_html += "\n  <img width=\"64\" height=\"64\" alt=\"Crown\" src=\"data:image/svg+xml;base64,%s\">".printf(splash_logo_svg_base64);
		index_html += "\n  <div id=\"loading_status\" role=\"status\">Downloading...</div>";
		index_html += "\n  <div id=\"loading_progress\" role=\"progressbar\" aria-label=\"Loading\" aria-valuemin=\"0\" aria-valuemax=\"100\"><div></div></div>";
		index_html += "\n</div>";
		index_html += "\n<script type='text/javascript'>";
		index_html += "\n  var loading = document.getElementById('loading');";
		index_html += "\n  var loadingStatus = document.getElementById('loading_status');";
		index_html += "\n  var loadingProgress = document.getElementById('loading_progress');";
		index_html += "\n  var loadingFailed = false;";
		index_html += "\n  var loadingPercent = 0;";
		index_html += "\n  function setLoadingProgress(percent) {";
		index_html += "\n    loadingPercent = Math.max(loadingPercent, percent);";
		index_html += "\n    loadingProgress.setAttribute('aria-valuenow', loadingPercent);";
		index_html += "\n    loadingProgress.style.setProperty('--progress', loadingPercent + '%');";
		index_html += "\n  }";
		index_html += "\n  function loadingError(message) {";
		index_html += "\n    loadingFailed = true;";
		index_html += "\n    loading.style.display = 'flex';";
		index_html += "\n    loadingStatus.textContent = 'Failed to load. Please reload the page.';";
		index_html += "\n    loadingProgress.removeAttribute('aria-valuenow');";
		index_html += "\n    console.error(message);";
		index_html += "\n  }";
		index_html += "\n  var Module = {";
		if (bundle_url.length != 0)
			index_html += "\n    arguments: ['--bundle-url', decodeURIComponent('%s')],".printf(GLib.Uri.escape_string(bundle_url, null, false));
		else
			index_html += "\n    arguments: [],";
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
		index_html += "\n    onRuntimeInitialized: () => { Module.onSplashProgress(0, 1); },";
		index_html += "\n    onSplashProgress: (completed, total) => {";
		index_html += "\n      if (loadingFailed) return;";
		index_html += "\n      loadingStatus.textContent = 'Initializing...';";
		index_html += "\n      setLoadingProgress(50 + 50 * completed / total);";
		index_html += "\n      if (completed === total) {";
		index_html += "\n        requestAnimationFrame(() => {";
		index_html += "\n          if (!loadingFailed) loading.style.display = 'none';";
		index_html += "\n        });";
		index_html += "\n      }";
		index_html += "\n    },";
		index_html += "\n    setStatus: (text) => {";
		index_html += "\n      if (loadingFailed || !text) return;";
		index_html += "\n      loadingStatus.textContent = text;";
		index_html += "\n      var match = text.match(/\\(([\\d.]+)\\/([\\d.]+)\\)/);";
		index_html += "\n      if (match && Number(match[2]) > 0) {";
		index_html += "\n        setLoadingProgress(Math.min(1, Number(match[1]) / Number(match[2])) * 50);";
		index_html += "\n      }";
		index_html += "\n    },";
		index_html += "\n    totalDependencies: 0,";
		index_html += "\n    monitorRunDependencies: (left) => {";
		index_html += "\n      Module.totalDependencies = Math.max(Module.totalDependencies, left);";
		index_html += "\n      Module.setStatus(left ? 'Preparing... (' + (Module.totalDependencies - left) + '/' + Module.totalDependencies + ')' : 'Preparing...');";
		index_html += "\n    },";
		index_html += "\n    onAbort: loadingError";
		index_html += "\n  };";
		index_html += "\n  window.onerror = loadingError;";
		index_html += "\n  window.addEventListener('error', (event) => {";
		index_html += "\n    if (event.target.tagName === 'SCRIPT')";
		index_html += "\n      loadingError('Failed to load ' + event.target.src);";
		index_html += "\n  }, true);";
		index_html += "\n  function startRuntime() {";
		index_html += "\n    var scripts = ['%s.js'];".printf(runtime_name);
		if (bundle_url.length == 0)
			index_html += "\n    scripts.unshift('data.js');";
		index_html += "\n    for (var src of scripts) {";
		index_html += "\n      var script = document.createElement('script');";
		index_html += "\n      script.src = src;";
		index_html += "\n      script.async = true;";
		index_html += "\n      document.body.appendChild(script);";
		index_html += "\n    }";
		index_html += "\n  }";
		index_html += "\n  window.addEventListener('load', () => {";
		index_html += "\n    if (PerformanceObserver.supportedEntryTypes.includes('paint')) {";
		index_html += "\n      var paintObserver = new PerformanceObserver((list) => {";
		index_html += "\n        if (!list.getEntriesByName('first-contentful-paint').length) return;";
		index_html += "\n        paintObserver.disconnect();";
		index_html += "\n        startRuntime();";
		index_html += "\n      });";
		index_html += "\n      paintObserver.observe({ type: 'paint', buffered: true });";
		index_html += "\n    } else {";
		index_html += "\n      requestAnimationFrame(() => {";
		index_html += "\n        setTimeout(startRuntime, 0);";
		index_html += "\n      });";
		index_html += "\n    }";
		index_html += "\n  });";
		index_html += "\n</script>";
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
		, TargetArch arch
		, string app_title
		, string html5_index_path
		, string exe_name
		, string bundle_url
		)
	{
		check_config();

		logi("Creating HTML5 package...");

		string package_path = package_dir.get_path();
		string dc_platform = null;
		string bin_folder = null;
		if (arch == TargetArch.WASM) {
			dc_platform = "html5";
			bin_folder = "wasm";
		} else if (arch == TargetArch.WASM64) {
			dc_platform = "html5-wasm64";
			bin_folder = "wasm64";
		} else {
			loge("Invalid architecture");
			return -1;
		}

		// Create data bundle.
		try {
			string[] args;
			string bundle_dir = bundle_url.length != 0
				? package_path
				: GLib.DirUtils.make_tmp("XXXXXX")
				;

			args = new string[]
			{
				ENGINE_EXE,
				"--source-dir",
				project.source_dir(),
				"--map-source-dir",
				"core",
				project.toolchain_dir(),
				"--bundle-dir",
				bundle_dir,
				"--compile",
				"--bundle",
				"--platform",
				dc_platform
			};

			if (config == TargetConfig.RELEASE)
				args += "--strip";

			var pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, ENGINE_DIR);
			var exit_status = yield wait_subprocess(pid);

			if (exit_status != 0) {
				loge("Failed to compile data. exit_status = %d".printf(exit_status));
				return exit_status;
			}

			// Copy runtime executables to package folder.
			var runtime_name_src = "crown-%s".printf(deploy_config_name(config));
			var runtime_path_src = Path.build_path(Path.DIR_SEPARATOR_S, "..", "..", bin_folder, "bin", runtime_name_src);
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

			if (bundle_url.length == 0) {
				// Package bundle data with emscripten's file_packager.
				args = new string[]
				{
					Path.build_path(Path.DIR_SEPARATOR_S, _emscripten_sdk_path, "tools", "file_packager"),
					Path.build_path(Path.DIR_SEPARATOR_S, package_path, "data.bin"),
					"--preload",
					"./data",
					"--js-output=" + Path.build_path(Path.DIR_SEPARATOR_S, package_path, "data.js")
				};

				pid = _subprocess_launcher.spawnv_async(subprocess_flags(), args, bundle_dir);
				exit_status = yield wait_subprocess(pid);
				project.delete_tree(GLib.File.new_for_path(bundle_dir));

				if (exit_status != 0) {
					loge("Failed to package data.js. exit_status %d".printf(exit_status));
					return exit_status;
				}
			}

			// Generate index.html.
			var index_html_path = Path.build_path(Path.DIR_SEPARATOR_S, package_path, "index.html");

			if (html5_index_path.length == 0) {
				if (HTML5Deployer.generate_index(index_html_path, runtime_name_src, bundle_url) != 0)
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
