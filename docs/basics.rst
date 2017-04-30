Basic concepts
==============

The source directory
--------------------

The source directory contains all the files that make up an application.

There is no fixed structure for the files and folders in the source directory, you can organize
your content as you see fit. You can for example decide to put all the textures and sounds in the
"textures" and "sounds" folders, or maybe sort the content on a per-object basis by putting all the
assets for an enemy in the "units/enemy" folder.

There is, however, a small number of required files which are needed for the engine to start-up
correctly:

.. code::

	.
	├── boot.config            <- First file loaded by the engine
	├── boot.package           <- Package to load on boot
	├── boot.lua               <- Lua script to launch on boot
	└── global.physics_config  <- Global physics-related configurations

The boot directory and the boot.config file
-------------------------------------------

Any directory within `the source directory`_ containing the file named ``boot.config`` is a boot
directory.

The ``boot.config`` is the first file read by Crown; it specifies the package to load and the lua
script to execute on boot and various other boot-time settings.
See `boot.config file reference`_ for more details.

Normally there is a single ``boot.config`` file, placed at the top of the source directory. This is
the file which Crown looks at by default.

In some circumstances is desirable to have multiple ``boot.config`` files.
You can set which boot directory Crown should use with the switch ``--boot-dir``.

In the example below, a minimal ``boot.config`` file tells the engine to load the package ``boot``
and run the Lua script ``lua/game``.

.. code::

	$ cat boot.config
	boot_package = "boot"      // Package to load on boot
	boot_script  = "lua/game"  // Lua script to execute on boot


The data directory
--------------------

Crown reads source data from `the source directory`_ and compiles it into efficient binary
representation. The result of the compilation process is stored in the data directory.

.. code::

	.
	├── data                <- Contains compiled data files
	|   ├── a14e8dfa2cd...  <- Compiled file
	|   ├── 72e3cc03787...  <- Another compiled file
	|   └── ...
	├── temp                <- Temporary files from data compilers
	└── last.log            <- Text log from the last engine execution

The .dataignore file
----------------------

The ``.dataignore`` file specifies files that Crown should ignore when compiling data.

When Crown bumps into unknown files in the source directory, it reports and error and quits the
compilation.
This is often desired behavior, since you do not want non-essential data in your source directory.

In all other cases, you should create a ``.dataignore`` in the source directory.

Example:

.. code::

	$ cat .dataignore
	# This is a comment.

	# Blank lines are ignored.
	# Everything else is simple glob pattern (*, ?).
	*.txt

Units of measurement
--------------------

Crown uses MKS (meters, kilograms and seconds) units and radians for angles.
