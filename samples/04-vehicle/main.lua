-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/game/camera"

Game = Game or {
	physics_world = nil,
	camera = nil,
	vehicle_unit = nil,
	vehicle = nil,
	debug_physics = false,
	last_input_is_pad = false
}

GameBase.game = Game
GameBase.game_level = "levels/vehicle"
GameBase.show_build = true
GameBase.touch_controls = {
	axes = { "left" },
	buttons = { "a", "b" },
}

local function k(bind)
	if Game.last_input_is_pad then
		if bind == "f1" then return "start"
		elseif bind == "w/s" then return "RT/LT"
		elseif bind == "a/d" then return "left thumb"
		elseif bind == "ctrl" then return "B"
		elseif bind == "space" then return "A"
		elseif bind == "r" then return "Y" end
	end

	return bind
end

function Game.level_loaded()
	Game.physics_world = World.physics_world(GameBase.world)
	Game.camera = FPSCamera(GameBase.world, GameBase.camera_unit)
	Game.vehicle_unit = World.unit_by_name(GameBase.world, "vehicle")
end

function Game.update(dt)
	local keyboard_used = Keyboard.any_pressed() ~= nil
	local pad_used = Pad1.any_pressed() ~= nil
	if not pad_used then
		local pad_left = Pad1.axis(Pad1.axis_id("left"))
		local pad_trigger_left = Pad1.axis(Pad1.axis_id("trigger_left"))
		local pad_trigger_right = Pad1.axis(Pad1.axis_id("trigger_right"))
		pad_used = pad_left.x ~= 0
			or pad_left.y ~= 0
			or pad_trigger_left.z ~= 0
			or pad_trigger_right.z ~= 0
	end

	if pad_used and not keyboard_used then
		Game.last_input_is_pad = true
	elseif keyboard_used and not pad_used then
		Game.last_input_is_pad = false
	end

	if Keyboard.released(Keyboard.button_id("escape")) then
		Device.quit()
	end
	if Keyboard.pressed(Keyboard.button_id("f1")) or Pad1.pressed(Pad1.button_id("start")) then
		GameBase.show_help = not GameBase.show_help
	end
	if Keyboard.pressed(Keyboard.button_id("z")) then
		Game.debug_physics = not Game.debug_physics
		PhysicsWorld.enable_debug_drawing(Game.physics_world, Game.debug_physics)
	end
	if Game.vehicle and (Keyboard.pressed(Keyboard.button_id("r")) or Pad1.pressed(Pad1.button_id("y"))) then
		Game.vehicle:reset()
	end
	World.set_listener_pose(GameBase.world, Game.camera:world_pose())

	local font = "core/game/hud/debug"
	local speed = Game.vehicle and Game.vehicle:speed() or 0
	local status = string.format("%3.0f km/h", speed * 3.6)
	Gui.text(GameBase.screen_gui, Vector2(24, 42), 22, status, font, font)

	local win_w, win_h = Device.resolution()

	local font_size = 16
	local timing = string.format("dt: %.2f ms  FPS: %.1f", dt * 1000, dt > 0 and 1 / dt or 0)
	local timing_extents = Gui.text_extents(GameBase.screen_gui, font_size, timing, font)
	Gui.text(GameBase.screen_gui, Vector2(win_w - timing_extents.x - 10, win_h - font_size - 10), font_size, timing, font, font)

	local help_scale = math.min(1, win_w / 1000, win_h / 650)
	local touch_pad = GameBase.virtual_joypad
	GameBase.draw_help({
		{ key = k("f1"), desc = "Toggle help" },
		{ key = k("w/s"), desc = "Accelerate / brake and reverse" },
		{ key = k("a/d"), desc = "Steer" },
		{ key = k("ctrl"), desc = "Brake" },
		{ key = k("space"), desc = "Handbrake" },
		{ key = k("r"), desc = "Reset vehicle" },
		{ key = k("z"), desc = "Toggle physics debug" },
		{ key = k("esc"), desc = "Quit" },
		touch_pad and touch_pad.visible and { key = "touch stick", desc = "Steer and accelerate / reverse" } or nil
	}, "Crown Vehicle Sample", help_scale)
	if GameBase.show_help then
		local credit_size = 18 * help_scale
		local credit_y = win_h - 110 * help_scale
		for _, text in ipairs({ "Car model", "BMW M3 GTR GT2", "Kevin Love SketchFab", "CC BY 4.0" }) do
			local extents = Gui.text_extents(GameBase.screen_gui, credit_size, text, font)
			Gui.text(GameBase.screen_gui, Vector2(win_w - extents.x - 40 * help_scale, credit_y), credit_size, text, font, font)
			credit_y = credit_y - 26 * help_scale
		end
	end
end

function Game.render(dt)
end

function Game.shutdown()
end
