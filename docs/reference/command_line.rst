Command line
============

Examples
--------

Compile and run the project ``/home/user/crown/samples/01-physics`` using a mapped ``core`` folder:

.. code::

	export CROWN=/home/user/crown
	cd $CROWN/platforms/linux64/bin
	./crown-development --compile --continue --source-dir $CROWN/samples/01-physics --map-source-dir core $CROWN/samples

Compile and run for HTML5:

.. code::

	./crown-development --compile --continue --platform html5 --source-dir $CROWN/samples/01-physics --map-source-dir core $CROWN/samples

Compile a "bundled" version of the project:

.. code::

	./crown-development --compile --bundle --source-dir $CROWN/samples/01-physics --map-source-dir core $CROWN/samples

Run the project from bundled data:

.. code::

	./crown-development --data-dir $CROWN/samples/01-physics_linux

Customize the data directory name:

.. code::

	./crown-development --compile --data-dir $CROWN/samples/01-physics_custom --source-dir $CROWN/samples/01-physics --map-source-dir core $CROWN/samples

Options
-------

``--``
	End of the runtime's options.

	All options following ``--`` are passed to the game but will not be
	interpreted by the runtime.

``-h`` ``--help``
	Display the help and quit.

``-v`` ``--version``
	Display engine version and quit.

``--source-dir <path>``
	Use <path> as the source directory for resource compilation.

	The <path> must be absolute.

``--data-dir <path>``
	Run with the data located at <path>.

	The <path> must be absolute.

``--bundle-dir <path>``
	Run with the bundles located at <path>.

	The <path> must be absolute.

``--map-source-dir <name> <path>``
	Mount <path>/<name> at <source-dir>/<name>.

	The <path> must be absolute.

``--boot-dir <path>``
	Boot the engine with the ``boot.config`` from given <path>.

	The <path> must be relative.

``--compile``
	Do a full compile of the resources.

	When using this option you must also specify ``--source-dir``.

``--bundle``
	Generate bundles after the data has been compiled.

``--platform <platform>``
	Compile resources for the given <platform>.
	Possible values for <platform> are:

	* ``android``
	* ``html5``
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

``--pumped``
	Do not advance the renderer unless explicitly requested via console.

``--hidden``
	Make the main window initially invisible.

``--window-rect <x y w h>``
	Sets the main window's position and size.

``--string-id <string>``
	Print the 32- and 64-bits IDs of <string>.

``--run-unit-tests``
	Run unit tests and quit.
