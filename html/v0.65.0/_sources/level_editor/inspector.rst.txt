=========
Inspector
=========

The Inspector lets you view and modify the properties of selected objects in
the :ref:`Level Viewport`. Select one or more objects to display their
components and editable fields. Changes made in the Inspector are applied
immediately and reflected in the Level Viewport.

.. figure:: images/inspector.png
   :align: center

   The Inspector showing the properties of a Unit.

Expression evaluator
====================

Numeric entries in the Inspector can evaluate simple mathematical expressions,
which is handy for precise adjustments. Type an expression and press
``Enter`` to evaluate it. The special variable ``x`` represents the current
value in the field. For example, if a field contains ``2``, entering ``x +
1`` changes it to ``3``.

.. figure:: images/inspector_entry_expression.png
   :align: center

   A simple expression in a numeric entry.

Revealing resources
===================

Properties that reference resources include a ``Reveal`` shortcut that opens
the :ref:`Project Browser` and highlights the referenced resource. The reveal
action attempts to locate the resource even when the resource type is hidden
or when the file resides in a mapped source directory that is not shown.

.. figure:: images/inspector_reveal_resource.svg
   :align: center

   The ``Reveal`` button.
