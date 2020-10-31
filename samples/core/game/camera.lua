require "core/lua/class"

local wkey = false
local skey = false
local akey = false
local dkey = false

FPSCamera = class(FPSCamera)

function FPSCamera:init(world, unit)
	self._world = world
	self._unit = unit
	self._sg = World.scene_graph(world)
	self._movement_speed = 20
	self._rotation_speed = 0.14
end

function FPSCamera:unit()
	return self._unit;
end

function FPSCamera:camera()
	return World.camera_instance(self._world, self._unit)
end

function FPSCamera:update(dt, dx, dy)
	if Keyboard.pressed(Keyboard.button_id("w")) then wkey = true end
	if Keyboard.pressed(Keyboard.button_id("s")) then skey = true end
	if Keyboard.pressed(Keyboard.button_id("a")) then akey = true end
	if Keyboard.pressed(Keyboard.button_id("d")) then dkey = true end
	if Keyboard.released(Keyboard.button_id("w")) then wkey = false end
	if Keyboard.released(Keyboard.button_id("s")) then skey = false end
	if Keyboard.released(Keyboard.button_id("a")) then akey = false end
	if Keyboard.released(Keyboard.button_id("d")) then dkey = false end

	local camera = self:camera()
	local tr = SceneGraph.instance(self._sg, self._unit)
	local camera_local_pose = SceneGraph.local_pose(self._sg, tr)
	local camera_right_vector = Matrix4x4.x(camera_local_pose)
	local camera_position = Matrix4x4.translation(camera_local_pose)
	local camera_rotation = Matrix4x4.rotation(camera_local_pose)
	local view_dir = Matrix4x4.z(camera_local_pose)

	-- Rotation
	if dx ~= 0 or dy ~= 0 then
		local rotation_speed = self._rotation_speed * dt
		local rotation_around_world_up = Quaternion(Vector3(0, 1, 0), dx * rotation_speed)
		local rotation_around_camera_right = Quaternion(camera_right_vector, dy * rotation_speed)
		local rotation = Quaternion.multiply(rotation_around_world_up, rotation_around_camera_right)

		local old_rotation = Matrix4x4.from_quaternion(camera_rotation)
		local delta_rotation = Matrix4x4.from_quaternion(rotation)
		local new_rotation = Matrix4x4.multiply(old_rotation, delta_rotation)
		Matrix4x4.set_translation(new_rotation, camera_position)

		-- Fixme
		SceneGraph.set_local_pose(self._sg, tr, new_rotation)
	end

	-- Translation
	local translation_speed = self._movement_speed * dt
	if wkey then camera_position = camera_position + view_dir * translation_speed end
	if skey then camera_position = camera_position - view_dir * translation_speed end
	if akey then camera_position = camera_position - camera_right_vector * translation_speed end
	if dkey then camera_position = camera_position + camera_right_vector * translation_speed end

	SceneGraph.set_local_position(self._sg, tr, camera_position)
end
