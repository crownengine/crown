timer = 0.0
cursor = false
world = nil
camera = nil
up_pressed = false
down_pressed = false
left_pressed = false
right_pressed = false
camera_size = 1.0

ovest = -10
east = 10
north = 10 / 1.6
south = -10 / 1.6
spd_x = 0
spd_y = 0

footsteps = nil
speed = 12

function spawn_obstacle(pos)
	if math.random(0, 10) >= 0 and math.random(0, 10) <= 3 then
		World.spawn_unit(world, "box", Vector3(pos, -2, 0))
	elseif math.random(0, 10) > 3 and math.random(0, 10) <=7 then
		World.spawn_unit(world, "box", Vector3(pos, 6, 0))
	else
		World.spawn_unit(world, "box", Vector3(pos, 3, 0))
	end
end

function spawn_obstacles(dim)
	for t = 0, dim, 15 do
		spawn_obstacle(t)
	end
end

function update_player(world, unit, dt)

	-- 3 m/s
	-- speed = speed + 0.1 * dt;

	if (Keyboard.button_pressed(Keyboard.w)) then up_pressed = true end
	if (Keyboard.button_pressed(Keyboard.s)) then down_pressed = true end
	if (Keyboard.button_pressed(Keyboard.a)) then left_pressed = true end
	if (Keyboard.button_pressed(Keyboard.d)) then right_pressed = true end

	if (Keyboard.button_released(Keyboard.w)) then up_pressed = false end
	if (Keyboard.button_released(Keyboard.s)) then down_pressed = false end
	if (Keyboard.button_released(Keyboard.a)) then left_pressed = 	false end
	if (Keyboard.button_released(Keyboard.d)) then right_pressed = 	false end

	if (Touch.pointer_down(1)) then up_pressed = true end
	if (Touch.pointer_up(1)) then up_pressed = false end


	contr = Unit.controller(unit)

	if up_pressed then
		spd_y = speed * dt
	end
	if right_pressed then
		spd_x = speed * dt
	end
	if left_pressed then
		spd_x = -speed * dt
	end

	if not left_pressed and not right_pressed then
		spd_x = 0
	end

	if not up_pressed and not down_pressed then
		spd_y = 0
	end
	Controller.move(contr, Vector3(spd_x, spd_y, 0.0))

	spd_y = spd_y - 0.5 * dt

	-- local k = 0
	-- if spd_x > 0.0 then k = -1
	-- elseif spd_x < 0.0 then k = 1 end

	-- spd_x = spd_x + k * dt
	-- if spd_x < 0.001 and spd_x > -0.001 then spd_x = 0.0 end
end

function update_camera(player_unit, camera_unit, camera)
	local player_pos = Unit.world_position(player_unit, 0)
	Camera.set_local_position(camera, camera_unit, Vector3(Vector3.x(player_pos), -south / 2.5, 1))
end

function init()
	-- -- Set the title of the main window
	Window.set_title("Hello world!")
	-- Window.show_cursor(cursor)
	-- -- Window.minimize();

	package = Device.create_resource_package("level")
	ResourcePackage.load(package)
	ResourcePackage.flush(package)

	world = Device.create_world()

	-- Spawn camera
	camera_unit = World.spawn_unit(world, "camera")
	camera = Unit.camera(camera_unit, "camera")

	-- Spawn terrain
	terrain = World.spawn_unit(world, "terrain", Vector3(0, -3.75, 0), Quaternion(Vector3(0, 0, 1), 1.57))

	-- Spawn bounce
	bounce = World.spawn_unit(world, "bounce")

	sun = World.spawn_unit(world, "sun", Vector3(5, 3, 0))

	spawn_obstacles(400)

	Camera.set_near_clip_distance(camera, 0.01)
	Camera.set_far_clip_distance(camera, 1000)
	Camera.set_projection_type(camera, Camera.ORTHOGRAPHIC)
	Camera.set_orthographic_metrics(camera, ovest, east, south, north)
	Camera.set_viewport_metrics(camera, 0, 0, 1000, 625)

	Camera.set_local_position(camera, camera_unit, Vector3(0, 0, 1))
end

function frame(dt)
	Device.update_world(world, dt)

	-- Update window's viewport
	win_w, win_h = Window.get_size()
	Camera.set_viewport_metrics(camera, 0, 0, win_w, win_h)

	-- Stop the engine when the 'ESC' key is released
	if Keyboard.button_released(Keyboard.ESCAPE) then
		Device.stop()
	end

	-- -- Spawn balls when SPACE pressed
	-- if Keyboard.button_pressed(Keyboard.SPACE) then
	-- 	World.spawn_unit(world, "b")
	-- end

	-- Play gunshoot on left click
	if Mouse.button_pressed(Mouse.LEFT) then
		World.play_sound(world, "sounds/gunshoot")
	end

	-- Print FPS on window title bar
	timer = timer + Device.last_delta_time()
		if (timer >= 5) then
		Window.set_title(1.0 / Device.last_delta_time())
		timer = 0.0
	end

	-- Update the player
	update_player(world, bounce, dt)

	-- Update the camera
	update_camera(bounce, camera_unit, camera)

	-- Render world
	Device.render_world(world, camera)
end

function shutdown()
	ResourcePackage.unload(package)
	Device.destroy_resource_package(package)
	Device.destroy_world(world)
end
