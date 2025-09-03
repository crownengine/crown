-- Note: the following table must be global and uniquely named.
SlidingDoorTrigger = SlidingDoorTrigger or {
	data = {}
}

local data = SlidingDoorTrigger.data

-- Called after units are spawned into a world.
function SlidingDoorTrigger.spawned(world, units)
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
function SlidingDoorTrigger.update(world, dt)
	local world_data = data[world]

	for unit, unit_data in pairs(world_data) do
		-- Update unit.
	end
end

-- Called before units are unspawned from a world.
function SlidingDoorTrigger.unspawned(world, units)
	local world_data = data[world]

	-- Cleanup.
	for _, unit in pairs(units) do
		if world_data[unit] then
			world_data[unit] = nil
		end
	end
end

function SlidingDoorTrigger.unspawned(world, units)
end

function SlidingDoorTrigger.trigger_enter(world, trigger_unit, other_unit)
	local player = World.unit_by_name(world, "character")
	local door = World.unit_by_name(world, "trigger_door")

	if door and player and other_unit == player then
		local scene_graph = World.scene_graph(world)
		local door_transform = SceneGraph.instance(scene_graph, door)
		local door_position = SceneGraph.local_position(scene_graph, door_transform)
		local new_door_position = door_position - Vector3(2.3, 0, 0)
		SceneGraph.set_local_position(scene_graph, door_transform, new_door_position)
	end
end

function SlidingDoorTrigger.trigger_leave(world, trigger_unit, other_unit)
	local player = World.unit_by_name(world, "character")
	local door = World.unit_by_name(world, "trigger_door")

	if door and player and other_unit == player then
		local scene_graph = World.scene_graph(world)
		local door_transform = SceneGraph.instance(scene_graph, door)
		local door_position = SceneGraph.local_position(scene_graph, door_transform)
		local new_door_position = door_position + Vector3(2.3, 0, 0)
		SceneGraph.set_local_position(scene_graph, door_transform, new_door_position)
	end
end

return SlidingDoorTrigger
