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

		local camera_pos = Vector3(1, 1, 1)
		local camera_rot = Quaternion.look(Vector3.normalize(-camera_pos))
		local camera_pose = Matrix4x4.from_quaternion_translation(camera_rot, camera_pos)

		self._camera:set_local_pose(camera_pose)
		self._camera:frame_obb(obb_tm, obb_he)
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
