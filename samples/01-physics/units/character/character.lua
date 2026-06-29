-- Note: the following table must be global and uniquely named.
require "core/game/camera"

local Character = require "character"

UnitsCharacterCharacter = UnitsCharacterCharacter or {
	data = {}
}

local data = UnitsCharacterCharacter.data
local PAD_LOOK_SPEED = 32

function UnitsCharacterCharacter.spawned(world, units)
	local world_data = data[world]
	if world_data == nil then
		world_data = {}
		data[world] = world_data
	end

	for _, unit in pairs(units) do
		world_data[unit] = world_data[unit] or {
			character = Game.camera and Character(world, unit, Game.camera) or nil
		}
	end
end

function UnitsCharacterCharacter.update(world, dt)
	local world_data = data[world]
	if world_data == nil then
		return
	end

	local move_dx = Keyboard.button(Keyboard.button_id("d")) - Keyboard.button(Keyboard.button_id("a"))
	local move_dy = Keyboard.button(Keyboard.button_id("w")) - Keyboard.button(Keyboard.button_id("s"))
	local look_dx = 0
	local look_dy = 0

	if Game and Game.cursor_disabled then
		local cursor_delta = Mouse.axis(Mouse.axis_id("cursor_delta"))
		look_dx = look_dx + cursor_delta.x
		look_dy = look_dy + cursor_delta.y
	end

	local pad_left = Pad1.axis(Pad1.axis_id("left"))
	local pad_right = Pad1.axis(Pad1.axis_id("right"))
	move_dx = move_dx + pad_left.x
	move_dy = move_dy + pad_left.y
	look_dx = look_dx + pad_right.x * PAD_LOOK_SPEED
	look_dy = look_dy - pad_right.y * PAD_LOOK_SPEED

	local jump_pressed = Keyboard.pressed(Keyboard.button_id("space"))
		or Pad1.pressed(Pad1.button_id("a"))
	local run_pressed = Keyboard.button(Keyboard.button_id("shift_left")) ~= 0
		or Pad1.button(Pad1.button_id("b")) ~= 0
	local crouch_pressed = Keyboard.button(Keyboard.button_id("ctrl_left")) ~= 0
		or Keyboard.button(Keyboard.button_id("ctrl_right")) ~= 0
		or Pad1.button(Pad1.button_id("shoulder_left")) ~= 0
	local toggle_camera_pressed = Keyboard.released(Keyboard.button_id("minus"))
		or Pad1.pressed(Pad1.button_id("thumb_right"))

	for _, unit_data in pairs(world_data) do
		if Game.camera and (math.abs(look_dx) > 0.0001 or math.abs(look_dy) > 0.0001) then
			Game.camera:rotate(dt, look_dx, look_dy)
		end

		if unit_data.character then
			if toggle_camera_pressed then
				unit_data.character:toggle_camera_mode()
			end

			unit_data.character:update(dt, move_dx, move_dy, jump_pressed, run_pressed, crouch_pressed)
		end
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
		local world_data = data[world]
		local unit_data = world_data and world_data[unit]
		local character = unit_data and unit_data.character
		if character and not character:is_grounded() then
			return
		end

		local scene_graph = World.scene_graph(world)
		local transform = SceneGraph.instance(scene_graph, unit)
		local position = SceneGraph.local_position(scene_graph, transform)
		World.play_sound(world, footsteps[math.random(21)], false, 0.1, 50, position)
	end
end

return UnitsCharacterCharacter
