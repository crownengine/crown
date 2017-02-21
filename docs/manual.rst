======
Manual
======

Introduction
============

What is it?
-----------

Pepper is a general purpose data-driven game engine, written from scratch in orthodox C++ with a minimalistic and data-oriented design philosophy in mind.

It is loosely inspired by Bitsquid (now Stingray) engine and its design principles.

System requirements
-------------------

**Development**

* Any 64-bit Linux distribution with GTK+ 3.14 or newer, 64-bit Ubuntu 16.04+ recommended
* 64-bit dual-core CPU or higher
* OpenGL 2.1+ graphics card with 512 MB of RAM, 1 GB recommended
* 4 GB of RAM minimum, 8 GB recommended
* 1280x768 display minimum, 1920x1080 recommended
* Three-button mouse

**Deployment**

* Android 2.3.7+ (ARMv7), Ubuntu 12.04+ (32 and 64-bit) or Windows 7, 8, 10 (32 and 64-bit)

Basic concepts
==============

The source directory
--------------------

The source directory contains all the files that make up an application.

There is no fixed structure for the files and folders in the source directory, you can organize your content as you see fit. You can for example decide to put all the textures and sounds in the "textures" and "sounds" folders respectively, or maybe sort the content on a per-object basis by putting all the assets for an enemy in the "units/enemy" folder.

There is, however, a small number of required files which are needed for the engine to start-up correctly:

.. code::

	.
	├── boot.config            <- First file loaded by the engine
	├── boot.package           <- Package to load on boot
	├── boot.lua               <- Lua script to launch on boot
	└── global.physics_config  <- Global physics-related configurations

The boot directory and the boot.config file
-------------------------------------------

Any directory within `the source directory`_ containing the file named ``boot.config`` is a boot directory.

The ``boot.config`` is the first file loaded by Pepper; it specifies the package to load and the lua script to execute on boot and various other boot-time settings. See `boot.config file reference`_ for more details.

There can be an arbitrary number of boot directories. You can set which boot directory Pepper should use with the switch ``--boot-dir``.

In the example below, the engine is told to load the package ``boot`` and run the Lua script ``lua/game``.

.. code::

	boot_package = "boot"      // Package to load on boot
	boot_script  = "lua/game"  // Lua script to execute on boot


The data directory
--------------------

Pepper reads source data from `the source directory`_ and compiles it into efficient binary representation.
The result of the compilation process is stored in the data directory.

.. code::

	.
	├── data                <- Contains compiled data files
	|   ├── a14e8dfa2cd...  <- Compiled file
	|   ├── 72e3cc03787...  <- Another compiled file
	|   └── ...
	├── temp                <- Temporary files from data compilers
	└── last.log            <- Text log from the last engine execution

The .bundleignore file
----------------------

Many programs store metadata files alongside edited files. This is often the case with text editors and version control systems.

When Pepper bumps into unknown files in the source directory, it quits the compilation and reports an error.

The ``.bundleignore`` file specifies files that Pepper should ignore when compiling data.

Example:

.. code::

	# This is a comment.

	# Blank lines are ignored.
	# Everything else is simple glob pattern (*, ?).
	*.txt

Units of measurement
--------------------

Pepper uses MKS (meters, kilograms and seconds) units and radians for angles.

boot.config file reference
==========================

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

Command line reference
======================

``-h`` ``--help``
	Display the help and quit.

``-v`` ``--version``
	Display engine version and quit.

``--source-dir <path>``
	Use <path> as the source directory for resource compilation.

	The <path> must be absolute.

``--data-dir <path>``
	Use <path> as the destination directory for compiled resources.

	The <path> must be absolute.

``--boot-dir <path>``
	Boot the engine with the ``boot.config`` from given <path>.

	The <path> must be relative.

``--compile``
	Do a full compile of the resources.

	When using this option you must also specify ``--platform``, ``--source-dir`` and ``--data-dir``.

``--platform <platform>``
	Compile resources for the given <platform>.
	Possible values for <platform> are:

	* ``android``
	* ``linux``
	* ``windows``

``--continue``
	Run the engine after resource compilation.

``--console-port <port>``
	Set port of the console.

	When no port is specified, the engine uses the port 10001.

``--wait-console``
	Wait for a console connection before starting up.

``--parent-window <handle>``
	Set the parent window <handle> of the main window.

	This option should be used only by the tools.

``--server``
	Run the engine in server mode.

	When using this option you must also specify ``--source-dir``.

``--run-unit-tests``
	Run unit tests and quit. Available only on ``linux`` and ``windows``.
