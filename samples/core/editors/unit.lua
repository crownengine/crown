-- Copyright (c) 2012-2026 Daniele Bartolini et al.
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

	self:freeze()
end

function UnitBox:freeze()
	UnitUtils.freeze(self._world, self._unit_id)
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

function UnitBox:is_spatial()
	return SceneGraph.instance(self._sg, self._unit_id) ~= nil
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

function UnitBox:set_parent(parent_id)
	local tr = SceneGraph.instance(self._sg, self._unit_id)
	local parent_unit = LevelEditor._objects[parent_id]:unit_id()
	local parent_tr = SceneGraph.instance(self._sg, parent_unit)

	if tr and parent_tr then
		SceneGraph.link(self._sg
			, parent_tr
			, tr
			, self:local_position()
			, self:local_rotation()
			, self:local_scale()
			)
		self._obb.dirty = true
	end
end

function UnitBox:on_selected(selected)
	local scene_graph = self._sg
	local unit_id = self._unit_id
	local children = {}
	UnitUtils.collect_children(scene_graph, unit_id, children)

	for _, child_id in ipairs(children) do
		RenderWorld.selection(self._rw, child_id, selected);
	end

	self._selected = selected
end

function UnitBox:mesh_tree_obb()
	local scene_graph = self._sg
	local unit_id = self._unit_id
	local obb_tm = self:local_pose()
	local obb_he = Vector3(0.01, 0.01, 0.01)
	local children = {}
	UnitUtils.collect_children(scene_graph, unit_id, children)

	for _, child_id in ipairs(children) do
		local mesh = RenderWorld.mesh_instance(self._rw, child_id)
		if mesh then
			obb_tm, obb_he = Math.obb_merge(obb_tm, obb_he, RenderWorld.mesh_obb(self._rw, mesh))
		end
	end

	return obb_tm, obb_he
end

function UnitBox:sprite_tree_obb()
	local scene_graph = self._sg
	local unit_id = self._unit_id
	local obb_tm = self:local_pose()
	local obb_he = Vector3(0.01, 0.01, 0.01)
	local children = {}
	UnitUtils.collect_children(scene_graph, unit_id, children)

	for _, child_id in ipairs(children) do
		local sprite = RenderWorld.sprite_instance(self._rw, child_id)
		if sprite then
			obb_tm, obb_he = Math.obb_merge(obb_tm, obb_he, RenderWorld.sprite_obb(self._rw, sprite))
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

	local children = {}
	UnitUtils.collect_children(scene_graph, unit_id, children)

	for _, child_id in ipairs(children) do
		local mesh = RenderWorld.mesh_instance(self._rw, child_id)
		if mesh then
			local t = RenderWorld.mesh_cast_ray(self._rw, mesh, pos, dir)
			if t ~= -1.0 then
				t_min = math.min(t, t_min)
			end
		end
	end

	return t_min == math.huge and -1.0 or t_min
end

function UnitBox:raycast_sprite_tree(pos, dir)
	local scene_graph = self._sg
	local unit_id = self._unit_id
	local t_min = math.huge
	local children = {}
	UnitUtils.collect_children(scene_graph, unit_id, children)

	for _, child_id in ipairs(children) do
		local sprite = RenderWorld.sprite_instance(self._rw, child_id)
		if sprite then
			local t = RenderWorld.sprite_cast_ray(self._rw, sprite, pos, dir)
			if t ~= -1.0 then
				t_min = math.min(t, t_min)
			end
		end
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
	if not self._selected then
		return
	end

	local light = RenderWorld.light_instance(self._rw, self._unit_id)
	if light then
		RenderWorld.light_debug_draw(self._rw, light, LevelEditor._lines)
	end

	local physics_world = World.physics_world(self._world)
	local mover = PhysicsWorld.mover_instance(physics_world, self._unit_id)
	if mover then
		PhysicsWorld.mover_debug_draw(physics_world, mover, LevelEditor._lines)
	end
end

function UnitBox:set_light(type, range, intensity, angle, color, bias, cast_shadows)
	local light = RenderWorld.light_instance(self._rw, self._unit_id)
	if light then
		RenderWorld.light_set_type(self._rw, light, type)
		RenderWorld.light_set_color(self._rw, light, color)
		RenderWorld.light_set_range(self._rw, light, range)
		RenderWorld.light_set_intensity(self._rw, light, intensity)
		RenderWorld.light_set_spot_angle(self._rw, light, angle)
		RenderWorld.light_set_shadow_bias(self._rw, light, bias)
		RenderWorld.light_set_cast_shadows(self._rw, light, cast_shadows)
	end
end

function UnitBox:set_animation_state_machine(state_machine_resource)
	local animation_state_machine = World.animation_state_machine(self._world)
	local state_machine = AnimationStateMachine.instance(animation_state_machine, self._unit_id)
	if state_machine then
		AnimationStateMachine.set_state_machine(animation_state_machine, state_machine, state_machine_resource)
	end
end

function UnitBox:set_mover(height, radius, max_slope_angle, center)
	local physics_world = World.physics_world(self._world)
	local mover = PhysicsWorld.mover_instance(physics_world, self._unit_id)
	if mover then
		PhysicsWorld.mover_set_height(physics_world, mover, height)
		PhysicsWorld.mover_set_radius(physics_world, mover, radius)
		PhysicsWorld.mover_set_max_slope_angle(physics_world, mover, max_slope_angle)
		PhysicsWorld.mover_set_center(physics_world, mover, center)
	end
end

function UnitBox:set_fog(color, density, range_min, range_max, sun_blend, enabled)
	local fog = RenderWorld.fog_instance(self._rw, self._unit_id)
	if fog then
		RenderWorld.fog_set_color(self._rw, fog, color)
		RenderWorld.fog_set_density(self._rw, fog, density)
		RenderWorld.fog_set_range_min(self._rw, fog, range_min)
		RenderWorld.fog_set_range_max(self._rw, fog, range_max)
		RenderWorld.fog_set_sun_blend(self._rw, fog, sun_blend)
		RenderWorld.fog_set_enabled(self._rw, fog, enabled)
	end
end

function UnitBox:set_global_lighting(skydome_map, skydome_intensity, ambient_color)
	local gl = RenderWorld.global_lighting_instance(self._rw, self._unit_id)
	if gl then
		RenderWorld.global_lighting_set_skydome_map(self._rw, skydome_map)
		RenderWorld.global_lighting_set_skydome_intensity(self._rw, skydome_intensity)
		RenderWorld.global_lighting_set_ambient_color(self._rw, ambient_color)
	end
end

function UnitBox:set_bloom(enabled, threshold, weight, intensity)
	local bloom = RenderWorld.bloom_instance(self._rw, self._unit_id)
	if bloom then
		RenderWorld.bloom_set_enabled(self._rw, enabled)
		RenderWorld.bloom_set_threshold(self._rw, threshold)
		RenderWorld.bloom_set_weight(self._rw, weight)
		RenderWorld.bloom_set_intensity(self._rw, intensity)
	end
end

function UnitBox:set_tonemap(type)
	local tonemap = RenderWorld.tonemap_instance(self._rw, self._unit_id)
	if tonemap then
		RenderWorld.tonemap_set_type(self._rw, type)
	end
end

function UnitBox:set_mesh(mesh_resource, geometry, material, visible, cast_shadows)
	local mesh = RenderWorld.mesh_instance(self._rw, self._unit_id)
	if mesh then
		RenderWorld.mesh_set_geometry(self._rw, mesh, mesh_resource, geometry)
		RenderWorld.mesh_set_material(self._rw, mesh, material)
		RenderWorld.mesh_set_visible(self._rw, mesh, visible)
		RenderWorld.mesh_set_cast_shadows(self._rw, mesh, cast_shadows)
		self._obb.dirty = true
	end
end

function UnitBox:set_sprite(sprite_resource_name, material, layer, depth, visible, flip_x, flip_y)
	local sprite = RenderWorld.sprite_instance(self._rw, self._unit_id)
	if sprite then
		RenderWorld.sprite_set_sprite(self._rw, sprite, sprite_resource_name)
		RenderWorld.sprite_set_material(self._rw, sprite, material)
		RenderWorld.sprite_set_layer(self._rw, sprite, layer)
		RenderWorld.sprite_set_depth(self._rw, sprite, depth)
		RenderWorld.sprite_set_visible(self._rw, sprite, visible)
		RenderWorld.sprite_flip_x(self._rw, sprite, flip_x)
		RenderWorld.sprite_flip_y(self._rw, sprite, flip_y)
		self._obb.dirty = true
	end
end

function UnitBox:set_camera(projection, fov, near_range, far_range)
	local camera = World.camera_instance(self._world, self._unit_id)
	if camera then
		World.camera_set_projection_type(self._world, camera, projection)
		World.camera_set_fov(self._world, camera, fov)
		World.camera_set_near_clip_distance(self._world, camera, near_range)
		World.camera_set_far_clip_distance(self._world, camera, far_range)
	end
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
