require "core/game/camera"

GameBase = GameBase or {}

GameBase.data = {
	move = false,

	world       = nil, -- Default world
	camera      = nil, -- Default camera
	game        = nil, -- User Game
	game_camera = nil,
	game_level  = nil,
}

function GameBase.init()
	Device.enable_resource_autoload(true)

	-- Create world
	GameBase.world = Device.create_world()

	-- Craete default camera
	local camera_unit = World.spawn_unit(GameBase.world, "core/units/camera")
	local scene_graph = World.scene_graph(GameBase.world)
	SceneGraph.set_local_position(scene_graph, camera_unit, Vector3(0, 6.5, -30))
	GameBase.camera = FPSCamera(GameBase.world, camera_unit)

	local level = TEST and "_level_editor_test" or GameBase.game_level
	if level then
		World.load_level(GameBase.world, level)
	end

	if GameBase.game and GameBase.game.level_loaded then
		GameBase.game.level_loaded()
	end
end

function GameBase.update(dt)
	-- Update world
	World.update(GameBase.world, dt)

	if TEST then
		-- Stop the engine when the 'ESC' key is released
		if Keyboard.released(Keyboard.button_id("escape")) then
			Device.quit()
		end
	end

	if not GameBase.game_camera then
		local delta = Vector3.zero()
		if Mouse.pressed(Mouse.button_id("right")) then move = true end
		if Mouse.released(Mouse.button_id("right")) then move = false end
		if move then delta = Mouse.axis(Mouse.axis_id("cursor_delta")) end
		GameBase.camera:update(dt, delta.x, delta.y)
	end

	if GameBase.game and GameBase.game.update then
		GameBase.game.update(dt)
	end
end

function GameBase.render(dt)
	if GameBase.game and GameBase.game.render then
		GameBase.game.render(dt)
	end

	Device.render(GameBase.world, GameBase.game_camera or GameBase.camera:unit())
end

function GameBase.shutdown()
	if GameBase.game and GameBase.game.shutdown then
		GameBase.game.shutdown()
	end

	Device.destroy_world(GameBase.world)
end
