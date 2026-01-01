/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public InputEnum make_deploy_config_combo()
{
	string? labels[] =
	{
		TargetConfig.RELEASE.to_label(),
		TargetConfig.DEVELOPMENT.to_label(),
#if CROWN_DEBUG
		TargetConfig.DEBUG.to_label(),
#endif
	};

	string? ids[] =
	{
		((int)TargetConfig.RELEASE).to_string(),
		((int)TargetConfig.DEVELOPMENT).to_string(),
#if CROWN_DEBUG
		((int)TargetConfig.DEBUG).to_string(),
#endif
	};

	return new InputEnum(ids[0], labels, ids);
}

public Gtk.Button make_deploy_button(TargetPlatform platform)
{
	var btn = new Gtk.Button();
	btn.label = "Package Project for %s".printf(platform.to_label());
	btn.margin_start = 12;
	btn.margin_end = 12;
	btn.margin_top = 12;
	btn.get_style_context().add_class("suggested-action");
	return btn;
}

public delegate int DeployerCheckConfig();

public class DeployerPage : Gtk.Box
{
	public Gtk.Stack _stack;
	public Gtk.Box _check_config_box;
	public Gtk.Widget _deployer_options;
	public unowned DeployerCheckConfig _check_config;

	public DeployerPage(TargetPlatform target_platform, Gtk.Widget deployer_options, DeployerCheckConfig? check_config = null)
	{
		Object(orientation: Gtk.Orientation.VERTICAL);
		_stack = new Gtk.Stack();
		_deployer_options = deployer_options;
		_check_config = check_config;

		string h1 = "<span font_weight=\"bold\" size=\"x-large\">Not configured</span>";
		string p1 = "The %s deployer has not been configured yet".printf(target_platform.to_label());
		string p2 = "Follow <a href=\"\">the instructions</a> to get started";

		var h1l = new Gtk.Label(null);
		h1l.set_markup(h1);
		h1l.valign = Gtk.Align.CENTER;

		var p1l = new Gtk.Label(null);
		p1l.set_markup(p1);
		p1l.valign = Gtk.Align.CENTER;

		var p2l = new Gtk.Label(null);
		p2l.get_style_context().add_class("colorfast-link");
		p2l.set_markup(p2);
		p2l.valign = Gtk.Align.CENTER;
		p2l.activate_link.connect(() => {
				try {
					string CROWN_DEPLOY_URL = CROWN_LATEST_DOCS_URL + "/deploying";
					string CROWN_DEPLOY_CONFIG_URL = CROWN_DEPLOY_URL + "/%s.html".printf(target_platform.to_key());
					AppInfo.launch_default_for_uri(CROWN_DEPLOY_CONFIG_URL, null);
				} catch (Error e) {
					loge(e.message);
				}
				return true;
			});
		_check_config_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 12);
		_check_config_box.valign = Gtk.Align.CENTER;
		_check_config_box.pack_start(h1l);
		_check_config_box.pack_start(p1l);
		_check_config_box.pack_start(p2l);

		_stack.add(_check_config_box);
		_stack.add(_deployer_options);

		this.pack_start(_stack);

		this.map.connect(on_map);
	}

	public void on_map()
	{
		if (_check_config != null) {
			if (_check_config() != 0)
				_stack.set_visible_child(_check_config_box);
			else
				_stack.set_visible_child(_deployer_options);
		} else {
				_stack.set_visible_child(_deployer_options);
		}
	}
}

#if CROWN_PLATFORM_WINDOWS
public bool can_build_32bit_arm = false;
#else
public bool can_build_32bit_arm = true;
#endif

public class DeployDialog : Gtk.Window
{
	public RuntimeInstance _editor;
	public Project _project;

	// Android page.
	public Gtk.Button _android_deploy_button;
	public InputFile _android_output_path;
	public InputEnum _android_config;
	public InputBool _android_armv7;
	public InputBool _android_armv8;
	public InputBool _android_use_debug_keystore;
	public InputFile _android_keystore;
	public Gtk.Entry _android_keystore_password;
	public Gtk.Entry _android_key_alias;
	public Gtk.Entry _android_key_password;
	public Gtk.Entry _android_app_title;
	public Gtk.Entry _android_app_identifier;
	public Gtk.Entry _android_app_version_code;
	public Gtk.Entry _android_app_version_name;
	public PropertyGridSet _android_set;
	public Gtk.Box _android_box;
	public AndroidDeployer _android;
	public DeployerPage _android_page;

	// HTML5 page.
	public Gtk.Button _html5_deploy_button;
	public InputFile _html5_output_path;
	public InputEnum _html5_config;
	public Gtk.Entry _html5_app_title;
	public PropertyGridSet _html5_set;
	public Gtk.Box _html5_box;
	public HTML5Deployer _html5;
	public DeployerPage _html5_page;

	// Linux page.
	public Gtk.Button _linux_deploy_button;
	public InputFile _linux_output_path;
	public InputEnum _linux_config;
	public Gtk.Entry _linux_app_title;
	public PropertyGridSet _linux_set;
	public Gtk.Box _linux_box;
	public DeployerPage _linux_page;

	// Windows page.
	public Gtk.Button _windows_deploy_button;
	public InputFile _windows_output_path;
	public InputEnum _windows_config;
	public InputString _windows_app_title;
	public PropertyGridSet _windows_set;
	public Gtk.Box _windows_box;
	public DeployerPage _windows_page;

	public Gtk.Notebook _notebook;
	public Gtk.EventControllerKey _controller_key;

	public DeployDialog(Project project, RuntimeInstance editor)
	{
		this.title = "Deploy";
		this.set_icon_name(CROWN_EDITOR_ICON_NAME);

		_project = project;
		_editor = editor;

		// Android page.
		_android_deploy_button = make_deploy_button(TargetPlatform.ANDROID);
		_android_deploy_button.clicked.connect(() => {
				// Validate input fields.
				string? output_path = _android_output_path.value;
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

				string? keystore_path = _android_use_debug_keystore.get_active()
					? GLib.Path.build_filename(GLib.Environment.get_home_dir(), ".android", "debug.keystore")
					: _android_keystore.value
					;
				if (keystore_path == null) {
					loge("Enter a valid Keystore file");
					return;
				}

				string keystore_pass = _android_use_debug_keystore.get_active()
					? "android"
					: _android_keystore_password.text
					;
				if (keystore_path.length == 0) {
					loge("Enter a valid Keystore Password");
					return;
				}

				string key_alias = _android_use_debug_keystore.get_active()
					? "androiddebugkey"
					: _android_key_alias.text
					;
				if (key_alias.length == 0) {
					loge("Enter a valid Key Alias");
					return;
				}

				string key_pass = _android_use_debug_keystore.get_active()
					? "android"
					: _android_key_password.text
					;
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

		_android_output_path = new InputFile(Gtk.FileChooserAction.SELECT_FOLDER);
		_android_config = make_deploy_config_combo();
		_android_armv7 = new InputBool();
		_android_armv7.value = false;
		_android_armv8 = new InputBool();
		_android_armv8.value = true;
		_android_use_debug_keystore = new InputBool();
		_android_use_debug_keystore.value_changed.connect(() => { android_set_debug_keystore(); });
		_android_use_debug_keystore.value = true;
		_android_keystore = new InputFile(Gtk.FileChooserAction.OPEN);
		_android_keystore_password = new Gtk.Entry();
		_android_keystore_password.set_visibility(false);
		_android_keystore_password.input_purpose = Gtk.InputPurpose.PASSWORD;
		_android_key_alias = new Gtk.Entry();
		_android_key_password = new Gtk.Entry();
		_android_key_password.set_visibility(false);
		_android_key_password.input_purpose = Gtk.InputPurpose.PASSWORD;
		_android_app_title = new InputString();
		_android_app_title.placeholder_text = "My Application";
		_android_app_title.text = _project.name();
		_android_app_identifier = new Gtk.Entry();
		_android_app_identifier.placeholder_text = "org.company.product";
		_android_app_version_code = new Gtk.Entry();
		_android_app_version_code.input_purpose = Gtk.InputPurpose.DIGITS;
		_android_app_version_code.placeholder_text = "1";
		_android_app_version_name = new Gtk.Entry();
		_android_app_version_name.placeholder_text = "1.0";

		android_set_debug_keystore();

		_android_set = new PropertyGridSet();

		// Android Output grid.
		PropertyGrid cv;
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Destination", _android_output_path);
		cv.add_row("Config", _android_config);
		cv.add_row("ARMv7-A", _android_armv7).sensitive = can_build_32bit_arm;
		_android_armv7.sensitive = can_build_32bit_arm;
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
		cv.add_row("Use debug keystore", _android_use_debug_keystore);
		cv.add_row("Keystore", _android_keystore);
		cv.add_row("Keystore password", _android_keystore_password);
		cv.add_row("Alias", _android_key_alias);
		cv.add_row("Key password", _android_key_password);
		_android_set.add_property_grid(cv, "Signing");

		// Android box.
		_android_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_android_box.pack_start(_android_deploy_button, false, true, 0);
		_android_box.pack_start(_android_set, false, true, 0);
		_android = new AndroidDeployer();
		_android_page = new DeployerPage(TargetPlatform.ANDROID, _android_box, _android.check_config);

		// HTML5 page.
		_html5_deploy_button = make_deploy_button(TargetPlatform.HTML5);
		_html5_deploy_button.clicked.connect(() => {
				// Validate input fields.
				string? output_path = _html5_output_path.value;
				if (output_path == null) {
					loge("Select a valid output Destination");
					return;
				}

				string app_title = _html5_app_title.text.strip();
				if (app_title.length == 0) {
					loge("Enter a valid Title");
					return;
				}

				// Create the package.
				GLib.Variant paramz[] =
				{
					(string)output_path,
					int.parse(_html5_config.get_active_id()),
					app_title
				};

				GLib.Application.get_default().activate_action("create-package-html5"
					, new GLib.Variant.tuple(paramz));
			});

		_html5_output_path = new InputFile(Gtk.FileChooserAction.SELECT_FOLDER);
		_html5_config = make_deploy_config_combo();
		_html5_app_title = new InputString();
		_html5_app_title.placeholder_text = "My Application";
		_html5_app_title.text = _project.name();

		_html5_set = new PropertyGridSet();

		// HTML5 box.
		_html5_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_html5_box.pack_start(_html5_deploy_button, false, true, 0);
		_html5_box.pack_start(_html5_set, false, true, 0);
		_html5 = new HTML5Deployer();
		_html5_page = new DeployerPage(TargetPlatform.HTML5, _html5_box, _html5.check_config);

		// HTML5 General page.
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Destination", _html5_output_path);
		cv.add_row("Config", _html5_config);
		_html5_set.add_property_grid(cv, "Output");

		// HTML5 Application.
		cv = new PropertyGrid();
		cv.column_homogeneous = true;
		cv.add_row("Title", _html5_app_title);
		_html5_set.add_property_grid(cv, "Application");

		// Linux page.
		_linux_deploy_button = make_deploy_button(TargetPlatform.LINUX);
		_linux_deploy_button.clicked.connect(() => {
				// Validate input fields.
				string? output_path = _linux_output_path.value;
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

		_linux_output_path = new InputFile(Gtk.FileChooserAction.SELECT_FOLDER);
		_linux_config = make_deploy_config_combo();
		_linux_app_title = new InputString();
		_linux_app_title.placeholder_text = "My Application";
		_linux_app_title.text = _project.name();

		_linux_set = new PropertyGridSet();

		// Linux box.
		_linux_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_linux_box.pack_start(_linux_deploy_button, false, true, 0);
		_linux_box.pack_start(_linux_set, false, true, 0);
		_linux_page = new DeployerPage(TargetPlatform.LINUX, _linux_box);

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
		_windows_deploy_button = make_deploy_button(TargetPlatform.WINDOWS);
		_windows_deploy_button.clicked.connect(() => {
				// Validate input fields.
				string? output_path = _windows_output_path.value;
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

		_windows_output_path = new InputFile(Gtk.FileChooserAction.SELECT_FOLDER);
		_windows_config = make_deploy_config_combo();
		_windows_app_title = new InputString();
		_windows_app_title.placeholder_text = "My Application";
		_windows_app_title.text = _project.name();

		_windows_set = new PropertyGridSet();

		// Windows box.
		_windows_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_windows_box.pack_start(_windows_deploy_button, false, true, 0);
		_windows_box.pack_start(_windows_set, false, true, 0);
		_windows_page = new DeployerPage(TargetPlatform.LINUX, _windows_box);

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
#if CROWN_PLATFORM_LINUX
		_notebook.append_page(_linux_page, new Gtk.Label(TargetPlatform.LINUX.to_label()));
#elif CROWN_PLATFORM_WINDOWS
		_notebook.append_page(_windows_page, new Gtk.Label(TargetPlatform.WINDOWS.to_label()));
#endif
		_notebook.append_page(_android_page, new Gtk.Label(TargetPlatform.ANDROID.to_label()));
		_notebook.append_page(_html5_page, new Gtk.Label(TargetPlatform.HTML5.to_label()));
		_notebook.vexpand = true;
		_notebook.show_border = false;

		_controller_key = new Gtk.EventControllerKey(this);
		_controller_key.key_pressed.connect(on_key_pressed);

		this.add(_notebook);
	}

	public bool on_key_pressed(uint keyval, uint keycode, Gdk.ModifierType state)
	{
		if (keyval == Gdk.Key.Escape)
			this.close();

		return Gdk.EVENT_PROPAGATE;
	}

	public void android_set_debug_keystore()
	{
		bool sensitive = !_android_use_debug_keystore.get_active();
		_android_keystore.sensitive = sensitive;
		_android_keystore_password.sensitive = sensitive;
		_android_key_alias.sensitive = sensitive;
		_android_key_password.sensitive = sensitive;
	}
}

} /* namespace Crown */
