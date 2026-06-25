-- Note: the following table must be global and uniquely named.
UnitsCharacterCharacter = UnitsCharacterCharacter or {
	data = {}
}

local data = UnitsCharacterCharacter.data

function UnitsCharacterCharacter.spawned(world, units)
	local world_data = data[world]
	if world_data == nil then
		world_data = {}
		data[world] = world_data
	end

	for _, unit in pairs(units) do
		world_data[unit] = world_data[unit] or {}
	end
end

function UnitsCharacterCharacter.unspawned(world, units)
	local world_data = data[world]
	if world_data == nil then
		return
	end

	for _, unit in pairs(units) do
		world_data[unit] = nil
	end
end

function UnitsCharacterCharacter.mover_actor_collision()
end

function UnitsCharacterCharacter.mover_mover_collision(_, mover_unit, other_mover_unit, mover, other_mover)
end

function UnitsCharacterCharacter.event(world, unit, event_name)
	local footsteps = {
		"sfx/footsteps/floor/floor_001",
		"sfx/footsteps/floor/floor_002",
		"sfx/footsteps/floor/floor_003",
		"sfx/footsteps/floor/floor_004",
		"sfx/footsteps/floor/floor_005",
		"sfx/footsteps/floor/floor_006",
		"sfx/footsteps/floor/floor_007",
		"sfx/footsteps/floor/floor_008",
		"sfx/footsteps/floor/floor_009",
		"sfx/footsteps/floor/floor_010",
		"sfx/footsteps/floor/floor_011",
		"sfx/footsteps/floor/floor_012",
		"sfx/footsteps/floor/floor_013",
		"sfx/footsteps/floor/floor_014",
		"sfx/footsteps/floor/floor_015",
		"sfx/footsteps/floor/floor_016",
		"sfx/footsteps/floor/floor_017",
		"sfx/footsteps/floor/floor_018",
		"sfx/footsteps/floor/floor_019",
		"sfx/footsteps/floor/floor_020",
		"sfx/footsteps/floor/floor_021",
	}

	if event_name == Device.string_id("footsteps_L") or event_name == Device.string_id("footsteps_R") then
		local scene_graph = World.scene_graph(world)
		local transform = SceneGraph.instance(scene_graph, unit)
		local position = SceneGraph.local_position(scene_graph, transform)
		World.play_sound(world, footsteps[math.random(21)], false, 0.1, 50, position)
	end
end

return UnitsCharacterCharacter
