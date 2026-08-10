Command line
============

Both the runtime and the editor support command line workflows. The recommended
way to access this functionality is to use the ``crown-launcher`` executable
from Crown's installation directory. It provides a common entry point that
dispatches commands to the appropriate executables.

All the examples below assume that you have defined the ``CROWN`` environment
variable to point to the absolute path of Crown's installation directory:

.. code::

	export CROWN=/home/user/crown

Examples
--------

Compile and run the project ``/home/user/crown/samples/01-physics`` using a mapped ``core`` folder:

.. code::

	./crown-launcher runtime --compile --continue --source-dir $CROWN/samples/01-physics --map-source-dir core $CROWN/samples

Compile a "bundled" version of the project:

.. code::

	./crown-launcher runtime --compile --bundle --source-dir $CROWN/samples/01-physics --map-source-dir core $CROWN/samples

Run the project from bundled data:

.. code::

	./crown-launcher runtime --data-dir $CROWN/samples/01-physics_linux

Customize the data directory name:

.. code::

	./crown-launcher runtime --compile --data-dir $CROWN/samples/01-physics_custom --source-dir $CROWN/samples/01-physics --map-source-dir core $CROWN/samples

Open a project in the editor:

.. code::

    ./crown-launcher editor --source-dir $CROWN/samples/01-physics

Import an FBX mesh:

.. code::

    ./crown-launcher editor --source-dir $CROWN/samples/01-physics --import chair.fbx units/chair

Import a PNG as a sprite:

.. code::

    ./crown-launcher editor --source-dir $CROWN/samples/01-physics --import hero.png units/hero --as sprite

Deploy a project for HTML5:

.. code::

    ./crown-launcher editor --source-dir $CROWN/samples/01-physics --deploy --platform html5 --output-dir $CROWN/deploy

Runtime Options
---------------

``--``
	End of the runtime's options.

	All options following ``--`` are passed to the game but will not be
	interpreted by the runtime.

``-h`` ``--help``
	Display this help.

``-v`` ``--version``
	Display engine version.

``--source-dir <path>``
	Specify the <path> of the project's source data.

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

``--boot-dir <prefix>``
	Use <prefix>/boot.config to boot the engine.

	The <prefix> must be relative.

``--compile``
	Compile the project's source data.

	When using this option you must also specify ``--source-dir``.

``--bundle``
	Generate bundles after the data has been compiled.

``--platform <platform>``
	Specify the target <platform> for data compilation.
	Possible values for <platform> are:

	* ``android``
	* ``html5``
	* ``linux``
	* ``windows``

``--continue``
	Run the engine after the data has been compiled.

``--console-port <port>``
	Set port of the console server.

	When no port is specified, the engine uses the port 10001.

``--port-file <path>``
	Write selected console port to <path>.
	The file is written after the console server has bound a port if any.

``--wait-console``
	Wait for a console connection before booting the engine.

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

``--keep-above``
	Keep the main window above other windows.

``--display-server <server>``
	Set the display server backend.
	Possible values for <server> are:

	* ``wayland``
	* ``x11``

``--renderer <renderer>``
	Set the renderer backend.
	Possible values for <renderer> are:

	* ``auto``
	* ``d3d11``
	* ``gl``
	* ``gles``
	* ``vk``

``--window-rect <x y w h>``
	Set the main window's position and size.

``--string-id <string>``
	Print the 32- and 64-bits IDs of <string>.

``--run-unit-tests``
	Run unit tests and quit.

Editor Options
--------------

``-h`` ``--help``
    Display the help and exit.

``-v`` ``--version``
    Display version information and exit.

``--source-dir <path>``
    Project source directory.

``--init``
    Create a new project.

``--import <file>... <path>``
    Import files into a source-dir relative path.

``--as <type>``
    Import files using the specified importer type.
    Possible values for <type> are:

    * ``font``
    * ``mesh``
    * ``sound``
    * ``sprite``
    * ``texture``

``--deploy``
    Deploy the project.

``--platform <platform>``
    Deploy target platform.
    Possible values for <platform> are:

    * ``android``
    * ``html5``
    * ``linux``
    * ``windows``

``--output-dir <path>``
    Deploy output directory.

``--config <config>``
    Deploy config.
    Possible values for <config> are:

    * ``release``
    * ``development``
    * ``debug``

``--app-title <title>``
    Application title.

``--force``
    Overwrite an existing package directory.

``--arch <arch>``
    Android architecture.
    Possible values for <arch>:

    * ``arm``
    * ``arm64``

``--app-id <id>``
    Android application identifier.

``--app-version-code <number>``
    Android version code.

``--app-version-name <name>``
    Android version name.

``--min-sdk-version <number>``
    Android minimum SDK version.

``--target-sdk-version <number>``
    Android target SDK version.

``--manifest <path>``
    Android manifest file.

``--keystore <path>``
    Android signing keystore.

``--keystore-pass <password>``
    Android keystore password.

``--key-alias <alias>``
    Android signing key alias.

``--key-pass <password>``
    Android signing key password.

``--index-html <path>``
    HTML5 index.html file.
