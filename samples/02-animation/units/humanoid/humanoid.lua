-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

-- Note: the following table must be global and uniquely named.
local Character = require "character"

Humanoid = Humanoid or {
	data = {}
}

local data = Humanoid.data
local FOOTSTEPS = {
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

function Humanoid.event(world, unit, event_name)
	if event_name ~= Device.string_id("footsteps_L") and event_name ~= Device.string_id("footsteps_R") then
		return
	end

	if Game == nil or Game.player ~= unit then
		return
	end

	local world_data = data[world]
	local unit_data = world_data and world_data[unit]
	local character = unit_data and unit_data.character
	if character == nil or not character:is_moving() then
		return
	end

	local scene_graph = World.scene_graph(world)
	local transform = SceneGraph.instance(scene_graph, unit)
	local position = SceneGraph.local_position(scene_graph, transform)
	World.play_sound(world, FOOTSTEPS[math.random(21)], false, 0.1, 50, position)
end

return Humanoid
