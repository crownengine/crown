-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/lua/class"

Character = class(Character)

function Character:init(world, unit, camera)
	self._world = world
	self._unit = unit
	self._camera = camera
	self._physics_world = World.physics_world(world)
	self._scene_graph = World.scene_graph(world)
	self._gravity = 6*9.8
	self._vertical_speed = 0
	self._walk_speed = 3
	self._run_multiplier = 4
	self._crouch_multiplier = 0.45
	self._crouch_camera_drop = 0.6
	self._mover_stand_height = 1.8
	self._mover_crouch_height = 1.0
	self._mover_crouching = false
	self._grounded = false
	self._support_actor = nil
	self._support_prev_pose = Matrix4x4Box()
	self._base_rotation = QuaternionBox()
	self._first_person_camera_local_offset = Vector3Box()
	self._first_person_height = 2.6
	self._third_person_camera = true
	self._third_person_distance = 8
	self._third_person_height = 1.4

	self._base_rotation:store(Quaternion.identity())

	local asm = World.animation_state_machine(self._world)
	if asm then
		local animation = AnimationStateMachine.instance(asm, self._unit)
		if animation then
			AnimationStateMachine.trigger(asm, animation, "idle")
		end
	end

	-- Keep camera unparented so it is unaffected by any character parent transform.
	local character_transform = SceneGraph.instance(self._scene_graph, self._unit)
	local camera_transform = SceneGraph.instance(self._scene_graph, self._camera:unit())
	local camera_parent = SceneGraph.parent(self._scene_graph, camera_transform)
	local fp_local_offset = camera_parent == character_transform
		and SceneGraph.local_position(self._scene_graph, camera_transform)
		or Vector3.zero()

	if camera_parent then
		local camera_world_pose = SceneGraph.world_pose(self._scene_graph, camera_transform)
		SceneGraph.unlink(self._scene_graph, camera_transform)
		SceneGraph.set_local_pose(self._scene_graph, camera_transform, camera_world_pose)
	end

	-- Preserve the authored mesh orientation so yaw can be applied on top of it.
	self._base_rotation:store(SceneGraph.local_rotation(self._scene_graph, character_transform))

	self._support_actor = nil
	self._support_prev_pose:store(Matrix4x4.identity())
	self._first_person_camera_local_offset:store(fp_local_offset)
end

function Character:toggle_camera_mode()
	self._third_person_camera = not self._third_person_camera
end

function Character:is_grounded()
	return self._grounded
end

function Character:update(dt, move_dx, move_dy, jump_pressed, run_pressed, crouch_pressed)
	local mover = PhysicsWorld.mover_instance(self._physics_world, self._unit)
	local character_transform = SceneGraph.instance(self._scene_graph, self._unit)
	assert(character_transform)

	-- Player mover.
	local coll_up   = PhysicsWorld.mover_collides_up(self._physics_world, mover)
	local coll_down = PhysicsWorld.mover_collides_down(self._physics_world, mover)

	-- Moving platforms character interaction.
	local mover_carry_delta = Vector3.zero()
	if coll_down then
		local support_actor = PhysicsWorld.mover_actor_colliding_down(self._physics_world, mover)
		if support_actor ~= nil and PhysicsWorld.actor_is_kinematic(self._physics_world, support_actor) then
			local support_pose = PhysicsWorld.actor_world_pose(self._physics_world, support_actor)
			if self._support_actor ~= nil and self._support_actor == support_actor then
				local support_delta = Matrix4x4.multiply(Matrix4x4.invert(self._support_prev_pose:unbox()), support_pose)
				local mover_pos = PhysicsWorld.mover_position(self._physics_world, mover)
				local mover_center = PhysicsWorld.mover_center(self._physics_world, mover)
				local current_mover_origin = mover_pos + mover_center
				local carried_mover_origin = Matrix4x4.transform(support_delta, current_mover_origin)
				mover_carry_delta = carried_mover_origin - current_mover_origin
			end

			self._support_actor = support_actor
			self._support_prev_pose:store(support_pose)
		else
			self._support_actor = nil
		end
	else
		self._support_actor = nil
	end

	local camera_pose = self._camera:world_pose()
	local camera_forward = Matrix4x4.y(camera_pose)
	local camera_right = Matrix4x4.x(camera_pose)
	local camera_transform = SceneGraph.instance(self._scene_graph, self._camera:unit())
	local delta = Vector3(camera_forward.x, camera_forward.y, 0) * move_dy
		+ Vector3(camera_right.x, camera_right.y, 0) * move_dx

	if Vector3.length(delta) > 0.0001 then
		Vector3.normalize(delta)
	end

	local crouch_active = crouch_pressed

	-- Prevent uncrouching while blocked overhead.
	if not crouch_active and self._mover_crouching and coll_up then
		crouch_active = true
	end

	local asm = World.animation_state_machine(self._world)
	local animation = AnimationStateMachine.instance(asm, self._unit)

	-- Resize capsule only when crouch state changes.
	if crouch_active ~= self._mover_crouching then
		local current_center = PhysicsWorld.mover_center(self._physics_world, mover)
		local center_dz = (self._mover_stand_height - self._mover_crouch_height) * 0.5
		self._mover_crouching = crouch_active
		if self._mover_crouching then
			PhysicsWorld.mover_set_height(self._physics_world, mover, self._mover_crouch_height)
			PhysicsWorld.mover_set_center(self._physics_world, mover, Vector3(current_center.x, current_center.y, current_center.z - center_dz))
			AnimationStateMachine.trigger(asm, animation, "crouch")
		else
			PhysicsWorld.mover_set_height(self._physics_world, mover, self._mover_stand_height)
			PhysicsWorld.mover_set_center(self._physics_world, mover, Vector3(current_center.x, current_center.y, current_center.z + center_dz))
			AnimationStateMachine.trigger(asm, animation, "uncrouch")
		end
	end

	local speed = self._walk_speed
	if run_pressed then
		speed = speed * self._run_multiplier
	end
	if crouch_active then
		speed = speed * self._crouch_multiplier
	end
	delta = delta * speed

	-- Animation.
	local move_speed = Vector3.length(delta)
	local animation_moving = move_speed > 0.0001
	local animation_running = animation_moving and run_pressed and not crouch_active

	if animation_moving then
		local yaw = math.atan2(delta.y, delta.x) + math.pi * 0.5
		local character_rotation = Quaternion.multiply(Quaternion.from_axis_angle(Vector3(0, 0, 1), yaw), self._base_rotation:unbox())
		SceneGraph.set_local_rotation(self._scene_graph, character_transform, character_rotation)
	end

	if asm then
		local speed_id = AnimationStateMachine.variable_id(asm, animation, "speed")

		AnimationStateMachine.set_variable(asm, animation, speed_id, math.min(1.0, move_speed / self._walk_speed))

		if animation_running then
			AnimationStateMachine.trigger(asm, animation, "run")
		elseif animation_moving then
			AnimationStateMachine.trigger(asm, animation, "walk")
		else
			AnimationStateMachine.trigger(asm, animation, "idle")
		end
	end

	if jump_pressed and coll_down then
		self._vertical_speed = self._gravity * 0.25
	else
		if coll_down then
			self._vertical_speed = -0.1
		else
			self._vertical_speed = math.max(-50, self._vertical_speed - self._gravity * dt)
		end
	end

	delta.z = self._vertical_speed
	if Vector3.length(mover_carry_delta) > 0.0001 then
		local mover_pos = PhysicsWorld.mover_position(self._physics_world, mover)
		PhysicsWorld.mover_set_position(self._physics_world, mover, mover_pos + mover_carry_delta)
	end
	PhysicsWorld.mover_move(self._physics_world, mover, delta*dt)
	self._grounded = PhysicsWorld.mover_collides_down(self._physics_world, mover)

	-- Copy mover position to character position.
	local mover_pos = PhysicsWorld.mover_position(self._physics_world, mover)
	SceneGraph.set_local_position(self._scene_graph, character_transform, mover_pos)
	local character_world_pose = SceneGraph.world_pose(self._scene_graph, character_transform)
	local character_world_position = Matrix4x4.translation(character_world_pose)

	if self._third_person_camera then
		local camera_pos = character_world_position
			+ Vector3(0, 0, self._third_person_height)
			- camera_forward * self._third_person_distance
		SceneGraph.set_local_position(self._scene_graph, camera_transform, camera_pos)
	else
		local fp_offset = self._first_person_camera_local_offset:unbox()
		local char_right = Matrix4x4.x(character_world_pose)
		local char_forward = Matrix4x4.y(character_world_pose)
		if Vector3.length(char_right) > 0.0001 then Vector3.normalize(char_right) end
		if Vector3.length(char_forward) > 0.0001 then Vector3.normalize(char_forward) end
		local height = self._first_person_height - (self._mover_crouching and self._crouch_camera_drop or 0)
		local camera_pos = character_world_position
			+ char_right * fp_offset.x
			+ char_forward * fp_offset.y
			+ Vector3(0, 0, height)
		SceneGraph.set_local_position(self._scene_graph, camera_transform, camera_pos)
	end
end

return Character
