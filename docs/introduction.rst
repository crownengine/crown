============
Introduction
============

What is it?
-----------

Crown is a complete and cross-platform game engine designed for flexibility,
performance, and fast-iterations.

It is not limited to a specific game type or genre; rather, it provides a set of
versatile primitives and tools that can be used to create a broad range of both
3D and 2D games.

.. image:: shots/level-editor.png

.. note::

    Crown is in active development and new features are added and improved all
    the time; You can follow the development progress in real-time on `GitHub
    <https://github.com/crownengine/crown/issues>`__ and join the discussions on
    the `Discord <https://discord.com/invite/CeXVWCT>`__ channel!

License
-------

The games you create with Crown is your sole property. All the data (models,
textures, levels etc.) and code you produce (including data and code that Crown
itself outputs) is free for you to use as you wish. See :doc:`copyright` for an
exhaustive list of the licenses involved.

Supported platforms
-------------------

* Runtime
	* Android 7.0+ (ARMv7-a, ARMv8-a)
	* HTML5 (Wasm/Emscripten)
	* Ubuntu 20.04+ (x86_64)
	* Windows 7+ (x86_64)

* Editor
	* Ubuntu 20.04+ (or any 64-bit Linux with GTK+ >= 3.24)
	* Windows 7+

Hardware requirements
---------------------

* Editor
	* 64-bit dual-core CPU or higher
	* OpenGL 3.2+ graphics card with 512 MB of RAM, 1 GB recommended
	* 4 GB of RAM minimum, 8 GB recommended
	* 1366x768 display resolution minimum, 1920x1080 recommended
	* Three-button mouse

Design Principles
-----------------

Crown is loosely inspired by the Bitsquid engine and shares with it many of its
design principles:

* Data-driven
	All aspects of the game are defined via text-based configuration files,
	which are compiled into native, efficient, platform-specific BLOBs before
	shipping. These files are human-readable, making them easy to inspect and
	compatible with traditional VCS and regular text-based utilities.

* Data-oriented
	Data in memory is organized to achieve the maximum performance possible on
	every supported platform.

* Hot-reload everything
	Every game asset is reloadable at run-time, including code.

* Multi-process Editor/Runtime architecture
	The Editor and the Runtime live in separate processes. The Editor helds all
	the important data: if the Runtime crashes you can restart it without losing
	any work.

* Lightweight Codebase and Runtime
	The Runtime (plus tools) consist of fewer than 70K lines of code. Written in
	straightforward 'C-style' C++, it is easily understood and extensible by
	anyone.

Features
--------

* Cross-platform Editor
	* DCC data importers for models, textures, sounds etc.
	* Scene Editor with place/move/rotate/scale controls, snapping, selection etc.
	* Deployer for Android, HTML5 and desktop platforms.
	* Project Browser with thumbnails and multiple view modes.
	* Scene Tree and object Inspector.
	* Console with Lua REPL.

* Graphics
	* High-level 3D & 2D objects (meshes, cameras, lights and sprites).
	* Cross-platform GLSL-like shader programming language.
	* Animation state machine with events, variables and blending with simple expressions evaluator.
	* Immediate-mode GUI API with customizable materials.
	* TrueType text rendering via texture atlas.
	* Flipbook sprite animation.
	* PNG, TGA, DDS, KTX and PVR texture formats.
	* D3D11, GL 3.2 and GLES 2.0 render backends.

* Physics
	* Static, dynamic and keyframed rigid bodies with multiple colliders.
	* Joints (fixed, spring and hinge).
	* Spatial queries: ray-, sphere- and box-casts.
	* Triggers.

* Scripting
	* Integrated Lua runtime can be used to control every aspect of the game.
	* On supported platforms, LuaJIT is used for even higher performances.
	* Integrated REPL to quickly test and experiment while the game is running.
	* Live reloading of gameplay code without needing to restart the game.

* Debugging
	* Integrated profiler and data plotter graph.
	* C++ and Lua callstack generation.

* Input
	* Unified interface for mice, keyboards, joypads and touchpads.
	* Full list of input events for each simulation frame.
	* Simplified polling interface for rapid prototyping.

* Audio
	* 3D audio sources with position and range-based attenuation.
	* Formats: WAV.

Contact
-------

* Website: https://www.crownengine.org
* Project page: https://github.com/crownengine/crown

Contributors
------------

In chronological order.

* Daniele Bartolini (`@dbartolini <https://github.com/dbartolini>`_)
* Simone Boscaratto (`@Xed89 <https://github.com/Xed89>`_)
* Michele Rossi (`@mikymod <https://github.com/mikymod>`_)
* Michela Iacchelli - Crown logo v1.
* Raphael de Vasconcelos Nascimento (`@vasconssa <https://github.com/vasconssa>`_)
* Giulia Gazzoli - Crown logo v2.
