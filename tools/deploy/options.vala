/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum TargetConfig
{
	RELEASE,
	DEVELOPMENT,
	DEBUG,

	COUNT;

	public string to_key()
	{
		switch (this) {
		case RELEASE:
			return "release";
		case DEVELOPMENT:
			return "development";
		case DEBUG:
			return "debug";
		default:
			return "unknown";
		}
	}

	public string to_label()
	{
		switch (this) {
		case RELEASE:
			return _("Release");
		case DEVELOPMENT:
			return _("Development");
		case DEBUG:
			return _("Debug");
		default:
			return _("unknown");
		}
	}
}

public enum TargetArch
{
	X86,
	X64,
	ARM,
	ARM64,
	WASM
}

public struct DeployOptions
{
	public TargetPlatform platform;
	public string? output_dir;
	public TargetConfig config;
	public string app_title;
	public bool force;
	public TargetArch arch;
	public string app_id;
	public int app_version_code;
	public string app_version_name;
	public int min_sdk_version;
	public int target_sdk_version;
	public string manifest;
	public string keystore;
	public string keystore_pass;
	public string key_alias;
	public string key_pass;
	public string index_html;

	public DeployOptions()
	{
		platform = TargetPlatform.COUNT;
		output_dir = null;
		config = TargetConfig.RELEASE;
		app_title = "";
		force = false;
		arch = TargetArch.ARM64;
		app_id = "org.crownengine.game";
		app_version_code = 1;
		app_version_name = "1.0";
		min_sdk_version = 24;
		target_sdk_version = 34;
		manifest = "";
		keystore = "";
		keystore_pass = "android";
		key_alias = "androiddebugkey";
		key_pass = "android";
		index_html = "";
	}
}

} /* namespace Crown */
