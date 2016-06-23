require "core/editors/level_editor/class"

FPSCamera = class(FPSCamera)

function FPSCamera:init(world, unit)
	self._world = world
	self._unit = unit
	self._translation_speed = 0.75
	self._rotation_speed = 0.006
end

function FPSCamera:unit()
	return self._unit;
end

function FPSCamera:camera()
	return World.camera(self._world, self._unit)
end

function FPSCamera:position()
	local sg = World.scene_graph(self._world)
	local camera_transform = SceneGraph.transform_instances(sg, self._unit)
	local camera_position = SceneGraph.world_position(sg, camera_transform)
	return camera_position
end

function FPSCamera:world_pose()
	local sg = World.scene_graph(self._world)
	local camera_transform = SceneGraph.transform_instances(sg, self._unit)
	return SceneGraph.world_pose(sg, camera_transform)
end

function FPSCamera:set_translation_speed(speed)
	self._translation_speed = speed
end

function FPSCamera:set_rotation_speed(speed)
	self._rotation_speed = speed
end

function FPSCamera:mouse_wheel(delta)
	self._translation_speed = math.max(0.001, self._translation_speed + delta * 0.2)
end

function FPSCamera:camera_ray(x, y)
	local camera = self:camera()
	local near = World.camera_screen_to_world(self._world, camera, Vector3(x, y, 0))
	local far = World.camera_screen_to_world(self._world, camera, Vector3(x, y, 1))
	local dir = Vector3.normalize(far - near)
	return self:position(), dir
end

function FPSCamera:screen_length_to_world_length(position, length)
	local right = Matrix4x4.x(self:world_pose())
	local a = World.camera_world_to_screen(self._world, self:camera(), position)
	local b = World.camera_world_to_screen(self._world, self:camera(), position + right)
	return length / Vector3.distance(a, b)
end

function FPSCamera:update(dx, dy, keyboard)
	local sg = World.scene_graph(self._world)

	local camera = self:camera()
	local camera_transform = SceneGraph.transform_instances(sg, self._unit)
	local camera_local_pose = SceneGraph.local_pose(sg, camera_transform)
	local camera_right_vector = Matrix4x4.x(camera_local_pose)
	local camera_position = Matrix4x4.translation(camera_local_pose)
	local camera_rotation = Matrix4x4.rotation(camera_local_pose)
	local view_dir = Matrix4x4.z(camera_local_pose)

	-- Rotation
	local rotation_around_world_up = Quaternion(Vector3(0, 1, 0), -dx * self._rotation_speed)
	local rotation_around_camera_right = Quaternion(camera_right_vector, -dy * self._rotation_speed)
	local rotation = Quaternion.multiply(rotation_around_world_up, rotation_around_camera_right)

	local old_rotation = Matrix4x4.from_quaternion(camera_rotation)
	local delta_rotation = Matrix4x4.from_quaternion(rotation)
	local new_rotation = Matrix4x4.multiply(old_rotation, delta_rotation)
	Matrix4x4.set_translation(new_rotation, camera_position)

	-- Fixme
	SceneGraph.set_local_pose(sg, camera_transform, new_rotation)

	-- Translation
	local speed = self._translation_speed;
	if keyboard.wkey then camera_position = camera_position + view_dir * speed end
	if keyboard.skey then camera_position = camera_position + view_dir * -1 * speed end
	if keyboard.akey then camera_position = camera_position + camera_right_vector * -1 * speed end
	if keyboard.dkey then camera_position = camera_position + camera_right_vector * speed end

	SceneGraph.set_local_position(sg, camera_transform, camera_position)
end
