-- Copyright (c) 2012-2025 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/lua/class"

UnitUtils = UnitUtils or {}

function UnitUtils.collect_children(scene_graph, unit_id, children)
	local transform = SceneGraph.instance(scene_graph, unit_id)
	if transform == nil then return end

	local cur_child = SceneGraph.first_child(scene_graph, transform)
	while cur_child ~= nil do
		local child_id = SceneGraph.owner(scene_graph, cur_child)
		UnitUtils.collect_children(scene_graph, child_id, children)
		cur_child = SceneGraph.next_sibling(scene_graph, cur_child)
	end

	table.insert(children, unit_id)
end

function UnitUtils.freeze(world, unit_id)
	local physics_world = World.physics_world(world)
	local scene_graph = World.scene_graph(world)
	local children = {}
	UnitUtils.collect_children(scene_graph, unit_id, children)

	for _, child_id in ipairs(children) do
		local actor = PhysicsWorld.actor_instance(physics_world, child_id)
		if actor ~= nil then
			PhysicsWorld.actor_set_kinematic(physics_world, actor, true)
			PhysicsWorld.actor_disable_collision(physics_world, actor)
		end
	end
end

function UnitUtils.destroy_tree(world, unit_id)
	local scene_graph = World.scene_graph(world)
	local children = {}
	UnitUtils.collect_children(scene_graph, unit_id, children)

	for _, child_id in ipairs(children) do
		World.destroy_unit(world, child_id)
	end
end

UnitBox = class(UnitBox)

function UnitBox:init(world, id, unit_id, prefab)
	self._world = world
	self._rw = World.render_world(world)
	self._id = id
	self._unit_id = unit_id
	self._prefab = prefab
	self._sg = World.scene_graph(world)
	self._selected = false
	self._obb = { pose = Matrix4x4Box(), half_extents = Vector3Box(), dirty = true }

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
	UnitUtils.destroy_tree(self._world, self._unit_id)
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
	if tr then
		SceneGraph.set_local_position(self._sg, tr, pos)
		self._obb.dirty = true
	end
end

function UnitBox:set_local_rotation(rot)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then
		SceneGraph.set_local_rotation(self._sg, tr, rot)
		self._obb.dirty = true
	end
end

function UnitBox:set_local_scale(scale)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then
		SceneGraph.set_local_scale(self._sg, tr, scale)
		self._obb.dirty = true
	end
end

function UnitBox:set_local_pose(pose)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	if tr then
		SceneGraph.set_local_pose(self._sg, tr, pose)
		self._obb.dirty = true
	end
end

function UnitBox:on_selected(selected)
	self._selected = selected

	-- Select this unit and all its children.
	local scene_graph = self._sg
	local unit_id = self._unit_id

	local transform = SceneGraph.instance(scene_graph, unit_id)
	if transform == nil then
		return
	end

	local cur_child = SceneGraph.first_child(scene_graph, transform)
	while cur_child ~= nil do
		local child_id = SceneGraph.owner(scene_graph, cur_child)
		RenderWorld.selection(self._rw, child_id, selected);
		cur_child = SceneGraph.next_sibling(scene_graph, cur_child)
	end

	RenderWorld.selection(self._rw, self._unit_id, selected);
end

function UnitBox:mesh_tree_obb()
	local scene_graph = self._sg
	local unit_id = self._unit_id
	local obb_tm = self:local_pose()
	local obb_he = Vector3(0.01, 0.01, 0.01)

	local child_id = unit_id
	local mesh = RenderWorld.mesh_instance(self._rw, child_id)
	if mesh then
		obb_tm, obb_he = Math.obb_merge(obb_tm, obb_he, RenderWorld.mesh_obb(self._rw, mesh))
	end

	local transform = SceneGraph.instance(scene_graph, unit_id)
	if transform ~= nil then
		local cur_child = SceneGraph.first_child(scene_graph, transform)
		while cur_child ~= nil do
			child_id = SceneGraph.owner(scene_graph, cur_child)
			mesh = RenderWorld.mesh_instance(self._rw, child_id)
			if mesh then
				obb_tm, obb_he = Math.obb_merge(obb_tm, obb_he, RenderWorld.mesh_obb(self._rw, mesh))
			end

			cur_child = SceneGraph.next_sibling(scene_graph, cur_child)
		end
	end

	return obb_tm, obb_he
end

function UnitBox:sprite_tree_obb()
	local scene_graph = self._sg
	local unit_id = self._unit_id
	local obb_tm = self:local_pose()
	local obb_he = Vector3(0.01, 0.01, 0.01)

	local child_id = unit_id
	local mesh = RenderWorld.sprite_instance(self._rw, child_id)
	if mesh then
		obb_tm, obb_he = Math.obb_merge(obb_tm, obb_he, RenderWorld.sprite_obb(self._rw, mesh))
	end

	local transform = SceneGraph.instance(scene_graph, unit_id)
	if transform ~= nil then
		local cur_child = SceneGraph.first_child(scene_graph, transform)
		while cur_child ~= nil do
			child_id = SceneGraph.owner(scene_graph, cur_child)
			mesh = RenderWorld.sprite_instance(self._rw, child_id)
			if mesh then
				obb_tm, obb_he = Math.obb_merge(obb_tm, obb_he, RenderWorld.sprite_obb(self._rw, mesh))
			end

			cur_child = SceneGraph.next_sibling(scene_graph, cur_child)
		end
	end

	return obb_tm, obb_he
end

-- Returns the Oriented Bounding-Box of the unit.
function UnitBox:obb()
	if self._obb.dirty then
		local obb_tm, obb_he = self:mesh_tree_obb()
		obb_tm, obb_he = Math.obb_merge(obb_tm, obb_he, self:sprite_tree_obb())
		self._obb.pose:store(obb_tm)
		self._obb.half_extents:store(obb_he)
		self._obb.dirty = false
	end

	return self._obb.pose:unbox(), self._obb.half_extents:unbox()
end

function UnitBox:raycast_mesh_tree(pos, dir)
	local scene_graph = self._sg
	local unit_id = self._unit_id
	local t_min = math.huge

	-- Raycast meshes in this unit and all its children.
	local child_id = unit_id
	local mesh = RenderWorld.mesh_instance(self._rw, child_id)
	if mesh then
		local t = RenderWorld.mesh_cast_ray(self._rw, mesh, pos, dir)
		if t ~= -1.0 then
			t_min = math.min(t, t_min)
		end
	end

	local transform = SceneGraph.instance(scene_graph, unit_id)
	if transform == nil then
		return -1.0
	end

	local cur_child = SceneGraph.first_child(scene_graph, transform)
	while cur_child ~= nil do
		child_id = SceneGraph.owner(scene_graph, cur_child)
		mesh = RenderWorld.mesh_instance(self._rw, child_id)
		if mesh then
			t = RenderWorld.mesh_cast_ray(self._rw, mesh, pos, dir)
			if t ~= -1.0 then
				t_min = math.min(t, t_min)
			end
		end
		cur_child = SceneGraph.next_sibling(scene_graph, cur_child)
	end

	return t_min == math.huge and -1.0 or t_min
end

function UnitBox:raycast_sprite_tree(pos, dir)
	local scene_graph = self._sg
	local unit_id = self._unit_id
	local t_min = math.huge

	-- Raycast sprites in this unit and all its children.
	local child_id = unit_id
	local mesh = RenderWorld.sprite_instance(self._rw, child_id)
	if mesh then
		local t = RenderWorld.sprite_cast_ray(self._rw, mesh, pos, dir)
		if t ~= -1.0 then
			t_min = math.min(t, t_min)
		end
	end

	local transform = SceneGraph.instance(scene_graph, unit_id)
	if transform == nil then
		return -1.0
	end

	local cur_child = SceneGraph.first_child(scene_graph, transform)
	while cur_child ~= nil do
		child_id = SceneGraph.owner(scene_graph, cur_child)
		mesh = RenderWorld.sprite_instance(self._rw, child_id)
		if mesh then
			t = RenderWorld.sprite_cast_ray(self._rw, mesh, pos, dir)
			if t ~= -1.0 then
				t_min = math.min(t, t_min)
			end
		end
		cur_child = SceneGraph.next_sibling(scene_graph, cur_child)
	end

	return t_min == math.huge and -1.0 or t_min
end

function UnitBox:raycast(pos, dir)
	local obb_tm, obb_he = self:obb()
	if Math.ray_obb_intersection(pos, dir, obb_tm, obb_he) == -1.0 then
		return -1.0
	end

	local t = self:raycast_mesh_tree(pos, dir)
	if t ~= -1.0 then
		return t
	end

	return self:raycast_sprite_tree(pos, dir)
end

function UnitBox:draw()
	if self._selected then
		local light = RenderWorld.light_instance(self._rw, self._unit_id)
		if light then
			RenderWorld.light_debug_draw(self._rw, light, LevelEditor._lines)
		end
	end
end

function UnitBox:set_light(type, range, intensity, angle, color)
	local light = RenderWorld.light_instance(self._rw, self._unit_id)
	RenderWorld.light_set_type(self._rw, light, type)
	RenderWorld.light_set_color(self._rw, light, color)
	RenderWorld.light_set_range(self._rw, light, range)
	RenderWorld.light_set_intensity(self._rw, light, intensity)
	RenderWorld.light_set_spot_angle(self._rw, light, angle)
end

function UnitBox:set_mesh(mesh_resource, geometry, material, visible)
	local mesh = RenderWorld.mesh_instance(self._rw, self._unit_id)
	RenderWorld.mesh_set_geometry(self._rw, mesh, mesh_resource, geometry)
	RenderWorld.mesh_set_material(self._rw, mesh, material)
	RenderWorld.mesh_set_visible(self._rw, mesh, visible)
	self._obb.dirty = true
end

function UnitBox:set_sprite(sprite_resource_name, material, layer, depth, visible)
	local sprite = RenderWorld.sprite_instance(self._rw, self._unit_id)
	RenderWorld.sprite_set_sprite(self._rw, sprite, sprite_resource_name)
	RenderWorld.sprite_set_material(self._rw, sprite, material)
	RenderWorld.sprite_set_layer(self._rw, sprite, layer)
	RenderWorld.sprite_set_depth(self._rw, sprite, depth)
	RenderWorld.sprite_set_visible(self._rw, sprite, visible)
	self._obb.dirty = true
end

function UnitBox:set_camera(projection, fov, near_range, far_range)
	local camera = World.camera_instance(self._world, self._unit_id)
	World.camera_set_projection_type(self._world, camera, projection)
	World.camera_set_fov(self._world, camera, fov)
	World.camera_set_near_clip_distance(self._world, camera, near_range)
	World.camera_set_far_clip_distance(self._world, camera, far_range)
end

function UnitBox:send()
	Device.console_send { type = "unit_spawned"
		, id = self._id
		, name = self:prefab()
		, position = self:local_position()
		, rotation = self:local_rotation()
		, scale = self:local_scale()
		}
end
