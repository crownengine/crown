-- Copyright (c) 2012-2023 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/lua/class"

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
	self._rw = World.render_world(world)
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
	RenderWorld.selection(self._rw, self._unit_id, selected);
end

-- Returns the Oriented Bounding-Box of the unit.
function UnitBox:obb()
	local mesh = RenderWorld.mesh_instance(self._rw, self._unit_id)
	if mesh then
		return RenderWorld.mesh_obb(self._rw, mesh)
	end

	local sprite = RenderWorld.sprite_instance(self._rw, self._unit_id)
	if sprite then
		return RenderWorld.sprite_obb(self._rw, sprite)
	end

	return Matrix4x4.identity(), Vector3(1, 1, 1)
end

function UnitBox:raycast(pos, dir)
	local mesh = RenderWorld.mesh_instance(self._rw, self._unit_id)
	if mesh then
		return RenderWorld.mesh_cast_ray(self._rw, mesh, pos, dir)
	end

	local sprite = RenderWorld.sprite_instance(self._rw, self._unit_id)
	if sprite then
		return RenderWorld.sprite_cast_ray(self._rw, sprite, pos, dir)
	end

	return -1.0
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

function UnitBox:set_mesh(material, visible)
	local mesh = RenderWorld.mesh_instance(self._rw, self._unit_id)
	RenderWorld.mesh_set_material(self._rw, mesh, material)
	RenderWorld.mesh_set_visible(self._rw, mesh, visible)
end

function UnitBox:set_sprite(sprite_resource_name, material, layer, depth, visible)
	local sprite = RenderWorld.sprite_instance(self._rw, self._unit_id)
	RenderWorld.sprite_set_sprite(self._rw, sprite, sprite_resource_name)
	RenderWorld.sprite_set_material(self._rw, sprite, material)
	RenderWorld.sprite_set_layer(self._rw, sprite, layer)
	RenderWorld.sprite_set_depth(self._rw, sprite, depth)
	RenderWorld.sprite_set_visible(self._rw, sprite, visible)
end

function UnitBox:set_camera(projection, fov, near_range, far_range)
	local camera = World.camera_instance(self._world, self._unit_id)
	World.camera_set_projection_type(self._world, camera, projection)
	World.camera_set_fov(self._world, camera, fov)
	World.camera_set_near_clip_distance(self._world, camera, near_range)
	World.camera_set_far_clip_distance(self._world, camera, far_range)
end
