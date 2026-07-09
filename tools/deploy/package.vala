/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public string deploy_config_name(TargetConfig config)
{
	switch (config) {
	case TargetConfig.RELEASE:
		return "release";
	case TargetConfig.DEVELOPMENT:
		return "development";
	case TargetConfig.DEBUG:
		return "debug";
	default:
		return "unknown";
	}
}

public string deploy_arch_name(TargetArch arch)
{
	switch (arch) {
	case TargetArch.X86:
		return "x86";
	case TargetArch.X64:
		return "x64";
	case TargetArch.ARM:
		return "arm";
	case TargetArch.ARM64:
		return "arm64";
	case TargetArch.WASM:
		return "wasm";
	default:
		return "unknown";
	}
}

public GLib.File deploy_package_dir(out string config_path, string output_path, string app_identifier, TargetPlatform platform, TargetArch arch, TargetConfig config)
{
	string platform_path = Path.build_path(Path.DIR_SEPARATOR_S, output_path, platform.to_key(), deploy_arch_name(arch));
	config_path = Path.build_path(Path.DIR_SEPARATOR_S, platform_path, deploy_config_name(config));
	return GLib.File.new_for_path(Path.build_path(Path.DIR_SEPARATOR_S, config_path, app_identifier));
}

public class DeployPackage
{
	public Project _project;
	public DeployOptions _options;
	public string _app_title;
	public string _app_identifier;
	public string _exe_name;
	public TargetArch _arch;
	public string _config_path;
	public string _result_path;
	public GLib.File _package_dir;

	public DeployPackage(Project project, DeployOptions options)
	{
		_project = project;
		_options = options;
		_app_title = options.app_title != ""
			? options.app_title
			: project.name()
			;
		_exe_name = _app_title.replace(" ", "_").down();
		_app_identifier = _exe_name;
		_arch = TargetArch.X64;

		if (options.platform == TargetPlatform.ANDROID) {
			_app_identifier = options.app_id + "-" + options.app_version_name;
			_arch = options.arch;
		} else if (options.platform == TargetPlatform.HTML5) {
			_arch = TargetArch.WASM;
		}

		_package_dir = deploy_package_dir(out _config_path
			, (string)options.output_dir
			, _app_identifier
			, options.platform
			, _arch
			, options.config
			);
		_result_path = options.platform == TargetPlatform.ANDROID
			? _config_path
			: _package_dir.get_path()
			;
	}

	public async int create(bool overwrite)
	{
		try {
			if (_package_dir.query_exists()) {
				if (!overwrite) {
					loge("Package directory already exists");
					return -1;
				}

				_project.delete_tree(_package_dir);
			}

			_package_dir.make_directory_with_parents();
		} catch (Error e) {
			loge(e.message);
			return -1;
		}

		switch (_options.platform) {
		case TargetPlatform.ANDROID: {
			AndroidDeployer deployer = new AndroidDeployer();
			return yield deployer.create_package(_project
					, _package_dir
					, _config_path
					, _options.config
					, _app_title
					, _options.app_id
					, _options.app_version_code
					, _options.app_version_name
					, _options.min_sdk_version
					, _options.target_sdk_version
					, _options.keystore
					, _options.keystore_pass
					, _options.key_alias
					, _options.key_pass
					, _arch
					, _options.manifest
					, _app_identifier
					);
		}
		case TargetPlatform.HTML5: {
			HTML5Deployer deployer = new HTML5Deployer();
			return yield deployer.create_package(_project
					, _package_dir
					, _options.config
					, _app_title
					, _options.index_html
					, _exe_name
					);
		}
		case TargetPlatform.LINUX: {
			LinuxDeployer deployer = new LinuxDeployer();
			return yield deployer.create_package(_project
					, _package_dir
					, _options.config
					, _app_title
					, _exe_name
					);
		}
		case TargetPlatform.WINDOWS: {
			WindowsDeployer deployer = new WindowsDeployer();
			return yield deployer.create_package(_project
					, _package_dir
					, _options.config
					, _app_title
					, _exe_name
					);
		}
		default:
			loge("Unknown platform");
			return -1;
		}
	}
}

} /* namespace Crown */
