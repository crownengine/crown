-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/game/camera"

GameBase = GameBase or {
	world       = nil, -- Default world
	camera_unit = nil, -- Default camera
	game        = nil, -- User Game
	game_level  = nil,
	screen_gui  = nil,
	show_help   = true,
	show_build  = false,

	_test_package = nil,
}

function GameBase.init()
	-- Create world.
	GameBase.world = Device.create_world()

	if GameBase.game and GameBase.game.init then
		GameBase.game.init()
	end

	-- Load test level if launched from Level Editor.
	if TEST then
		GameBase._test_package = Device.create_resource_package("_level_editor_test")
		ResourcePackage.load(GameBase._test_package)
		ResourcePackage.flush(GameBase._test_package)
		World.load_level(GameBase.world, "_level_editor_test")
	else
		if GameBase.game_level then
			World.load_level(GameBase.world, GameBase.game_level)
		end
	end

	-- Create default camera if it doesn't exist.
	GameBase.camera_unit = World.unit_by_name(GameBase.world, "camera")
	if GameBase.camera_unit == nil then
		GameBase.camera_unit = World.spawn_unit(GameBase.world, "core/units/camera")
		local scene_graph = World.scene_graph(GameBase.world)
		local camera_transform = SceneGraph.instance(scene_graph, GameBase.camera_unit)
		SceneGraph.set_local_position(scene_graph, camera_transform, Vector3(0, -15, 6.5))
	end

	if GameBase.game and GameBase.game.level_loaded then
		GameBase.game.level_loaded()
	end

	GameBase.screen_gui = World.create_screen_gui(GameBase.world)
end

function GameBase.update(dt)
	-- Update world
	World.update(GameBase.world, dt)

	if TEST then
		-- Stop the engine when the 'ESC' key is released.
		if Keyboard.released(Keyboard.button_id("escape")) then
			Device.quit()
		end
	end

	if GameBase.game and GameBase.game.update then
		GameBase.game.update(dt)
	end
end

function GameBase.render(dt)
	if GameBase.game and GameBase.game.render then
		GameBase.game.render(dt)
	end

	if GameBase.show_build then
		GameBase.draw_build()
	end

	Device.render(GameBase.world, GameBase.camera_unit)
end

function GameBase.shutdown()
	if GameBase.game and GameBase.game.shutdown then
		GameBase.game.shutdown()
	end

	World.destroy_gui(GameBase.world, GameBase.screen_gui)
	Device.destroy_world(GameBase.world)

	if GameBase._test_package then
		Device.destroy_resource_package(GameBase._test_package)
	end
end

function GameBase.draw_help(controls, title)
	if not GameBase.show_help then
		return
	end

	local window_w, window_h = Device.resolution()
	local line_y = window_h - 60
	local key_x = 40
	local desc_x = key_x + 180
	local title_size = 32
	local title_margin = title_size + 15
	local paragraph_size = 21
	local paragraph_margin = paragraph_size + 10
	local font = "core/game/hud/debug"
	local material = "core/game/hud/debug"
	local background_color = Color4(0, 0, 0, 200)
	local title_color = Color4(220, 220, 220, 255)
	local paragraph_color = Color4(200, 200, 200, 255)

	Gui.rect(GameBase.screen_gui, Vector2(0, 0), Vector2(window_w, window_h), background_color)
	Gui.text(GameBase.screen_gui, Vector2(key_x, line_y), title_size, title, font, material, title_color)
	line_y = line_y - title_margin

	for _, v in pairs(controls) do
		Gui.text(GameBase.screen_gui, Vector2(key_x, line_y), paragraph_size, v.key, font, material, paragraph_color)
		Gui.text(GameBase.screen_gui, Vector2(desc_x, line_y), paragraph_size, v.desc, font, material, paragraph_color)
		line_y = line_y - paragraph_margin
	end
end

function GameBase.draw_build()
	local window_w, window_h = Device.resolution()
	local font_size = 14
	local font = "core/game/hud/debug"
	local material = "core/game/hud/debug"
	local color = Color4(255, 255, 255, 160)
	local build = Device.version()
		.. " " .. Device.platform()
		.. " " .. Device.architecture() .. " "
		.. "(" .. Device.build() .. ")"

	local extents = Gui.text_extents(GameBase.screen_gui, font_size, build, font)
	local position = Vector2(window_w - extents.x - 12, 12)

	Gui.text(GameBase.screen_gui, position, font_size, build, font, material, color)
end
