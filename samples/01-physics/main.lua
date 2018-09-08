require "core/game/camera"

Game = Game or {}

Game = {
	pw = nil,
	rw = nil,
	sg = nil,
	debug_graphics = false,
	debug_physics = false,
	camera = nil,
}

GameBase.game = Game
GameBase.game_level = "test"

function Game.level_loaded()
	Device.enable_resource_autoload(true)

	Game.pw = World.physics_world(GameBase.world)
	Game.rw = World.render_world(GameBase.world)
	Game.sg = World.scene_graph(GameBase.world)

	-- Spawn camera
	local camera_unit = World.spawn_unit(GameBase.world, "core/units/camera")
	SceneGraph.set_local_position(Game.sg, camera_unit, Vector3(0, 6.5, -30))

	GameBase.game_camera = camera_unit

	Game.camera = FPSCamera(GameBase.world, camera_unit)

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
		local pos = SceneGraph.local_position(Game.sg, Game.camera:unit())
		local dir = Matrix4x4.z(SceneGraph.local_pose(Game.sg, Game.camera:unit()))
		local u1 = World.spawn_unit(GameBase.world, "sphere", pos)
		local a1 = PhysicsWorld.actor_instances(Game.pw, u1)
		Vector3.normalize(dir)
		PhysicsWorld.actor_add_impulse(Game.pw, a1, dir * 500.0)
	end

	-- Perform a raycast when middle mouse button is pressed
	if Mouse.pressed(Mouse.button_id("middle")) then
		local pos = SceneGraph.local_position(Game.sg, Game.camera:unit())
		local dir = Matrix4x4.z(SceneGraph.local_pose(Game.sg, Game.camera:unit()))
		local hit, pos, normal, time, unit, actor = PhysicsWorld.cast_ray(Game.pw, pos, dir, 100)
		if hit then
			PhysicsWorld.actor_add_impulse(Game.pw, actor, dir * 400.0)
		end
	end

	-- Update camera
	local delta = Vector3.zero()
	if Mouse.button(Mouse.button_id("right")) > 0 then
		delta = Mouse.axis(Mouse.axis_id("cursor_delta"))
	end
	Game.camera:update(dt, delta.x, delta.y)
end

function Game.render(dt)
end

function Game.shutdown()
end
