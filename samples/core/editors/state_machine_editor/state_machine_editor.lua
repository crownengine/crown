-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/editors/level_editor/camera"
require "core/editors/unit"
require "core/lua/class"

LevelEditor = LevelEditor or {}

function LevelEditor:init()
	self._world = Device.create_world()
	World.disable_unit_callbacks(self._world)
	self._pw = World.physics_world(self._world)
	self._rw = World.render_world(self._world)
	self._sg = World.scene_graph(self._world)
	self._camera = Camera(self._world, World.spawn_unit(self._world, "core/units/camera"))
	self._mouse = { x = 0, y = 0, x_last = 0, y_last = 0, button = { left = false, middle = false, right = false }, wheel = { delta = 0 }}
	self._keyboard = { ctrl = false, shift = false, alt = false }

	self._unit = nil

	-- Adjust camera position and orientation.
	local pos = Vector3(20, -20, 20)
	local zero_pos = Vector3.zero() - pos
	local len = math.abs(Vector3.length(zero_pos))
	local dir = Vector3.normalize(zero_pos)
	self._camera._target_distance = len
	local tr = SceneGraph.instance(self._sg, self._camera:unit())
	SceneGraph.set_local_rotation(self._sg, tr, Quaternion.look(dir))
	SceneGraph.set_local_position(self._sg, tr, pos)

	World.load_level(self._world, "core/editors/levels/empty")
end

function LevelEditor:shutdown()
	Device.destroy_world(self._world)
end

function LevelEditor:reset()
	LevelEditor:shutdown()
	LevelEditor:init()
end

function LevelEditor:update(dt)
	World.update(self._world, dt)

	self._mouse.dx = self._mouse.x - self._mouse.x_last
	self._mouse.dy = self._mouse.y - self._mouse.y_last
	self._mouse.x_last = self._mouse.x
	self._mouse.y_last = self._mouse.y

	self._camera:mouse_wheel(self._mouse.wheel.delta)
	self._camera:update(dt, self._mouse.x, self._mouse.y, self._mouse.dx, self._mouse.dy)

	self._mouse.dx = 0
	self._mouse.dy = 0
	self._mouse.wheel.delta = 0
end

function LevelEditor:render(dt)
	Device.render(self._world, self._camera:unit())
end

function LevelEditor:set_mouse_state(x, y, left, middle, right)
	local resol_x, resol_y = Device.resolution()
	self._mouse.x = x
	self._mouse.y = resol_y - y
	self._mouse.left = left
	self._mouse.middle = middle
	self._mouse.right = right
end

function LevelEditor:mouse_wheel(delta)
	self._mouse.wheel.delta = self._mouse.wheel.delta + delta;
end

function LevelEditor:mouse_down(x, y)
end

function LevelEditor:mouse_up(x, y)
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

function LevelEditor:enable_debug_render_world(enabled)
	RenderWorld.enable_debug_drawing(self._rw, enabled)
end

function LevelEditor:enable_debug_physics_world(enabled)
	PhysicsWorld.enable_debug_drawing(self._pw, enabled)
end

function LevelEditor:spawn_skydome(name)
	World.spawn_skydome(self._world, name)
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

function LevelEditor:camera_drag_start(mode)
	self._camera:set_mode(mode, self._mouse.x, self._mouse.y)
end

function LevelEditor:camera_drag_start_relative(mode)
	self._camera:set_mode(mode, 0, 0)
end

function LevelEditor:unspawn_unit()
	if not self._unit then return end
	self._unit:destroy()
end

function LevelEditor:set_unit(unit_name)
	self:unspawn_unit()
	local unit_id = World.spawn_unit(self._world, unit_name, Vector3.zero())
	self._unit = UnitBox(self._world, Device.guid(), unit_id, unit_name)
	local obb_tm, obb_he = self._unit:obb()
	self._camera:frame_obb(obb_tm, obb_he)
end

function LevelEditor:trigger_animation_event(event_name)
	local state_machine = World.animation_state_machine(self._world)
	local instance = AnimationStateMachine.instance(state_machine, self._unit:unit_id())

	if not instance then return end
	AnimationStateMachine.trigger(state_machine, instance, event_name)
end

function LevelEditor:set_variable(variable_name, value)
	local state_machine = World.animation_state_machine(self._world)
	local instance = AnimationStateMachine.instance(state_machine, self._unit:unit_id())

	if not instance then return end

	local variable_id = AnimationStateMachine.variable_id(state_machine, instance, variable_name)
	if variable_id then
		AnimationStateMachine.set_variable(state_machine, instance, variable_id, value)
	end
end
