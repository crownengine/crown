-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/lua/class"

local function camera_tumble(self, x, y)
	if self:is_orthographic() then
		return
	end

	local drag_delta = Vector3(x, y, 0) - self._drag_start_cursor_xy:unbox()

	local camera_pose     = self._drag_start_camera_pose:unbox()
	local camera_position = Matrix4x4.translation(camera_pose)
	local camera_rotation = Matrix4x4.rotation(camera_pose)
	local camera_right    = Matrix4x4.x(camera_pose)
	local camera_forward  = Matrix4x4.y(camera_pose)

	local rotation_up    = Quaternion.from_axis_angle(Vector3.up(), -drag_delta.x * self._rotation_speed)
	local rotation_right = Quaternion.from_axis_angle(camera_right, drag_delta.y * self._rotation_speed)
	local rotate_delta   = Matrix4x4.from_quaternion(Quaternion.multiply(rotation_up, rotation_right))

	local target_position = camera_position + (camera_forward * self._target_distance)

	local rotate_original = Matrix4x4.from_quaternion(camera_rotation)
	local move_to_target  = Matrix4x4.from_translation(target_position)
	local move_to_origin  = Matrix4x4.from_translation(Vector3(0, -self._target_distance, 0));

	local pose = Matrix4x4.identity()
	pose = Matrix4x4.multiply(pose, move_to_origin)
	pose = Matrix4x4.multiply(pose, rotate_original)
	pose = Matrix4x4.multiply(pose, rotate_delta)
	pose = Matrix4x4.multiply(pose, move_to_target)
	local tr = SceneGraph.instance(self._sg, self._unit)
	SceneGraph.set_local_pose(self._sg, tr, pose)
end

local function camera_track(self, x, y)
	local drag_delta = Vector3(x, y, 0) - self._drag_start_cursor_xy:unbox()

	local camera_pose     = self._drag_start_camera_pose:unbox()
	local camera_position = Matrix4x4.translation(camera_pose)
	local camera_right    = Matrix4x4.x(camera_pose)
	local camera_up       = Matrix4x4.z(camera_pose)

	local pan_speed = 0

	if self:is_orthographic() then
		-- Measure how many pixels is 1 unit
		local camera = self:camera()
		local a = World.camera_world_to_screen(self._world, camera, Vector3.zero())
		local b = World.camera_world_to_screen(self._world, camera, camera_right)
		local pixels_per_unit = Vector3.length(a - b)
		pan_speed = 1 / pixels_per_unit
	else
		-- Speed is proportional to initial distance from target
		pan_speed = self._drag_start_target_distance / 800 * self._movement_speed
	end

	local delta = (drag_delta.y * pan_speed) * camera_up
				+ (drag_delta.x * pan_speed) * camera_right
	local tr = SceneGraph.instance(self._sg, self._unit)
	SceneGraph.set_local_position(self._sg, tr, camera_position - delta)
end

local function camera_dolly(self, x, y)
	local drag_delta = Vector3(x, y, 0) - self._drag_start_cursor_xy:unbox()

	if self:is_orthographic() then
		-- Zoom speed is proportional to initial orthographic size
		local zoom_speed = self._drag_start_orthographic_size / 400
		local zoom_delta = drag_delta.y * zoom_speed
		self._orthographic_size = math.max(1, self._drag_start_orthographic_size - zoom_delta)

		World.camera_set_orthographic_size(self._world, self:camera(), self._orthographic_size)
	else
		-- Speed is proportional to initial distance from target
		local move_speed  = self._drag_start_target_distance / 400
		local mouse_delta = drag_delta.y * move_speed

		self._target_distance = math.max(1, self._drag_start_target_distance - mouse_delta)
		local move_delta = self._target_distance - self._drag_start_target_distance

		local camera_pose     = self._drag_start_camera_pose:unbox()
		local camera_position = Matrix4x4.translation(camera_pose)
		local camera_forward  = Matrix4x4.y(camera_pose);
		local tr = SceneGraph.instance(self._sg, self._unit)
		SceneGraph.set_local_position(self._sg, tr, camera_position - camera_forward*move_delta)
	end
end

Camera = class(Camera)

function Camera:init(world, unit)
	self._world = world
	self._unit = unit
	self._sg = World.scene_graph(world)
	self._movement_speed = 1
	self._rotation_speed = 0.002
	self._orthographic_size = 10
	self._target_distance = 10

	self._move_callback = nil

	self._drag_start_cursor_xy = Vector3Box()
	self._drag_start_camera_pose = Matrix4x4Box()
	self._drag_start_orthographic_size = self._orthographic_size
	self._drag_start_target_distance = self._target_distance

	-- Hide camera mesh.
	local render_world = World.render_world(self._world)
	local camera_mesh = RenderWorld.mesh_instance(render_world, self._unit)
	if camera_mesh ~= nil then
		RenderWorld.mesh_set_visible(render_world, camera_mesh, false)
	end
end

function Camera:unit()
	return self._unit;
end

function Camera:camera()
	return World.camera_instance(self._world, self._unit)
end

function Camera:local_position()
	local tr = SceneGraph.instance(self._sg, self._unit)
	return SceneGraph.local_position(self._sg, tr)
end

function Camera:local_pose()
	local tr = SceneGraph.instance(self._sg, self._unit)
	return SceneGraph.local_pose(self._sg, tr)
end

function Camera:set_local_pose(pose)
	local tr = SceneGraph.instance(self._sg, self._unit)
	return SceneGraph.set_local_pose(self._sg, tr, pose)
end

function Camera:right()
	return Matrix4x4.x(self:local_pose())
end

function Camera:forward()
	return Matrix4x4.y(self:local_pose())
end

function Camera:up()
	return Matrix4x4.z(self:local_pose())
end

function Camera:is_orthographic()
	return World.camera_projection_type(self._world, self:camera()) == "orthographic"
end

function Camera:mouse_wheel(delta)
	self._movement_speed = math.max(0.001, self._movement_speed + delta * 0.005)
end

function Camera:camera_ray(x, y)
	local camera = self:camera()
	local near = World.camera_screen_to_world(self._world, camera, Vector3(x, y, 0))
	local far = World.camera_screen_to_world(self._world, camera, Vector3(x, y, 1))
	local dir = World.camera_projection_type(self._world, camera) == "orthographic"
		and Matrix4x4.y(self:local_pose())
		or Vector3.normalize(far - near)
	return near, dir
end

function Camera:set_perspective()
	if not self:is_orthographic() then
		return
	end

	local camera = self:camera()
	local tr = SceneGraph.instance(self._sg, self._unit)

	local camera_pose     = SceneGraph.local_pose(self._sg, tr)
	local camera_position = Matrix4x4.translation(camera_pose)
	local camera_forward  = Matrix4x4.y(camera_pose)
	local target_position = camera_position + camera_forward * self._target_distance

	local ortho_len = self:screen_length_to_world_length(target_position, 10)
	World.camera_set_projection_type(self._world, camera, "perspective")
	local persp_len = self:screen_length_to_world_length(target_position, 10)

	self._target_distance = self._target_distance * (ortho_len / persp_len)
	SceneGraph.set_local_position(self._sg, tr, target_position - camera_forward * self._target_distance)

	self:send_state()
end

function Camera:set_orthographic(dir, up)
	local camera = self:camera()
	local tr = SceneGraph.instance(self._sg, self._unit)

	local camera_pose     = SceneGraph.local_pose(self._sg, tr)
	local camera_position = Matrix4x4.translation(camera_pose)
	local camera_forward  = Matrix4x4.y(camera_pose)
	local target_position = camera_position + camera_forward * self._target_distance

	local new_camera_position = target_position - dir * self._target_distance
	local persp_len = self:screen_length_to_world_length(target_position, 10)
	SceneGraph.set_local_rotation(self._sg, tr, Quaternion.look(dir, up))
	SceneGraph.set_local_position(self._sg, tr, new_camera_position)
	World.camera_set_projection_type(self._world, camera, "orthographic")
	World.camera_set_orthographic_size(self._world, camera, self._orthographic_size)
	local ortho_len = self:screen_length_to_world_length(target_position, 10)

	self._orthographic_size = self._orthographic_size * (persp_len / ortho_len)
	World.camera_set_orthographic_size(self._world, camera, self._orthographic_size)

	self:send_state()
end

function Camera:screen_length_to_world_length(position, length)
	local camera = self:camera()
	local right = Matrix4x4.x(self:local_pose())
	local a = World.camera_world_to_screen(self._world, camera, position)
	local b = World.camera_world_to_screen(self._world, camera, position + right)
	return length / Vector3.distance(a, b)
end

function Camera:update(dt, x, y, dx, dy)
	local old_pose = self:local_pose()
	local old_target_distance = self._target_distance
	local old_orthographic_size = self._orthographic_size

	if dx ~= 0 or dy ~= 0 then
		if self._move_callback ~= nil then
			self._move_callback(self, x, y)
		end
	end

	if not Matrix4x4.equal(old_pose, self:local_pose())
		or old_target_distance ~= self._target_distance
		or old_orthographic_size ~= self._orthographic_size then
		self:send_state()
	end
end

function Camera:send_state()
	if self._last_send_state ~= nil and os.clock() - self._last_send_state < 1/60 then
		return
	end
	self._last_send_state = os.clock()

	Device.console_send { type = "camera"
		, position = Matrix4x4.translation(self:local_pose())
		, rotation = Matrix4x4.rotation(self:local_pose())
		, orthographic_size = self._orthographic_size
		, target_distance = self._target_distance
		}
end

function Camera:restore(position, rotation, ortho_size, target_distance, projection_type)
	self:set_local_pose(Matrix4x4.from_quaternion_translation(rotation, position))
	self._orthographic_size = ortho_size
	self._target_distance = target_distance
	World.camera_set_projection_type(self._world, self:camera(), projection_type)
	World.camera_set_orthographic_size(self._world, self:camera(), ortho_size)
	self._drag_start_orthographic_size = self._orthographic_size
	self._drag_start_target_distance = self._target_distance
end

function Camera:set_mode(mode, x, y)
	self._drag_start_cursor_xy:store(x, y, 0)
	self._drag_start_camera_pose:store(self:local_pose())
	self._drag_start_orthographic_size = self._orthographic_size
	self._drag_start_target_distance = self._target_distance

	if mode == "tumble" then self._move_callback = camera_tumble
	elseif mode == "track" then self._move_callback = camera_track
	elseif mode == "dolly" then self._move_callback = camera_dolly
	elseif mode == "idle" then self._move_callback = nil
	end
end

function Camera:is_idle()
	return self._move_callback == nil
end

function Camera:near_clip_distance()
	local camera = self:camera()
	return World.camera_near_clip_distance(self._world, camera)
end

function Camera:far_clip_distance()
	local camera = self:camera()
	return World.camera_far_clip_distance(self._world, camera)
end

function Camera:frame_obb(obb_tm, obb_he)
	local obb_position = Matrix4x4.translation(obb_tm)
	local obb_scale = Matrix4x4.scale(obb_tm)
	obb_he.x = obb_he.x * obb_scale.x
	obb_he.y = obb_he.y * obb_scale.y
	obb_he.z = obb_he.z * obb_scale.z
	local obb_radius = Vector3.distance(obb_position, obb_position + obb_he)

	local camera_pose = self:local_pose()
	local camera_target_distance = obb_radius*3
	local camera_forward  = Matrix4x4.y(camera_pose)
	Matrix4x4.set_translation(camera_pose, obb_position - camera_forward*camera_target_distance)

	self:set_local_pose(camera_pose)
	self._target_distance = camera_target_distance
	self:send_state()
end
