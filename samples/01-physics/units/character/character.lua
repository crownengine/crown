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

return UnitsCharacterCharacter
