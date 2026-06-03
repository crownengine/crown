================
Scripting in Lua
================

All gameplay code in Crown is written in Lua. This section will give you a basic
understanding of how Lua is used and can be used in Crown.

What is Lua
-----------

Lua is a simple, fast and lightweight scripting language which is easy to learn
and supports all features needed to write any type of game.

Lua is widely known in the games industry and has been used for decades to write
many successful commercial games.

Runtime entry points
--------------------

When Crown runs, it calls a small set of predefined global functions at
specific times:

	- init()
	- shutdown()
	- update(dt)
	- render(dt)

The init() function is called right after the runtime starts. It can be used to
initialize global state etc. while shutdown() is called just before the runtime
exits.

update(dt) and render(dt) are called periodically as part of the game loop. You
can use them to implement any gameplay logic, input handling and other per-frame
updates your game may need.

The boot script
---------------

The boot script is the first Lua script that is executed when Crown starts.

In the boot script you will typically define the `runtime entry points`_ and
possibly require() additional lua scripts, like you would do in regular Lua
programs. One difference is that Crown expects the .lua extension to be omitted
(i.e. require("foo/bar") instead of "foo/bar.lua"). This is for consistency with
how resources are referenced elsewhere.

Projects may contain multiple boot scripts for different purposes (for example,
separate boot scripts for the editors and the game). You can specify which boot
script to use in the :doc:`Boot Config <../reference/boot_config>` file.

The GameBase framework
----------------------

Crown automatically generates a ``main.lua`` script in the root folder of
:doc:`new projects <../getting_started/create_new_project>`.

You may notice that it contains some strangely named
Game.init()/Game.update()/etc. functions, as well as other utility functions.

Those functions are part of the GameBase framework, which is a tiny layer built
on top of the fundamental `runtime entry points`_.

The GameBase layer allows for integration with the editors and adds support to
common functionalities such as loading levels, creating a default camera etc.

Expert users can avoid the GameBase althogheter by defining plain
init/update/shutdown but we recommend to stick with it if you plan to use our
tools.

Hot reloading
-------------

Crown fully supports reloading of gameplay code while the game is running.

Hot reloading is achieved by re-executing modified Lua files. This method works
well in general, but needs some care in specific occasions. Consider the
following script:

.. code:: lua

	Foo = {}

Every time Crown reloads such file, a new table will be created and its
reference will be stored in the variable Foo, making the previous state
unreachable.

To make it hot-reload safe you could check whether the Foo objects exists
already, and skip its creation if that is the case:

.. code:: lua

	Foo = Foo or {}

