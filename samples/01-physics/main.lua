require "core/game/camera"

Game = Game or {
	pw = nil,
	rw = nil,
	sg = nil,
	debug_graphics = false,
	debug_physics = false,
	camera = nil,
}

GameBase.game = Game
GameBase.game_level = "levels/test"
local cursor_modes = {"normal", "disabled"}
local cursor_mode_nxt_idx = 2

function Game.level_loaded()
	Game.pw = World.physics_world(GameBase.world)
	Game.rw = World.render_world(GameBase.world)
	Game.sg = World.scene_graph(GameBase.world)
	Game.camera = FPSCamera(GameBase.world, GameBase.camera_unit)

	-- Debug
	PhysicsWorld.enable_debug_drawing(Game.pw, debug_physics)
	RenderWorld.enable_debug_drawing(Game.rw, debug_graphics)
end

function Game.update(dt)
	-- Stop the engine when the 'ESC' key is released
	if Keyboard.released(Keyboard.button_id("escape")) then
		Device.quit()
	end

	if Keyboard.released(Keyboard.button_id("z")) then
		debug_physics = not debug_physics
		PhysicsWorld.enable_debug_drawing(Game.pw, debug_physics)
	end

	if Keyboard.released(Keyboard.button_id("x")) then
		debug_graphics = not debug_graphics
		RenderWorld.enable_debug_drawing(Game.rw, debug_graphics)
	end

	-- Spawn a sphere when left mouse button is pressed
	if Mouse.pressed(Mouse.button_id("left")) then
		local tr = SceneGraph.instance(Game.sg, Game.camera:unit())
		local pos = SceneGraph.local_position(Game.sg, tr)
		local dir = Matrix4x4.z(SceneGraph.local_pose(Game.sg, tr))
		local u1 = World.spawn_unit(GameBase.world, "units/sphere", pos)
		local a1 = PhysicsWorld.actor_instance(Game.pw, u1)
		Vector3.normalize(dir)
		PhysicsWorld.actor_add_impulse(Game.pw, a1, dir * 500.0)
	end

	-- Perform a raycast when middle mouse button is pressed
	if Mouse.pressed(Mouse.button_id("middle")) then
		local tr = SceneGraph.instance(Game.sg, Game.camera:unit())
		local pos = SceneGraph.local_position(Game.sg, tr)
		local dir = Matrix4x4.z(SceneGraph.local_pose(Game.sg, tr))
		local hit, pos, normal, time, unit, actor = PhysicsWorld.cast_ray(Game.pw, pos, dir, 100)
		if hit then
			PhysicsWorld.actor_add_impulse(Game.pw, actor, dir * 400.0)
		end
	end

	-- Toggle mouse cursor modes
	if Keyboard.released(Keyboard.button_id("space")) then
		Window.set_cursor_mode(cursor_modes[cursor_mode_nxt_idx])
		cursor_mode_nxt_idx = 1 + cursor_mode_nxt_idx % 2
	end

	-- Update camera
	local delta = Mouse.axis(Mouse.axis_id("cursor_delta"))
	Game.camera:update(dt, delta.x, delta.y)
end

function Game.render(dt)
end

function Game.shutdown()
end
