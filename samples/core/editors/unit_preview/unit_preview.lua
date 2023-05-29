-- Copyright (c) 2012-2023 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/editors/level_editor/camera"
require "core/editors/unit"

UnitPreview = UnitPreview or {}

function UnitPreview:init()
	self._world = Device.create_world()
	self._sg = World.scene_graph(self._world)
	self._rw = World.render_world(self._world)
	self._pw = World.physics_world(self._world)
	self._camera = Camera(self._world, World.spawn_unit(self._world, "core/units/camera"))
	self._object = nil

	World.spawn_unit(self._world, "core/units/light", Vector3(1000, 1000, -1000))
end

function UnitPreview:update(dt)
	World.update(self._world, dt)

	if self._object then
		local obb_tm, obb_he = self._object:obb()

		local radius = Vector3.length(obb_he)
		radius = math.ceil(radius / 2)  * 2
		radius = radius <   1 and   1 or radius
		radius = radius > 100 and 100 or radius

		local camera_unit = self._camera:unit()
		local pos = Vector3(radius, radius, -radius) * 2
		local camera_pos = Matrix4x4.translation(obb_tm) + pos
		local target_pos = Matrix4x4.translation(obb_tm)
		local tr = SceneGraph.instance(self._sg, camera_unit)
		SceneGraph.set_local_rotation(self._sg, tr, Quaternion.look(Vector3.normalize(target_pos - camera_pos)))
		SceneGraph.set_local_position(self._sg, tr, camera_pos)
	end

	self._camera:update(dt, 0, 0, {})
end

function UnitPreview:render(dt)
	Device.render(self._world, self._camera:unit())
end

function UnitPreview:shutdown()
	Device.destroy_world(self._world)
end

function UnitPreview:set_preview_resource(type, name)
	if type == "sound" then
		name = "core/units/sound"
	elseif type ~= "unit" then
		return
	end

	if self._object ~= nil then
		if self._object:prefab() == name then return end
		self._object:destroy()
	end

	self._object = UnitBox(self._world
		, Device.guid()
		, World.spawn_unit(self._world, name)
		, name
		)
end
