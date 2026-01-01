-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/game/camera"

Game = Game or {
	dl = nil
}

GameBase.game = Game
GameBase.game_level = nil

function Game.init()
	-- Override default camera's settings
	local camera = World.camera_instance(GameBase.world, GameBase.camera_unit)
	World.camera_set_projection_type(GameBase.world, camera, "orthographic")
	World.camera_set_orthographic_size(GameBase.world, camera, 7)

	local scene_graph = World.scene_graph(GameBase.world)
	local camera_transform = SceneGraph.instance(scene_graph, GameBase.camera_unit)
	SceneGraph.set_local_position(scene_graph, camera_transform, Vector3(0, -1, 0))

	-- Create debug line to draw joypad's buttons
	Game.dl = World.create_debug_line(GameBase.world, true)
end

function Game.update(dt)
	-- Stop the engine when the 'ESC' key is released
	if Keyboard.released(Keyboard.button_id("escape")) then
		Device.quit()
	end

	local function axes(center, pad_axis)
		local c = center
		local a = pad_axis

		local r = c + a
		r.z = 0.0
		local k = c + a
		k.x = c.x - 1.5
		k.y = c.y + k.z
		k.z = 0.0

		return r, k
	end

	local c = Vector3(-2, 0, 0)
	local d = Vector3( 2, 0, 0)
	local a = Pad1.axis(Pad1.axis_id("left")) + Pad1.axis(Pad1.axis_id("trigger_left"))
	local b = Pad1.axis(Pad1.axis_id("right")) + Pad1.axis(Pad1.axis_id("trigger_right"))
	local r, k = axes(c, a)
	local t, l = axes(d, b)

	-- Left thumb
	DebugLine.add_circle(Game.dl, c, 1.0, Vector3.forward(), Color4.red())
	DebugLine.add_circle(Game.dl, r, 0.1, Vector3.forward(), Pad1.button(Pad1.button_id("thumb_left")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, k, 0.1, Vector3.forward(), Color4.red())

	-- Right thumb
	DebugLine.add_circle(Game.dl, d, 1.0, Vector3.forward(), Color4.red())
	DebugLine.add_circle(Game.dl, t, 0.1, Vector3.forward(), Pad1.button(Pad1.button_id("thumb_right")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, l, 0.1, Vector3.forward(), Color4.red())

	-- Left/right shoulder
	DebugLine.add_circle(Game.dl, Vector3(0, 0, 1.5) + c, 0.1, Vector3.forward(), Pad1.button(Pad1.button_id("shoulder_left")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, Vector3(0, 0, 1.5) + d, 0.1, Vector3.forward(), Pad1.button(Pad1.button_id("shoulder_right")) == 1 and Color4.green() or Color4.red())

	-- Dpad
	local upos = Vector3( 0, 0,  1)
	local dpos = Vector3( 0, 0, -1)
	local lpos = Vector3(-1, 0,  0)
	local rpos = Vector3( 1, 0,  0)
	local orig = Vector3(-4, 0,  3)
	DebugLine.add_circle(Game.dl, orig + upos, 0.5, Vector3.forward(), Pad1.button(Pad1.button_id("up")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, orig + dpos, 0.5, Vector3.forward(), Pad1.button(Pad1.button_id("down")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, orig + lpos, 0.5, Vector3.forward(), Pad1.button(Pad1.button_id("left")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, orig + rpos, 0.5, Vector3.forward(), Pad1.button(Pad1.button_id("right")) == 1 and Color4.green() or Color4.red())

	-- Buttons
	local xpos = Vector3(-1, 0,  0)
	local ypos = Vector3( 0, 0,  1)
	local bpos = Vector3( 1, 0,  0)
	local apos = Vector3( 0, 0, -1)
	local o = Vector3(4, 0, 3)
	DebugLine.add_circle(Game.dl, o + apos, 0.5, Vector3.forward(), Pad1.button(Pad1.button_id("a")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, o + bpos, 0.5, Vector3.forward(), Pad1.button(Pad1.button_id("b")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, o + xpos, 0.5, Vector3.forward(), Pad1.button(Pad1.button_id("x")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, o + ypos, 0.5, Vector3.forward(), Pad1.button(Pad1.button_id("y")) == 1 and Color4.green() or Color4.red())

	-- Extra
	local start_pos = Vector3( 1, 0, 0)
	local back_pos  = Vector3(-1, 0, 0)
	local guide_pos = Vector3( 0, 0, 0)
	local hh = Vector3(0, 0, 2)

	DebugLine.add_circle(Game.dl, hh + start_pos, 0.3, Vector3.forward(), Pad1.button(Pad1.button_id("start")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, hh + back_pos, 0.3, Vector3.forward(), Pad1.button(Pad1.button_id("back")) == 1 and Color4.green() or Color4.red())
	DebugLine.add_circle(Game.dl, hh + guide_pos, 0.4, Vector3.forward(), Pad1.button(Pad1.button_id("guide")) == 1 and Color4.green() or Color4.red())

	DebugLine.submit(Game.dl)
	DebugLine.reset(Game.dl)
end

function Game.render(dt)
end

function Game.shutdown()
	World.destroy_debug_line(GameBase.world, Game.dl)
end
