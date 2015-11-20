Glossary
========

This page lists definitions for terms used in Crown and this manual.

.. glossary::
	:sorted:

	Component
		A collection of data that is necessary for a given simulation to be computed.

	Unit
		A collection of distinct :term:`components<Component>`.
		This is the basic building block used to represent all kinds of different
		objects in a :term:`World` simulation.

	Level
		A collection of :term:`units<Unit>`, :term:`sounds<Sound>` and other objects
		to be used together in a :term:`World`.

	Resource
		The file that describes a particular piece of data to be used by the
		:term:`Runtime`. Resource files typically contain definitions for :term:`units
		<Unit>`, :term:`levels <Level>`, :term:`Lua` scripts etc.

	Source Data
		The collection of :term:`Resource` files stored inside the :term:`Source
		Directory`.

	Source Directory
		The directory where the :term:`Source Data` is stored on disk.

	Data Directory
		The directory where the :term:`Data Compiler` output is stored on disk.

	Data Compiler
		The incremental compiler that transforms generic, human-readable :term:`Source
		Data` into specialized, high-performance binary blobs ready to be loaded in
		memory and consumed by the :term:`Runtime`.

	Runtime
		The executable that reads data from the :term:`Data Directory` and transforms
		it to produce a particular output based on the logic defined by the programmer
		and the input given by the user.
		This is the executable that will be distributed to the end user of your game.

	World
		The object where the simulation of a number of :term:`units<Unit>` occur. This
		simulation can be advanced in time and its results can be renderd on screen
		from arbitrary view points defined by a :term:`Camera`.

	Camera
		An object that describes a position and orientation in space, along with other
		geometric parameters, used to render a :term:`World` on screen from a specific
		perspective.

	Lua
		The scripting language used in Crown.

	Shader
		A program that runs on dedicated hardware to produce graphical output on the
		screen or other off-screen buffers.

	Sound
		A 3D object used to play sounds in a :term:`World`.

	Sprite
		An object used to represent animated 2D graphics on the screen.

	Mesh
		An object used to represent animated 3D graphics on the screen.

	Texture
		An object used to describes visual features for :term:`sprites<Sprite>`,
		:term:`meshes<Mesh>` etc.
