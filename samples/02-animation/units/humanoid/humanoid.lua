-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

-- Note: the following table must be global and uniquely named.
local Character = require "character"

Humanoid = Humanoid or {
	data = {}
}

local data = Humanoid.data

function Humanoid.spawned(world, units)
	local world_data = data[world]
	if world_data == nil then
		world_data = {}
		data[world] = world_data
	end

	for _, unit in pairs(units) do
		world_data[unit] = {
			character = Character(world, unit)
		}
	end
end

function Humanoid.update(world, dt)
	local world_data = data[world]
	local move_dx = Keyboard.button(Keyboard.button_id("d")) - Keyboard.button(Keyboard.button_id("a"))
	local move_dy = Keyboard.button(Keyboard.button_id("w")) - Keyboard.button(Keyboard.button_id("s"))
	local pad_left = Pad1.axis(Pad1.axis_id("left"))
	move_dx = move_dx + pad_left.x
	move_dy = move_dy + pad_left.y

	for unit, unit_data in pairs(world_data) do
		if Game.player == unit then
			unit_data.character:update(dt, move_dx, move_dy)
		else
			unit_data.character:update(dt, 0, 0)
		end
	end
end

function Humanoid.unspawned(world, units)
	local world_data = data[world]
	for _, unit in pairs(units) do
		world_data[unit] = nil
	end
end

return Humanoid
