-- Note: the following table must be global and uniquely named.
Bullet = Bullet or {
	data = {}
}

local data = Bullet.data

-- Called after units are spawned into a world.
function Bullet.spawned(world, units)
	if data[world] == nil then
		data[world] = {}
	end

	local world_data = data[world]

	for _, unit in pairs(units) do
		-- Store instance-specific data.
		if world_data[unit] == nil then
			world_data[unit] = { }
		end
	end
end

-- Called once per frame.
function Bullet.update(world, dt)
	local world_data = data[world]

	for unit, unit_data in pairs(world_data) do
		-- Update unit.
	end
end

-- Called before units are unspawned from a world.
function Bullet.unspawned(world, units)
	local world_data = data[world]

	-- Cleanup.
	for _, unit in pairs(units) do
		if world_data[unit] then
			world_data[unit] = nil
		end
	end
end

function Bullet.collision_begin(world, unit, other_unit, actor, other_actor, position, normal, distance)
	World.play_sound(world, "sfx/bounce", false, 1.0, 200, position)
end

return Bullet

