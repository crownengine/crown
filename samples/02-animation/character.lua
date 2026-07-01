-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/lua/class"

Character = class(Character)

function Character:init(world, unit)
	self._world = world
	self._unit = unit
	self._move_speed = 4
	self._moving = false
	self._facing = Vector3Box()

	self._facing:store(Vector3(0, -1, 0))

	local physics_world = World.physics_world(self._world)
	local mover = PhysicsWorld.mover_instance(physics_world, self._unit)
	local scene_graph = World.scene_graph(self._world)
	local transform = SceneGraph.instance(scene_graph, self._unit)
	local position = SceneGraph.local_position(scene_graph, transform)
	PhysicsWorld.mover_set_position(physics_world, mover, position)

	local asm = World.animation_state_machine(self._world)
	local animation = AnimationStateMachine.instance(asm, self._unit)
	local speed_x = AnimationStateMachine.variable_id(asm, animation, "speed_x")
	local speed_y = AnimationStateMachine.variable_id(asm, animation, "speed_y")
	local speed = AnimationStateMachine.variable_id(asm, animation, "speed")

	AnimationStateMachine.set_variable(asm, animation, speed_x, 0)
	AnimationStateMachine.set_variable(asm, animation, speed_y, -1)
	AnimationStateMachine.set_variable(asm, animation, speed, 0)
	AnimationStateMachine.trigger(asm, animation, "idle")
end

function Character:is_moving()
	return self._moving
end

function Character:update(dt, move_dx, move_dy)
	local scene_graph = World.scene_graph(self._world)
	local transform = SceneGraph.instance(scene_graph, self._unit)

	local direction = Vector3(move_dx, move_dy, 0)
	if Vector3.length(direction) > 0.0001 then
		Vector3.normalize(direction)
	end

	local speed = self._move_speed
	local delta = direction * speed

	local position = SceneGraph.local_position(scene_graph, transform)
	local physics_world = World.physics_world(self._world)
	local mover = PhysicsWorld.mover_instance(physics_world, self._unit)
	PhysicsWorld.mover_move(physics_world, mover, Vector3(delta.x * dt, delta.y * dt, 0))

	local mover_position = PhysicsWorld.mover_position(physics_world, mover)
	position = Vector3(mover_position.x, mover_position.y, position.z)
	PhysicsWorld.mover_set_position(physics_world, mover, position)

	SceneGraph.set_local_position(scene_graph, transform, position)

	-- Sprite depth is proportional to its Y position.
	local render_world = World.render_world(self._world)
	local depth = math.floor(1000 + (1000 - 32 * position.y))
	local sprite = RenderWorld.sprite_instance(render_world, self._unit)
	RenderWorld.sprite_set_depth(render_world, sprite, depth)

	local move_speed = Vector3.length(delta)
	local animation_moving = move_speed > 0.0001
	local animation_direction = self._facing:unbox()

	if animation_moving then
		self._facing:store(direction)
		animation_direction = direction
	end

	local asm = World.animation_state_machine(self._world)
	local animation = AnimationStateMachine.instance(asm, self._unit)
	local speed_x = AnimationStateMachine.variable_id(asm, animation, "speed_x")
	local speed_y = AnimationStateMachine.variable_id(asm, animation, "speed_y")
	local speed_id = AnimationStateMachine.variable_id(asm, animation, "speed")

	AnimationStateMachine.set_variable(asm, animation, speed_x, animation_direction.x)
	AnimationStateMachine.set_variable(asm, animation, speed_y, animation_direction.y)
	AnimationStateMachine.set_variable(asm, animation, speed_id, math.min(1.0, move_speed / self._move_speed))

	if animation_moving then
		AnimationStateMachine.trigger(asm, animation, "run")
	else
		AnimationStateMachine.trigger(asm, animation, "idle")
	end

	self._moving = animation_moving
end

return Character
