require "core/level_editor/camera"
require "core/level_editor/class"

Colors = {
	grid          = function() return Color4(0.75, 0.75, 0.75, 1.0) end,
	grid_disabled = function() return Color4(0.5, 0.5, 0.5, 1.0) end,
	axis_x        = function() return Color4.red() end,
	axis_y        = function() return Color4.green() end,
	axis_z        = function() return Color4.blue() end,
	axis_selected = function() return Color4.yellow() end,
}

function log(msg)
	Device.console_send { type = "message", message = msg, severity = "info" }
end

-- From Bitsquid's grid_plane.lua
function snap_vector(tm, vector, size)
	if size == 0 then return vector end

	local origin = Matrix4x4.translation(tm)
	vector = vector - origin

	local x_axis = Matrix4x4.x(tm)
	local y_axis = Matrix4x4.y(tm)
	local z_axis = Matrix4x4.z(tm)
	local x_dp = Vector3.dot(vector, x_axis)
	local y_dp = Vector3.dot(vector, y_axis)
	local z_dp = Vector3.dot(vector, z_axis)

	local snapped =
		x_axis * math.floor(x_dp / size + 0.5) * size +
		y_axis * math.floor(y_dp / size + 0.5) * size +
		z_axis * math.floor(z_dp / size + 0.5) * size

	return snapped + origin
end

-- From Bitsquid's grid_plane.lua
function draw_grid(lines, tm, center, size, axis, color)
	local nv, nq, nm = Device.temp_count()

	local pos = snap_vector(tm, center, size)
	local x = nil
	local y = nil

	if axis == "x" or axis == "xz" then
		x = Matrix4x4.x(tm)
		y = Matrix4x4.z(tm)
	elseif axis == "y" or axis == "yz" then
		x = Matrix4x4.z(tm)
		y = Matrix4x4.y(tm)
	elseif axis == "z" then
		x = Matrix4x4.x(tm)
		y = Matrix4x4.z(tm)
	else
		x = Matrix4x4.x(tm)
		y = Matrix4x4.y(tm)
	end

	for i = -10, 10 do
		for j = -10, 10 do
			local p = pos + (x * i * size) + (y * j * size)
			DebugLine.add_line(lines, -size / 2 * x + p, size / 2 * x + p, color)
			DebugLine.add_line(lines, -size / 2 * y + p, size / 2 * y + p, color)
		end
	end

	Device.set_temp_count(nv, nq, nm)
end

function raycast(pos, dir, objects)
	local nearest = 999999.0
	local hit = nil

	for k, v in pairs(objects) do
		local t = v:raycast(pos, dir)
		if t ~= -1.0 then
			if t < nearest then
				nearest = t
				hit = nearest
			end
		end
	end

	return hit
end

Selection = class(Selection)

function Selection:init()
	self._objects = {}
end

function Selection:clear()
	self._objects = {}
end

function Selection:has(id)
	for k, v in pairs(self._objects) do
		if v == id then return true end
	end

	return false
end

function Selection:add(id)
	if not self:has(id) then
		self._objects[#self._objects + 1] = id
	end
end

function Selection:remove(id)
	-- FIXME
end

function Selection:last_selected_object()
	local last = self._objects[#self._objects]
	return last and LevelEditor._objects[last] or nil
end

function Selection:objects()
	local objs = {}
	for k, v in pairs(self._objects) do
		objs[#objs + 1] = LevelEditor._objects[v]
	end
	return objs
end

function Selection:world_poses()
	local objs = self:objects()
	local poses = {}
	for k, v in pairs(objs) do
		poses[#poses + 1] = Matrix4x4Box(v:local_pose())
	end
	return poses
end

function Selection:send()
	Device.console_send { type = "selection", objects = self._objects }
end

function Selection:send_move_objects()
	local ids = {}
	local new_positions = {}
	local new_rotations = {}
	local new_scales = {}

	local objs = self:objects()
	for k, v in ipairs(objs) do
		table.insert(ids, v:id())
		table.insert(new_positions, v:local_position())
		table.insert(new_rotations, v:local_rotation())
		table.insert(new_scales, v:local_scale())
	end

	Device.console_send { type = "move_objects"
		, ids = ids
		, new_positions = new_positions
		, new_rotations = new_rotations
		, new_scales = new_scales
		}
end

UnitBox = class(UnitBox)

function UnitBox:init(world, id, unit_id, prefab)
	self._world = world
	self._id = id
	self._unit_id = unit_id
	self._prefab = prefab
	self._sg = World.scene_graph(world)

	local pw = World.physics_world(world)

	local actor = PhysicsWorld.actor_instances(pw, unit_id)
	if actor then
		PhysicsWorld.set_actor_kinematic(pw, actor, true)
	end
end

function UnitBox:id()
	return self._id
end

function UnitBox:prefab()
	return self._prefab
end

function UnitBox:unit_id()
	return self._unit_id
end

function UnitBox:destroy()
	World.destroy_unit(self._world, self._unit_id)
end

function UnitBox:local_position()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.local_position(self._sg, tr) or Vector3.zero()
end

function UnitBox:local_rotation()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.local_rotation(self._sg, tr) or Quaternion.identity()
end

function UnitBox:local_scale()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.local_scale(self._sg, tr) or Vector3(1, 1, 1)
end

function UnitBox:local_pose()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.local_pose(self._sg, tr) or Matrix4x4.identity()
end

function UnitBox:world_position()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.world_position(self._sg, tr) or Vector3.zero()
end

function UnitBox:world_rotation()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.world_rotation(self._sg, tr) or Quaternion.identity()
end

function UnitBox:world_scale()
	return Vector3(1, 1, 1)
end

function UnitBox:world_pose()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.world_pose(self._sg, tr) or Matrix4x4.identity()
end

function UnitBox:set_local_position(pos)
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_position(self._sg, tr, pos) end
end

function UnitBox:set_local_rotation(rot)
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_rotation(self._sg, tr, rot) end
end

function UnitBox:set_local_scale(scale)
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_scale(self._sg, tr, scale) end
end

function UnitBox:set_local_pose(pose)
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_pose(self._sg, tr, pose) end
end

function UnitBox:raycast(pos, dir)
	-- FIXME: Handle units w/o transform
	local rw = LevelEditor._rw
	local sg = LevelEditor._sg

	local meshes = RenderWorld.mesh_instances(rw, self._unit_id)
	local tm, hext = RenderWorld.mesh_obb(rw, meshes[1])

	local pose = self:world_pose()
	-- return Math.ray_obb_intersection(pos, dir, Matrix4x4.multiply(tm, pose), hext)
	return RenderWorld.mesh_raycast(rw, meshes[1], pos, dir)
end

SoundObject = class(SoundObject)

function SoundObject:init(world, id, name, range, volume, loop)
	self._world = world
	self._id = id
	self._name = name
	self._range = range
	self._volume = volume
	self._loop = loop
	self._unit_id = World.spawn_unit(world, "core/units/sound")
	self._sg = World.scene_graph(world)

	local pw = World.physics_world(world)
end

function SoundObject:id()
	return self._id
end

function SoundObject:unit_id()
	return self._unit_id
end

function SoundObject:destroy()
	World.destroy_unit(self._world, self._unit_id)
end

function SoundObject:name()
	return self._name
end

function SoundObject:local_position()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.local_position(self._sg, tr) or Vector3.zero()
end

function SoundObject:local_rotation()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.local_rotation(self._sg, tr) or Quaternion.identity()
end

function SoundObject:local_scale()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.local_scale(self._sg, tr) or Vector3(1, 1, 1)
end

function SoundObject:local_pose()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.local_pose(self._sg, tr) or Matrix4x4.identity()
end

function SoundObject:world_position()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.world_position(self._sg, tr) or Vector3.zero()
end

function SoundObject:world_rotation()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.world_rotation(self._sg, tr) or Quaternion.identity()
end

function SoundObject:world_scale()
	return Vector3(1, 1, 1)
end

function SoundObject:world_pose()
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	return tr and SceneGraph.world_pose(self._sg, tr) or Matrix4x4.identity()
end

function SoundObject:set_local_position(pos)
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_position(self._sg, tr, pos) end
end

function SoundObject:set_local_rotation(rot)
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_rotation(self._sg, tr, rot) end
end

function SoundObject:set_local_scale(scale)
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_scale(self._sg, tr, scale) end
end

function SoundObject:set_local_pose(pose)
	local tr = SceneGraph.transform_instances(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_pose(self._sg, tr, pose) end
end

function SoundObject:raycast(pos, dir)
	-- FIXME: Handle units w/o transform
	local rw = LevelEditor._rw
	local sg = LevelEditor._sg

	local meshes = RenderWorld.mesh_instances(rw, self._unit_id)
	local tm, hext = RenderWorld.mesh_obb(rw, meshes[1])

	local pose = self:world_pose()
	-- return Math.ray_obb_intersection(pos, dir, Matrix4x4.multiply(tm, pose), hext)
	return RenderWorld.mesh_raycast(rw, meshes[1], pos, dir)
end

SelectTool = class(SelectTool)

function SelectTool:init()
end

function SelectTool:update(dt, x, y)
end

function SelectTool:mouse_move(x, y)
end

function SelectTool:mouse_down(x, y)
	local pos, dir = LevelEditor:camera():camera_ray(x, y)

	-- raycast()
	local nearest = 999999.0
	local hit = nil
	local selected_object = nil

	for k, v in pairs(LevelEditor._objects) do
		local t = v:raycast(pos, dir)
		if t ~= -1.0 then
			if t < nearest then
				nearest = t
				hit = nearest
				selected_object = v
			end
		end
	end

	if hit ~= nil then
		if not LevelEditor:multiple_selection_enabled() then
			LevelEditor._selection:clear()
		end
		LevelEditor._selection:add(selected_object:id())
	end
end

function SelectTool:mouse_up(x, y)
	if LevelEditor._selection:last_selected_object() ~= nil then
		LevelEditor._selection:send()
	end
end

PlaceTool = class(PlaceTool)

function PlaceTool:init()
	-- Data
	self._position       = Vector3Box(Vector3.zero())
	self._placeable_type = nil
	self._placeable      = nil

	self:set_state("idle")
end

function PlaceTool:is_idle()
	return self._state == "idle"
end

function PlaceTool:is_placing()
	return self._state == "placing"
end

function PlaceTool:set_state(state)
	assert(state == "idle" or state == "placing")
	self._state = state
end

function PlaceTool:position()
	return self._position:unbox()
end

function PlaceTool:set_position(pos)
	self._position:store(pos)
end

function PlaceTool:set_placeable(placeable_type, name)
	assert(placeable_type == "unit" or placeable_type == "sound")
	self._placeable_type = placeable_type
	self._placeable = name
end

function PlaceTool:update(dt, x, y)
	local pos = self:position()
	LevelEditor:draw_grid(Matrix4x4.from_translation(pos), self:position(), LevelEditor._grid.size, "z")

	if self._placeable ~= nil then
		local lines = LevelEditor._lines
		local tm = Matrix4x4.from_translation(pos)
		if self._placeable_type == "unit" then
			DebugLine.add_unit(lines, tm, self._placeable, Color4.green())
		elseif self._placeable_type == "sound" then
			DebugLine.add_unit(lines, tm, "core/units/sound", Color4.green())
		end
	end
end

function PlaceTool:mouse_move(x, y)
	if self:is_idle() then
		local target = LevelEditor:find_spawn_point(x, y)
		target = LevelEditor:snap(Matrix4x4.identity(target), target) or target
		self:set_position(target)
	end

	if self:is_placing() then
		local pos, dir = LevelEditor:camera():camera_ray(x, y)
		local point = self:position()
		local normal = Vector3.up()

		local t = Math.ray_plane_intersection(pos, dir, point, normal)
		if t ~= -1.0 then
			local target = pos + dir * t
			target = LevelEditor:snap(Matrix4x4.identity(target), target) or target
			self:set_position(target)
		end
	end
end

function PlaceTool:mouse_down(x, y)
	self:set_state("placing")
end

function PlaceTool:mouse_up(x, y)
	self:set_state("idle")

	if self._placeable == nil then
		return
	end

	local level_object = nil
	if self._placeable_type == "unit" then
		local guid = Device.guid()
		local unit = World.spawn_unit(LevelEditor._world, self._placeable, self:position())
		level_object = UnitBox(LevelEditor._world, guid, unit, self._placeable)

		Device.console_send { type = "unit_spawned"
			, id = guid
			, name = level_object:prefab()
			, position = level_object:local_position()
			, rotation = level_object:local_rotation()
			, scale = level_object:local_scale()
			}

		LevelEditor._objects[guid] = level_object
	elseif self._placeable_type == "sound" then
		local guid = Device.guid()
		level_object = SoundObject(LevelEditor._world, guid, self._placeable, 50.0, 1.0, false)
		level_object:set_local_position(self:position())
		level_object:set_local_rotation(Quaternion.identity())

		Device.console_send { type = "sound_spawned"
			, id = guid
			, name = level_object:name()
			, position = level_object:local_position()
			, rotation = level_object:local_rotation()
			, scale = level_object:local_scale()
			, range = level_object._range
			, volume = level_object._volume
			, loop = level_object._loop
			}

		LevelEditor._objects[guid] = level_object
	end

	LevelEditor._selection:clear()
	LevelEditor._selection:add(level_object:id())
	LevelEditor._selection:send()
end

MoveTool = class(MoveTool)

function MoveTool:init()
	-- Data
	self._position    = Vector3Box(Vector3.zero())
	self._rotation    = QuaternionBox(Quaternion.identity())
	self._drag_start  = Vector3Box(Vector3.zero())
	self._drag_delta  = Vector3Box(Vector3.zero())
	self._drag_offset = Vector3Box(Vector3.zero())
	self._selected    = nil

	self._poses_start = {}

	self:set_state("idle")
end

function MoveTool:is_idle()
	return self._state == "idle"
end

function MoveTool:is_moving()
	return self._state == "moving"
end

function MoveTool:set_state(state)
	assert(state == "idle" or state == "moving")
	self._state = state
end

function MoveTool:position()
	return self._position:unbox()
end

function MoveTool:rotation()
	return self._rotation:unbox()
end

function MoveTool:pose()
	return Matrix4x4.from_quaternion_translation(self:rotation(), self:position())
end

function MoveTool:set_position(pos)
	self._position:store(pos)
end

function MoveTool:set_pose(pose)
	self._position:store(Matrix4x4.translation(pose))
	self._rotation:store(Matrix4x4.rotation(pose))
end

function MoveTool:x_axis()
	return Quaternion.right(self._rotation:unbox())
end

function MoveTool:y_axis()
	return Quaternion.up(self._rotation:unbox())
end

function MoveTool:z_axis()
	return Quaternion.forward(self._rotation:unbox())
end

function MoveTool:is_axis_selected(axis)
	return self._selected and string.find(self._selected, axis) or false
end

function MoveTool:axis_selected()
	return self._selected == "x"
		or self._selected == "y"
		or self._selected == "z"
		or self._selected == "xy"
		or self._selected == "yz"
		or self._selected == "xz"
end

function MoveTool:drag_start()
	return self._drag_start:unbox()
end

function MoveTool:drag_plane()
	if self._selected == "x"  then return self:position(),  self:y_axis() end
	if self._selected == "y"  then return self:position(),  self:x_axis() end
	if self._selected == "z"  then return self:position(),  self:y_axis() end
	if self._selected == "xy" then return self:position(),  Vector3.cross(self:x_axis(), self:y_axis()) end
	if self._selected == "yz" then return self:position(),  Vector3.cross(self:y_axis(), self:z_axis()) end
	if self._selected == "xz" then return self:position(), -Vector3.cross(self:x_axis(), self:z_axis()) end
	return nil, nil
end

function MoveTool:update(dt, x, y)
	local selected = LevelEditor._selection:last_selected_object()

	-- Update gizmo pose
	if selected then
		self:set_pose(LevelEditor._reference_system == "world" and Matrix4x4.from_translation(selected:local_position()) or selected:world_pose())
	end

	local tm = self:pose()
	local p  = self:position()
	local l  = LevelEditor:camera():screen_length_to_world_length(self:position(), 85)

	local function transform(tm, offset)
		local m = Matrix4x4.copy(tm)
		Matrix4x4.set_translation(m, Matrix4x4.transform(tm, offset))
		return m
	end

	-- Select axis
	if self:is_idle() and selected then
		local pos, dir = LevelEditor:camera():camera_ray(x, y)

		local axis = {
			Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0.5, 0.0, 0.0)), l * Vector3(0.50, 0.07, 0.07)),
			Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0.0, 0.5, 0.0)), l * Vector3(0.07, 0.50, 0.07)),
			Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0.0, 0.0, 0.5)), l * Vector3(0.07, 0.07, 0.50)),
			Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0.4, 0.4, 0.0)), l * Vector3(0.1, 0.1, 0.0)),
			Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0.0, 0.4, 0.4)), l * Vector3(0.0, 0.1, 0.1)),
			Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0.4, 0.0, 0.4)), l * Vector3(0.1, 0.0, 0.1))
		}

		local nearest = nil
		local t = 9999999.0
		local axis_names = { "x", "y", "z", "xy", "yz", "xz" }
		for i, name in ipairs(axis_names) do
			if axis[i] ~= -1.0 then
				if axis[i] < t then
					nearest = i
					t = axis[i]
				end
			end
		end
		self._selected = nearest and axis_names[nearest] or nil
	end

	-- Drawing
	if selected then
		local lines = LevelEditor._lines_no_depth
		DebugLine.add_line(lines, p, p + l * Matrix4x4.x(tm), self:is_axis_selected("x") and Colors.axis_selected() or Colors.axis_x())
		DebugLine.add_line(lines, p, p + l * Matrix4x4.y(tm), self:is_axis_selected("y") and Colors.axis_selected() or Colors.axis_y())
		DebugLine.add_line(lines, p, p + l * Matrix4x4.z(tm), self:is_axis_selected("z") and Colors.axis_selected() or Colors.axis_z())
		DebugLine.add_obb(lines, transform(tm, l * Vector3(0.4, 0.4, 0.0)), l * Vector3(0.1, 0.1, 0.0), self:is_axis_selected("xy") and Colors.axis_selected() or Colors.axis_x())
		DebugLine.add_obb(lines, transform(tm, l * Vector3(0.0, 0.4, 0.4)), l * Vector3(0.0, 0.1, 0.1), self:is_axis_selected("yz") and Colors.axis_selected() or Colors.axis_y())
		DebugLine.add_obb(lines, transform(tm, l * Vector3(0.4, 0.0, 0.4)), l * Vector3(0.1, 0.0, 0.1), self:is_axis_selected("xz") and Colors.axis_selected() or Colors.axis_z())

		if self:axis_selected() then
			DebugLine.add_sphere(lines, self:drag_start(), 0.05, Color4.green())
			LevelEditor:draw_grid(LevelEditor:grid_pose(self:pose()), self:position(), LevelEditor._grid.size, self._selected)
		end
	end
end

function MoveTool:drag_offset(x, y)
	local drag_plane = self:drag_plane()

	if (drag_plane ~= nil) then
		local pos, dir = LevelEditor:camera():camera_ray(x, y)
		local t = Math.ray_plane_intersection(pos, dir, self:drag_plane())

		if t ~= -1.0 then
			local point_on_plane = pos + dir*t
			local offset = point_on_plane - self:drag_start()
			return offset;
		end
	end

	return Vector3.zero()
end

function MoveTool:mouse_down(x, y)
	if self:is_idle() then
		if self:axis_selected() and LevelEditor._selection:last_selected_object() then
			self._drag_start:store(self:position())
			self._drag_offset:store(self:drag_offset(x, y) or Vector3.zero())

			self._poses_start = LevelEditor._selection:world_poses()

			self:set_state("moving")
		else
			LevelEditor.select_tool:mouse_down(x, y)
		end
	end
end

function MoveTool:mouse_up(x, y)
	if self:is_idle() then
		LevelEditor.select_tool:mouse_up(x, y)
	end

	if self:is_moving() then
		LevelEditor._selection:send_move_objects()
	end

	self:set_state("idle")
	self._drag_start:store(self:position())
	self._drag_offset:store(Vector3.zero())
	self._selected = nil
	self._poses_start = {}
end

function MoveTool:mouse_move(x, y)
	if self:is_idle() then
		LevelEditor.select_tool:mouse_move(x, y)
	end

	if self:is_moving() then
		local delta = self:drag_offset(x, y) - self._drag_offset:unbox()
		local drag_vector = Vector3.zero()

		for _, a in ipairs { "x", "y", "z" } do
			local axis = Vector3.zero()

			if self:is_axis_selected(a) then
				if a == "x" then axis = self:x_axis() end
				if a == "y" then axis = self:y_axis() end
				if a == "z" then axis = self:z_axis() end
			end

			local contribution = Vector3.dot(axis, delta)
			drag_vector = drag_vector + axis*contribution
		end

		-- FIXME Move selected objects
		local objects = LevelEditor._selection:objects()
		for k, v in pairs(objects) do
			local pos = Matrix4x4.translation(self._poses_start[k]:unbox()) + drag_vector
			v:set_local_position(LevelEditor:snap(self:pose(), pos) or pos)
		end
	end
end

RotateTool = class(RotateTool)

function RotateTool:init()
	-- Data
	self._position      = Vector3Box(Vector3.zero())
	self._rotation      = QuaternionBox(Quaternion.identity())
	self._drag_start    = Vector3Box(Vector3.zero())
	self._start_pose    = Matrix4x4Box(Matrix4x4.identity())
	self._rotation_axis = Vector3Box(Vector3.zero())
	self._selected      = nil

	self._poses_start   = {}

	self:set_state("idle")
end

function RotateTool:position()
	return self._position:unbox()
end

function RotateTool:rotation()
	return self._rotation:unbox()
end

function RotateTool:pose()
	return Matrix4x4.from_quaternion_translation(self:rotation(), self:position())
end

function RotateTool:set_pose(pose)
	self._position:store(Matrix4x4.translation(pose))
	self._rotation:store(Matrix4x4.rotation(pose))
end

function RotateTool:set_state(state)
	assert(state == "idle" or state == "rotating")
	self._state = state
end

function RotateTool:is_idle()
	return self._state == "idle"
end

function RotateTool:is_rotating()
	return self._state == "rotating"
end

function RotateTool:axis_selected()
	return self._selected == "x"
		or self._selected == "y"
		or self._selected == "z"
end

function RotateTool:drag_start()
	return self._drag_start:unbox()
end

function RotateTool:rotate_normal()
	if self._selected == "x" then return Quaternion.right(self:rotation()) end
	if self._selected == "y" then return Quaternion.up(self:rotation()) end
	if self._selected == "z" then return Quaternion.forward(self:rotation()) end
	return nil
end

function RotateTool:update(dt, x, y)
	local selected = LevelEditor._selection:last_selected_object()

	-- Update gizmo pose
	if selected then
		self:set_pose(LevelEditor._reference_system == "world" and Matrix4x4.from_translation(selected:local_position()) or selected:world_pose())
	end

	local tm = self:pose()
	local p  = self:position()
	local l  = LevelEditor:camera():screen_length_to_world_length(self:position(), 85)

	-- Select axis
	if self:is_idle() and selected then
		local pos, dir = LevelEditor:camera():camera_ray(x, y)

		local axis = {
			Math.ray_disc_intersection(pos, dir, p, l, Matrix4x4.x(tm)),
			Math.ray_disc_intersection(pos, dir, p, l, Matrix4x4.y(tm)),
			Math.ray_disc_intersection(pos, dir, p, l, Matrix4x4.z(tm))
		}

		local nearest = nil
		local t = 9999999.0
		local axis_names = { "x", "y", "z" }
		for i, name in ipairs(axis_names) do
			if axis[i] ~= -1.0 then
				if axis[i] < t then
					nearest = i
					t = axis[i]
				end
			end
		end
		self._selected = nearest and axis_names[nearest] or nil
	end

	-- Drawing
	if selected then
		local lines = LevelEditor._lines_no_depth
		DebugLine.add_circle(lines, p, l, Matrix4x4.x(tm), self._selected == "x" and Colors.axis_selected() or Colors.axis_x())
		DebugLine.add_circle(lines, p, l, Matrix4x4.y(tm), self._selected == "y" and Colors.axis_selected() or Colors.axis_y())
		DebugLine.add_circle(lines, p, l, Matrix4x4.z(tm), self._selected == "z" and Colors.axis_selected() or Colors.axis_z())
	end
end

function RotateTool:drag_offset(x, y)
	local rotate_normal = self:rotate_normal()

	if rotate_normal ~= nil then
		local pos, dir = LevelEditor:camera():camera_ray(x, y)
		local t = Math.ray_plane_intersection(pos, dir, self:position(), rotate_normal)

		if t ~= -1.0 then
			return pos + dir*t
		end
	end

	return Vector3.zero()
end

function RotateTool:mouse_down(x, y)
	if self:is_idle() then
		if self:axis_selected() and LevelEditor._selection:last_selected_object() then
			self._drag_start:store(self:drag_offset(x, y))
			self._start_pose:store(self:pose())
			self._rotation_axis:store(self:rotate_normal())

			self._poses_start = LevelEditor._selection:world_poses()

			self:set_state("rotating")
		else
			LevelEditor.select_tool:mouse_down(x, y)
		end
	end
end

function RotateTool:mouse_up(x, y)
	if self:is_idle() then
		LevelEditor.select_tool:mouse_up(x, y)
	end

	if self:is_rotating() then
		LevelEditor._selection:send_move_objects()
	end

	self._drag_start:store(Vector3.zero())
	self._start_pose:store(Matrix4x4.identity())
	self._selected = nil
	self._poses_start = {}
	self:set_state("idle")
end

function RotateTool:mouse_move(x, y)
	if self:is_idle() then
		LevelEditor.select_tool:mouse_move(x, y)
	end

	if self:is_rotating() then
		local point_on_plane = self:drag_offset(x, y)
		local drag_start = self:drag_start()
		local drag_handle = Vector3.normalize(point_on_plane - self:position())

		local v1 = Vector3.normalize(drag_start - self:position())
		local v2 = Vector3.normalize(point_on_plane - self:position())
		local v1_dot_v2 = Vector3.dot(v1, v2)
		local v1_cross_v2 = Vector3.cross(v1, v2)

		local rotation_normal = self._rotation_axis:unbox()
		local y_axis = v1
		local x_axis = Vector3.cross(rotation_normal, v1)
		local radians = math.atan2(Vector3.dot(v1, y_axis), Vector3.dot(v1, x_axis)) - math.atan2(Vector3.dot(v2, y_axis), Vector3.dot(v2, x_axis))

		if LevelEditor:rotation_snap_enabled() then
			radians = radians - radians % LevelEditor._rotation_snap
		end

		local delta_rotation = Quaternion(rotation_normal, radians)

		local pivot = self:position()
		local translate_to_origin = Matrix4x4.from_translation(-pivot)
		local apply_delta_rotation = Matrix4x4.from_quaternion(delta_rotation)
		local translate_back_to_origin = Matrix4x4.from_translation(pivot)

		-- FIXME Rotate selected objects
		local objects = LevelEditor._selection:objects()
		for k, v in pairs(objects) do
			local start_pose = self._poses_start[k]:unbox()
			local new_pose = Matrix4x4.multiply(start_pose, translate_to_origin)
			new_pose = Matrix4x4.multiply(new_pose, apply_delta_rotation)
			new_pose = Matrix4x4.multiply(new_pose, translate_back_to_origin)
			v:set_local_position(Matrix4x4.translation(new_pose))
			v:set_local_rotation(Matrix4x4.rotation(new_pose))
		end

		-- Drawing
		local lines = LevelEditor._lines_no_depth
		DebugLine.add_sphere(lines, point_on_plane, 0.1, Color4.green())
		DebugLine.add_line(lines, self:position(), self:position() + Vector3.normalize(drag_start - self:position()), Color4.yellow())
		DebugLine.add_line(lines, self:position(), self:position() + Vector3.normalize(point_on_plane - self:position()), Color4.yellow())
	end
end

ScaleTool = class(ScaleTool)

function ScaleTool:init()
	-- Data
	self._rotation    = QuaternionBox(Quaternion.identity())
	self._position    = Vector3Box(Vector3.zero())
	self._drag_start  = Vector3Box(Vector3.zero())
	self._drag_offset = Vector3Box(Vector3.zero())
	self._selected    = nil
	-- States
	self._state = "idle"
end

function ScaleTool:position()
	return self._position:unbox()
end

function ScaleTool:rotation()
	return self._rotation:unbox()
end

function ScaleTool:set_position(pos)
	self._position:store(pos)
end

function ScaleTool:set_rotation(rot)
	self._rotation:store(rot)
end

function ScaleTool:x_axis()
	return Quaternion.right(self:rotation())
end

function ScaleTool:y_axis()
	return Quaternion.up(self:rotation())
end

function ScaleTool:z_axis()
	return Quaternion.forward(self:rotation())
end

function ScaleTool:drag_start()
	return self._drag_start:unbox()
end

function ScaleTool:axis_selected()
	return self._selected == "x"
		or self._selected == "y"
		or self._selected == "z"
		or self._selected == "xyz"
end

function ScaleTool:selected_axis()
	if self._selected == "x"   then return Quaternion.right(self._rotation) end
	if self._selected == "y"   then return Quaternion.up(self._selected) end
	if self._selected == "z"   then return Quaternion.forward(self._selected) end
	if self._selected == "xyz" then return Quaternion.righ(self._selected) end
	return nil
end

function ScaleTool:is_idle()
	return self._state == "idle"
end

function ScaleTool:set_state(state)
	assert(state == "idle" or state == "scaling")
	self._state = state
end

function ScaleTool:set_pose(pose)
	self._position:store(Matrix4x4.translation(pose))
	self._rotation:store(Matrix4x4.rotation(pose))
end

function ScaleTool:world_pose()
	return Matrix4x4.from_quaternion_translation(self:rotation(), self:position())
end

function ScaleTool:update(dt, x, y)
	local function transform(tm, offset)
		local m = Matrix4x4.copy(tm)
		Matrix4x4.set_translation(m, Matrix4x4.transform(tm, offset))
		return m
	end

	local selected = LevelEditor._selection:last_selected_object()
	if not selected then return end

	self:set_pose(selected:world_pose())

	local l = LevelEditor:camera():screen_length_to_world_length(self:position(), 85)

	-- Select axis
	if self:is_idle() then
		local tm = self:world_pose()

		-- Select axis
		local pos, dir = LevelEditor:camera():camera_ray(x, y)

		self._selected = "none"
		if Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(1, 0, 0)), l * Vector3(0.1, 0.1, 0.1)) ~= -1.0 then self._selected = "x" end
		if Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0, 1, 0)), l * Vector3(0.1, 0.1, 0.1)) ~= -1.0 then self._selected = "y" end
		if Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0, 0, 1)), l * Vector3(0.1, 0.1, 0.1)) ~= -1.0 then self._selected = "z" end
		if Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3.zero()), l * Vector3(0.1, 0.1, 0.1)) ~= -1.0 then self._selected = "xyz" end
	end

	-- Drawing
	local tm = self:world_pose()
	local p = self:position()

	local lines = LevelEditor._lines_no_depth
	DebugLine.add_line(lines, p, p + l * Matrix4x4.x(tm), self._selected == "x" and Colors.axis_selected() or Colors.axis_x())
	DebugLine.add_line(lines, p, p + l * Matrix4x4.y(tm), self._selected == "y" and Colors.axis_selected() or Colors.axis_y())
	DebugLine.add_line(lines, p, p + l * Matrix4x4.z(tm), self._selected == "z" and Colors.axis_selected() or Colors.axis_z())

	DebugLine.add_obb(lines, transform(tm, l * Vector3(1, 0, 0)), l * Vector3(0.1, 0.1, 0.1), self._selected == "x" and Colors.axis_selected() or Colors.axis_x())
	DebugLine.add_obb(lines, transform(tm, l * Vector3(0, 1, 0)), l * Vector3(0.1, 0.1, 0.1), self._selected == "y" and Colors.axis_selected() or Colors.axis_y())
	DebugLine.add_obb(lines, transform(tm, l * Vector3(0, 0, 1)), l * Vector3(0.1, 0.1, 0.1), self._selected == "z" and Colors.axis_selected() or Colors.axis_z())
	DebugLine.add_obb(lines, transform(tm, l * Vector3.zero()), l * Vector3(0.1, 0.1, 0.1), self._selected == "xyz" and Colors.axis_selected() or Colors.axis_z())
end

function ScaleTool:mouse_move(x, y)
	if self:is_idle() then return end

	if self:axis_selected() then
			local delta = self:drag_offset(x, y) - self._drag_offset:unbox()
			local drag_vector = Vector3.zero()

			for _, a in ipairs{"x", "y", "z"} do
				local axis = Vector3.zero()

				if self:is_axis_selected(a) then
					if a == "x" then axis = Vector3.right() end
					if a == "y" then axis = Vector3.up() end
					if a == "z" then axis = Vector3.forward() end
				end

				local contribution = Vector3.dot(axis, delta)
				drag_vector = drag_vector + axis*contribution
			end

			local selected = LevelEditor._selection:last_selected_object()
			local pos = Vector3(1, 1, 1) + drag_vector
			-- log(Vector3.to_string(self:drag_start()))
			-- log(Vector3.to_string(pos))
			-- log(Matrix4x4.to_string(selected:world_pose()) .. "\n")
			-- selected:set_local_scale(LevelEditor:snap(self:world_pose(), pos) or pos)
	end
end

function ScaleTool:mouse_down(x, y)
	if self:axis_selected() then
		self:set_state("scaling")
		self._drag_start:store(self:position())
		self._drag_offset:store(self:drag_offset(x, y))
	else
		LevelEditor.select_tool:mouse_down(x, y)
	end
end

function ScaleTool:mouse_up(x, y)
	self:set_state("idle")

	if self:axis_selected() then
		self._drag_start:store(Vector3.zero())
		self._drag_offset:store(Vector3(1, 1, 1))
	else
		LevelEditor.select_tool:mouse_up(x, y)
	end
end

function ScaleTool:is_axis_selected(axis)
	return string.find(self._selected, axis)
end

function ScaleTool:drag_plane()
	if self._selected == "x"   then return self:position(), self:y_axis() end
	if self._selected == "y"   then return self:position(), self:x_axis() end
	if self._selected == "z"   then return self:position(), self:y_axis() end
	if self._selected == "xyz" then return self:position(), self:x_axis() end
	return nil
end

function ScaleTool:drag_offset(x, y)
	local drag_plane = self:drag_plane()

	if (drag_plane ~= nil) then
		local pos, dir = LevelEditor:camera():camera_ray(x, y)
		local t = Math.ray_plane_intersection(pos, dir, self:drag_plane())

		if t ~= -1.0 then
			local point_on_plane = pos + dir*t
			local offset = point_on_plane - self:drag_start()
			return offset;
		end
	end

	return Vector3.zero()
end

LevelEditor = LevelEditor or {}

function LevelEditor:init()
	self._world = Device.create_world()
	self._pw = World.physics_world(self._world)
	self._rw = World.render_world(self._world)
	self._sg = World.scene_graph(self._world)
	self._lines_no_depth = World.create_debug_line(self._world, false)
	self._lines = World.create_debug_line(self._world, true)
	self._fpscamera = FPSCamera(self._world, World.spawn_unit(self._world, "core/units/camera"))
	self._mouse = { x = 0, y = 0, dx = 0, dy = 0, button = { left = false, middle = false, right = false }, wheel = { delta = 0 }}
	self._keyboard = { ctrl = false, shift = false }
	self._grid = { size = 1 }
	self._rotation_snap = 45.0 * math.pi / 180.0
	self._objects = {}
	self._selection = Selection()
	self._show_grid = true
	self._snap_to_grid = true
	self._snap_mode = "relative"
	self._reference_system = "local"
	self._spawn_height = 0.0

	self.select_tool = SelectTool()
	self.place_tool = PlaceTool()
	self.move_tool = MoveTool()
	self.rotate_tool = RotateTool()
	self.scale_tool = ScaleTool()
	self.tool = self.place_tool

	-- Spawn camera
	local camera_transform = SceneGraph.transform_instances(self._sg, self._fpscamera:unit())
	local pos = Vector3(20, 20, -20)
	local dir = Vector3.normalize(Vector3.zero() - pos)
	SceneGraph.set_local_rotation(self._sg, camera_transform, Quaternion.look(dir))
	SceneGraph.set_local_position(self._sg, camera_transform, pos)
end

function LevelEditor:update(dt)
	World.update(self._world, dt)

	DebugLine.reset(self._lines_no_depth)
	DebugLine.reset(self._lines)

	local delta = Vector3.zero();
	if self._mouse.right then
		delta.x = self._mouse.dx;
		delta.y = self._mouse.dy;
	end

	self._fpscamera:mouse_wheel(self._mouse.wheel.delta)
	self._fpscamera:update(-delta.x, -delta.y, self._keyboard)

	self.tool:update(dt, self._mouse.x, self._mouse.y)

	self._mouse.dx = 0
	self._mouse.dy = 0
	self._mouse.wheel.delta = 0

	local pos, dir = self._fpscamera:camera_ray(self._mouse.x, self._mouse.y)
	local t = raycast(pos, dir, self._objects)
	self._spawn_height = t and (pos + dir * t).y or 0

	if self._show_grid then
		self:draw_grid(Matrix4x4.identity(), Vector3.zero(), self._grid.size, "xz")
	end

	DebugLine.submit(self._lines_no_depth)
	DebugLine.submit(self._lines)
end

function LevelEditor:render(dt)
	Device.render(self._world, self._fpscamera:camera())
end

function LevelEditor:shutdown()
	World.destroy_debug_line(self._world, self._lines)
	World.destroy_debug_line(self._world, self._lines_no_depth)
	Device.destroy_world(self._world)
end

function LevelEditor:reset()
	LevelEditor:shutdown()
	LevelEditor:init()
end

function LevelEditor:set_mouse_state(x, y, left, middle, right)
	self._mouse.x = x
	self._mouse.y = y
	self._mouse.left = left
	self._mouse.middle = middle
	self._mouse.right = right
end

function LevelEditor:set_mouse_move(x, y, dx, dy)
	self._mouse.x = x
	self._mouse.y = y
	self._mouse.dx = dx
	self._mouse.dy = dy

	self.tool:mouse_move(x, y)
end

function LevelEditor:set_mouse_wheel(delta)
	self._mouse.wheel.delta = self._mouse.wheel.delta + delta;
end

function LevelEditor:mouse_down(x, y)
	self.tool:mouse_down(x, y)
end

function LevelEditor:mouse_up(x, y)
	self.tool:mouse_up(x, y)
end

function LevelEditor:key_down(key)
	if (key == "w") then self._keyboard.wkey = true end
	if (key == "a") then self._keyboard.akey = true end
	if (key == "s") then self._keyboard.skey = true end
	if (key == "d") then self._keyboard.dkey = true end
	if (key == "left_ctrl") then self._keyboard.ctrl = true end
	if (key == "left_shift") then self._keyboard.shift = true end
end

function LevelEditor:key_up(key)
	if (key == "w") then self._keyboard.wkey = false end
	if (key == "a") then self._keyboard.akey = false end
	if (key == "s") then self._keyboard.skey = false end
	if (key == "d") then self._keyboard.dkey = false end
	if (key == "left_ctrl") then self._keyboard.ctrl = false end
	if (key == "left_shift") then self._keyboard.shift = false end
end

function LevelEditor:camera()
	return self._fpscamera
end

function LevelEditor:set_tool(tool)
	self.tool = tool
end

function LevelEditor:multiple_selection_enabled()
	return self._keyboard.shift
end

function LevelEditor:snap_to_grid_enabled()
	return self._snap_to_grid and not self._keyboard.ctrl
end

function LevelEditor:rotation_snap_enabled()
	return self._snap_to_grid and not self._keyboard.ctrl
end

function LevelEditor:enable_show_grid(enabled)
	self._show_grid = enabled
end

function LevelEditor:enable_snap_to_grid(enabled)
	self._snap_to_grid = enabled
end

function LevelEditor:enable_debug_render_world(enabled)
	RenderWorld.enable_debug_drawing(self._rw, enabled)
end

function LevelEditor:enable_debug_physics_world(enabled)
	PhysicsWorld.enable_debug_drawing(self._pw, enabled)
end

function LevelEditor:set_snap_mode(mode)
	assert(mode == "absolute" or mode == "relative")
	self._snap_mode = mode
end

function LevelEditor:set_reference_system(system)
	assert(system == "local" or system == "world")
	self._reference_system = system
end

function LevelEditor:set_grid_size(size)
	self._grid.size = size
end

function LevelEditor:set_rotation_snap(angle)
	self._rotation_snap = angle
end

function LevelEditor:grid_pose(tm)
	local pose = Matrix4x4.copy(tm)
	if self._snap_mode == "absolute" then
		Matrix4x4.set_translation(pose, Vector3.zero())
	end
	return pose
end

function LevelEditor:snap(grid_tm, pos)
	local grid_pose = LevelEditor:grid_pose(grid_tm)

	if not self:snap_to_grid_enabled() then
		return nil
	end

	return snap_vector(grid_pose, pos, LevelEditor._grid.size)
end

function LevelEditor:find_spawn_point(x, y)
	local pos, dir = self._fpscamera:camera_ray(x, y)
	local spawn_height = LevelEditor._spawn_height
	local point = Vector3(0, spawn_height, 0)
	local normal = Vector3.up()
	local t = Math.ray_plane_intersection(pos, dir, point, normal)
	return t ~= -1.0 and pos + dir * t or nil
end

function LevelEditor:draw_grid(tm, center, size, axis)
	local color = self:snap_to_grid_enabled() and Colors.grid() or Colors.grid_disabled()
	draw_grid(self._lines, tm, center, size, axis, color)
end

function LevelEditor:spawn_unit(id, type, pos, rot, scale)
	local unit = World.spawn_unit(self._world, type, pos, rot)
	local unit_box = UnitBox(self._world, id, unit, type)
	self._objects[id] = unit_box
end

function LevelEditor:spawn_empty_unit(id)
	local unit = World.spawn_empty_unit(self._world)
	local unit_box = UnitBox(self._world, id, unit, nil)
	self._objects[id] = unit_box
end

function LevelEditor:spawn_sound(id, pos, rot, range, volume, loop)
	local sound = SoundObject(self._world, id, range, volume, loop)
	sound:set_local_position(pos)
	sound:set_local_rotation(rot)
	self._objects[id] = sound
end

function LevelEditor:add_transform_component(id, component_id, pos, rot, scale)
	local unit_box = self._objects[id]
	local unit_id = unit_box:unit_id()
	SceneGraph.create(self._sg, unit_id, pos, rot, scale)
end

function LevelEditor:add_mesh_component(id, component_id, mesh_resource, geometry_name, material_resource, visible)
	local unit_box = self._objects[id]
	local unit_id = unit_box:unit_id()
	RenderWorld.create_mesh(self._rw, unit_id, mesh_resource, geometry_name, material_resource, visible, unit_box:world_pose())
end

function LevelEditor:add_light_component(id, component_id, type, range, intensity, spot_angle, color)
	local unit_box = self._objects[id]
	local unit_id = unit_box:unit_id()
	RenderWorld.create_light(self._rw, unit_id, type, range, intensity, spot_angle, color, unit_box:world_pose())
end

function LevelEditor:move_object(id, pos, rot, scale)
	local unit_box = self._objects[id]
	unit_box:set_local_position(pos)
	unit_box:set_local_rotation(rot)
	unit_box:set_local_scale(scale)
end

function LevelEditor:set_placeable(placeable_type, name)
	self.place_tool:set_placeable(placeable_type, name)
end

function LevelEditor:set_selected_unit(id)
	local unit_box = self._objects[id]

	self._selection:clear()
	self._selection:add(unit_box:id())
end

function LevelEditor:destroy(id)
	local unit_box = self._objects[id]
	unit_box:destroy()

	self._objects[id] = nil

	-- FIXME
	self._selection:clear()
	self._selection:send()
end
