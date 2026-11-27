-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/game/game"

world = world or nil
camera_unit = camera_unit or nil
screen_gui = screen_gui or nil

function init()
	-- Create world and camera
	world = Device.create_world()
	camera_unit = World.spawn_unit(world, "core/units/camera")
	screen_gui = World.create_screen_gui(world)
end

function update(dt)
	-- Advance the simulation
	World.update(world, dt)

	-- Stop the engine when the 'ESC' key is released
	if Keyboard.released(Keyboard.button_id("escape")) then
		Device.quit()
	end
end

function render(dt)
	GameBase.draw_fps(dt, screen_gui)

	-- Render the world
	Device.render(world, camera_unit)
end

function shutdown()
	-- Cleanup
	World.destroy_gui(world, screen_gui)
	Device.destroy_world(world)
end
