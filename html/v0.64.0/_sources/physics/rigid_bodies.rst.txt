============
Rigid Bodies
============

Crown provides rigid body simulation through the Actor Component. Actors are
the core objects that participate in the physics simulation, giving realistic
physical behaviour.

.. figure:: images/actor_component.png
   :align: center

   The Actor Component shown in the Inspector.

Colliders
=========

Colliders define an Actor's physical shape. The shape can be derived from mesh
geometry or specified manually. When using a mesh as the source you can
choose between fast analytic shapes (``sphere``, ``capsule`` or ``box``) or
more accurate representations such as ``convex_hull`` or ``mesh``.

.. figure:: images/collider_component.png
   :align: center

   The Collider Component shown in the Inspector.

Analytic shapes are much cheaper to simulate and are recommended whenever they
provide acceptable results; use convex or full mesh shapes only when you need
precise collision geometry.

Only analytic shapes can be specified manually.

Actors
======

Actors provide physical behaviour to the Unit they are attached to. An Actor
defines the physical object's :ref:`class <Actor classes>`,
its :ref:`material <physics_materials>`, :ref:`collision filter <Collision
filter>`, mass, and other simulation properties.

Actor Class
-----------

A project defines a set of global actor classes. The default classes are
described below:

* ``static``: immovable actors, they just collide.
* ``dynamic``: fully simulated actors affected by forces and collisions.
* ``keyframed``: actors driven by animation or code; they push away dynamic
  actors but are not influenced by them.
* ``trigger``: non-blocking actors used to detect overlap events.

You can define additional classes and tune class behavior in the :ref:`Global
Physics Config`.

Collision Filter
----------------

Collision filters control which actors interact with which. The ``default``
filter applied to new actors allows interaction with other actors using the
same ``default`` filter. A ``no_collision`` filter is available to make
actors ignore collisions entirely.

Custom, arbitrarily complex filters can be added and configured via
the :ref:`Global Physics Config`.

Material
--------

Materials specify the physical properties of actors. Create new materials in
the :ref:`Global Physics Config`.

Movers
======

The Mover Component is a specialized kinematic actor that implements classic
collide-and-slide behavior and is better suited for character controllers and
NPCs that require deterministic movement under direct control of the player
or scripts.

.. figure:: images/mover_component.png
   :align: center

   The Mover Component shown in the Inspector.

The Mover uses a configurable capsule collider aligned vertically to the world
Z axis. It supports configurable maximum slope angle to limit climbing and
adjustable center point.
