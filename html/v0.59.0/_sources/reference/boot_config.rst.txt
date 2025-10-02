boot.config reference
=====================

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

Physics configurations
~~~~~~~~~~~~~~~~~~~~~~

``step_frequency = 60``
	The frequency at which the physics simulation is stepped.
	Higher values means a more accurate simulation at the expense of compute time.

``max_substeps = 4``
	Maximum number of physics sub-steps allowed in a frame.
	A value of 4 at 60 Hz means the physics simulation is allowed to simulate up to ~0.067 seconds (4/60) worth of physics per frame.
	If one frame takes longer than ``max_substeps/step_frequency`` then physics will appear slowed down.

