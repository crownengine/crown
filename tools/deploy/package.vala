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

} /* namespace Crown */
