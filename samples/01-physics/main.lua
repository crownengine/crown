-- Copyright (c) 2012-2024 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/game/camera"

Game = Game or {
	pw = nil,
	rw = nil,
	sg = nil,
	debug_graphics = false,
	debug_physics = false,
	camera = nil,
	cursor = { modes = { "normal", "disabled"}, current_mode = 1 },
}

GameBase.game = Game
GameBase.game_level = "levels/test"

function Game.level_loaded()
	Game.pw = World.physics_world(GameBase.world)
	Game.rw = World.render_world(GameBase.world)
	Game.sg = World.scene_graph(GameBase.world)
	Game.camera = FPSCamera(GameBase.world, GameBase.camera_unit)

	-- Debug.
	PhysicsWorld.enable_debug_drawing(Game.pw, Game.debug_physics)
	RenderWorld.enable_debug_drawing(Game.rw, Game.debug_graphics)
end

function Game.update(dt)
	-- Stop the engine when the 'ESC' key is released.
	if Keyboard.released(Keyboard.button_id("escape")) then
		Device.quit()
	end

	-- Toggle debug drawing.
	if Keyboard.released(Keyboard.button_id("z")) then
		Game.debug_physics = not Game.debug_physics
		PhysicsWorld.enable_debug_drawing(Game.pw, Game.debug_physics)
	end

	if Keyboard.released(Keyboard.button_id("x")) then
		Game.debug_graphics = not Game.debug_graphics
		RenderWorld.enable_debug_drawing(Game.rw, Game.debug_graphics)
	end

	-- Shoot a sphere when left mouse button is pressed.
	if Mouse.pressed(Mouse.button_id("left")) then
		local tr = SceneGraph.instance(Game.sg, Game.camera:unit())
		local pos = SceneGraph.local_position(Game.sg, tr)
		local dir = Matrix4x4.z(SceneGraph.local_pose(Game.sg, tr))
		local u1 = World.spawn_unit(GameBase.world, "units/sphere", pos)
		local a1 = PhysicsWorld.actor_instance(Game.pw, u1)
		Vector3.normalize(dir)
		PhysicsWorld.actor_add_impulse(Game.pw, a1, dir * 500.0)
	end

	-- Perform a raycast when middle mouse button is pressed.
	if Mouse.pressed(Mouse.button_id("middle")) then
		local tr = SceneGraph.instance(Game.sg, Game.camera:unit())
		local pos = SceneGraph.local_position(Game.sg, tr)
		local dir = Matrix4x4.z(SceneGraph.local_pose(Game.sg, tr))
		local hit, pos, normal, time, unit, actor = PhysicsWorld.cast_ray(Game.pw, pos, dir, 100)
		if hit then
			PhysicsWorld.actor_add_impulse(Game.pw, actor, dir * 400.0)
		end
	end

	-- Toggle mouse cursor modes.
	if Keyboard.released(Keyboard.button_id("space")) then
		Game.cursor.current_mode = 1 + Game.cursor.current_mode % #Game.cursor.modes
		Window.set_cursor_mode(Game.cursor.modes[Game.cursor.current_mode])
	end

	-- Update camera.
	local delta = Mouse.axis(Mouse.axis_id("cursor_delta"))
	Game.camera:update(dt, delta.x, delta.y)
end

function Game.render(dt)
end

function Game.shutdown()
end
