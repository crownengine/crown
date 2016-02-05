world = world or nil
camera = camera or nil

function init()
	-- Set the title of the main window
	Window.set_title("00-hello-world")

	-- Create world and camera
	world = Device.create_world()
	local camera_unit = World.spawn_unit(world, "camera")
	camera = World.camera(world, camera_unit)
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
	-- Render the world
	Device.render(world, camera)
end

function shutdown()
	-- Cleanup
	Device.destroy_world(world)
end
