require "core/game/camera"

GameBase = GameBase or {
	world       = nil, -- Default world
	camera_unit = nil, -- Default camera
	game        = nil, -- User Game
	game_level  = nil,

	_test_package = nil,
}

function GameBase.init()
	-- Create world
	GameBase.world = Device.create_world()

	-- Create default camera
	GameBase.camera_unit = World.spawn_unit(GameBase.world, "core/units/camera")
	local scene_graph = World.scene_graph(GameBase.world)
	local camera_transform = SceneGraph.instance(scene_graph, GameBase.camera_unit)
	SceneGraph.set_local_position(scene_graph, camera_transform, Vector3(0, 6.5, -30))

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

	if GameBase.game and GameBase.game.update then
		GameBase.game.update(dt)
	end
end

function GameBase.render(dt)
	if GameBase.game and GameBase.game.render then
		GameBase.game.render(dt)
	end

	Device.render(GameBase.world, GameBase.camera_unit)
end

function GameBase.shutdown()
	if GameBase.game and GameBase.game.shutdown then
		GameBase.game.shutdown()
	end

	Device.destroy_world(GameBase.world)

	if GameBase._test_package then
		Device.destroy_resource_package(GameBase._test_package)
	end
end
