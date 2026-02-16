.. _console_commands:

Console commands
================

The following are commands that can be typed in the Console tab. In order for a
command to be interpreted as such, you must prefix it with a colon symbol when
entering it in the command prompt:

.. figure:: images/console_command.png
   :align: center

   Typing a console command with the ``:`` prefix.

crash
-----

Crash the engine for testing purposes. Accepts the following parameters:

``div_by_zero``
	Divide a number by zero.

``unaligned``
	Do an unaligned memory access.

``segfault``
	Trigger a segmentation fault.

``oom``
	Allocate too much memory.

``assert``
	Call CE_ASSERT(false).

game
----

Pause/resume the game. Accepts one of the following parameters:

``pause``
	Pause the game.

``resume``
	Resume the game.

help
----

List all commands.

unpause
-------

Resume the engine. Deprecated, use :ref:`game` instead.

pause
-----

Pause the engine. Deprecated, use :ref:`game` instead.

graph
-----

Plot selected profiler data over time. The <field> parameter must be the name
of a profiler record that the engine provides itself or one that you recorded
via the :ref:`Profiler` API. The command accepts the following
parameters:

``make <name>``
	Create a new graph.

``list``
	List all the graphs.

``range <graph> [min max]``
	Set the range of a graph. If the range is omitted, it will be automatically
	computed.

``add <graph> <field>``
	Add a field to a graph.

``remove <graph> <field>``
	Remove a field from a graph.

``hide <graph>``
	Hide a graph.

``show <graph>``
	Show a graph.

``layout <graph> <type>``
	Set the layout of a graph.
	Type can be one of: ``fill``, ``left``, ``right``, ``bottom`` or ``top``.

``color <graph> <field> <color>``
	Set the color of a field in a graph. Color can be specified as a 3- or
	6-digits hexadecimal RGB number or can be any of: ``yellow``, ``red``, ``green``
	or ``blue``.

``samples <graph> <samples>``
	Set the number of samples to show in a graph.
