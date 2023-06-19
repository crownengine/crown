/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;
using Gee;

namespace Crown
{
public ComboBoxMap make_deploy_config_combo()
{
	return new ComboBoxMap(0
		, new string?[] { "Release", "Development" }
		, new string?[] { ((int)TargetConfig.RELEASE).to_string(), ((int)TargetConfig.DEVELOPMENT).to_string() }
		);
}

public Gtk.Button make_deploy_button(string platform_name)
{
	var btn = new Gtk.Button();
	btn.label = "Package Project for %s".printf(platform_name);
	btn.margin_start = 12;
	btn.margin_end = 12;
	btn.margin_top = 12;
	btn.get_style_context().add_class(Gtk.STYLE_CLASS_SUGGESTED_ACTION);
	return btn;
}

public class DeployDialog : Gtk.Dialog
{
	public RuntimeInstance _editor;

	// Android page.
	public Gtk.Button _android_deploy_button;
	public Gtk.FileChooserButton _android_output_path;
	public ComboBoxMap _android_config;
	public CheckBox _android_armv7;
	public CheckBox _android_armv8;
	public Gtk.FileChooserButton _android_keystore;
	public Gtk.Entry _android_keystore_password;
	public Gtk.Entry _android_key_alias;
	public Gtk.Entry _android_key_password;
	public Gtk.Entry _android_app_title;
	public Gtk.Entry _android_app_identifier;
	public Gtk.Entry _android_app_version_code;
	public Gtk.Entry _android_app_version_name;
	public PropertyGridSet _android_set;
	public Gtk.Box _android_box;

	// Linux page.
	public Gtk.Button _linux_deploy_button;
	public Gtk.FileChooserButton _linux_output_path;
	public ComboBoxMap _linux_config;
	public Gtk.Entry _linux_app_title;
	public PropertyGridSet _linux_set;
	public Gtk.Box _linux_box;

	// Windows page.
	public Gtk.Button _windows_deploy_button;
	public Gtk.FileChooserButton _windows_output_path;
	public ComboBoxMap _windows_config;
	public EntryText _windows_app_title;
	public PropertyGridSet _windows_set;
	public Gtk.Box _windows_box;

	public Gtk.Notebook _notebook;

	public DeployDialog(RuntimeInstance editor)
	{
		this.title = "Deploy";
		this.border_width = 0;
		this.set_icon_name(CROWN_ICON_NAME);

		_editor = editor;

		// Widgets
		_android_set = new PropertyGridSet();
		_android_set.border_width = 12;
		_linux_set = new PropertyGridSet();
		_linux_set.border_width = 12;
		_windows_set = new PropertyGridSet();
		_windows_set.border_width = 12;

		// Android page.
		_android_deploy_button = make_deploy_button("Android");
		_android_deploy_button.clicked.connect(() => {
				// Validate input fields.
				string? output_path = _android_output_path.get_filename();
				if (output_path == null) {
					loge("Select a valid output Destination");
					return;
				}

				string app_title = _android_app_title.text.strip();
				if (app_title.length == 0) {
					loge("Enter a valid App Title");
					return;
				}

				string app_identifier = _android_app_identifier.text.strip();
				if (app_title.length == 0 || app_identifier.split(".").length != 3) {
					loge("Enter a valid App Identifier");
					return;
				}

				int app_version_code;
				if (int.try_parse(_android_app_version_code.text, out app_version_code) == false) {
					loge("Enter a valid App Version Code");
					return;
				}

				string app_version_name = _android_app_version_name.text.strip();
				if (app_version_name.length == 0) {
					loge("Enter a valid App Version Name");
					return;
				}

				string? keystore_path = _android_keystore.get_filename();
				if (keystore_path == null) {
					loge("Enter a valid Keystore file");
					return;
				}

				string keystore_pass = _android_keystore_password.text;
				if (keystore_path.length == 0) {
					loge("Enter a valid Keystore Password");
					return;
				}

				string key_alias = _android_key_alias.text;
				if (key_alias.length == 0) {
					loge("Enter a valid Key Alias");
					return;
				}

				string key_pass = _android_key_password.text;
				if (key_pass.length == 0) {
					loge("Enter a valid Key Password");
					return;
				}

				TargetArch[] archs =
				{
					TargetArch.ARM,
					TargetArch.ARM64
				};

				for (int ii = 0; ii < archs.length; ++ii) {
					if (archs[ii] == TargetArch.ARM && !_android_armv7.value)
						continue;
					if (archs[ii] == TargetArch.ARM64 && !_android_armv8.value)
						continue;

					// Create the package.
					GLib.Variant paramz[] =
					{
						(string)output_path,
						int.parse(_android_config.get_active_id()),
						app_title,
						app_identifier,
						app_version_code,
						app_version_name,
						keystore_path,
						keystore_pass,
						key_alias,
						key_pass,
						archs[ii]
					};

					GLib.Application.get_default().activate_action("create-package-android"
						, new GLib.Variant.tuple(paramz));
				}
			});

		_android_output_path = new Gtk.FileChooserButton("Select folder", Gtk.FileChooserAction.SELECT_FOLDER);
		_android_config = make_deploy_config_combo();
		_android_armv7 = new CheckBox();
		_android_armv7.value = false;
		_android_armv8 = new CheckBox();
		_android_armv8.value = true;
		_android_keystore = new Gtk.FileChooserButton("Select file", Gtk.FileChooserAction.OPEN);
		_android_keystore_password = new Gtk.Entry();
		_android_keystore_password.set_visibility(false);
		_android_keystore_password.input_purpose = Gtk.InputPurpose.PASSWORD;
		_android_key_alias = new Gtk.Entry();
		_android_key_password = new Gtk.Entry();
		_android_key_password.set_visibility(false);
		_android_key_password.input_purpose = Gtk.InputPurpose.PASSWORD;
		_android_app_title = new EntryText();
		_android_app_title.placeholder_text = "My Application";
		_android_app_identifier = new Gtk.Entry();
		_android_app_identifier.placeholder_text = "org.company.product";
		_android_app_version_code = new Gtk.Entry();
		_android_app_version_code.input_purpose = Gtk.InputPurpose.DIGITS;
		_android_app_version_code.placeholder_text = "1";
		_android_app_version_name = new Gtk.Entry();
		_android_app_version_name.placeholder_text = "1.0";

		// Android Output grid.
		PropertyGrid cv;
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Destination", _android_output_path);
		cv.add_row("Config", _android_config);
		cv.add_row("ARMv7-A", _android_armv7);
		cv.add_row("ARMv8-A", _android_armv8);
		_android_set.add_property_grid(cv, "Output");

		// Android Application.
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Title", _android_app_title);
		cv.add_row("Identifier", _android_app_identifier);
		cv.add_row("Version Code", _android_app_version_code);
		cv.add_row("Version Name", _android_app_version_name);
		_android_set.add_property_grid(cv, "Application");

		// Android Signing.
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Keystore", _android_keystore);
		cv.add_row("Keystore password", _android_keystore_password);
		cv.add_row("Alias", _android_key_alias);
		cv.add_row("Key password", _android_key_password);
		_android_set.add_property_grid(cv, "Signing");

		// Android box.
		_android_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_android_box.pack_start(_android_deploy_button, false, true, 0);
		_android_box.pack_start(_android_set, false, true, 0);

		// Linux page.
		_linux_deploy_button = make_deploy_button("Linux");
		_linux_deploy_button.clicked.connect(() => {
				// Validate input fields.
				string? output_path = _linux_output_path.get_filename();
				if (output_path == null) {
					loge("Select a valid output Destination");
					return;
				}

				string app_title = _linux_app_title.text.strip();
				if (app_title.length == 0) {
					loge("Enter a valid Title");
					return;
				}

				// Create the package.
				GLib.Variant paramz[] =
				{
					(string)output_path,
					int.parse(_linux_config.get_active_id()),
					app_title
				};

				GLib.Application.get_default().activate_action("create-package-linux"
					, new GLib.Variant.tuple(paramz));
			});

		_linux_output_path = new Gtk.FileChooserButton("Select folder", Gtk.FileChooserAction.SELECT_FOLDER);
		_linux_config = make_deploy_config_combo();
		_linux_app_title = new EntryText();
		_linux_app_title.placeholder_text = "My Application";

		// Linux box.
		_linux_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_linux_box.pack_start(_linux_deploy_button, false, true, 0);
		_linux_box.pack_start(_linux_set, false, true, 0);

		// Linux General page.
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Destination", _linux_output_path);
		cv.add_row("Config", _linux_config);
		_linux_set.add_property_grid(cv, "Output");

		// Linux Application.
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Title", _linux_app_title);
		_linux_set.add_property_grid(cv, "Application");

		// Windows page.
		_windows_deploy_button = make_deploy_button("Windows");
		_windows_deploy_button.clicked.connect(() => {
				// Validate input fields.
				string? output_path = _windows_output_path.get_filename();
				if (output_path == null) {
					loge("Select a valid output Destination");
					return;
				}

				string app_title = _windows_app_title.text.strip();
				if (app_title.length == 0) {
					loge("Enter a valid Title");
					return;
				}

				// Create the package.
				GLib.Variant paramz[] =
				{
					(string)output_path,
					int.parse(_windows_config.get_active_id()),
					app_title
				};

				GLib.Application.get_default().activate_action("create-package-windows"
					, new GLib.Variant.tuple(paramz));
			});

		_windows_output_path = new Gtk.FileChooserButton("Select folder", Gtk.FileChooserAction.SELECT_FOLDER);
		_windows_config = make_deploy_config_combo();
		_windows_app_title = new EntryText();
		_windows_app_title.placeholder_text = "My Application";

		// Windows box.
		_windows_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_windows_box.pack_start(_windows_deploy_button, false, true, 0);
		_windows_box.pack_start(_windows_set, false, true, 0);

		// Windows General page.
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Destination", _windows_output_path);
		cv.add_row("Config", _windows_config);
		_windows_set.add_property_grid(cv, "Output");

		// Windows Application.
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Title", _windows_app_title);
		_windows_set.add_property_grid(cv, "Application");

		// Add pages.
		_notebook = new Gtk.Notebook();
		_notebook.append_page(_android_box, new Gtk.Label("Android"));
#if CROWN_PLATFORM_LINUX
		_notebook.append_page(_linux_box, new Gtk.Label("Linux"));
#elif CROWN_PLATFORM_WINDOWS
		_notebook.append_page(_windows_box, new Gtk.Label("Windows"));
#endif
		_notebook.vexpand = true;
		_notebook.show_border = false;

		this.get_content_area().border_width = 0;
		this.get_content_area().add(_notebook);
	}
}

} /* namespace Crown */

