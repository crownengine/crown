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
