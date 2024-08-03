-- Copyright (c) 2012-2024 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/editors/level_editor/camera"
require "core/editors/unit"

Thumbnail = Thumbnail or {}

function Thumbnail:init()
	self._world = Device.create_world()
	self._sg = World.scene_graph(self._world)
	self._rw = World.render_world(self._world)
	self._pw = World.physics_world(self._world)
	self._camera = Camera(self._world, World.spawn_unit(self._world, "core/units/camera"))
	self._object = nil
	self._gui = World.create_screen_gui(self._world)
	self._requests = {}
	self._pending = {}

	World.spawn_unit(self._world, "core/units/light", Vector3(1000, 1000, -1000))
end

function Thumbnail:thumbnail_ready(screenshot_path)
	local req = table.remove(self._pending, 1)
	if req == nil then
		return
	end
	Device.console_send { type = "thumbnail", resource_type = req.type, resource_name = req.name, path = req.path }
end

function Thumbnail:update(dt)
	World.update(self._world, dt)

	local req = table.remove(self._requests, 1)
	if req == nil then
		return
	end

	if req.type == "sound" then
		name = "core/units/sound"
	elseif req.type ~= "unit" then
		return
	end

	if self._object ~= nil then
		if self._object:prefab() == req.name then return end
		self._object:destroy()
	end

	self._object = UnitBox(self._world
		, Device.guid()
		, World.spawn_unit(self._world, req.name)
		, req.name
		)

	if self._object then
		local obb_tm, obb_he = self._object:obb()

		local camera_pos
		local camera_rot

		if RenderWorld.sprite_instance(self._rw, self._object._unit_id) then
			camera_pos = Vector3(0, 1, 0)
			camera_rot = Quaternion.look(Vector3.normalize(-camera_pos), Vector3(0, 0, 1))
		else
			camera_pos = Vector3(1, 1, -1)
			camera_rot = Quaternion.look(Vector3.normalize(-camera_pos))
		end

		local camera_pose = Matrix4x4.from_quaternion_translation(camera_rot, camera_pos)

		self._camera:set_local_pose(camera_pose)
		self._camera:frame_obb(obb_tm, obb_he)
	end

	self._camera:update(dt, 0, 0, {})

	Device.render(self._world, self._camera:unit())
	table.insert(self._pending, req)
	Device.screenshot(req.path)
end

function Thumbnail:render(dt)
end

function Thumbnail:shutdown()
	World.destroy_gui(self._world, self._gui)
	Device.destroy_world(self._world)
end

function Thumbnail:add_request(type, name, thumbnail_path)
	table.insert(self._requests, { type = type, name = name, path = thumbnail_path })
end

function init()
	Device.enable_resource_autoload(true)
	Thumbnail:init()
end

function update(dt)
	Thumbnail:update(dt)
end

function render(dt)
	Thumbnail:render(dt)
end

function shutdown()
	Thumbnail:shutdown()
end

function screenshot(path)
	Thumbnail:thumbnail_ready(path)
end

