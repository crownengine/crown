boot.config reference
=====================

Generic configurations
----------------------

``boot_script = "lua/game"``
	Lua script to launch on boot.

``boot_package = "boot"``
	Package to load on boot.

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

