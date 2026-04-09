=====================
Global Physics Config
=====================

The ``global.physics_config`` resource defines physics materials, collision
filters, and actor classes used by Crown's physics simulation. The file is
stored in SJSON format and must be edited manually.

Example
=======

.. code-block::

    materials = {
        default = { friction = 0.8 restitution = 0.81 }
    }

    collision_filters = {
        no_collision = { collides_with = [] }
        default = { collides_with_all_except = [ "no_collision" ] }
    }

    actors = {
        static = { dynamic = false }
        dynamic = { dynamic = true }
        keyframed = { dynamic = true kinematic = true disable_gravity = true }
        trigger = { trigger = true }
    }

.. _physics_materials:

Materials
=========

Physics materials define surface response parameters for :ref:`actors
<Actors>`. Create a new material by adding an entry to the ``materials``
object:

.. code-block::

    materials = {
        default = ...
        slippery = { friction = 0.1 }
    }

Common material properties:

* ``friction``: general contact friction; larger values increase resistance to
  sliding.
* ``rolling_friction``: torsional friction orthogonal to the contact normal;
  helps stop spheres rolling forever.
* ``spinning_friction``: torsional friction around the contact normal; useful
  for grasping.
* ``restitution``: coefficient of restitution; 1 = perfectly elastic
  collision, < 1 = inelastic.

Collision filters
=================

Collision filters control which :ref:`actors <Actors>` interact with which. Define a
filter by adding an entry to the ``collision_filters`` object:

.. code-block::

    collision_filters = {
        ...
        wall = {}
        player = { collides_with = [ "wall" ] }
    }

The filter name (for example ``player``) can then be assigned to actors. Use
``collides_with`` to list specific filters this filter should collide with,
or ``collides_with_all_except`` to collide with everything except the
specified filters.

Collision rule
--------------

Two actors A and B generate collisions if either of the following is true:

* A's filter has B in its ``collides_with`` set, or
* B's filter has A in its ``collides_with`` set.

Actor classes
=============

Actor classes are used to specify simulation properties to apply
to :ref:`Actors`. Define classes inside the ``actors`` object:

.. code-block::

    actors = {
        static = { dynamic = false }
        dynamic = { dynamic = true }
        keyframed = { dynamic = true kinematic = true disable_gravity = true }
        trigger = { trigger = true }
    }

Common actor class properties:

* ``linear_damping``: resistance applied to linear motion; useful to slow down
  objects unaffected by friction.
* ``angular_damping``: resistance applied to angular motion.
* ``dynamic``: when true, the actor is simulated each frame by the physics
  engine. If false, the actor cannot move but just collide.
* ``kinematic``: when true, the actor is driven externally (animation or code)
  rather than by physics. Kinematic objects push dynamic objects away but are
  unaffected by them.
* ``disable_gravity``: disables gravity for the actor.
* ``trigger``: actor does not produce collision responses but still generates
  overlap events.
