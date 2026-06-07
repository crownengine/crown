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
}

} /* namespace Crown */
