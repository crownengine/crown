==============
Creating Units
==============

Overview
========

There are two distinct concepts you need to keep in mind:

- Unit **prefab**: a ``.unit`` resource stored in your project that describes
  a Unit hierarchy and all its components and properties.
- Unit **instance**: a concrete instance of that prefab that exists in a
  running World.

You author and edit prefabs in the tools (Level Editor, Unit Editor and
importers). At runtime you instantiate prefabs from Lua and manipulate the
resulting Unit instances.

You do *not* write ``.unit`` files from Lua, and you do *not* author prefabs
directly from gameplay code.

From the Level Editor
=====================

Prefabs can be created in a number of ways. Arguably the simplest way  to
create new Units is to use the :ref:`Level Editor <level_editor>` and then
save what you have built as a prefab.

1. **Open or create a Level**
    - Open an existing Level from the :doc:`../level_editor/project_browser`, or
    - create a new one with ``File`` -> ``New Level``.

2. **Place a Unit in the Level**
    - Place a new unit with ``Spawn`` -> ``Primitives``, or drag an existing
      Unit prefab from the Project Browser into
      the :doc:`../level_editor/level_viewport`
    - Use the :doc:`../level_editor/inspector` to add or remove components and
      to fine‑tune their properties

3. **Save the Unit as prefab**
    - In the :doc:`../level_editor/level_tree`, right‑click the Unit you want
      to turn into a prefab
    - Choose ``Save as Prefab...``
    - In the file dialog, pick a target folder and a unique name, then click
      ``Save``

The editor will:

- create a new ``.unit`` resource at the chosen location
- automatically replace the Unit in the Level with an instance of the newly
  created prefab

From now on:

- editing the prefab will update all instances that use it unless they
  explicitly override properties or children
- editing the instance in the Level will create per‑instance overrides

From the Project Browser
========================

Sometimes you want to start from a blank prefab instead of capturing an
existing Unit from a Level. You can do this directly from the
:doc:`../level_editor/project_browser`:

1. Right‑click the folder where you want to create the prefab
2. Choose ``New Unit...``
3. Enter a name for the prefab and confirm

This creates an empty ``.unit`` resource that you can then edit:

- by opening it in the Unit Editor, or
- by selecting any instance that uses the prefab in a Level and clicking
  ``Open Prefab`` in the :doc:`../level_editor/inspector`.

From imported resources
=======================

Some importers generate Units automatically as part of their workflow:

- :doc:`../importing_resources/importing_scenes` generates Units for each
  geometry, light and camera in a FBX file.
- :doc:`../importing_resources/importing_sprites` generates a Unit prefab,
  materials and other resources from a spritesheet.

These generated Units appear in the Project Browser like any other ``.unit``
resource and can be:

- placed in Levels
- opened and modified in the Unit Editor
- used as base prefabs for derived units

Spawning Units
==============

Lua gameplay code never creates new ``.unit`` resources. Instead, it creates
instances of existing prefabs inside a World. The act of creating unit
instances at runtime is called **spawning**.

.. code:: lua

   player = World.spawn_unit(world, "units/player/player")

This:

- instantiates the unit ``units/player/player`` into the specified ``world``
- returns a Unit handle you can store and use from Lua

See :doc:`../gameplay/unit_interaction` for a detailed walkthrough of unit
handles, Script Component and callbacks.
