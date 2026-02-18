======================
Interacting with Units
======================

At some point you will probably need to reference units from the Lua scripts to
manipulate their properties, be notified of particular events and make them do
something interesting.

.. _getting_unit_handles:

Getting Unit handles
--------------------

The simplest way of getting a handle to a unit is to spawn it directly from a
script:

.. code:: lua

	player = World.spawn_unit(world, "units/player/player")

In most cases, however, units are not spawned directly but rather as a
consequence of loading levels in a world. You can get a table with all units
spawned in a world this way:

.. code:: lua

	local units = World.units(world)

	for _, u in ipairs(units) do
		-- Do something with Unit u.
	end

To obtain a specific Unit by name (its name as set in the :ref:`Level Editor
<Renaming objects>`, *not* the unit name itself):

.. code:: lua

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

.. code:: lua

	MyScript = MyScript or {
		data = {}
	}

	local data = MyScript.data

	-- Called after units are spawned into a world.
	function MyScript.spawned(world, units)
		if data[world] == nil then
			data[world] = {}
		end

		local world_data = data[world]

		for _, unit in pairs(units) do
			-- Store instance-specific data.
			if world_data[unit] == nil then
				world_data[unit] = {}
			end
		end
	end

	-- Called once per frame.
	function MyScript.update(world, dt)
		local world_data = data[world]

		for unit, unit_data in pairs(world_data) do
			-- Update unit.
		end
	end

	-- Called before units are unspawned from a world.
	function MyScript.unspawned(world, units)
		local world_data = data[world]

		-- Cleanup.
		for _, unit in pairs(units) do
			if world_data[unit] then
				world_data[unit] = nil
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

.. code:: lua

	function MyScript.collision_begin(world, unit, other_unit, actor, other_actor, position, normal, distance)
		-- Called when unit and other_unit begin touching.
	end

	function MyScript.collision_end(world, unit, other_unit)
		-- Called when unit and other_unit end touching.
	end

	function MyScript.collision_stay(world, unit, other_unit, actor, other_actor, position, normal, distance)
		-- Called between collision_begin() and collision_end() while the units remain touching.
	end

Triggers
--------

Units whose actor is configured as a trigger (actor class 'trigger') will not
receive regular collision events, instead, they will receive trigger events:

.. code:: lua

	function MyScript.trigger_enter(world, trigger_unit, other_unit)
		-- Called when other_unit begins touching trigger_unit.
	end

	function MyScript.trigger_leave(world, trigger_unit, other_unit)
		-- Called when other_unit ends touching trigger_unit.
	end
