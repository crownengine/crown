/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#if CROWN_PLATFORM_WINDOWS
extern uint GetCurrentProcessId();
extern uintptr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, uint dwProcessId);
#elif CROWN_PLATFORM_LINUX
extern Posix.pid_t getpid();
#endif

namespace Crown
{
// Global paths
public static GLib.File _toolchain_dir;
public static GLib.File _templates_dir;
public static GLib.File _po_dir;
public static GLib.File _data_dir;
public static GLib.File _config_dir;
public static GLib.File _cache_dir;
public static GLib.File _logs_dir;
public static GLib.File _thumbnails_dir;
public static GLib.File _thumbnails_normal_dir;
public static GLib.File _documents_dir;
public static GLib.File _log_file;
public static GLib.File _settings_file;
public static GLib.File _user_file;
public static GLib.File _console_history_file;
public static GLib.File _window_state_file;

public static GLib.FileStream _log_stream;
public static string _log_prefix;

public static void log(string system, string severity, string message)
{
	GLib.DateTime now = new GLib.DateTime.now_local();
	int now_us = now.get_microsecond();
	string now_str = now.format("%H:%M:%S");

	if (_log_stream != null) {
		string plain_text_line = "%s.%06d  %.4s %s: %s\n".printf(now_str
			, now_us
			, severity.ascii_up()
			, system
			, message
			);
		_log_stream.puts(plain_text_line);
		_log_stream.flush();
	}

	if (_console_view_valid) {
		string time = "%s.%06d  ".printf(now_str, now_us);
		_console_view.log(time, system, severity, message);
	}
}

public static void logi(string message)
{
	log(_log_prefix, "info", message);
}

public static void logw(string message)
{
	log(_log_prefix, "warning", message);
}

public static void loge(string message)
{
	log(_log_prefix, "error", message);
}

public struct CommandLineOptions
{
	public bool show_help;
	public bool show_version;
	public bool run_unit_tests;
	public string? source_dir;
	public string? positional_source_dir;
	public string level_resource;
	public bool do_init;
	public bool do_import;
	public string[] import_filenames;
	public string import_destination;
	public string? import_as;
	public bool do_deploy;
	public DeployOptions deploy;

	public void print_help(GLib.FileStream stream)
	{
		stream.printf("Crown Editor\n"
			+ "Copyright (c) 2012-2026 Daniele Bartolini et al.\n"
			+ "SPDX-License-Identifier: GPL-3.0-or-later\n"
			+ "\n"
			+ "Usage:\n"
			+ "  crown-editor [options] [<source-dir> [<level>]]\n"
			+ "\n"
			+ "Options:\n"
			+ "  -h, --help                       Display this help and exit.\n"
			+ "  -v, --version                    Display version information and exit.\n"
			+ "  --run-unit-tests                 Run unit tests and exit.\n"
			+ "  --source-dir <path>              Project source directory.\n"
			+ "  --init                           Create a new project.\n"
			+ "  --import <file>... <path>        Import files into a source-dir relative path.\n"
			+ "  --as <type>                      Importer type.\n"
			+ "      font\n"
			+ "      mesh\n"
			+ "      sound\n"
			+ "      sprite\n"
			+ "      texture\n"
			+ "  --deploy                         Deploy the project.\n"
			+ "  --platform <platform>            Deploy target platform.\n"
			+ "      android\n"
			+ "      html5\n"
			+ "      linux\n"
			+ "      windows\n"
			+ "  --output-dir <path>              Deploy output directory.\n"
			+ "  --config <config>                Deploy config.\n"
			+ "      release\n"
			+ "      development\n"
			+ "      debug\n"
			+ "  --app-title <title>              Application title.\n"
			+ "  --force                          Overwrite an existing package directory.\n"
			+ "  --arch <arch>                    Android or HTML5 architecture.\n"
			+ "      arm\n"
			+ "      arm64\n"
			+ "      wasm\n"
			+ "      wasm64\n"
			+ "  --app-id <id>                    Android application identifier.\n"
			+ "  --app-version-code <number>      Android version code.\n"
			+ "  --app-version-name <name>        Android version name.\n"
			+ "  --min-sdk-version <number>       Android minimum SDK version.\n"
			+ "  --target-sdk-version <number>    Android target SDK version.\n"
			+ "  --manifest <path>                Android manifest file.\n"
			+ "  --keystore <path>                Android signing keystore.\n"
			+ "  --keystore-pass <password>       Android keystore password.\n"
			+ "  --key-alias <alias>              Android signing key alias.\n"
			+ "  --key-pass <password>            Android signing key password.\n"
			+ "  --index-html <path>              HTML5 index.html file.\n"
			);
	}

	public bool parse(ref string[] args, out string error)
	{
		show_help = false;
		show_version = false;
		run_unit_tests = false;
		source_dir = null;
		positional_source_dir = null;
		level_resource = "";
		do_init = false;
		do_import = false;
		import_filenames = {};
		import_destination = "";
		import_as = null;
		do_deploy = false;
		deploy = DeployOptions();
		error = "";

		string? option_source_dir = null;
		bool option_show_help = false;
		bool option_show_version = false;
		bool option_run_unit_tests = false;
		bool option_do_init = false;
		bool option_do_import = false;
		string? option_import_as = null;
		bool option_do_deploy = false;
		string? option_deploy_platform = null;
		string? option_deploy_output_dir = null;
		string? option_deploy_config = null;
		string? option_deploy_app_title = null;
		bool option_deploy_force = false;
		string? option_deploy_arch = null;
		string? option_deploy_app_id = null;
		string? option_deploy_app_version_code = null;
		string? option_deploy_app_version_name = null;
		string? option_deploy_min_sdk_version = null;
		string? option_deploy_target_sdk_version = null;
		string? option_deploy_manifest = null;
		string? option_deploy_keystore = null;
		string? option_deploy_keystore_pass = null;
		string? option_deploy_key_alias = null;
		string? option_deploy_key_pass = null;
		string? option_deploy_index_html = null;

		GLib.OptionEntry[] option_entries =
		{
			{ "help",               'h', 0, GLib.OptionArg.NONE,     ref option_show_help,                 "Display this help and exit.",           null       },
			{ "version",            'v', 0, GLib.OptionArg.NONE,     ref option_show_version,              "Display version information and exit.", null       },
			{ "run-unit-tests",     0,   0, GLib.OptionArg.NONE,     ref option_run_unit_tests,            "Run unit tests and exit.",              null       },
			{ "source-dir",         0,   0, GLib.OptionArg.FILENAME, ref option_source_dir,                "Project source directory.",             "path"     },
			{ "init",               0,   0, GLib.OptionArg.NONE,     ref option_do_init,                   "Create a new project.",                 null       },
			{ "import",             0,   0, GLib.OptionArg.NONE,     ref option_do_import,                 "Import files.",                         null       },
			{ "as",                 0,   0, GLib.OptionArg.STRING,   ref option_import_as,                 "Importer type.",                        "type"     },
			{ "deploy",             0,   0, GLib.OptionArg.NONE,     ref option_do_deploy,                 "Deploy the project.",                   null       },
			{ "platform",           0,   0, GLib.OptionArg.STRING,   ref option_deploy_platform,           "Deploy target platform.",               "platform" },
			{ "output-dir",         0,   0, GLib.OptionArg.FILENAME, ref option_deploy_output_dir,         "Deploy output directory.",              "path"     },
			{ "config",             0,   0, GLib.OptionArg.STRING,   ref option_deploy_config,             "Deploy config.",                        "config"   },
			{ "app-title",          0,   0, GLib.OptionArg.STRING,   ref option_deploy_app_title,          "Application title.",                    "title"    },
			{ "force",              0,   0, GLib.OptionArg.NONE,     ref option_deploy_force,              "Overwrite package directory.",          null       },
			{ "arch",               0,   0, GLib.OptionArg.STRING,   ref option_deploy_arch,               "Android or HTML5 architecture.",        "arch"     },
			{ "app-id",             0,   0, GLib.OptionArg.STRING,   ref option_deploy_app_id,             "Android application identifier.",       "id"       },
			{ "app-version-code",   0,   0, GLib.OptionArg.STRING,   ref option_deploy_app_version_code,   "Android version code.",                 "number"   },
			{ "app-version-name",   0,   0, GLib.OptionArg.STRING,   ref option_deploy_app_version_name,   "Android version name.",                 "name"     },
			{ "min-sdk-version",    0,   0, GLib.OptionArg.STRING,   ref option_deploy_min_sdk_version,    "Android minimum SDK version.",          "number"   },
			{ "target-sdk-version", 0,   0, GLib.OptionArg.STRING,   ref option_deploy_target_sdk_version, "Android target SDK version.",           "number"   },
			{ "manifest",           0,   0, GLib.OptionArg.FILENAME, ref option_deploy_manifest,           "Android manifest file.",                "path"     },
			{ "keystore",           0,   0, GLib.OptionArg.FILENAME, ref option_deploy_keystore,           "Android signing keystore.",             "path"     },
			{ "keystore-pass",      0,   0, GLib.OptionArg.STRING,   ref option_deploy_keystore_pass,      "Android keystore password.",            "password" },
			{ "key-alias",          0,   0, GLib.OptionArg.STRING,   ref option_deploy_key_alias,          "Android signing key alias.",            "alias"    },
			{ "key-pass",           0,   0, GLib.OptionArg.STRING,   ref option_deploy_key_pass,           "Android signing key password.",         "password" },
			{ "index-html",         0,   0, GLib.OptionArg.FILENAME, ref option_deploy_index_html,         "HTML5 index.html file.",                "path"     },
			{ null }
		};

		GLib.OptionContext opt_context = new GLib.OptionContext("[<source-dir> [<level>]]");
		opt_context.set_help_enabled(false);
		opt_context.add_main_entries(option_entries, null);

		try {
			opt_context.parse_strv(ref args);
		} catch (GLib.OptionError e) {
			error = e.message;
			return false;
		}

		show_help = option_show_help;
		show_version = option_show_version;
		run_unit_tests = option_run_unit_tests;
		source_dir = option_source_dir;
		do_init = option_do_init;
		do_import = option_do_import;
		import_as = option_import_as;
		do_deploy = option_do_deploy;

		if (show_help || show_version || run_unit_tests)
			return true;

		int positional_args_begin = 1;
		if (source_dir == null && args.length > 1) {
			positional_source_dir = args[1];
			source_dir = positional_source_dir;
			positional_args_begin = 2;
		}

		bool deploy_option_seen = option_deploy_platform != null
			|| option_deploy_output_dir != null
			|| option_deploy_config != null
			|| option_deploy_app_title != null
			|| option_deploy_force
			|| option_deploy_arch != null
			|| option_deploy_app_id != null
			|| option_deploy_app_version_code != null
			|| option_deploy_app_version_name != null
			|| option_deploy_min_sdk_version != null
			|| option_deploy_target_sdk_version != null
			|| option_deploy_manifest != null
			|| option_deploy_keystore != null
			|| option_deploy_keystore_pass != null
			|| option_deploy_key_alias != null
			|| option_deploy_key_pass != null
			|| option_deploy_index_html != null
			;

		int num_commands = (int)do_init
			+ (int)do_import
			+ (int)do_deploy
			;

		if (num_commands > 1) {
			error = "Only one command can be specified.";
			return false;
		}

		if (!do_deploy && deploy_option_seen) {
			error = "--deploy must be specified.";
			return false;
		}

		if (!do_import && option_import_as != null) {
			error = "--import must be specified.";
			return false;
		}

		if (do_init) {
			if (option_source_dir == null) {
				error = "Source dir must be specified.";
				return false;
			}

			if (args.length > positional_args_begin) {
				error = "Too many positional arguments.";
				return false;
			}

			return true;
		}

		if (do_import) {
			if (source_dir == null) {
				error = "Source dir must be specified.";
				return false;
			}

			if (args.length - positional_args_begin < 2) {
				error = "Usage: --import <file>... <path>.";
				return false;
			}

			string[] filenames = {};
			for (int ii = positional_args_begin; ii < args.length - 1; ++ii)
				filenames += args[ii];

			import_filenames = filenames;
			import_destination = args[args.length - 1];

			if (import_destination == "") {
				error = "Import destination must be specified.";
				return false;
			}

			return true;
		}

		if (do_deploy) {
			if (source_dir == null) {
				error = "Source dir must be specified.";
				return false;
			}

			if (args.length > positional_args_begin) {
				error = "Too many positional arguments.";
				return false;
			}

			if (option_deploy_platform == null) {
				error = "Platform must be specified.";
				return false;
			}
			switch (option_deploy_platform) {
			case "android":
				deploy.platform = TargetPlatform.ANDROID;
				break;
			case "html5":
				deploy.platform = TargetPlatform.HTML5;
				break;
			case "linux":
				deploy.platform = TargetPlatform.LINUX;
				break;
			case "windows":
				deploy.platform = TargetPlatform.WINDOWS;
				break;
			default:
				error = "Unknown platform.";
				return false;
			}

			if (option_deploy_output_dir == null) {
				error = "Output dir must be specified.";
				return false;
			}
			deploy.output_dir = option_deploy_output_dir;

			if (option_deploy_config != null) {
				switch (option_deploy_config) {
				case "release":
					deploy.config = TargetConfig.RELEASE;
					break;
				case "development":
					deploy.config = TargetConfig.DEVELOPMENT;
					break;
				case "debug":
					deploy.config = TargetConfig.DEBUG;
					break;
				default:
					error = "Unknown config.";
					return false;
				}
			}

			if (option_deploy_app_title != null)
				deploy.app_title = option_deploy_app_title.strip();

			deploy.force = option_deploy_force;

			if (option_deploy_arch != null) {
				if (deploy.platform == TargetPlatform.ANDROID) {
					switch (option_deploy_arch) {
					case "arm":
						deploy.arch = TargetArch.ARM;
						break;
					case "arm64":
						deploy.arch = TargetArch.ARM64;
						break;
					default:
						error = "Unknown Android arch.";
						return false;
					}
				} else if (deploy.platform == TargetPlatform.HTML5) {
					switch (option_deploy_arch) {
					case "wasm":
						deploy.arch = TargetArch.WASM;
						break;
					case "wasm64":
						deploy.arch = TargetArch.WASM64;
						break;
					default:
						error = "Unknown HTML5 arch.";
						return false;
					}
				} else {
					error = "Architecture cannot be specified for this platform.";
					return false;
				}
			}

			if (option_deploy_app_id != null) {
				deploy.app_id = option_deploy_app_id.strip();
				if (deploy.app_id == "") {
					error = "App id is invalid.";
					return false;
				}
			}

			if (option_deploy_app_version_code != null) {
				int app_version_code;
				if (!int.try_parse(option_deploy_app_version_code, out app_version_code)) {
					error = "App version code is invalid.";
					return false;
				}
				deploy.app_version_code = app_version_code;
			}

			if (option_deploy_app_version_name != null) {
				deploy.app_version_name = option_deploy_app_version_name.strip();
				if (deploy.app_version_name == "") {
					error = "App version name is invalid.";
					return false;
				}
			}

			if (option_deploy_min_sdk_version != null) {
				int min_sdk_version;
				if (!int.try_parse(option_deploy_min_sdk_version, out min_sdk_version)) {
					error = "Min SDK version is invalid.";
					return false;
				}
				deploy.min_sdk_version = min_sdk_version;
			}

			if (option_deploy_target_sdk_version != null) {
				int target_sdk_version;
				if (!int.try_parse(option_deploy_target_sdk_version, out target_sdk_version)) {
					error = "Target SDK version is invalid.";
					return false;
				}
				deploy.target_sdk_version = target_sdk_version;
			}

			if (option_deploy_manifest != null)
				deploy.manifest = option_deploy_manifest;
			if (option_deploy_keystore != null)
				deploy.keystore = option_deploy_keystore;
			if (option_deploy_keystore_pass != null)
				deploy.keystore_pass = option_deploy_keystore_pass;
			if (option_deploy_key_alias != null)
				deploy.key_alias = option_deploy_key_alias;
			if (option_deploy_key_pass != null)
				deploy.key_pass = option_deploy_key_pass;
			if (option_deploy_index_html != null)
				deploy.index_html = option_deploy_index_html;

			return true;
		}

		if (option_source_dir != null && args.length > 1) {
			if (GLib.FileUtils.test(args[1], FileTest.EXISTS)
				&& GLib.FileUtils.test(args[1], FileTest.IS_DIR)
				) {
				error = "Source dir specified twice.";
				return false;
			}

			level_resource = args[1];
			positional_args_begin = 2;
		} else if (args.length > positional_args_begin) {
			level_resource = args[positional_args_begin];
			++positional_args_begin;
		}

		if (args.length > positional_args_begin) {
			error = "Too many positional arguments.";
			return false;
		}

		return true;
	}
}

public static int main(string[] args)
{
	// If args does not contain --child, spawn the launcher.
	int ii;
	for (ii = 0; ii < args.length; ++ii) {
		if (args[ii] == "--launcher") {
			break;
		}
	}

	if (ii == args.length) {
		_log_prefix = "editor";
	} else {
		_log_prefix = "launcher";

		// Remove --child from args for backward compatibility.
		if (args.length > 1)
			args = args[0 : args.length - 1];
	}

	CommandLineOptions command_line_options = CommandLineOptions();
	if (_log_prefix == "editor") {
		string[] parse_args = {};
		for (ii = 0; ii < args.length; ++ii)
			parse_args += args[ii];

		string error;
		if (!command_line_options.parse(ref parse_args, out error)) {
			stderr.printf("crown-editor: %s\n", error);
			stderr.printf("Try 'crown-editor --help' for more information.\n");
			return 1;
		}

		if (command_line_options.show_help) {
			command_line_options.print_help(stdout);
			return 0;
		}

		if (command_line_options.show_version) {
			stdout.printf("%s %s\n", CROWN_EDITOR_NAME, CROWN_VERSION);
			return 0;
		}

		if (command_line_options.run_unit_tests)
			return main_unit_tests();
	}

	// Redirect GLib logs to internal log*().
	GLib.set_print_handler((msg) => { logi(msg); });
	GLib.set_printerr_handler((msg) => { loge(msg); });

	// code-format off
	GLib.Log.set_writer_func((log_level, fields) => {
			foreach (var field in fields) {
				if (field.key == "MESSAGE") {
					switch (log_level) {
					case LEVEL_DEBUG:
#if CROWN_DEBUG
						logi((string)field.value);
#endif
						break;

					case LEVEL_INFO:
					case LEVEL_MESSAGE:
						logi((string)field.value);
						break;

					case LEVEL_CRITICAL:
					case LEVEL_WARNING:
						logw((string)field.value);
						break;

					case LEVEL_ERROR:
						loge((string)field.value);
						break;

					default:
						logw((string)field.value);
						break;
					}

					return GLib.LogWriterOutput.HANDLED;
				}
			}

			return GLib.LogWriterOutput.UNHANDLED;
		});
	// code-format on

	// Global paths
	_data_dir = GLib.File.new_for_path(GLib.Path.build_filename(GLib.Environment.get_user_data_dir(), "crown"));
	try {
		_data_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}
	_config_dir = GLib.File.new_for_path(GLib.Path.build_filename(GLib.Environment.get_user_config_dir(), "crown"));
	try {
		_config_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}
	_cache_dir = GLib.File.new_for_path(GLib.Path.build_filename(GLib.Environment.get_user_cache_dir(), "crown"));
	try {
		_cache_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}
	_logs_dir = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "logs"));
	try {
		_logs_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}
	_documents_dir = GLib.File.new_for_path(GLib.Environment.get_user_special_dir(GLib.UserDirectory.DOCUMENTS));

	_thumbnails_dir = GLib.File.new_for_path(GLib.Path.build_filename(GLib.Environment.get_user_cache_dir(), "thumbnails"));
	try {
		_thumbnails_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}

	_thumbnails_normal_dir = GLib.File.new_for_path(GLib.Path.build_filename(_thumbnails_dir.get_path(), "normal"));
	try {
		_thumbnails_normal_dir.make_directory();
	} catch (Error e) {
		/* Nobody cares */
	}

	_log_file = GLib.File.new_for_path(GLib.Path.build_filename(_logs_dir.get_path(), new GLib.DateTime.now_local().format("%Y-%m-%d") + ".log"));
	_log_stream = GLib.FileStream.open(_log_file.get_path(), "a");

	if (_log_prefix == "launcher")
		return launcher_main(args);

	// Spawn launcher process.
	try {
		string[] child_args = args;
		child_args += "--launcher";
#if CROWN_PLATFORM_WINDOWS
		child_args += ((uint64)OpenProcess(0x00100000 /* SYNCHRONIZE */, true, GetCurrentProcessId())).to_string();
#elif CROWN_PLATFORM_LINUX
		child_args += ((uint64)getpid()).to_string();
#endif
		GLib.Pid launcher_pid;

		GLib.Process.spawn_async(null
			, child_args
			, null
			, 0
			, null
			, out launcher_pid
			);
	} catch (GLib.SpawnError e) {
		loge("%s".printf(e.message));
		return 1;
	}

	_settings_file = GLib.File.new_for_path(GLib.Path.build_filename(_config_dir.get_path(), "settings.sjson"));
	_window_state_file = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "window.sjson"));
	_user_file = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "user.sjson"));
	_console_history_file = GLib.File.new_for_path(GLib.Path.build_filename(_data_dir.get_path(), "console_history.txt"));

	// Find toolchain path, more desirable paths come first.
	ii = 0;
	string toolchain_paths[] =
	{
		"../../..",         // Relative path in release package.
		"../../../samples", // Relative path in git worktree.
		".",
	};
	for (ii = 0; ii < toolchain_paths.length; ++ii) {
		string path = Path.build_filename(toolchain_paths[ii], "core");
		if (GLib.FileUtils.test(path, FileTest.EXISTS) && GLib.FileUtils.test(path, FileTest.IS_DIR)) {
			_toolchain_dir = File.new_for_path(path).get_parent();
			break;
		}
	}
	if (ii == toolchain_paths.length) {
		loge("Unable to find the toolchain directory");
		return 1;
	}

	Project project = new Project();
	project.set_toolchain_dir(_toolchain_dir.get_path());
	project.register_importer("sprite", { "png" }, SpriteResource.import, 2.1);
	project.register_importer("mesh", { "mesh", "fbx", "gltf", "glb", "obj" }, MeshResource.import, 1.0);
	project.register_importer("sound", { "wav", "ogg", "mp3" }, SoundResource.import, 2.0);
	project.register_importer("texture", { "dds", "exr", "jpg", "ktx", "png", "pvr", "tga", }, TextureResource.import, 2.0);
	project.register_importer("font", { "ttf", "otf" }, FontResource.import, 3.0);

	Database database = new Database(project);

	if (command_line_options.do_init) {
		string error;
		if (Project.create(out error, command_line_options.source_dir, "", false) != 0) {
			stderr.printf("crown-editor: %s\n", error);
			return 1;
		}

		return 0;
	}

	if (command_line_options.do_import) {
		if (project.load(command_line_options.source_dir) != 0) {
			stderr.printf("crown-editor: Unable to load project: %s.\n", command_line_options.source_dir);
			return 1;
		}

		bool import_failed = false;
		int num_import_results = 0;
		StringId32? importer_name = null;
		if (command_line_options.import_as != null)
			importer_name = StringId32(command_line_options.import_as);

		project.import(command_line_options.import_destination
			, command_line_options.import_filenames
			, (result, primary_resource_path) => {
				++num_import_results;
				if (result != ImportResult.SUCCESS)
					import_failed = true;
			}
			, database
			, null
			, importer_name
			);

		if (num_import_results == 0) {
			stderr.printf("crown-editor: Import did not produce a result.\n");
			return 1;
		}

		if (import_failed) {
			stderr.printf("crown-editor: Failed to import resource(s).\n");
			return 1;
		}

		return 0;
	}

	if (command_line_options.do_deploy) {
		if (project.load(command_line_options.source_dir) != 0) {
			stderr.printf("crown-editor: Unable to load project: %s.\n", command_line_options.source_dir);
			return 1;
		}

		DeployPackage package = new DeployPackage(project, command_line_options.deploy);
		string deploy_error = "";
		DeployPackageFlags deploy_flags = command_line_options.deploy.force
			? DeployPackageFlags.FORCE
			: DeployPackageFlags.NONE
			;

		int deploy_status = -1;
		bool deploy_started = false;
		bool deploy_finished = false;
		bool deploy_error_reported = false;
		GLib.MainLoop deploy_loop = new GLib.MainLoop(null, false);

		uint timeout_id = 0;
		timeout_id = GLib.Timeout.add(5000, () => {
				timeout_id = 0;
				if (!deploy_started) {
					stderr.printf("crown-editor: Unable to connect to subprocess launcher.\n");
					deploy_error_reported = true;
					deploy_loop.quit();
				}
				return GLib.Source.REMOVE;
			});

		uint watch_id = GLib.Bus.watch_name(GLib.BusType.SESSION
			, CROWN_SUBPROCESS_LAUNCHER
			, GLib.BusNameWatcherFlags.NONE
			, (connection, name) => {
				GLib.Error? error = null;
				if (!connect_subprocess_launcher(out error)) {
					if (error != null)
						stderr.printf("crown-editor: %s\n", error.message);
					deploy_error_reported = true;
					deploy_loop.quit();
					return;
				}

				deploy_started = true;
				if (timeout_id != 0) {
					GLib.Source.remove(timeout_id);
					timeout_id = 0;
				}

				// code-format off
				package.create.begin(deploy_flags, (obj, res) => {
						DeployResult result = package.create.end(res);
						deploy_status = result.status;
						deploy_error = result.error;
						deploy_finished = true;
						deploy_loop.quit();
					});
				// code-format on
			}
			, (connection, name) => {
				if (deploy_started && !deploy_finished) {
					stderr.printf("crown-editor: Subprocess launcher disconnected.\n");
					deploy_error_reported = true;
					deploy_loop.quit();
				}
			}
			);

		deploy_loop.run();
		GLib.Bus.unwatch_name(watch_id);
		if (timeout_id != 0)
			GLib.Source.remove(timeout_id);

		if (deploy_status != 0) {
			if (!deploy_error_reported)
				stderr.printf("crown-editor: %s\n", deploy_error.length > 0 ? deploy_error : "Failed to deploy project.");
			return 1;
		}

		return 0;
	}

	// Find templates path, more desirable paths come first.
	string templates_path[] =
	{
		"../../..", // Relative path in release package or git worktree.
		".",
	};
	for (ii = 0; ii < templates_path.length; ++ii) {
		string path = Path.build_filename(templates_path[ii], "samples");
		if (GLib.FileUtils.test(path, FileTest.EXISTS) && GLib.FileUtils.test(path, FileTest.IS_DIR)) {
			_templates_dir = File.new_for_path(path);
			break;
		}
	}
	if (ii == templates_path.length) {
		loge("Unable to find the templates directory");
		return 1;
	}

	// Find gettext catalog path, more desirable paths come first.
	string po_paths[] =
	{
		"../../..", // Relative path in release package.
		".",       // Relative path in build dir.
	};
	for (ii = 0; ii < po_paths.length; ++ii) {
		string path = Path.build_filename(po_paths[ii], "po");
		if (GLib.FileUtils.test(path, FileTest.EXISTS) && GLib.FileUtils.test(path, FileTest.IS_DIR)) {
			_po_dir = File.new_for_path(path);
			break;
		}
	}
	if (ii == po_paths.length)
		_po_dir = File.new_for_path(".");

#if CROWN_PLATFORM_LINUX
	Gdk.set_allowed_backends("x11");
#endif

	// Use fontconfig backend.
	Pango.FontMap fontmap = Pango.CairoFontMap.new_for_font_type(Cairo.FontType.FT);
	Pango.CairoFontMap.set_default((Pango.CairoFontMap)fontmap);

	LevelEditorApplication app = new LevelEditorApplication(command_line_options, project, database);
	return app.run({ args[0] });
}

} /* namespace Crown */
