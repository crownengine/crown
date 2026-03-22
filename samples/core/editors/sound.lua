-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/lua/class"

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
	self._hidden = false
	self._selectable = true
	self._mesh_visible = true
end

function SoundObject:id()
	return self._id
end

function SoundObject:unit_id()
	return self._unit_id
end

function SoundObject:destroy()
	UnitUtils.destroy_tree(self._world, self._unit_id)
end

function SoundObject:name()
	return self._name
end

function SoundObject:is_spatial()
	return true
end

function SoundObject:is_selectable()
	return self._selectable and not self._hidden
end

function SoundObject:set_selectable(selectable)
	self._selectable = selectable
	if not selectable then
		self:on_selected(false)
	end
end

function SoundObject:set_hidden(hidden)
	if self._hidden == hidden then
		return
	end

	self._hidden = hidden
	if hidden then
		self:on_selected(false)
	end

	local rw = LevelEditor._rw
	local mesh = RenderWorld.mesh_instance(rw, self._unit_id)
	if mesh then
		RenderWorld.mesh_set_visible(rw, mesh, self._mesh_visible and not self._hidden)
	end
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
	RenderWorld.selection(LevelEditor._rw, self._unit_id, selected);
end

function SoundObject:obb()
	local rw = LevelEditor._rw
	return RenderWorld.mesh_obb(rw, RenderWorld.mesh_instance(rw, self._unit_id))
end

function SoundObject:raycast(pos, dir)
	if not self:is_selectable() then
		return -1.0
	end

	local rw = LevelEditor._rw
	local mesh = RenderWorld.mesh_instance(rw, self._unit_id)
	local tm, hext = RenderWorld.mesh_obb(rw, mesh)
	return RenderWorld.mesh_cast_ray(rw, mesh, pos, dir)
end

function SoundObject:draw()
	if self._hidden or not self._selected then
		return
	end

	DebugLine.add_sphere(LevelEditor._lines
		, self:local_position()
		, self._range
		, Color4.yellow()
		)
end

function SoundObject:set_range(range)
	self._range = range
end

function SoundObject:send()
	Device.console_send { type = "sound_spawned"
		, id = self._id
		, name = self:name()
		, position = self:local_position()
		, rotation = self:local_rotation()
		, scale = self:local_scale()
		, range = self._range
		, volume = self._volume
		, loop = self._loop
		}
end
