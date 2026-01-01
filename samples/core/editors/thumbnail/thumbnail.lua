-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/editors/level_editor/camera"
require "core/editors/unit"

UnitPreview = class(UnitPreview)

function UnitPreview:init(world, unit_name)
	self._object = UnitBox(world
		, Device.guid()
		, World.spawn_unit(world, unit_name)
		, unit_name
		)
end

function UnitPreview:destroy()
	self._object:destroy()
end

function UnitPreview:render(camera)
	local obb_tm, obb_he = self._object:obb()

	local camera_pos
	local camera_rot

	if RenderWorld.sprite_instance(Thumbnail._rw, self._object._unit_id) then
		camera_pos = Vector3.up()
		camera_rot = Quaternion.look(Vector3.normalize(-camera_pos), Vector3.forward())
	else
		camera_pos = Vector3(1, -1, 1)
		camera_rot = Quaternion.look(Vector3.normalize(-camera_pos))
	end

	local camera_pose = Matrix4x4.from_quaternion_translation(camera_rot, camera_pos)

	camera:set_local_pose(camera_pose)
	camera:frame_obb(obb_tm, obb_he)
end

TexturePreview = class(TexturePreview)

function TexturePreview:init(world, texture_name)
	self._texture_name = texture_name
	self._material_name = "core/editors/thumbnail/gui"
	self._material = Gui.material(Thumbnail._gui, self._material_name)
end

function TexturePreview:destroy()
end

function TexturePreview:render(camera)
	Material.set_texture(self._material, "u_albedo_map", self._texture_name)
	Gui.image(Thumbnail._gui, Vector2(0, 0), Vector2(128, 128), self._material_name, Color4.white())
end

MaterialPreview = class(MaterialPreview)

function MaterialPreview:init(world, material_name)
	self._unit_preview = UnitPreview(world, "core/units/primitives/sphere")
	local unit_box = self._unit_preview._object
	local render_world = unit_box._rw
	local mesh_instance = RenderWorld.mesh_instance(render_world, unit_box:unit_id())
	RenderWorld.mesh_set_material(render_world, mesh_instance, material_name)
end

function MaterialPreview:destroy()
	self._unit_preview:destroy()
end

function MaterialPreview:render(camera)
	local camera_pos = Vector3(1, -1, 0.5)
	local camera_rot = Quaternion.from_axis_angle(Vector3.up(), 45.0 * (math.pi / 180.0))
	local camera_pose = Matrix4x4.from_quaternion_translation(camera_rot, camera_pos)

	camera:set_local_pose(camera_pose)
end

Thumbnail = Thumbnail or {}

function Thumbnail:init()
	self._world = Device.create_world()
	World.disable_unit_callbacks(self._world)
	self._sg = World.scene_graph(self._world)
	self._rw = World.render_world(self._world)
	self._pw = World.physics_world(self._world)
	self._camera = Camera(self._world, World.spawn_unit(self._world, "core/units/camera"))
	self._object = nil
	self._gui = World.create_screen_gui(self._world)
	self._requests = {}
	self._pending = {}

	local light_unit = World.spawn_unit(self._world, "core/units/light"
		, Vector3(1000, 0, 0)
		, Quaternion.from_axis_angle(Vector3.forward(), 45.0 * (math.pi / 180.0))
		)
	local light = RenderWorld.light_instance(self._rw, light_unit)
	if light ~= nil then
		RenderWorld.light_set_type(self._rw, light, "directional")
		RenderWorld.light_set_intensity(self._rw, light, 3)
	end
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

	if self._object ~= nil then
		self._object:destroy()
		self._object = nil
	end

	if req.type == "unit" then
		self._object = UnitPreview(self._world, req.name)
	elseif req.type == "sound" then
		self._object = UnitPreview(self._world, "core/units/sound")
	elseif req.type == "texture" then
		self._object = TexturePreview(self._world, req.name)
	elseif req.type == "material" then
		self._object = MaterialPreview(self._world, req.name)
	else
		return
	end

	if self._object ~= nil then
		self._object:render(self._camera)
	end

	self._camera:update(dt, 0, 0, 0, 0)
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

