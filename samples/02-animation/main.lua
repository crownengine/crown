require "core/game/camera"

Game = Game or {}

Game = {
	pw = nil,
	rw = nil,
	sg = nil,
	sm = nil,
	camera = nil,
	player = nil,
	players = {},
	player_i = 1,
	fullscreen = false
}

GameBase.game = Game
GameBase.game_level = "game"

function Game.level_loaded()
	Device.enable_resource_autoload(true)

	Game.pw = World.physics_world(GameBase.world)
	Game.rw = World.render_world(GameBase.world)
	Game.sg = World.scene_graph(GameBase.world)
	Game.sm = World.animation_state_machine(GameBase.world)

	-- Spawn camera
	local camera_unit = World.spawn_unit(GameBase.world, "core/units/camera")
	World.camera_set_orthographic_size(GameBase.world, camera_unit, 540/2/32)
	World.camera_set_projection_type(GameBase.world, camera_unit, "orthographic")
	SceneGraph.set_local_position(Game.sg, camera_unit, Vector3(0, 8, 0))
	SceneGraph.set_local_rotation(Game.sg, camera_unit, Quaternion.from_axis_angle(Vector3.right(), 90*(math.pi/180.0)))

	GameBase.game_camera = camera_unit

	-- Spawn characters
	Game.players[1] = World.spawn_unit(GameBase.world, "units/soldier", Vector3(-2, 0, 0))
	Game.players[2] = World.spawn_unit(GameBase.world, "units/princess", Vector3(2, 0, 0))
	Game.player = Game.players[1]
end

function Game.update(dt)
	-- Stop the engine when the 'ESC' key is released
	if Keyboard.released(Keyboard.button_id("escape")) then
		Device.quit()
	end

	-- Toggle fullscreen
	if Keyboard.released(Keyboard.button_id("0")) then
		Game.fullscreen = not Game.fullscreen
		Window.set_fullscreen(Game.fullscreen)
	end

	-- Cycle through characters
	if Pad1.pressed(Pad1.button_id("shoulder_right")) or Keyboard.pressed(Keyboard.button_id("j")) then
		AnimationStateMachine.trigger(Game.sm, Game.player, "idle")
		Game.player_i = Game.player_i % #Game.players + 1
		Game.player = Game.players[Game.player_i]
	end

	-- Player movement
	local function swap_yz(vector3_xy)
		return Vector3(vector3_xy.x, 0, vector3_xy.y)
	end

	-- Read direction from joypad
	local pad_dir = Pad1.axis(Pad1.axis_id("left"))
	-- Add keyboard contribution
	if pad_dir.x == 0.0 and pad_dir.y == 0.0 then
		pad_dir.x = pad_dir.x + Keyboard.button(Keyboard.button_id("d")) - Keyboard.button(Keyboard.button_id("a"))
		pad_dir.y = pad_dir.y + Keyboard.button(Keyboard.button_id("w")) - Keyboard.button(Keyboard.button_id("s"))
	end
	-- Compute new player position
	local player_speed = 4
	local player_position = SceneGraph.local_position(Game.sg, Game.player)
	SceneGraph.set_local_position(Game.sg, Game.player, player_position + swap_yz(pad_dir)*player_speed*dt)

	-- Sprite depth is proportional to its Z position
	for i=1, #Game.players do
		local pos = SceneGraph.local_position(Game.sg, Game.players[i])
		local depth = math.floor(1000 + (1000 - 32*pos.z))
		RenderWorld.sprite_set_depth(Game.rw, Game.players[i], depth)
	end

	if pad_dir.x ~= 0.0 or pad_dir.y ~= 0.0 then
		local speed_x = AnimationStateMachine.variable_id(Game.sm, Game.player, "speed_x")
		local speed_y = AnimationStateMachine.variable_id(Game.sm, Game.player, "speed_y")
		local speed   = AnimationStateMachine.variable_id(Game.sm, Game.player, "speed")
		AnimationStateMachine.set_variable(Game.sm, Game.player, speed_x, pad_dir.x)
		AnimationStateMachine.set_variable(Game.sm, Game.player, speed_y, pad_dir.y)
		AnimationStateMachine.set_variable(Game.sm, Game.player, speed, math.max(0.2, Vector3.length(pad_dir)))
		AnimationStateMachine.trigger(Game.sm, Game.player, "run")
	else
		AnimationStateMachine.trigger(Game.sm, Game.player, "idle")
	end
end

function Game.render(dt)
end

function Game.shutdown()
end
