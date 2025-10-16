============================
Objects binding and lifetime
============================

Crown has a native runtime written in C++. To guarantee good performance and
ease of use, we employ a number of different techniques when binding C-side
objects to the Lua environment, depending on their type and lifetime.

Light userdata binding
----------------------

Most objects are exposed to Lua via Light userdata. Light userdata objects are
owned by the C side and do not require garbage collection.

One disadvantage is that individual Light userdata objects cannot have a
metatable, so the regular object notation is not available when calling them:

.. code:: lua

	world:spawn_unit("units/grass")

Instead you must explicitly look up the function in the World table and pass
`world` as a parameter:

.. code:: lua

	World.spawn_unit(world, "units/grass")

Full userdata binding
---------------------

Full userdata objects are owned by the Lua side, they are heap-allocated and
subject to gargbage collection. They support metatables and thus offer object
notation and operator overloading.

We make sporadic use of full userdata, due to their runtime overhead. One such
example are the "boxed" versions of regular math objects (Vector3Box,
Matrix4x4Box etc).

Singleton objects
-----------------

Objects which only have a single instance of them in the runtime are called
singletons. Singletons are typically created at program start and live until the
runtime exits.

Since they always refer to the same instance, you don't need to specify it when
calling their functions:

.. code:: lua

	Device.version()

Temporary objects
-----------------

Sometimes you want an object to be owned by the Lua side, but without the
performance implications of a garbage-collected Full userdata object. Vectors
and matrices are a perfect example of that.

Crown uses a fixed-size memory pool to allocate temporary objects from. The pool
is reset at each frame so temporaries are only valid in the frame they are
created.

To store a temporary Vector3 for use in later frames, you need to copy its value
into something else with a longer lifespan, for example, you could "box" it in a
Vector3Box:

.. code:: lua

	local pos = Vector3Box()

	function update(dt)
		local p = SceneGraph.local_position(...)
		pos:store(p)
	end

And later retrieve its value with:

.. code:: lua

	local p = pos:unbox()

Crown will report an error when it detects a stale temporary object that is used
across-frames.

If you have particularly long computations, you may run out of temporary
objects. This could happen when doing math inside loops for example. You can use
``Device.temp_count()`` and ``Device.set_temp_count()`` to save and restore the
number of temporary objects used before and after computations:

.. code:: lua

	for i, v in pairs(objects) do
		local nv, nq, nm = Device.temp_count()

		...

		Device.set_temp_count(nv, nq, nm)
	end

