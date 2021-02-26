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
	local camera_forward  = Matrix4x4.z(camera_pose)

	local rotation_up    = Quaternion.from_axis_angle(Vector3.up(), drag_delta.x * self._rotation_speed)
	local rotation_right = Quaternion.from_axis_angle(camera_right, drag_delta.y * self._rotation_speed)
	local rotate_delta   = Matrix4x4.from_quaternion(Quaternion.multiply(rotation_up, rotation_right))

	local target_position = camera_position + (camera_forward * self._target_distance)

	local rotate_original = Matrix4x4.from_quaternion(camera_rotation)
	local move_to_target  = Matrix4x4.from_translation(target_position)
	local move_to_origin  = Matrix4x4.from_translation(Vector3(0, 0, -self._target_distance));

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
	local camera_up       = Matrix4x4.y(camera_pose)

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
				- (drag_delta.x * pan_speed) * camera_right
	local tr = SceneGraph.instance(self._sg, self._unit)
	SceneGraph.set_local_position(self._sg, tr, camera_position + delta)
end

local function camera_dolly(self, x, y)
	local drag_delta = Vector3(x, y, 0) - self._drag_start_cursor_xy:unbox()

	if self:is_orthographic() then
		-- Zoom speed is proportional to initial orthographic size
		local zoom_speed = self._drag_start_orthographic_size / 400
		local zoom_delta = drag_delta.y * zoom_speed
		self._orthographic_size = math.max(1, self._drag_start_orthographic_size + zoom_delta)

		World.camera_set_orthographic_size(self._world, self:camera(), self._orthographic_size)
	else
		-- Speed is proportional to initial distance from target
		local move_speed  = self._drag_start_target_distance / 400
		local mouse_delta = drag_delta.y * move_speed

		self._target_distance = math.max(1, self._drag_start_target_distance + mouse_delta)
		local move_delta = self._target_distance - self._drag_start_target_distance

		local camera_pose     = self._drag_start_camera_pose:unbox()
		local camera_position = Matrix4x4.translation(camera_pose)
		local camera_forward  = Matrix4x4.z(camera_pose);
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
	self._perspective_local_pose = Matrix4x4Box()
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
	RenderWorld.mesh_set_visible(render_world, camera_mesh, false)
end

function Camera:unit()
	return self._unit;
end

function Camera:camera()
	return World.camera_instance(self._world, self._unit)
end

function Camera:local_pose()
	local tr = SceneGraph.instance(self._sg, self._unit)
	return SceneGraph.local_pose(self._sg, tr)
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
		and Matrix4x4.z(self:local_pose())
		or Vector3.normalize(far - near)
	return near, dir
end

function Camera:set_perspective()
	if not self:is_orthographic() then
		return
	end

	local tr = SceneGraph.instance(self._sg, self._unit)
	SceneGraph.set_local_pose(self._sg, tr, self._perspective_local_pose:unbox())
	World.camera_set_projection_type(self._world, self:camera(), "perspective")
end

function Camera:set_orthographic(world_center, world_radius, dir, up)
	if not self:is_orthographic() then
		self._perspective_local_pose:store(self:local_pose())
	end

	local tr = SceneGraph.instance(self._sg, self._unit)
	SceneGraph.set_local_rotation(self._sg, tr, Quaternion.look(dir, up))
	SceneGraph.set_local_position(self._sg, tr, world_center - dir * math.abs(Vector3.dot(dir, world_radius)) * 100) -- FIXME
	local camera = self:camera()
	World.camera_set_orthographic_size(self._world, camera, self._orthographic_size)
	World.camera_set_projection_type(self._world, camera, "orthographic")
end

function Camera:screen_length_to_world_length(position, length)
	local camera = self:camera()
	local right = Matrix4x4.x(self:local_pose())
	local a = World.camera_world_to_screen(self._world, camera, position)
	local b = World.camera_world_to_screen(self._world, camera, position + right)
	return length / Vector3.distance(a, b)
end

function Camera:update(dt, dx, dy, keyboard, mouse)
	if dx ~= 0 or dy ~= 0 then
		if self._move_callback ~= nil then
			self._move_callback(self, mouse.x, mouse.y)
		end
	end
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
