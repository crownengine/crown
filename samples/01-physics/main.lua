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
	gui = nil,
	show_help = true,
	title = "Crown Physics Sample",
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
	Game.gui = World.create_screen_gui(GameBase.world)

	-- Debug
	PhysicsWorld.enable_debug_drawing(Game.pw, debug_physics)
	RenderWorld.enable_debug_drawing(Game.rw, debug_graphics)
end

function Game.update(dt)
	-- Stop the engine when the 'ESC' key is released.
	if Keyboard.released(Keyboard.button_id("escape")) then
		Device.quit()
	end

	-- Toggle debug drawing.
	if Keyboard.released(Keyboard.button_id("z")) then
		debug_physics = not debug_physics
		PhysicsWorld.enable_debug_drawing(Game.pw, debug_physics)
	end
	if Keyboard.released(Keyboard.button_id("x")) then
		debug_graphics = not debug_graphics
		RenderWorld.enable_debug_drawing(Game.rw, debug_graphics)
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
		Window.set_cursor_mode(cursor_modes[cursor_mode_nxt_idx])
		cursor_mode_nxt_idx = 1 + cursor_mode_nxt_idx % 2
	end

	-- Draw help.
	Game.help({
		{ key = "f1", desc = "Toggle help" },
		{ key = "w/a/s/d", desc = "Move" },
		{ key = "left click", desc = "Shoot" },
		{ key = "space", desc = "Toggle mouse lock" },
		{ key = "esc", desc = "Quit" },
		{ key = "z", desc = "Toggle physics debug" },
		{ key = "x", desc = "Toggle graphics debug" },
	})

	-- Update camera.
	local delta = Mouse.axis(Mouse.axis_id("cursor_delta"))
	Game.camera:update(dt, delta.x, delta.y)
end

function Game.help(gui)
	local win_x, win_y = Device.resolution()
	local line_y = win_y - 60
	local line_height = 10
	local key_x = 40
	local desc_x = key_x + 180
	local title_size = 32
	local font_size = 21
	local font = "core/game/hud/debug"
	local material = "core/game/hud/debug"
	local background_color = Color4(0, 0, 0, 200)
	local title_color = Color4(220, 220, 220, 255)
	local paragraph_color = Color4(200, 200, 200, 255)

	if Keyboard.pressed(Keyboard.button_id("f1")) then
		Game.show_help = not Game.show_help
	end

	if not Game.show_help then
		return
	end

	-- Draw help.
	Gui.rect(Game.gui, Vector2(0, 0), Vector2(win_x, win_y), background_color)
	Gui.text(Game.gui, Vector2(key_x, line_y), title_size, Game.title, font, material, title_color)
	line_y = line_y - title_size - line_height

	for _, v in pairs(gui) do
		Gui.text(Game.gui, Vector2(key_x, line_y), font_size, v.key, font, material, paragraph_color)
		Gui.text(Game.gui, Vector2(desc_x, line_y), font_size, v.desc, font, material, paragraph_color)
		line_y = line_y - font_size - line_height
	end
end

function Game.render(dt)
end

function Game.shutdown()
	World.destroy_gui(GameBase.world, Game.gui)
end
