-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/lua/class"

FPSCamera = class(FPSCamera)

function FPSCamera:init(world, unit)
	self._world = world
	self._unit = unit
	self._scene_graph = World.scene_graph(world)
	self._movement_speed = 20
	self._rotation_speed = 0.14
end

function FPSCamera:unit()
	return self._unit;
end

function FPSCamera:camera()
	return World.camera_instance(self._world, self._unit)
end

function FPSCamera:world_pose()
	local camera_transform = SceneGraph.instance(self._scene_graph, self._unit)
	return SceneGraph.world_pose(self._scene_graph, camera_transform)
end

function FPSCamera:rotate(dt, dx, dy)
	local camear_transform = SceneGraph.instance(self._scene_graph, self._unit)
	local camera_local_pose = SceneGraph.local_pose(self._scene_graph, camear_transform)
	local camera_right_vector = Matrix4x4.x(camera_local_pose)
	local camera_position = Matrix4x4.translation(camera_local_pose)
	local camera_rotation = Matrix4x4.rotation(camera_local_pose)
	local view_dir = Matrix4x4.y(camera_local_pose)

	-- Rotate.
	if dx ~= 0 or dy ~= 0 then
		local rotation_speed = self._rotation_speed * dt
		local rotation_around_world_up = Quaternion(Vector3(0, 0, 1), -dx * rotation_speed)
		local rotation_around_camera_right = Quaternion(camera_right_vector, -dy * rotation_speed)
		local rotation = Quaternion.multiply(rotation_around_world_up, rotation_around_camera_right)

		local old_rotation = Matrix4x4.from_quaternion(camera_rotation)
		local delta_rotation = Matrix4x4.from_quaternion(rotation)
		local new_rotation = Matrix4x4.multiply(old_rotation, delta_rotation)
		Matrix4x4.set_translation(new_rotation, camera_position)

		-- Fixme
		SceneGraph.set_local_pose(self._scene_graph, camear_transform, new_rotation)
	end
end

function FPSCamera:move(dt, dx, dy)
	local camera_transform = SceneGraph.instance(self._scene_graph, self._unit)
	local camera_local_pose = SceneGraph.local_pose(self._scene_graph, camera_transform)
	local camera_right_vector = Matrix4x4.x(camera_local_pose)
	local camera_position = Matrix4x4.translation(camera_local_pose)
	local view_dir = Matrix4x4.y(camera_local_pose)

    -- Translate.
    local translation_speed = self._movement_speed * dt
    camera_position = camera_position + view_dir * translation_speed * dy
    camera_position = camera_position + camera_right_vector * translation_speed * dx

    SceneGraph.set_local_position(self._scene_graph, camera_transform, camera_position)
end
