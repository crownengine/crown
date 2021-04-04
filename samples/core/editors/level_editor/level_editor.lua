require "core/editors/level_editor/camera"
require "core/lua/class"

Colors = Colors or {
	grid          = function() return Color4(102, 102, 102, 255) end,
	grid_disabled = function() return Color4(102, 102, 102, 102) end,
	axis_x        = function() return Color4(217,   0,   0, 255) end,
	axis_y        = function() return Color4(  0, 217,   0, 255) end,
	axis_z        = function() return Color4(  0,   0, 217, 255) end,
	axis_selected = function() return Color4(217, 217,   0, 255) end,
}

Gizmo = Gizmo or {
	size = 85,
	axis_fadeout_threshold  = 1.38,
	axis_hidden_threshold   = 1.52,
	plane_fadeout_threshold = 1.03,
	plane_hidden_threshold  = 1.17
}

function Color4.lerp_alpha(color, alpha)
	local cr, cg, cb, ca = Quaternion.elements(color)
	local color_transparent = Quaternion.from_elements(cr, cg, cb, 0)
	return Color4.lerp(color_transparent, color, alpha)
end

function dot_alpha(dot, fadeout_threshold, hidden_threshold)
	local f0 = math.cos(fadeout_threshold)
	local h0 = math.cos(hidden_threshold)
	assert(f0 <= 1 and f0 >= 0)
	assert(h0 <= 1 and h0 >= 0)
	assert(f0 > h0)

	--           f0
	--      h0   |
	--      |    |
	-- 00000fffff11111
	--
	if dot < h0 then -- The axis is parallel to the viewer.
		return 0
	elseif dot < f0 then -- The axis is starting to get perpendicular to the viewer.
		return (dot - h0) / (f0 - h0)
	end

	-- The axis is perpendicular to the viewer.
	return 1
end

function axis_alpha(axis, camera_dir, fadeout_threshold, hidden_threshold)
	return dot_alpha(1-math.abs(Vector3.dot(axis, camera_dir)), fadeout_threshold, hidden_threshold)
end

function plane_alpha(axis, camera_dir, fadeout_threshold, hidden_threshold)
	return dot_alpha(math.abs(Vector3.dot(axis, camera_dir)), fadeout_threshold, hidden_threshold)
end

function axis_enabled(axis, camera_dir)
	local alpha = axis_alpha(axis
		, camera_dir
		, Gizmo.axis_fadeout_threshold
		, Gizmo.axis_hidden_threshold
		)
	return alpha ~= 0
end

function axis_color(axis, camera_dir, color)
	local alpha = axis_alpha(axis
		, camera_dir
		, Gizmo.axis_fadeout_threshold
		, Gizmo.axis_hidden_threshold
		)
	return Color4.lerp_alpha(color, alpha)
end

function plane_enabled(axis, camera_dir)
	local alpha = plane_alpha(axis
		, camera_dir
		, Gizmo.plane_fadeout_threshold
		, Gizmo.plane_hidden_threshold
		)
	return alpha ~= 0
end

function plane_color(axis, camera_dir, color)
	local alpha = plane_alpha(axis
		, camera_dir
		, Gizmo.plane_fadeout_threshold
		, Gizmo.plane_hidden_threshold
		)
	return Color4.lerp_alpha(color, alpha)
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
	elseif axis == "xy" then
		x = Matrix4x4.x(tm)
		y = Matrix4x4.y(tm)
	else
		x = Matrix4x4.x(tm)
		y = Matrix4x4.z(tm)
	end

	local cr, cg, cb, ca = Quaternion.elements(color)
	local color_transparent = Quaternion.from_elements(cr, cg, cb, 0)

	for i = -10, 10 do
		local abs_i = math.abs(i)

		for j = -10, 10 do
			local abs_j = math.abs(j)

			local alpha = math.min(abs_i * abs_i + abs_j * abs_j + 25, 100) / 100
			local line_color = Color4.lerp(color, color_transparent, alpha)

			local p = pos + (x * i * size) + (y * j * size)
			DebugLine.add_line(lines, -size / 2 * x + p, size / 2 * x + p, line_color)
			DebugLine.add_line(lines, -size / 2 * y + p, size / 2 * y + p, line_color)
		end
	end

	Device.set_temp_count(nv, nq, nm)
end

-- From Bitsquid's math.lua
function line_line(line_a_pt1, line_a_pt2, line_b_pt1, line_b_pt2)
	local line_a_vector = line_a_pt2 - line_a_pt1
	local line_b_vector = line_b_pt2 - line_b_pt1
	local a = Vector3.dot(line_a_vector, line_a_vector)
	local e = Vector3.dot(line_b_vector, line_b_vector)
	local b = Vector3.dot(line_a_vector, line_b_vector)
	local d = a * e - b * b

	if d < 0.001 then
		-- The lines are parallel. There is no intersection.
		return nil, nil
	end

	local r = line_a_pt1 - line_b_pt1
	local c = Vector3.dot(line_a_vector, r)
	local f = Vector3.dot(line_b_vector, r)
	local normalized_distance_along_line_a = (b * f - c * e) / d
	local normalized_distance_along_line_b = (a * f - b * c) / d
	return normalized_distance_along_line_a, normalized_distance_along_line_b
end

function draw_world_origin_grid(lines, size, step)
	local nv, nq, nm = Device.temp_count()

	local n = size / step
	local r = n * step

	for i = 1, n do
		local s = i*step
		DebugLine.add_line(lines, Vector3(-r, 0, -s), Vector3( r, 0, -s), Colors.grid())
		DebugLine.add_line(lines, Vector3(-r, 0,  s), Vector3( r, 0,  s), Colors.grid())
		DebugLine.add_line(lines, Vector3(-s, 0, -r), Vector3(-s, 0,  r), Colors.grid())
		DebugLine.add_line(lines, Vector3( s, 0, -r), Vector3( s, 0,  r), Colors.grid())
	end

	DebugLine.add_line(lines, Vector3(-r, 0,  0), Vector3(r, 0, 0), Color4.black())
	DebugLine.add_line(lines, Vector3( 0, 0, -r), Vector3(0, 0, r), Color4.black())

	Device.set_temp_count(nv, nq, nm)
end

function draw_mesh_obb(render_world, unit_id, lines)
	local mesh = RenderWorld.mesh_instance(render_world, unit_id)
	local tm, hext = RenderWorld.mesh_obb(render_world, mesh)
	DebugLine.add_obb(lines, tm, hext, Color4(0, 170, 0, 150))
end

function draw_sprite_obb(render_world, unit_id, lines)
	local sprite = RenderWorld.sprite_instance(render_world, unit_id)
	local tm, hext = RenderWorld.sprite_obb(render_world, sprite)
	DebugLine.add_obb(lines, tm, hext, Color4(0, 170, 0, 150))
end

function raycast(objects, pos, dir)
	local object = nil
	local nearest = math.huge
	local layer = 0
	local depth = 0

	for k, v in pairs(objects) do
		local t, l, d = v:raycast(pos, dir)
		if t == -1.0 then
			goto continue
		end
		-- If sprite
		if l and d then
			if l >= layer and d >= depth then
				layer = l
				depth = d
				nearest = t
				object = v
			end
		else
			if t <= nearest then
				nearest = t
				object = v
			end
		end
		::continue::
	end

	return object, nearest
end

Selection = class(Selection)

function Selection:init()
	self._ids = {}
end

function Selection:clear()
	for k, v in pairs(self._ids) do
		LevelEditor._objects[v]:on_selected(false)
	end
	self._ids = {}
end

function Selection:has(id)
	for k, v in pairs(self._ids) do
		if v == id then return true end
	end

	return false
end

function Selection:add(id)
	if not self:has(id) then
		self._ids[#self._ids + 1] = id
		LevelEditor._objects[id]:on_selected(true)
	end
end

function Selection:remove(id)
	if self:has(id) then
		-- Remove id from selection
		local new_ids = {}
		for k, v in pairs(self._ids) do
			if self._ids[k] ~= id then
				new_ids[#new_ids + 1] = self._ids[k]
			end
		end
		self._ids = new_ids

		local obj = LevelEditor._objects[id]
		if obj ~= nil then
			obj:on_selected(false)
		end
	end
end

function Selection:set(ids)
	self:clear()
	for k, v in pairs(ids) do
		LevelEditor._objects[v]:on_selected(true)
	end
	self._ids = ids
end

function Selection:last_selected_object()
	local last = self._ids[#self._ids]
	return last and LevelEditor._objects[last] or nil
end

function Selection:objects()
	local objs = {}
	for k, v in pairs(self._ids) do
		objs[#objs + 1] = LevelEditor._objects[v]
	end
	return objs
end

function Selection:world_poses()
	local objs = self:objects()
	local poses = {}
	for k, v in pairs(objs) do
		poses[#poses + 1] = Matrix4x4Box(v:world_pose())
	end
	return poses
end

function Selection:send()
	Device.console_send { type = "selection", objects = self._ids }
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

UnitUtils = UnitUtils or {}

function UnitUtils.freeze(world, unit_id)
	local pw = World.physics_world(world)
	local actor = PhysicsWorld.actor_instance(pw, unit_id)
	if (actor ~= nil) then
		PhysicsWorld.actor_set_kinematic(pw, actor, true)
	end
end

UnitBox = class(UnitBox)

function UnitBox:init(world, id, unit_id, prefab)
	self._world = world
	self._id = id
	self._unit_id = unit_id
	self._prefab = prefab
	self._sg = World.scene_graph(world)
	self._selected = false

	UnitUtils.freeze(world, unit_id)
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
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.local_position(self._sg, tr) or Vector3.zero()
end

function UnitBox:local_rotation()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.local_rotation(self._sg, tr) or Quaternion.identity()
end

function UnitBox:local_scale()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.local_scale(self._sg, tr) or Vector3(1, 1, 1)
end

function UnitBox:local_pose()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.local_pose(self._sg, tr) or Matrix4x4.identity()
end

function UnitBox:world_position()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.world_position(self._sg, tr) or Vector3.zero()
end

function UnitBox:world_rotation()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.world_rotation(self._sg, tr) or Quaternion.identity()
end

function UnitBox:world_scale()
	return Vector3(1, 1, 1)
end

function UnitBox:world_pose()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.world_pose(self._sg, tr) or Matrix4x4.identity()
end

function UnitBox:set_local_position(pos)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_position(self._sg, tr, pos) end
end

function UnitBox:set_local_rotation(rot)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_rotation(self._sg, tr, rot) end
end

function UnitBox:set_local_scale(scale)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_scale(self._sg, tr, scale) end
end

function UnitBox:set_local_pose(pose)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_pose(self._sg, tr, pose) end
end

function UnitBox:on_selected(selected)
	self._selected = selected
end

function UnitBox:raycast(pos, dir)
	local rw = LevelEditor._rw
	local mesh = RenderWorld.mesh_instance(rw, self._unit_id)
	if mesh then
		return RenderWorld.mesh_cast_ray(rw, mesh, pos, dir)
	end

	local sprite = RenderWorld.sprite_instance(rw, self._unit_id)
	if sprite then
		return RenderWorld.sprite_cast_ray(rw, sprite, pos, dir)
	end

	return -1.0
end

function UnitBox:draw()
	if self._selected then
		local light = RenderWorld.light_instance(LevelEditor._rw, self._unit_id)
		if light then
			RenderWorld.light_debug_draw(LevelEditor._rw, light, LevelEditor._lines)
		end

		local mesh = RenderWorld.mesh_instance(LevelEditor._rw, self._unit_id)
		if mesh then
			draw_mesh_obb(LevelEditor._rw, self._unit_id, LevelEditor._lines)
		end

		local sprite = RenderWorld.sprite_instance(LevelEditor._rw, self._unit_id)
		if sprite then
			draw_sprite_obb(LevelEditor._rw, self._unit_id, LevelEditor._lines)
		end
	end
end

function UnitBox:set_light(type, range, intensity, angle, color)
	local light = RenderWorld.light_instance(LevelEditor._rw, self._unit_id)
	RenderWorld.light_set_type(LevelEditor._rw, light, type)
	RenderWorld.light_set_color(LevelEditor._rw, light, color)
	RenderWorld.light_set_range(LevelEditor._rw, light, range)
	RenderWorld.light_set_intensity(LevelEditor._rw, light, intensity)
	RenderWorld.light_set_spot_angle(LevelEditor._rw, light, angle)
end

function UnitBox:set_mesh(instance_id, material, visible)
	local mesh = RenderWorld.mesh_instance(LevelEditor._rw, self._unit_id)
	RenderWorld.mesh_set_material(LevelEditor._rw, mesh, material)
	RenderWorld.mesh_set_visible(LevelEditor._rw, mesh, visible)
end

function UnitBox:set_sprite(material, layer, depth, visible)
	local sprite = RenderWorld.sprite_instance(LevelEditor._rw, self._unit_id)
	RenderWorld.sprite_set_material(LevelEditor._rw, sprite, material)
	RenderWorld.sprite_set_layer(LevelEditor._rw, sprite, layer)
	RenderWorld.sprite_set_depth(LevelEditor._rw, sprite, depth)
	RenderWorld.sprite_set_visible(LevelEditor._rw, sprite, visible)
end

function UnitBox:set_camera(projection, fov, near_range, far_range)
	local camera = World.camera_instance(LevelEditor._world, self._unit_id)
	World.camera_set_projection_type(LevelEditor._world, camera, projection)
	World.camera_set_fov(LevelEditor._world, camera, fov)
	World.camera_set_near_clip_distance(LevelEditor._world, camera, near_range)
	World.camera_set_far_clip_distance(LevelEditor._world, camera, far_range)
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
	self._selected = false
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
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.local_position(self._sg, tr) or Vector3.zero()
end

function SoundObject:local_rotation()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.local_rotation(self._sg, tr) or Quaternion.identity()
end

function SoundObject:local_scale()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.local_scale(self._sg, tr) or Vector3(1, 1, 1)
end

function SoundObject:local_pose()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.local_pose(self._sg, tr) or Matrix4x4.identity()
end

function SoundObject:world_position()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.world_position(self._sg, tr) or Vector3.zero()
end

function SoundObject:world_rotation()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.world_rotation(self._sg, tr) or Quaternion.identity()
end

function SoundObject:world_scale()
	return Vector3(1, 1, 1)
end

function SoundObject:world_pose()
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	return tr and SceneGraph.world_pose(self._sg, tr) or Matrix4x4.identity()
end

function SoundObject:set_local_position(pos)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_position(self._sg, tr, pos) end
end

function SoundObject:set_local_rotation(rot)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_rotation(self._sg, tr, rot) end
end

function SoundObject:set_local_scale(scale)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_scale(self._sg, tr, scale) end
end

function SoundObject:set_local_pose(pose)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then SceneGraph.set_local_pose(self._sg, tr, pose) end
end

function SoundObject:on_selected(selected)
	self._selected = selected
end

function SoundObject:raycast(pos, dir)
	local rw = LevelEditor._rw
	local mesh = RenderWorld.mesh_instance(rw, self._unit_id)
	local tm, hext = RenderWorld.mesh_obb(rw, mesh)
	return RenderWorld.mesh_cast_ray(rw, mesh, pos, dir)
end

function SoundObject:draw()
	if self._selected then
		draw_mesh_obb(LevelEditor._rw, self._unit_id, LevelEditor._lines)
		DebugLine.add_sphere(LevelEditor._lines
			, self:local_position()
			, self._range
			, Color4.yellow()
			)
	end
end

function SoundObject:set_range(range)
	self._range = range
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

	local selected_object, t = raycast(LevelEditor._objects, pos, dir)

	if selected_object ~= nil then
		if not LevelEditor:multiple_selection_enabled() then
			LevelEditor._selection:clear()
		end
		if LevelEditor._selection:has(selected_object:id()) then
			LevelEditor._selection:remove(selected_object:id())
		else
			LevelEditor._selection:add(selected_object:id())
		end
		LevelEditor._selection:send()
	else
		LevelEditor._selection:clear()
		LevelEditor._selection:send()
	end
end

function SelectTool:mouse_up(x, y)
	LevelEditor._selection:send()
end

PlaceTool = class(PlaceTool)

function PlaceTool:init()
	-- Data
	self._position       = Vector3Box(Vector3.zero())
	self._placeable_type = nil
	self._placeable_name = nil
	self._placeable_id   = nil
	self._spawn_height       = 0 -- The spawn height at the time of "idle" -> "placing" transition.
	self._spawn_point_height = 0 -- The spawn point height at the time of "idle" -> "placing" transition.

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
	assert(placeable_type == nil or placeable_type == "unit" or placeable_type == "sound")
	self._placeable_type = placeable_type
	self._placeable_name = name

	if (self._placeable_id ~= nil) then
		World.destroy_unit(LevelEditor._world, self._placeable_id)
		self._placeable_id = nil
	end
end

function PlaceTool:update(dt, x, y)
	if self._placeable_type == nil then
		return
	end

	local pos = self:position()
	LevelEditor:draw_grid(Matrix4x4.from_translation(pos), self:position(), LevelEditor._grid.size, "z")

	-- Create placeable preview if it does not exist yet
	if self._placeable_id == nil then
		if self._placeable_type == "unit" then
			self._placeable_id = World.spawn_unit(LevelEditor._world, self._placeable_name, pos)
		elseif self._placeable_type == "sound" then
			self._placeable_id = World.spawn_unit(LevelEditor._world, "core/units/sound", pos)
		end

		UnitUtils.freeze(LevelEditor._world, self._placeable_id)
	end

	-- Update placeable position
	if (self._placeable_id ~= nil) then
		local sg = World.scene_graph(LevelEditor._world)
		local tr = SceneGraph.instance(sg, self._placeable_id)
		if tr ~= nil then
			SceneGraph.set_local_position(sg, tr, pos)
		end
	end
end

function PlaceTool:mouse_move(x, y)
	if self:is_idle() then
		local spawn_point = LevelEditor:find_spawn_point(x, y)
		self._spawn_height = LevelEditor._spawn_height
		self._spawn_point_height = spawn_point.y
		self:set_position(spawn_point)
	elseif self:is_placing() then
		local pos, dir = LevelEditor:camera():camera_ray(x, y)
		local gizmo_pos = self:position()
		local point = Vector3(gizmo_pos.x, self._spawn_height, gizmo_pos.y)
		local normal = Vector3.up()

		local t = Math.ray_plane_intersection(pos, dir, point, normal)
		if t ~= -1.0 then
			local spawn_point = pos + dir * t
			spawn_point = LevelEditor:snap(Matrix4x4.identity(), spawn_point) or spawn_point
			spawn_point.y = self._spawn_point_height
			if LevelEditor.debug then
				DebugLine.add_sphere(LevelEditor._lines_no_depth
					, spawn_point
					, 0.1
					, Color4.blue()
					)
			end
			self:set_position(spawn_point)
		end
	end
end

function PlaceTool:mouse_down(x, y)
	self:set_state("placing")
end

function PlaceTool:mouse_up(x, y)
	self:set_state("idle")

	if self._placeable_name == nil then
		return
	end

	local level_object = nil
	if self._placeable_type == "unit" then
		local guid = Device.guid()
		local unit = World.spawn_unit(LevelEditor._world, self._placeable_name, self:position())
		level_object = UnitBox(LevelEditor._world, guid, unit, self._placeable_name)

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
		level_object = SoundObject(LevelEditor._world, guid, self._placeable_name, 10.0, 1.0, false)
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

function PlaceTool:on_leave()
	if self._placeable_id ~= nil then
		World.destroy_unit(LevelEditor._world, self._placeable_id)
		self._placeable_id = nil
	end
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
		or self._selected == "xyz"
end

function MoveTool:drag_start()
	return self._drag_start:unbox()
end

function MoveTool:drag_axis()
	if self._selected == "x"  then return self:x_axis() end
	if self._selected == "y"  then return self:y_axis() end
	if self._selected == "z"  then return self:z_axis() end
	return nil
end

function MoveTool:drag_plane()
	if self._selected == "xy" then return self:drag_start(),  Vector3.cross(self:x_axis(), self:y_axis()) end
	if self._selected == "yz" then return self:drag_start(),  Vector3.cross(self:y_axis(), self:z_axis()) end
	if self._selected == "xz" then return self:drag_start(), -Vector3.cross(self:x_axis(), self:z_axis()) end
	if self._selected == "xyz" then return self:drag_start(), -Matrix4x4.z(LevelEditor:camera():local_pose()) end
	return nil, nil
end

function MoveTool:update(dt, x, y)
	local selected = LevelEditor._selection:last_selected_object()
	if not selected then
		return
	end

	-- Update gizmo pose
	self:set_pose(LevelEditor._reference_system == "world" and Matrix4x4.from_translation(selected:local_position()) or selected:world_pose())

	local tm = self:pose()
	local p  = self:position()
	local l  = LevelEditor:camera():screen_length_to_world_length(self:position(), Gizmo.size)
	local cam_z = Matrix4x4.z(LevelEditor:camera():local_pose())
	local cam_to_gizmo = Vector3.normalize(p-Matrix4x4.translation(LevelEditor:camera():local_pose()))

	local function transform(tm, offset)
		local m = Matrix4x4.copy(tm)
		Matrix4x4.set_translation(m, Matrix4x4.transform(tm, offset))
		return m
	end

	-- Select axis
	if self:is_idle() then
		local pos, dir = LevelEditor:camera():camera_ray(x, y)

		local axis = {
			axis_enabled(self:x_axis(), cam_to_gizmo) and Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0.30+(1.1-0.30)/2, 0   , 0   )), l * Vector3((1.1-0.30)/2, 0.07, 0.07)) or -1,
			axis_enabled(self:y_axis(), cam_to_gizmo) and Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0   , 0.30+(1.1-0.30)/2, 0   )), l * Vector3(0.07, (1.1-0.30)/2, 0.07)) or -1,
			axis_enabled(self:z_axis(), cam_to_gizmo) and Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0   , 0   , 0.30+(1.1-0.30)/2)), l * Vector3(0.07, 0.07, (1.1-0.30)/2)) or -1,
			plane_enabled(self:z_axis(), cam_to_gizmo) and Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0.4, 0.4, 0.0)), l * Vector3(0.1, 0.1, 0.0)) or -1,
			plane_enabled(self:x_axis(), cam_to_gizmo) and Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0.0, 0.4, 0.4)), l * Vector3(0.0, 0.1, 0.1)) or -1,
			plane_enabled(self:y_axis(), cam_to_gizmo) and Math.ray_obb_intersection(pos, dir, transform(tm, l * Vector3(0.4, 0.0, 0.4)), l * Vector3(0.1, 0.0, 0.1)) or -1,
			Math.ray_disc_intersection(pos, dir, self:position(), l * 2*0.07, -cam_z)
		}

		local nearest = nil
		local dist = math.huge
		local axis_names = { "x", "y", "z", "xy", "yz", "xz", "xyz" }
		for ii, name in ipairs(axis_names) do
			if axis[ii] ~= -1.0 and axis[ii] < dist then
				nearest = ii
				dist = axis[ii]
			end
		end
		self._selected = nearest and axis_names[nearest] or nil
	end

	-- Drawing
	local lines = LevelEditor._lines_no_depth
	-- Draw axes.
	DebugLine.add_line(lines, p + 0.30*l*Matrix4x4.x(tm), p + l*Matrix4x4.x(tm), axis_color(self:x_axis(), cam_to_gizmo, self:is_axis_selected("x") and Colors.axis_selected() or Colors.axis_x()))
	DebugLine.add_line(lines, p + 0.30*l*Matrix4x4.y(tm), p + l*Matrix4x4.y(tm), axis_color(self:y_axis(), cam_to_gizmo, self:is_axis_selected("y") and Colors.axis_selected() or Colors.axis_y()))
	DebugLine.add_line(lines, p + 0.30*l*Matrix4x4.z(tm), p + l*Matrix4x4.z(tm), axis_color(self:z_axis(), cam_to_gizmo, self:is_axis_selected("z") and Colors.axis_selected() or Colors.axis_z()))
	-- Draw axis tips.
	DebugLine.add_cone(lines, p + Matrix4x4.x(tm) * l * 0.9, p + Matrix4x4.x(tm) * l * 1.1, l * 0.05, axis_color(self:x_axis(), cam_to_gizmo, self:is_axis_selected("x") and Colors.axis_selected() or Colors.axis_x()))
	DebugLine.add_cone(lines, p + Matrix4x4.y(tm) * l * 0.9, p + Matrix4x4.y(tm) * l * 1.1, l * 0.05, axis_color(self:y_axis(), cam_to_gizmo, self:is_axis_selected("y") and Colors.axis_selected() or Colors.axis_y()))
	DebugLine.add_cone(lines, p + Matrix4x4.z(tm) * l * 0.9, p + Matrix4x4.z(tm) * l * 1.1, l * 0.05, axis_color(self:z_axis(), cam_to_gizmo, self:is_axis_selected("z") and Colors.axis_selected() or Colors.axis_z()))
	-- Draw plane handles.
	DebugLine.add_obb(lines, transform(tm, l * Vector3(0.4, 0.4, 0.0)), l * Vector3(0.1, 0.1, 0.0), plane_color(self:z_axis(), cam_to_gizmo, self._selected == "xy" and Colors.axis_selected() or Colors.axis_x()))
	DebugLine.add_obb(lines, transform(tm, l * Vector3(0.0, 0.4, 0.4)), l * Vector3(0.0, 0.1, 0.1), plane_color(self:x_axis(), cam_to_gizmo, self._selected == "yz" and Colors.axis_selected() or Colors.axis_y()))
	DebugLine.add_obb(lines, transform(tm, l * Vector3(0.4, 0.0, 0.4)), l * Vector3(0.1, 0.0, 0.1), plane_color(self:y_axis(), cam_to_gizmo, self._selected == "xz" and Colors.axis_selected() or Colors.axis_z()))
	-- Draw camera plane handle.
	DebugLine.add_circle(lines, p, l * 2*0.07, -cam_z, self:is_axis_selected("xyz") and Colors.axis_selected() or Colors.grid())

	if self:axis_selected() then
		LevelEditor:draw_grid(LevelEditor:grid_pose(self:pose()), self:position(), LevelEditor._grid.size, self._selected)
	end
end

function MoveTool:drag_offset(x, y)
	local pos, dir = LevelEditor:camera():camera_ray(x, y)
	local drag_axis = self:drag_axis()
	local drag_plane = self:drag_plane()

	if drag_axis ~= nil then
		local _, delta = line_line(pos, pos + dir, self:drag_start(), self:drag_start() + drag_axis)
		if delta ~= nil then
			return drag_axis * delta
		end
	elseif drag_plane ~= nil then
		local t = Math.ray_plane_intersection(pos, dir, self:drag_plane())
		if t ~= -1.0 then
			return pos + dir*t - self:drag_start()
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
	elseif self:is_moving() then
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
	elseif self:is_moving() then
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

		-- Apply translation to selected objects.
		local objects = LevelEditor._selection:objects()
		for ii, obj in pairs(objects) do
			local obj_position = Matrix4x4.translation(self._poses_start[ii]:unbox())
			local gizmo_position = self:position()
			local gizmo_obj_distance = obj_position - gizmo_position
			local new_gizmo_position = gizmo_position + drag_vector
			obj:set_local_position((LevelEditor:snap(self:pose(), new_gizmo_position) or new_gizmo_position) + gizmo_obj_distance)
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
		or self._selected == "xyz"
end

function RotateTool:drag_start()
	return self._drag_start:unbox()
end

function RotateTool:rotate_normal()
	if self._selected == "x" then return Quaternion.right(self:rotation()) end
	if self._selected == "y" then return Quaternion.up(self:rotation()) end
	if self._selected == "z" then return Quaternion.forward(self:rotation()) end
	if self._selected == "xyz" then return -Matrix4x4.z(LevelEditor:camera():local_pose()) end
	return nil
end

function RotateTool:update(dt, x, y)
	local selected = LevelEditor._selection:last_selected_object()
	if not selected then
		return
	end

	-- Update gizmo pose
	self:set_pose(LevelEditor._reference_system == "world" and Matrix4x4.from_translation(selected:local_position()) or selected:world_pose())

	local tm = self:pose()
	local p  = self:position()
	local l  = LevelEditor:camera():screen_length_to_world_length(self:position(), Gizmo.size)
	local cam_z = Matrix4x4.z(LevelEditor:camera():local_pose())

	-- Select axis
	if self:is_idle() then
		local pos, dir = LevelEditor:camera():camera_ray(x, y)

		local axis = {
			Math.ray_disc_intersection(pos, dir, p, l, Matrix4x4.x(tm)),
			Math.ray_disc_intersection(pos, dir, p, l, Matrix4x4.y(tm)),
			Math.ray_disc_intersection(pos, dir, p, l, Matrix4x4.z(tm)),
			Math.ray_disc_intersection(pos, dir, p, l*1.25, -cam_z)
		}

		local nearest = nil
		local dist = math.huge
		local axis_names = { "x", "y", "z", "xyz" }
		for ii, name in ipairs(axis_names) do
			if axis[ii] ~= -1.0 and axis[ii] < dist then
				nearest = ii
				dist = axis[ii]
			end
		end
		self._selected = nearest and axis_names[nearest] or nil
	end

	function DebugLine.add_arc(lines, center, radius, normal, color, segments)
		if not segments then
			segments = 36
		end

		-- Pick a right axis.
		local right = Vector3(normal.z, normal.z , -normal.x-normal.y)
		if normal.z ~= 0 and -normal.x ~= normal.y then
			 right = Vector3(-normal.y-normal.z, normal.x, normal.x)
		end
		Vector3.normalize(right)

		local x = right * radius
		local y = Vector3.cross(right, normal) * radius
		local step = (2*math.pi) / (segments > 3 and segments or 3)
		local from = center - y

		local cam_to_center = Vector3.normalize(center-Matrix4x4.translation(LevelEditor:camera():local_pose()))

		for i=0,segments+1 do
			local t = step * i - (math.pi/2)
			local to = center + x*math.cos(t) + y*math.sin(t)

			local center_to_vertex = Vector3.normalize(to - center)
			local ctc_dot_ctv = Vector3.dot(center_to_vertex, cam_to_center)
			if ctc_dot_ctv <= 0 then
				DebugLine.add_line(lines, from, to, color)
			end
			from = to
		end
	end

	-- Drawing
	local lines = LevelEditor._lines_no_depth
	-- Draw major planes handles.
	DebugLine.add_arc(lines, p, l, Matrix4x4.x(tm), self._selected == "x" and Colors.axis_selected() or Colors.axis_x())
	DebugLine.add_arc(lines, p, l, Matrix4x4.y(tm), self._selected == "y" and Colors.axis_selected() or Colors.axis_y())
	DebugLine.add_arc(lines, p, l, Matrix4x4.z(tm), self._selected == "z" and Colors.axis_selected() or Colors.axis_z())
	-- Draw camera plane handle.
	DebugLine.add_circle(lines, p, l*1.25, cam_z, self._selected == "xyz" and Colors.axis_selected() or Colors.grid())


	if self:is_rotating() then
		local radius = self._selected == "xyz" and l*1.25 or l
		DebugLine.add_line(lines, self:position(), self:position() - radius*Vector3.normalize(self:position() - self:drag_start()), Colors.axis_selected())
		DebugLine.add_line(lines, self:position(), self:position() - radius*Vector3.normalize(self:position() - self:drag_offset(x, y)), Colors.axis_selected())
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
	elseif self:is_rotating() then
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
	elseif self:is_rotating() then
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
	end
end

ScaleTool = class(ScaleTool)

function ScaleTool:init()
	-- Data
	self._rotation    = QuaternionBox(Quaternion.identity()) -- Rotation of the gizmo.
	self._position    = Vector3Box(Vector3.zero())           -- Position of the gizmo.
	self._drag_offset = Vector3Box(Vector3.zero())
	self._start_positions = {} -- Initial positions of all selected objects.
	self._start_scales = {}    -- Initial scales of all selected objects.
	self._selected    = nil    -- Set of axes that have been selected.
	self.SCALE_MIN    = 0.01   -- Minimum absolute scale value for any given object.
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

function ScaleTool:axis_selected()
	return self._selected == "x"
		or self._selected == "y"
		or self._selected == "z"
		or self._selected == "xy"
		or self._selected == "yz"
		or self._selected == "xz"
		or self._selected == "xyz"
end

function ScaleTool:is_idle()
	return self._state == "idle"
end

function ScaleTool:is_scaling()
	return self._state == "scaling"
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
	local selected = LevelEditor._selection:last_selected_object()
	if not selected then
		return
	end

	local function transform(tm, offset)
		local m = Matrix4x4.copy(tm)
		Matrix4x4.set_translation(m, Matrix4x4.transform(tm, offset))
		return m
	end

	self:set_pose(selected:world_pose())

	local tm = self:world_pose()
	local p = self:position()
	local axis_len = LevelEditor:camera():screen_length_to_world_length(self:position(), Gizmo.size)
	local cam_z = Matrix4x4.z(LevelEditor:camera():local_pose())
	local cam_to_gizmo = Vector3.normalize(p-Matrix4x4.translation(LevelEditor:camera():local_pose()))

	if self:is_idle() then
		-- Select axis
		local pos, dir = LevelEditor:camera():camera_ray(x, y)

		local axis = {
			{ axis_enabled(self:x_axis(), cam_to_gizmo) , 1, Math.ray_obb_intersection(pos, dir, transform(tm, axis_len*Vector3(0.5, 0.0, 0.0)), axis_len*Vector3(0.50, 0.07, 0.07))    },
			{ axis_enabled(self:y_axis(), cam_to_gizmo) , 1, Math.ray_obb_intersection(pos, dir, transform(tm, axis_len*Vector3(0.0, 0.5, 0.0)), axis_len*Vector3(0.07, 0.50, 0.07))    },
			{ axis_enabled(self:z_axis(), cam_to_gizmo) , 1, Math.ray_obb_intersection(pos, dir, transform(tm, axis_len*Vector3(0.0, 0.0, 0.5)), axis_len*Vector3(0.07, 0.07, 0.50))    },
			{ plane_enabled(self:z_axis(), cam_to_gizmo), 1, Math.ray_obb_intersection(pos, dir, transform(tm, axis_len*Vector3(0.25, 0.25, 0   )), axis_len*Vector3(0.25, 0.25, 0.07)) },
			{ plane_enabled(self:x_axis(), cam_to_gizmo), 1, Math.ray_obb_intersection(pos, dir, transform(tm, axis_len*Vector3(0   , 0.25, 0.25)), axis_len*Vector3(0.07, 0.25, 0.25)) },
			{ plane_enabled(self:y_axis(), cam_to_gizmo), 1, Math.ray_obb_intersection(pos, dir, transform(tm, axis_len*Vector3(0.25, 0   , 0.25)), axis_len*Vector3(0.25, 0.07, 0.25)) },
			{ 1                                         , 2, Math.ray_disc_intersection(pos, dir, Matrix4x4.translation(tm), axis_len*1.25, cam_z)                                      }
		}

		local nearest = nil
		local dist = math.huge
		local prio = math.huge
		local axis_names = { "x", "y", "z", "xy", "yz", "xz", "xyz" }
		for ii, name in ipairs(axis_names) do
			if axis[ii][1] and axis[ii][2] <= prio and axis[ii][3] ~= -1.0 and axis[ii][3] < dist then
				nearest = ii
				dist = axis[ii][3]
				prio = axis[ii][2]
			end
		end
		self._selected = nearest and axis_names[nearest] or nil
	end

	-- Drawing
	local hs = 0.05 -- Axis handle half-size
	local lines = LevelEditor._lines_no_depth
	-- Draw axes.
	DebugLine.add_line(lines, p, p + Matrix4x4.x(tm)*axis_len*(1-2*hs), axis_color(self:x_axis(), cam_to_gizmo, self._selected == "x" and Colors.axis_selected() or Colors.axis_x()))
	DebugLine.add_line(lines, p, p + Matrix4x4.y(tm)*axis_len*(1-2*hs), axis_color(self:y_axis(), cam_to_gizmo, self._selected == "y" and Colors.axis_selected() or Colors.axis_y()))
	DebugLine.add_line(lines, p, p + Matrix4x4.z(tm)*axis_len*(1-2*hs), axis_color(self:z_axis(), cam_to_gizmo, self._selected == "z" and Colors.axis_selected() or Colors.axis_z()))
	-- Draw axis handles.
	DebugLine.add_obb(lines, transform(tm, axis_len*Vector3(1-hs, 0   , 0   )), axis_len*Vector3(hs, hs, hs), axis_color(self:x_axis(), cam_to_gizmo, self._selected == "x" and Colors.axis_selected() or Colors.axis_x()))
	DebugLine.add_obb(lines, transform(tm, axis_len*Vector3(0   , 1-hs, 0   )), axis_len*Vector3(hs, hs, hs), axis_color(self:y_axis(), cam_to_gizmo, self._selected == "y" and Colors.axis_selected() or Colors.axis_y()))
	DebugLine.add_obb(lines, transform(tm, axis_len*Vector3(0   , 0   , 1-hs)), axis_len*Vector3(hs, hs, hs), axis_color(self:z_axis(), cam_to_gizmo, self._selected == "z" and Colors.axis_selected() or Colors.axis_z()))
	-- Draw plane handles.
	DebugLine.add_line(lines, p + Matrix4x4.x(tm)*axis_len*0.5, p + Matrix4x4.x(tm)*axis_len*0.5 + Matrix4x4.y(tm)*axis_len*0.5, plane_color(self:z_axis(), cam_to_gizmo, self._selected == "xy" and Colors.axis_selected() or Colors.axis_x()))
	DebugLine.add_line(lines, p + Matrix4x4.y(tm)*axis_len*0.5, p + Matrix4x4.y(tm)*axis_len*0.5 + Matrix4x4.x(tm)*axis_len*0.5, plane_color(self:z_axis(), cam_to_gizmo, self._selected == "xy" and Colors.axis_selected() or Colors.axis_y()))
	DebugLine.add_line(lines, p + Matrix4x4.y(tm)*axis_len*0.5, p + Matrix4x4.y(tm)*axis_len*0.5 + Matrix4x4.z(tm)*axis_len*0.5, plane_color(self:x_axis(), cam_to_gizmo, self._selected == "yz" and Colors.axis_selected() or Colors.axis_y()))
	DebugLine.add_line(lines, p + Matrix4x4.z(tm)*axis_len*0.5, p + Matrix4x4.z(tm)*axis_len*0.5 + Matrix4x4.y(tm)*axis_len*0.5, plane_color(self:x_axis(), cam_to_gizmo, self._selected == "yz" and Colors.axis_selected() or Colors.axis_z()))
	DebugLine.add_line(lines, p + Matrix4x4.x(tm)*axis_len*0.5, p + Matrix4x4.x(tm)*axis_len*0.5 + Matrix4x4.z(tm)*axis_len*0.5, plane_color(self:y_axis(), cam_to_gizmo, self._selected == "xz" and Colors.axis_selected() or Colors.axis_x()))
	DebugLine.add_line(lines, p + Matrix4x4.z(tm)*axis_len*0.5, p + Matrix4x4.z(tm)*axis_len*0.5 + Matrix4x4.x(tm)*axis_len*0.5, plane_color(self:y_axis(), cam_to_gizmo, self._selected == "xz" and Colors.axis_selected() or Colors.axis_z()))
	-- Draw camera plane handle.
	DebugLine.add_circle(lines, p, axis_len*1.25, cam_z, self._selected == "xyz" and Colors.axis_selected() or Colors.grid())
end

function ScaleTool:mouse_move(x, y)
	if self:is_idle() then
		LevelEditor.select_tool:mouse_move(x, y)
	elseif self:axis_selected() then
		local end_scale = self:drag_offset(x, y)
		local start_scale = self._drag_offset:unbox()
		local scale_ratio = Vector3(end_scale.x/start_scale.x
			, end_scale.y/start_scale.y
			, end_scale.z/start_scale.z
			)

		-- Apply transformation to all selected objects.
		local selection = LevelEditor._selection:objects()
		for ii, obj in pairs(selection) do
			-- Apply scale.
			local obj_scale = self._start_scales[ii]:unbox()
			obj_scale.x = math.max(self.SCALE_MIN, obj_scale.x * scale_ratio.x)
			obj_scale.y = math.max(self.SCALE_MIN, obj_scale.y * scale_ratio.y)
			obj_scale.z = math.max(self.SCALE_MIN, obj_scale.z * scale_ratio.z)
			obj:set_local_scale(LevelEditor:snap(self:world_pose(), obj_scale) or obj_scale)

			-- Apply translation. Selected objects new positions are proportional to
			-- their distance from the gizmo times the scale factor.
			local obj_position = self._start_positions[ii]:unbox()
			local gizmo_position = self:position()
			local gizmo_obj_distance = obj_position - gizmo_position
			gizmo_obj_distance.x = gizmo_obj_distance.x * scale_ratio.x
			gizmo_obj_distance.y = gizmo_obj_distance.y * scale_ratio.y
			gizmo_obj_distance.z = gizmo_obj_distance.z * scale_ratio.z
			local obj_new_position = gizmo_position + gizmo_obj_distance
			obj:set_local_position(LevelEditor:snap(self:world_pose(), obj_new_position) or obj_new_position)
		end
	end
end

function ScaleTool:mouse_down(x, y)
	if self:is_idle() then
		if self:axis_selected() and LevelEditor._selection:last_selected_object() then
			self._drag_offset:store(self:drag_offset(x, y))

			-- Store initial positions and scales for all selected objects.
			local selection = LevelEditor._selection:objects()
			for _, obj in pairs(selection) do
				self._start_positions[#self._start_positions + 1] = Vector3Box(obj:local_position())
				self._start_scales[#self._start_scales + 1] = Vector3Box(obj:local_scale())
			end

			self:set_state("scaling")
		else
			LevelEditor.select_tool:mouse_down(x, y)
		end
	end
end

function ScaleTool:mouse_up(x, y)
	if self:is_scaling() then
		LevelEditor._selection:send_move_objects()

		self._drag_offset:store(Vector3(1, 1, 1))
		self._start_positions = {}
		self._start_scales = {}
	else
		LevelEditor.select_tool:mouse_up(x, y)
	end

	self:set_state("idle")
end

function ScaleTool:is_axis_selected(axis)
	return string.find(self._selected, axis)
end

function ScaleTool:drag_axis()
	if self._selected == "x" then return self:x_axis()
	elseif self._selected == "y" then return self:y_axis()
	elseif self._selected == "z" then return self:z_axis()
	else return nil end
end

function ScaleTool:drag_plane()
	local camera_dir = Matrix4x4.z(LevelEditor:camera():local_pose())
	if self._selected == "xy" then return self:position(), self:z_axis()
	elseif self._selected == "yz" then return self:position(), self:x_axis()
	elseif self._selected == "xz" then return self:position(), self:y_axis()
	elseif self._selected == "xyz" then return self:position(), -camera_dir
	else return nil end
end

function ScaleTool:drag_offset(x, y)
	local drag_axis = self:drag_axis()
	local drag_plane = self:drag_plane()
	local pos, dir = LevelEditor:camera():camera_ray(x, y)
	local distance = 1

	if drag_axis ~= nil then
		local _, dist = line_line(pos, pos + dir, self:position(), self:position() + drag_axis)
		distance = dist and math.max(0, dist) or 1
	elseif drag_plane ~= nil then
		local dist = Math.ray_plane_intersection(pos, dir, self:drag_plane())
		if dist ~= -1.0 then
			local point_on_plane = pos + dir*dist
			distance = Vector3.distance(point_on_plane, self:position())
		end
	end

	local scale = Vector3(1, 1, 1)
	for _, a in ipairs{"x", "y", "z"} do
		if self:is_axis_selected(a) then
			if a == "x" then scale.x = distance end
			if a == "y" then scale.y = distance end
			if a == "z" then scale.z = distance end
		end
	end
	return scale
end

LevelEditor = LevelEditor or {}

function LevelEditor:init()
	self._world = Device.create_world()
	World.disable_unit_callbacks(self._world)
	self._pw = World.physics_world(self._world)
	self._rw = World.render_world(self._world)
	self._sg = World.scene_graph(self._world)
	self._lines_no_depth = World.create_debug_line(self._world, false)
	self._lines = World.create_debug_line(self._world, true)
	self._camera = Camera(self._world, World.spawn_unit(self._world, "core/units/camera"))
	self._mouse = { x = 0, y = 0, x_last = 0, y_last = 0, button = { left = false, middle = false, right = false }, wheel = { delta = 0 }}
	self._keyboard = { ctrl = false, shift = false, alt = false }
	self._grid = { size = 1 }
	self._rotation_snap = 15.0 * math.pi / 180.0
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
	self.debug = false

	-- Spawn camera
	local pos = Vector3(20, 20, -20)
	local zero_pos = Vector3.zero() - pos
	local len = math.abs(Vector3.length(zero_pos))
	local dir = Vector3.normalize(zero_pos)
	self._camera._target_distance = len
	local tr = SceneGraph.instance(self._sg, self._camera:unit())
	SceneGraph.set_local_rotation(self._sg, tr, Quaternion.look(dir))
	SceneGraph.set_local_position(self._sg, tr, pos)
end

function LevelEditor:update(dt)
	World.update(self._world, dt)

	if self._show_grid then
		draw_world_origin_grid(self._lines, 10, self._grid.size)
	end

	self._mouse.dx = self._mouse.x - self._mouse.x_last
	self._mouse.dy = self._mouse.y - self._mouse.y_last
	self._mouse.x_last = self._mouse.x
	self._mouse.y_last = self._mouse.y

	self._camera:mouse_wheel(self._mouse.wheel.delta)
	self._camera:update(dt, self._mouse.dx, self._mouse.dy, self._keyboard, self._mouse)

	if self._camera:is_idle() then
		self.tool:mouse_move(self._mouse.x, self._mouse.y)
	end
	self.tool:update(dt, self._mouse.x, self._mouse.y)

	self._mouse.dx = 0
	self._mouse.dy = 0
	self._mouse.wheel.delta = 0

	local pos, dir = self._camera:camera_ray(self._mouse.x, self._mouse.y)
	local selected_object, t = raycast(self._objects, pos, dir)
	self._spawn_height = selected_object and (pos + dir * t).y or 0

	-- Draw level objects
	for k, v in pairs(self._objects) do
		self._objects[k]:draw()
	end
end

function LevelEditor:render(dt)
	Device.render(self._world, self._camera:unit())

	DebugLine.submit(self._lines)
	DebugLine.reset(self._lines)
	DebugLine.submit(self._lines_no_depth)
	DebugLine.reset(self._lines_no_depth)
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

function LevelEditor:mouse_wheel(delta)
	self._mouse.wheel.delta = self._mouse.wheel.delta + delta;
end

function LevelEditor:mouse_down(x, y)
	if self._camera:is_idle() then
		self.tool:mouse_down(x, y)
	end
end

function LevelEditor:mouse_up(x, y)
	if self._camera:is_idle() then
		self.tool:mouse_up(x, y)
	end
end

function LevelEditor:key_down(key)
	if (key == "w") then self._keyboard.wkey = true end
	if (key == "a") then self._keyboard.akey = true end
	if (key == "s") then self._keyboard.skey = true end
	if (key == "d") then self._keyboard.dkey = true end
	if (key == "ctrl_left") then self._keyboard.ctrl = true end
	if (key == "shift_left") then self._keyboard.shift = true end
	if (key == "alt_left") then self._keyboard.alt = true end
end

function LevelEditor:key_up(key)
	if (key == "w") then self._keyboard.wkey = false end
	if (key == "a") then self._keyboard.akey = false end
	if (key == "s") then self._keyboard.skey = false end
	if (key == "d") then self._keyboard.dkey = false end
	if (key == "ctrl_left") then self._keyboard.ctrl = false end
	if (key == "shift_left") then self._keyboard.shift = false end
	if (key == "alt_left") then self._keyboard.alt = false end
end

function LevelEditor:camera()
	return self._camera
end

function LevelEditor:set_tool(tool)
	if self.tool == tool then
		return
	end

	if self.tool.on_leave ~= nil then
		self.tool:on_leave()
	end

	self.tool = tool

	if self.tool.on_enter ~= nil then
		self.tool:on_enter()
	end
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
	local pos, dir = self._camera:camera_ray(x, y)
	local spawn_height = LevelEditor._spawn_height
	local point = Vector3(0, spawn_height, 0)
	local normal = Vector3.up()
	local t = Math.ray_plane_intersection(pos, dir, point, normal)
	local target = t == -1.0 and point or pos + dir * t
	local result = LevelEditor:snap(Matrix4x4.identity(), target) or target
	if self.debug then
		-- Debug
		DebugLine.add_sphere(self._lines_no_depth
			, target
			, 0.1
			, Color4.green()
			)
		DebugLine.add_sphere(self._lines_no_depth
			, result
			, 0.1
			, Color4.red()
			)
	end
	return result
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

function LevelEditor:spawn_sound(id, name, pos, rot, range, volume, loop)
	local sound = SoundObject(self._world, id, name, range, volume, loop)
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
	RenderWorld.mesh_create(self._rw, unit_id, mesh_resource, geometry_name, material_resource, visible, unit_box:world_pose())
end

function LevelEditor:add_sprite_component(id, component_id, sprite_resource, material_resource, layer, depth, visible)
	local unit_box = self._objects[id]
	local unit_id = unit_box:unit_id();
	RenderWorld.sprite_create(self._rw, unit_id, sprite_resource, material_resource, layer, depth, visible, unit_box:world_pose())
end

function LevelEditor:add_light_component(id, component_id, type, range, intensity, spot_angle, color)
	local unit_box = self._objects[id]
	local unit_id = unit_box:unit_id()
	RenderWorld.light_create(self._rw, unit_id, type, range, intensity, spot_angle, color, unit_box:world_pose())
end

function LevelEditor:add_camera_component(id, component_id, projection, fov, far_range, near_range)
	local unit_box = self._objects[id]
	local unit_id = unit_box:unit_id();
	World.camera_create(self._world, unit_id, projection, fov, far_range, near_range, unit_box:world_pose());
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

function LevelEditor:destroy(id)
	self._objects[id]:destroy()
	self._objects[id] = nil
	self._selection:remove(id)
	self._selection:send()
end

function LevelEditor:camera_view_perspective()
	self._camera:set_perspective()
end

function LevelEditor:camera_view_front()
	self._camera:set_orthographic(Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(0, 0, 1), Vector3(0, 1, 0))
end

function LevelEditor:camera_view_back()
	self._camera:set_orthographic(Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(0, 0, -1), Vector3(0, 1, 0))
end

function LevelEditor:camera_view_right()
	self._camera:set_orthographic(Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(-1, 0, 0), Vector3(0, 1, 0))
end

function LevelEditor:camera_view_left()
	self._camera:set_orthographic(Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(1, 0, 0), Vector3(0, 1, 0))
end

function LevelEditor:camera_view_top()
	self._camera:set_orthographic(Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(0, -1, 0), Vector3(0, 0, 1))
end

function LevelEditor:camera_view_bottom()
	self._camera:set_orthographic(Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(0, 1, 0), Vector3(0, 0, 1))
end

function LevelEditor:camera_drag_start(mode)
	self._camera:set_mode(mode, self._mouse.x, self._mouse.y)
end
