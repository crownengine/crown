boot.config reference
=====================

Path templates
--------------

Some settings accept path templates. A path template is a path that may contain
variables in the form ``$VARNAME``.

Supported variables:

* ``$DATE``: The local date in ``YYYY-MM-DD`` format.
* ``$UTC_DATE``: The UTC date in ``YYYY-MM-DD`` format.
* ``$TIME``: The local time in ``HH-MM-SS`` format.
* ``$UTC_TIME``: The UTC time in ``HH-MM-SS`` format.
* ``$USER_DATA``: The user data directory. On Android, this expands to the
  activity internal data path.
* ``$TMP``: The temporary directory.
* ``$RANDOM``: An 8-character random string.
* ``$OBB_PATH``: Android only. The activity OBB path.

Generic configurations
----------------------

``boot_script = "lua/game"``
	Lua script to launch on boot.

``boot_package = "boot"``
	Package to load on boot.

``render_config = "core/renderer/default"``
	Render configuration to use.

``window_title = "My window"``
	Title of the main window on platforms that support it.

Platform-specific configurations
--------------------------------

All configurations for a given *platform* are placed under a key named *platform*. E.g.:

.. code::

	// Linux-only configs
	linux = {
	  renderer = {
	      resolution = [ 1280 720 ]
	      aspect_ratio = -1
	      vsync = true
	  }
	}

Renderer configurations
~~~~~~~~~~~~~~~~~~~~~~~

``resolution = [ 1280 720 ]``
	Sets the width and height of the main window.

``aspect_ratio = -1``
	Sets the aspect ratio.
	If the value is set to ``-1``, the aspect ratio is computed as ``width/height`` of the main window.

``vsync = true``
	Sets whether to enable the vsync.

``fullscreen = false``
	Sets whether to enable fullscreen.

``device_id = "0x0000"``
    Sets which GPU to use for rendering. The ID is in hexadecimal notation.
    Default value selects the first available device.

Physics configurations
~~~~~~~~~~~~~~~~~~~~~~

``step_frequency = 60``
	The frequency at which the physics simulation is stepped.
	Higher values means a more accurate simulation at the expense of compute time.

``max_substeps = 4``
	Maximum number of physics sub-steps allowed in a frame.
	A value of 4 at 60 Hz means the physics simulation is allowed to simulate up to ~0.067 seconds (4/60) worth of physics per frame.
	If one frame takes longer than ``max_substeps/step_frequency`` then physics will appear slowed down.

Other settings
~~~~~~~~~~~~~~

``save_dir = $USER_DATA/mygame``
    Sets the directory where save files will be stored. Setting the save
    directory is mandatory if you plan to use the SaveGame system.

``user_config = $USER_DATA/user.conf``
    Sets the file where user-specific settings are stored.
