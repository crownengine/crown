======================
Interacting with Units
======================

At some point you will probably need to reference units from the Lua scripts to
manipulate their properties, be notified of particular events and make them do
something interesting.

Getting Unit handles
--------------------

The simplest way of getting a handle to a unit is to spawn it directly from a
script:

.. code::

	player = World.spawn_unit(world, "units/player/player")

In most cases, however, units are not spawned directly but rather as a
consequence of loading levels in a world. You can get a table with all units
spawned in a world this way:

.. code::

	local units = World.units(world)

	for _, u in ipairs(units) do
		-- Do something with Unit u.
	end

To obtain a specific Unit by name (its name as set in the Level Editor, *not*
the unit name itself):

.. code::

	door = World.unit_by_name(world, "main_door")

The Script component
--------------------

Obtaining unit handles is useful but might not be enough. With a unit handle
alone you can modify properties but you cannot receive events.

Creating a Unit script
----------------------

Unit scripts are a particular type of scripts that can be attached to units via
a Script Component. To create a Unit script, right click on the Project Browser
and choose ``New (Unit) script``.

Crown will create a new Unit script similar to the following:

.. code::

	MyScript = MyScript or {
		data = {}
	}

	local data = MyScript.data

	function MyScript.spawned(units, world)
		if data[world] == nil then data[world] = {} end

		for uu = 1, #units do
			local unit = units[uu]

			-- Store instance-specific data.
			if data[world][unit] == nil then
				-- data[world][unit] = {}
			end

			-- Do something with the unit.
		end
	end

	function MyScript.update(world, dt)
		-- Called once per frame.
	end

	function MyScript.unspawned(units, world)
		-- Cleanup.
		for uu = 1, #units do
			if data[world][units] then
				data[world][units] = nil
			end
		end
	end

	return MyScript

Unit scripts work differently than similar solutions in other engines. Instead
of getting many individual update() calls for each individual Unit, you will
receive a single update() for *every* unit that has that particular script
attached to it.

This allows for efficient bulk updates, state sharing and it also make profiling
code easier.

Receiving collision events
--------------------------

To get physics collision notification events, implement any of the following
callbacks in your script component:

.. code::

	function MyScript.collision_begin(unit0, unit1, actor, position, normal, distance)
		-- Called when unit0 and unit1 begin touching.
	end

	function MyScript.collision_end()
		-- Called when unit0 and unit1 end touching.
	end

	function MyScript.collision_stay(unit0, unit1, actor, position, normal, distance)
		-- Called between collision_begin() and collision_end() while the units remain touching.
	end
