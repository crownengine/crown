============
Introduction
============

What is it?
-----------

Crown is a general purpose and data-driven game engine, written in `orthodox
<https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b>`__ C++ with a
minimalistic and data-oriented design philosophy in mind.

Crown isn't tied to a particular game type or genre but instead it offers a set
of generic primitives and facilities to help you create a wide gamut of
interactive 2D and 3D products.

Crown also offers a complete cross-platform editor to create and put together
all the elements that make up your game.

Crown is loosely inspired by the Bitsquid engine and many of its design
principles.

Features
--------

Note: Crown is in active development and new features are added and improved all
the time; you can check out the development efforts in real-time on the Crown
Trello `board <https://trello.com/b/h88kbJNm/crown>`__. You can also join our
Discord `channel <https://discord.com/invite/CeXVWCT>`__!

* Data-driven
	Every aspect of the game is controlled through text configuration files. Text
	files are human-readable for easy inspection and play nicely with traditional
	version control systems. Before shipping, configuration files are compiled to
	efficient platform-specific binary blobs.

* Data-oriented
	Data in memory is organized to achieve the maximum performance possible on
	every platform.

* Lightweight codebase and runtime
	Engine plus tools amounts to less than 50K LOC. Written in simple 'C-style'
	C++. It is easy for anyone to understand and make modifications.

* Editor
	* Runs and looks equally well both on Linux and on Windows.
	* Source data importers (meshes, sprites, sounds, textures etc.).
	* All the editing data is held by the editor which runs in its own process, if the Runtime crashes you can restart it without losing your work.
	* TCP/IP communication with Runtime allows editing to be mirrored to remote devices (phones, consoles etc.).
	* Hot-reloading of every asset including code.

* Graphics
	* High-level 2D and 3D objects: sprites, meshes and lights (directional, omni and spot).
	* Cross-plaform GLSL-like shader programming language with data-driven definitions of render states and permutations.
	* D3D11, GL 3.2 and GLES 2.0 render backends.
	* PNG, TGA, DDS, KTX and PVR texture formats.

* Animation
	* Animation state machine with events, variables and blending with simple expressions evaluator.
	* Flipbook sprite animation.

* Physics
	* Static, dynamic and keyframed rigid bodies, triggers and joints (fixed, spring and hinge).
	* Spatial queries: ray-, sphere- and box-casts.
	* Multiple collision geometries per rigid body.
	* Collision geometry can be generated from mesh data, authored by artists or manually specified.
	* Uniform scaling of physics objects.

* Audio
	* 3D audio sources with position and range-based attenuation.
	* Volume control.
	* WAV format supported.

* Scripting
	* Integrated Lua runtime can be used to control every aspect of the game.
	* LuaJIT is used on supported platforms for maximum performance.
	* Integrated REPL to quickly test and experiment while the game is running.
	* Live reloading of gameplay code without needing to restart the game.

* Debugging
	* Integrated profiler.
	* Callstack generation for C++ and Lua.

* Gui
	* Immediate-mode GUI with custom materials.
	* TrueType text rendering via texture atlas.

* Input
	* Unified interface for accessing mice, keyboards, touchpads and joypads.

* Exporters
	* Mesh exporter for Blender >= 2.80.

Supported platforms
-------------------

* Runtime
	* Android 7.0+ (ARMv7-a, ARMv8-a)
	* Ubuntu 12.04+ (x86_64)
	* Windows 7, 8, 10 (x86_64)

* Editor
	* Any 64-bit Linux distribution with GTK+ 3.16 or newer, 64-bit Ubuntu 16.04.2+ recommended
	* 64-bit dual-core CPU or higher
	* OpenGL 3.2+ graphics card with 512 MB of RAM, 1 GB recommended
	* 4 GB of RAM minimum, 8 GB recommended
	* 1280x768 display resolution minimum, 1920x1080 recommended
	* Three-button mouse
