require "core/lua/class"

Camera = class(Camera)

function Camera:init(world, unit)
	self._world = world
	self._unit = unit
	self._translation_speed = 20
	self._rotation_speed = 0.14
	self._perspective_local_pose = Matrix4x4Box(Matrix4x4.identity())
end

function Camera:unit()
	return self._unit;
end

function Camera:camera()
	return World.camera_instances(self._world, self._unit)
end

function Camera:local_pose()
	local sg = World.scene_graph(self._world)
	return SceneGraph.local_pose(sg, self._unit)
end

function Camera:is_orthographic()
	return World.camera_projection_type(self._world, self._unit) == "orthographic"
end

function Camera:mouse_wheel(delta)
	self._translation_speed = math.max(0.001, self._translation_speed + delta * 0.2)
end

function Camera:camera_ray(x, y)
	local near = World.camera_screen_to_world(self._world, self._unit, Vector3(x, y, 0))
	local far = World.camera_screen_to_world(self._world, self._unit, Vector3(x, y, 1))
	local dir = World.camera_projection_type(self._world, self._unit) == "orthographic"
		and Matrix4x4.z(self:local_pose())
		or Vector3.normalize(far - near)
	return near, dir
end

function Camera:set_perspective()
	if not self:is_orthographic() then
		return
	end

	local sg = World.scene_graph(self._world)
	SceneGraph.set_local_pose(sg, self._unit, self._perspective_local_pose:unbox())
	World.camera_set_projection_type(self._world, self._unit, "perspective")
end

function Camera:set_orthographic(world_center, world_radius, dir, up)
	if not self:is_orthographic() then
		self._perspective_local_pose:store(self:local_pose())
	end

	local sg = World.scene_graph(self._world)
	SceneGraph.set_local_rotation(sg, self._unit, Quaternion.look(dir, up))
	SceneGraph.set_local_position(sg, self._unit, world_center - dir * math.abs(Vector3.dot(dir, world_radius)))
	World.camera_set_orthographic_size(self._world, self._unit, 10)
	World.camera_set_projection_type(self._world, self._unit, "orthographic")
end

function Camera:screen_length_to_world_length(position, length)
	local right = Matrix4x4.x(self:local_pose())
	local a = World.camera_world_to_screen(self._world, self._unit, position)
	local b = World.camera_world_to_screen(self._world, self._unit, position + right)
	return length / Vector3.distance(a, b)
end

function Camera:update(dt, dx, dy, keyboard, mouse)
	local sg = World.scene_graph(self._world)

	local camera_local_pose = SceneGraph.local_pose(sg, self._unit)
	local camera_right_vector = Matrix4x4.x(camera_local_pose)
	local camera_up_vector = Matrix4x4.y(camera_local_pose)
	local camera_view_vector = Matrix4x4.z(camera_local_pose)
	local camera_position = Matrix4x4.translation(camera_local_pose)
	local camera_rotation = Matrix4x4.rotation(camera_local_pose)

	-- Rotation
	if dx ~= 0 or dy ~= 0 then
		if not self:is_orthographic() and mouse.right then
			local rotation_speed = self._rotation_speed * dt
			local rotation_around_world_up = Quaternion(Vector3(0, 1, 0), dx * rotation_speed)
			local rotation_around_camera_right = Quaternion(camera_right_vector, dy * rotation_speed)
			local rotation = Quaternion.multiply(rotation_around_world_up, rotation_around_camera_right)

			local old_rotation = Matrix4x4.from_quaternion(camera_rotation)
			local delta_rotation = Matrix4x4.from_quaternion(rotation)
			local new_rotation = Matrix4x4.multiply(old_rotation, delta_rotation)
			Matrix4x4.set_translation(new_rotation, camera_position)

			-- Fixme
			SceneGraph.set_local_pose(sg, self._unit, new_rotation)
		end
	end

	-- Translation
	local translation_speed = self._translation_speed * dt

	if self:is_orthographic() then
		if keyboard.wkey then camera_position = camera_position + camera_up_vector * translation_speed end
		if keyboard.skey then camera_position = camera_position + camera_up_vector * -translation_speed end
		if keyboard.akey then camera_position = camera_position + camera_right_vector * -translation_speed end
		if keyboard.dkey then camera_position = camera_position + camera_right_vector * translation_speed end
	else
		if keyboard.wkey then camera_position = camera_position + camera_view_vector * translation_speed end
		if keyboard.skey then camera_position = camera_position - camera_view_vector * translation_speed end
		if keyboard.akey then camera_position = camera_position - camera_right_vector * translation_speed end
		if keyboard.dkey then camera_position = camera_position + camera_right_vector * translation_speed end
	end
	SceneGraph.set_local_position(sg, self._unit, camera_position)
end
