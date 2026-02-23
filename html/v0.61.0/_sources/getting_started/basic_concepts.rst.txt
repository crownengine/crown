Basic Concepts
==============

This page describes basic concepts and terminology commonly used when working
with Crown.

.. _project:

Project
-------

A project in Crown is a folder that contains all the resources that describe
your game. Resources are plain-text files (with some exceptions) identified
by their relative path (to the project's root).

You are free to organize resources in a project however you like. However, to
avoid confusion it is recommended to put units in their own folder because
they often consist of multiple files (e.g. a resource for a tree may consist
of a ``tree.unit``, ``tree.material`` and ``tree.lua`` files).

Resource paths and names
------------------------

All paths in Crown are unix-style (forward slashes). Only canonical/normalized
paths are allowed.

Resources in a project are identified by their relative path to the project's
folder, without extension.

For example, to refer to the unit named "units/player/player.unit" you will
use the string "units/player/player".

Unit and Components
-------------------

Units are the fundamental building blocks in Crown. Units can be used to
represent any object that exists in a game world - a character, a tree, a
vehicle, a light etc.

By default, units are empty. You add functionality to a Unit by adding
Components to it. No component is mandatory, units can be just empty IDs.

Crown includes a number of predefined components, some examples are:

- ``Transform`` - specifies a position, rotation and scale in the world
- ``MeshRenderer`` - draws a 3D mesh
- ``Actor`` - specifies physics properties for a collider, such as mass, filter etc.
- ``Script`` - adds logic via a Lua script
- ``Light``, ``Camera``, ``Fog``, ``Bloom`` etc.

Components can be added or removed at any time in the editor or at runtime.

Units support parent-child relationships, allowing complex hierarchies of
objects to be built from many smaller units (e.g. a car with wheels, body,
lights etc.).

Unit Prefab
-----------

Units can be saved to disk as a *.unit* resource, creating a prefab. Other
units can then reference this prefab, instantly inheriting all of its
components and hierarchy. Changes to a prefab are automatically propagated to
every instance of that prefab.

Unit Prefabs are created by saving existing units inside
the :ref:`level_editor`, or, more commonly, by importing them
from :ref:`scenes <importing_scenes>` or :ref:`sprites <importing_sprites>`.

World
-----

A World is the runtime container that holds Units and other objects and
advances the game simulation. When a World is running it:

- updates animations/audio/particles;
- steps physics and handles collisions;
- dispatches logic events to script components;
- renders the visible scene;
- etc.

A World can be populated manually via code or automatically by loading Level
resources into it.

A game always has at least one active world. Multiple worlds can exist
simultaneously (for example, one for the main game and one for a UI overlay),
though this is uncommon.

Level
-----

A Level (*.level* resource) is a saved collection of units, sounds and other
objects.

Levels are authored in the :ref:`level_editor`. A world can load any number of levels
at the same time, enabling complex multi-layered scenes.
