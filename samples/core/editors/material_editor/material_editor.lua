-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/editors/level_editor/camera"
require "core/editors/unit"

-- EditorView sends its camera input through the LevelEditor protocol.
LevelEditor = LevelEditor or {}

function LevelEditor:init()
	self._world = Device.create_world()
	World.disable_unit_callbacks(self._world)
	self._rw = World.render_world(self._world)
	self._camera = Camera(self._world, World.spawn_unit(self._world, "core/units/camera"))
	self._mouse = { x = 0, y = 0, x_last = 0, y_last = 0, wheel = { delta = 0 } }
	self._unit = nil
	self._material_name = nil
	self._library_name = nil
	self._unit_name = nil

	local position = Vector3(1, -1, 0.75)
	self._camera:set_local_pose(Matrix4x4.from_quaternion_translation(Quaternion.look(Vector3.normalize(-position)), position))
	local light_unit = World.spawn_unit(self._world, "core/units/light"
		, Vector3(1000, 0, 0)
		, Quaternion.from_axis_angle(Vector3.forward(), math.pi / 4)
		)
	local light = RenderWorld.light_instance(self._rw, light_unit)
	RenderWorld.light_set_type(self._rw, light, "directional")
	RenderWorld.light_set_intensity(self._rw, light, 3)
	RenderWorld.global_lighting_set_ambient_color(self._rw, Color4(26, 26, 26, 255))
end

function LevelEditor:reset()
	if self._unit then
		self._unit:destroy()
		self._unit = nil
		World.update(self._world, 0)
		Device.scoped_autoload_end()
	end
	self._material_name = nil
	self._library_name = nil
	self._unit_name = nil
end

function LevelEditor:shutdown()
	self:reset()
	Device.destroy_world(self._world)
end

function LevelEditor:update(dt)
	World.update(self._world, dt)
	local dx = self._mouse.x - self._mouse.x_last
	local dy = self._mouse.y - self._mouse.y_last
	self._camera:mouse_wheel(self._mouse.wheel.delta)
	self._camera:update(dt, self._mouse.x, self._mouse.y, dx, dy)
	self._mouse.x_last = self._mouse.x
	self._mouse.y_last = self._mouse.y
	self._mouse.wheel.delta = 0
	World.update_render_transforms(self._world)
end

function LevelEditor:render(dt)
	Device.render(self._world, self._camera:unit())
end

function LevelEditor:set_material(name, library, unit_name)
	if self._material_name == name and self._library_name == library and self._unit_name == unit_name then
		return
	end
	self:reset()
	Device.scoped_autoload_begin()
	self._unit = UnitBox(self._world, Device.guid(), World.spawn_unit(self._world, unit_name), unit_name)
	self._material_name = name
	self._library_name = library
	self._unit_name = unit_name
	local mesh = RenderWorld.mesh_instance(self._rw, self._unit:unit_id())
	RenderWorld.mesh_set_material(self._rw, mesh, name)
	self:frame_objects()
end

function LevelEditor:material()
	if not self._unit then return nil end
	local mesh = RenderWorld.mesh_instance(self._rw, self._unit:unit_id())
	return RenderWorld.mesh_material(self._rw, mesh)
end

function LevelEditor:set_uniform(name, x, y, z, w)
	local material = self:material()
	if not material then return end
	Material.set_vector4(material, name, Quaternion.from_elements(x, y, z, w))
end

function LevelEditor:set_texture(name, texture)
	local material = self:material()
	if not material then return end
	Material.set_texture(material, name, texture)
end

function LevelEditor:frame_objects()
	if not self._unit then return end
	local pose, half_extents = self._unit:obb()
	self._camera:frame_obb(pose, half_extents)
end

function LevelEditor:set_mouse_state(x, y)
	local _, height = Device.resolution()
	self._mouse.x = x
	self._mouse.y = height - y
end

function LevelEditor:mouse_wheel(delta)
	self._mouse.wheel.delta = self._mouse.wheel.delta + delta
end

function LevelEditor:mouse_down(x, y)
end

function LevelEditor:mouse_up(x, y)
end

function LevelEditor:key_down(key)
end

function LevelEditor:key_up(key)
end

function LevelEditor:camera_drag_start(mode)
	self._camera:set_mode(mode, self._mouse.x, self._mouse.y)
end

function LevelEditor:camera_drag_start_relative(mode)
	self._camera:set_mode(mode, 0, 0)
end

function LevelEditor:camera_view_perspective()
	self._camera:set_perspective()
end

function LevelEditor:camera_view_front()
	self._camera:set_orthographic(Vector3.forward(), Vector3.up())
end

function LevelEditor:camera_view_back()
	self._camera:set_orthographic(Vector3.backward(), Vector3.up())
end

function LevelEditor:camera_view_right()
	self._camera:set_orthographic(Vector3.left(), Vector3.up())
end

function LevelEditor:camera_view_left()
	self._camera:set_orthographic(Vector3.right(), Vector3.up())
end

function LevelEditor:camera_view_top()
	self._camera:set_orthographic(Vector3.down(), Vector3.forward())
end

function LevelEditor:camera_view_bottom()
	self._camera:set_orthographic(Vector3.up(), Vector3.forward())
end
