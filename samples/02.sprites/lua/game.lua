objects = {}
max_objects = 70

function init()
	-- Set the title of the main window
	Window.set_title("Hello world!")

	-- Create the resource package
	package = Device.create_resource_package("level")
	-- Load resource package
	ResourcePackage.load(package)
	-- Wait for completion
	ResourcePackage.flush(package)

	-- Create world
	world = Device.create_world()

	-- Spawn camera
	camera_unit = World.spawn_unit(world, "units/camera")
	camera = Unit.camera(camera_unit, "camera")

	-- Setup camera
	Camera.set_near_clip_distance(camera, 0.01)
	Camera.set_far_clip_distance(camera, 1000)
	Camera.set_projection_type(camera, Camera.ORTHOGRAPHIC)
	Camera.set_orthographic_metrics(camera, -6, 6, -6 / 1.6, 6 / 1.6)
	Camera.set_viewport_metrics(camera, 0, 0, 1000, 625)
	Unit.set_local_position(camera_unit, Vector3(0, 0, 1))

	local unit_names = { "units/star", "units/circle", "units/pentagon", "units/square" }
	-- Spawn units randomly
	math.randomseed(os.time())
	for i = 1, max_objects do
		objects[i] = {
						unit = World.spawn_unit(world, unit_names[math.random(#unit_names)],
									Vector3(math.random(-6, 6), math.random(-6 / 1.6, 6 / 1.6), 0)),
						rot_speed = math.random(1.5),
						cur_rot = math.random(3.14)
					}
	end
end

function frame(dt)
	-- Stop the engine when the 'ESC' key is released
	if Keyboard.button_released(Keyboard.ESCAPE) then
		Device.stop()
	end

	for i = 1, max_objects do
		local obj = objects[i]
		local r = obj.rot_speed
		obj.cur_rot = obj.cur_rot + r * dt

		Unit.set_local_rotation(obj.unit, Quaternion(Vector3(0, 0, 1), obj.cur_rot))
	end

	Device.render_world(world, camera, dt)
end

function shutdown()
	-- Destroy all units
	for i = 1, max_objects do
		World.destroy_unit(world, objects[i].unit)
	end
	World.destroy_unit(world, camera_unit)

	-- Destroy world
	Device.destroy_world(world)
	-- Unload package
	ResourcePackage.unload(package)
	Device.destroy_resource_package(package)
end