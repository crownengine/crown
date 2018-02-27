Command line reference
======================

Examples
--------

Compile source data from ``/home/user/demo`` using a mapped ``core`` folder.

.. code::

	$ crown-development64 --source-dir /home/user/demo --map-source-dir core /home/user --compile

Run the engine using compiled data from ``/home/user/demo_linux``:

.. code::

	$ crown-development64 --data-dir /home/user/demo_linux

Options
-------

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
